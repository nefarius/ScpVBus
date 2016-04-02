// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <winioctl.h>
#include "XOutput.h"
#include <stdlib.h>
#include <mutex>
#include <SetupAPI.h>

#define FEEDBACK_BUFFER_LENGTH 9
static BYTE g_Feedback[XUSER_MAX_COUNT][FEEDBACK_BUFFER_LENGTH] = {};
std::once_flag initFlag;
HANDLE g_hScpVBus = INVALID_HANDLE_VALUE;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>	Attempts to find and open the first instance of the virtual bus. </summary>
///
/// <remarks>	Gets only called once. If no virtual bus is present, 
/// 			all XOutput functions report ERROR_VBUS_NOT_CONNECTED. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
void Initialize()
{
	std::call_once(initFlag, []()
	{
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
		deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
		GUID deviceClassGuid = { 0xF679F562, 0x3164, 0x42CE,{ 0xA4, 0xDB, 0xE7 ,0xDD ,0xBE ,0x72 ,0x39 ,0x09 } };
		DWORD memberIndex = 0;
		DWORD requiredSize = 0;

		auto deviceInfoSet = SetupDiGetClassDevs(&deviceClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		while (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &deviceClassGuid, memberIndex, &deviceInterfaceData))
		{
			// get required target buffer size
			SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);

			// allocate target buffer
			auto detailDataBuffer = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(requiredSize));
			detailDataBuffer->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			// get detail buffer
			if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailDataBuffer, requiredSize, &requiredSize, nullptr))
			{
				SetupDiDestroyDeviceInfoList(deviceInfoSet);
				free(detailDataBuffer);
				continue;
			}

			// bus found, open it
			g_hScpVBus = CreateFile(detailDataBuffer->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
				nullptr);

			free(detailDataBuffer);
			break;
		}

		SetupDiDestroyDeviceInfoList(deviceInfoSet);
	});
}


///-------------------------------------------------------------------------------------------------
/// <summary>	Output set state. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
/// <param name="pGamepad">	  	[in,out] If non-null, the state. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputSetState(DWORD dwUserIndex, XINPUT_GAMEPAD* pGamepad)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return ERROR_VBUS_INDEX_OUT_OF_RANGE;
	}

	if (pGamepad == nullptr)
	{
		return ERROR_VBUS_INVALID_STATE_INFO;
	}

	DWORD trasfered = 0;
	BYTE buffer[28] = {};
	auto busIndex = dwUserIndex + 1;

	buffer[0] = 0x1C;

	// encode user index
	buffer[4] = ((busIndex >> 0) & 0xFF);
	buffer[5] = ((busIndex >> 8) & 0xFF);
	buffer[6] = ((busIndex >> 16) & 0xFF);
	buffer[7] = ((busIndex >> 24) & 0xFF);

	buffer[9] = 0x14;

	// concat gamepad info to buffer
	memcpy_s(&buffer[10], _countof(buffer), pGamepad, sizeof(XINPUT_GAMEPAD));

	// vibration and LED info end up here
	BYTE output[FEEDBACK_BUFFER_LENGTH] = {};

	// send report to bus, receive vibration and LED status
	auto retval = DeviceIoControl(g_hScpVBus, 0x2A400C, buffer, _countof(buffer), output, FEEDBACK_BUFFER_LENGTH, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	// cache feedback
	memcpy_s(g_Feedback[dwUserIndex], FEEDBACK_BUFFER_LENGTH, output, FEEDBACK_BUFFER_LENGTH);

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output get state. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
/// <param name="bVibrate">   	Gets set to 0x01 if vibration is requested, 0x00 otherwise. </param>
/// <param name="bLargeMotor">	If non-null, the large motor. </param>
/// <param name="bSmallMotor">	If non-null, the small motor. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputGetState(DWORD dwUserIndex, PBYTE bVibrate, PBYTE bLargeMotor, PBYTE bSmallMotor)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return ERROR_VBUS_INDEX_OUT_OF_RANGE;
	}

	auto pad = g_Feedback[dwUserIndex];

	if (bVibrate != nullptr)
	{
		*bVibrate = (pad[1] == 0x08) ? 0x01 : 0x00;
	}

	if (bLargeMotor != nullptr)
	{
		*bLargeMotor = pad[3];
	}

	if (bSmallMotor != nullptr)
	{
		*bSmallMotor = pad[4];
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output get real user index. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputGetRealUserIndex(DWORD dwUserIndex, DWORD* dwRealIndex)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (dwRealIndex != nullptr)
	{
		*dwRealIndex = g_Feedback[dwUserIndex][8];
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output plug in. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputPlugIn(DWORD dwUserIndex)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return ERROR_VBUS_INDEX_OUT_OF_RANGE;
	}

	DWORD trasfered = 0;
	BYTE buffer[16] = {};
	auto busIndex = dwUserIndex + 1;

	buffer[0] = 0x10;

	buffer[4] = ((busIndex >> 0) & 0xFF);
	buffer[5] = ((busIndex >> 8) & 0xFF);
	buffer[6] = ((busIndex >> 16) & 0xFF);
	buffer[8] = ((busIndex >> 24) & 0xFF);

	auto retval = DeviceIoControl(g_hScpVBus, 0x2A4000, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output un plug. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputUnPlug(DWORD dwUserIndex)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return ERROR_VBUS_INDEX_OUT_OF_RANGE;
	}

	DWORD trasfered = 0;
	BYTE buffer[16] = {};
	auto busIndex = dwUserIndex + 1;

	buffer[0] = 0x10;

	buffer[4] = ((busIndex >> 0) & 0xFF);
	buffer[5] = ((busIndex >> 8) & 0xFF);
	buffer[6] = ((busIndex >> 16) & 0xFF);
	buffer[8] = ((busIndex >> 24) & 0xFF);

	auto retval = DeviceIoControl(g_hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output un plug all. </summary>
///
/// <remarks>	Benjamin, 24.03.2016. </remarks>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputUnPlugAll()
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	BYTE buffer[16] = {};

	buffer[0] = 0x10;

	auto retval = DeviceIoControl(g_hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Checks if a device is plugged in. </summary>
///
/// <param name="dwUserIndex">	One-based index of the device. </param>
///
///  <param name="Exist">	Pointer to BOOL result: True if device exists. </param>
///
/// <remarks>	Shaul, 02.04.2016. </remarks>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------

XOUTPUT_API DWORD XOutputIsCtrlExist(
	_In_    DWORD dwUserIndex,
	_Out_ PBOOL Exist
	)
{
	ULONG buffer[1];
	ULONG output[1];
	DWORD trasfered = 0;

	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	// Prepare the User Index for sending
	buffer[0] = dwUserIndex;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_ISDEVPLUGGED, buffer, _countof(buffer), output, 4, &trasfered, nullptr);
	if (!retval)
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;

	if (*output != 0)
		*Exist = TRUE;
	else
		*Exist = FALSE;

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Get the number of empty Virtual Bus slots. </summary>
///
/// <param name="dwUserIndex">	One-based index of the device. </param>
///
///  <param name="Exist">	Pointer to Number of empty slots. </param>
///
/// <remarks>	Shaul, 02.04.2016. </remarks>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------

XOUTPUT_API DWORD  XOutputNumEmptyBusSlots(
	_In_    DWORD  dwUserIndex,
	_Out_	PUCHAR nSlots
	)

{
	UCHAR output[1];
	DWORD trasfered = 0;

	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_EMPTY_SLOTS, nullptr, 0, output, 1, &trasfered, nullptr);
	if (!retval)
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;

	*nSlots = *output;

	return ERROR_SUCCESS;
}