// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <winioctl.h>
#include "XOutput.h"
#include "..\ScpVBus\inc\ScpVBus.h"
#include <stdlib.h>
#include <mutex>
#include <SetupAPI.h>
#include <IoCtrl.h>

#define FEEDBACK_BUFFER_LENGTH 9
static BYTE g_Feedback[XUSER_MAX_COUNT][FEEDBACK_BUFFER_LENGTH] = {};
std::once_flag initFlag;
HANDLE g_hScpVBus = INVALID_HANDLE_VALUE;

///-------------------------------------------------------------------------------------------------
/// <summary>	Attempts to find and open the first instance of the virtual bus. </summary>
///
/// <remarks>
/// Gets only called once. If no virtual bus is present, all XOutput functions report
/// ERROR_VBUS_NOT_CONNECTED.
/// </remarks>
///-------------------------------------------------------------------------------------------------
void Initialize()
{
	std::call_once(initFlag, []()
	{
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
		deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
		GUID deviceClassGuid = { 0xF679F562, 0x3164, 0x42CE,{0xA4, 0xDB, 0xE7 ,0xDD ,0xBE ,0x72 ,0x39 ,0x09} };
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

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_PLUGIN_HARDWARE, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

DWORD XOutputUnPlug_opt(_In_ DWORD dwUserIndex, _In_ BOOL  bForce)
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
	BUSENUM_UNPLUG_HARDWARE buffer = {};
	auto busIndex = dwUserIndex + 1;

	buffer.Size = sizeof(BUSENUM_UNPLUG_HARDWARE);
	buffer.SerialNo = busIndex ;

	if (bForce)
		buffer.Flags = 0x0001;
	else
 		buffer.Flags = 0x0000;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_UNPLUG_HARDWARE, (LPVOID)(&buffer), buffer.Size, nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

DWORD XOutputUnPlug(_In_ DWORD dwUserIndex)
{
	return  XOutputUnPlug_opt(dwUserIndex, FALSE);
}

DWORD XOutputUnPlugForce( _In_ DWORD dwUserIndex )
{
	return  XOutputUnPlug_opt(dwUserIndex, TRUE);
}

DWORD XOutputUnPlugAll_opt(BOOL bForce)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	BUSENUM_UNPLUG_HARDWARE buffer = {};
	buffer.Size = sizeof(BUSENUM_UNPLUG_HARDWARE);
	buffer.SerialNo = 0;

	if (bForce)
		buffer.Flags = 0x0001;
	else
		buffer.Flags = 0x0000;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_UNPLUG_HARDWARE, (LPVOID)(&buffer), buffer.Size, nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}

DWORD XOutputUnPlugAll()
{
	return XOutputUnPlugAll_opt(FALSE);
}

DWORD XOutputUnPlugAllForce()
{
	return XOutputUnPlugAll_opt(TRUE);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Checks if a device is plugged in. </summary>
///
/// <remarks>	Shaul, 02.04.2016. </remarks>
///
/// <param name="dwUserIndex">	One-based index of the device. </param>
/// <param name="Exist">	  	Pointer to BOOL result: True if device exists. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputIsPluggedIn(DWORD dwUserIndex, PBOOL Exist)
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

	ULONG buffer[1] = {};
	ULONG output[1] = {};
	DWORD trasfered = 0;

	// Prepare the User Index for sending
	buffer[0] = dwUserIndex;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_ISDEVPLUGGED, buffer, _countof(buffer), output, 4, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	if (Exist != nullptr)
	{
		*Exist = (*output != 0) ? TRUE : FALSE;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Get the number of empty Virtual Bus slots. </summary>
///
/// <remarks>	Shaul, 02.04.2016. </remarks>
///
/// <param name="dwUserIndex">	One-based index of the device. </param>
/// <param name="nSlots">	  	Pointer to Number of empty slots. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputGetFreeSlots(DWORD dwUserIndex, PUCHAR nSlots)
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

	UCHAR output[1] = {};
	DWORD trasfered = 0;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_EMPTY_SLOTS, nullptr, 0, output, 1, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	if (nSlots != nullptr)
	{
		*nSlots = *output;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Test if a device is owned by the calling process. </summary>
///
/// <param name="dwUserIndex">	One-based index of the device. </param>
///
///  <param name="Owned">	Pointer to result. </param>
///
/// <remarks>	Shaul, 02.04.2016. </remarks>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputIsOwned(DWORD dwUserIndex, PBOOL Owned)
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

	ULONG buffer[1] = {};
	ULONG output[1] = {};
	DWORD trasfered = 0;

	// Prepare the User Index for sending
	buffer[0] = dwUserIndex;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_PROC_ID, buffer, _countof(buffer), output, 4, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval) || *output == 0)
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	if (Owned != nullptr)
	{
		*Owned = (GetCurrentProcessId() == *output) ? TRUE : FALSE;
	}

	return ERROR_SUCCESS;
}

