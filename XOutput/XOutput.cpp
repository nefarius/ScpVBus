// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <winioctl.h>
#include <initguid.h>
#include "XOutput.h"
#include <ScpVBus.h>
#include <stdlib.h>
#include <mutex>
#include <SetupAPI.h>
#include <IoCtrl.h>

#define MAX_NUMBER_XBOX_CTRLS 4
#define FEEDBACK_BUFFER_LENGTH 10
static BYTE g_Feedback[XUSER_MAX_COUNT][FEEDBACK_BUFFER_LENGTH] = {};
std::once_flag initFlag;
HANDLE g_hScpVBus = INVALID_HANDLE_VALUE;
XINPUT_GAMEPAD g_Gamepad[MAX_NUMBER_XBOX_CTRLS];

/// Forward declarations of internal functions
void InitAllGamePads(void);
DWORD XOutputSetGetState(DWORD dwUserIndex, XINPUT_GAMEPAD * pGamepad, PBYTE bVibrate, PBYTE bLargeMotor, PBYTE bSmallMotor, PBYTE bLed);
DWORD XOutputUnPlug_Internal(DWORD dwUserIndex, BOOL bForce);


///-------------------------------------------------------------------------------------------------
/// <summary>	Attempts to find and open the first instance of the virtual bus. </summary>
///
/// <remarks>
/// Gets only called once. If no virtual bus is present, all XOutput functions report
/// XOUTPUT_VBUS_NOT_CONNECTED.
/// </remarks>
///-------------------------------------------------------------------------------------------------
void Initialize()
{
    std::call_once(initFlag, []()
    {
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
        deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
        DWORD memberIndex = 0;
        DWORD requiredSize = 0;

		InitAllGamePads();

        auto deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_SCPVBUS, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        while (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &GUID_DEVINTERFACE_SCPVBUS, memberIndex, &deviceInterfaceData))
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
DWORD XOutputGetDriverPackageVersion(PDWORDLONG  Version)
{
	DWORD Status = ERROR_INVALID_FUNCTION;
	// Get the "device info set" for our driver GUID
	HDEVINFO devInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_SCPVBUS, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	// Cycle through all devices currently present (Only One)
	for (int i = 0; ; i++)
	{
		// Get the device info for this device
		SP_DEVINFO_DATA devInfo;
		devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiEnumDeviceInfo(devInfoSet, i, &devInfo))
			break;


		SP_DEVINSTALL_PARAMS InstallParams;
		InstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
		if (!SetupDiGetDeviceInstallParams(devInfoSet, &devInfo, &InstallParams))
		{
			return GetLastError();
		}
		else
		{
			InstallParams.FlagsEx |= DI_FLAGSEX_INSTALLEDDRIVER;
			if (!SetupDiSetDeviceInstallParams(devInfoSet, &devInfo, &InstallParams))
			{
				return GetLastError();
			}
		}


		// Build a list of driver info items that we will retrieve below
		if (!SetupDiBuildDriverInfoList(devInfoSet, &devInfo, SPDIT_COMPATDRIVER))
			return GetLastError(); // Exit on error

								   // Get all the info items for this driver 
		for (int j = 0; ; j++)
		{
			SP_DRVINFO_DATA drvInfo;
			drvInfo.cbSize = sizeof(SP_DRVINFO_DATA);
			if (!SetupDiEnumDriverInfo(devInfoSet, &devInfo, SPDIT_COMPATDRIVER, j, &drvInfo))
				break;
			*Version = drvInfo.DriverVersion;
			Status = ERROR_SUCCESS;
		}

	}

	return Status;

}

DWORD XOutputSetState(DWORD dwUserIndex, XINPUT_GAMEPAD* pGamepad)
{
	// Save last  position data
	memcpy_s(&g_Gamepad[dwUserIndex], sizeof(XINPUT_GAMEPAD), pGamepad, sizeof(XINPUT_GAMEPAD));

	//  Set State
	return XOutputSetGetState(dwUserIndex, pGamepad, NULL, NULL, NULL, NULL);
}

DWORD XoutputGetLedNumber(DWORD dwUserIndex, PBYTE bLed)
{
	DWORD retval = XOutputSetGetState(dwUserIndex, &g_Gamepad[dwUserIndex], nullptr, nullptr, nullptr, bLed);
	if (retval == ERROR_SUCCESS)
		(*bLed)++;
	return retval;
}

DWORD XoutputGetVibration(UINT dwUserIndex, PXINPUT_VIBRATION pVib)
{
	BYTE LargeMotor, SmallMotor, Vibrate;
	DWORD retval = XOutputSetGetState(dwUserIndex, &g_Gamepad[dwUserIndex], &Vibrate, &LargeMotor, &SmallMotor, nullptr);
	if (retval == ERROR_SUCCESS)
	{
		if (Vibrate)
		{
			(*pVib).wLeftMotorSpeed = LargeMotor * 256;
			(*pVib).wRightMotorSpeed = SmallMotor * 256;
		}
		else
			(*pVib).wLeftMotorSpeed = (*pVib).wRightMotorSpeed = 0;
	};
	return retval;
}

DWORD XOutputGetState(DWORD dwUserIndex, PBYTE bVibrate, PBYTE bLargeMotor, PBYTE bSmallMotor, PBYTE bLed)
{
    Initialize();

    if (VBUS_NOT_INITIALIZED())
    {
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
    {
        return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
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

    if (bLed != nullptr)
    {
        *bLed = pad[8];
    }

    return ERROR_SUCCESS;
}

DWORD XOutputGetRealUserIndex(DWORD dwUserIndex, DWORD* dwRealIndex)
{
    Initialize();

    if (VBUS_NOT_INITIALIZED())
    {
        return XOUTPUT_VBUS_NOT_CONNECTED;
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
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
    {
        return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
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
        return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
    }

    return ERROR_SUCCESS;
}

DWORD XOutputUnPlug(DWORD dwUserIndex)
{
    return XOutputUnPlug_Internal(dwUserIndex, FALSE);
}

DWORD XOutputUnPlugForce(DWORD dwUserIndex)
{
    return XOutputUnPlug_Internal(dwUserIndex, TRUE);
}

DWORD XOutputUnPlugAll_Internal(BOOL bForce)
{
    Initialize();

    if (VBUS_NOT_INITIALIZED())
    {
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    DWORD trasfered = 0;
    BUSENUM_UNPLUG_HARDWARE buffer = {};
    buffer.Size = sizeof(BUSENUM_UNPLUG_HARDWARE);
    buffer.SerialNo = 0;

    if (bForce)
        buffer.Flags = 0x0001;
    else
        buffer.Flags = 0x0000;

    auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_UNPLUG_HARDWARE, static_cast<LPVOID>(&buffer), buffer.Size, nullptr, 0, &trasfered, nullptr);

    if (DEVICE_IO_CONTROL_FAILED(retval))
    {
        return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
    }

    return ERROR_SUCCESS;
}

DWORD XOutputUnPlugAll()
{
    return XOutputUnPlugAll_Internal(FALSE);
}

DWORD XOutputUnPlugAllForce()
{
    return XOutputUnPlugAll_Internal(TRUE);
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
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
    {
        return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
    }

    ULONG buffer[1] = {};
    ULONG output[1] = {};
    DWORD trasfered = 0;

    // Prepare the User Index for sending
    buffer[0] = dwUserIndex+1;

    auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_ISDEVPLUGGED, buffer, _countof(buffer), output, 4, &trasfered, nullptr);

    if (DEVICE_IO_CONTROL_FAILED(retval))
    {
        return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
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
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
    {
        return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
    }

    UCHAR output[1] = {};
    DWORD trasfered = 0;

    auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_EMPTY_SLOTS, nullptr, 0, output, 1, &trasfered, nullptr);

    if (DEVICE_IO_CONTROL_FAILED(retval))
    {
        return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
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
        return XOUTPUT_VBUS_NOT_CONNECTED;
    }

    if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
    {
        return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
    }

    ULONG buffer[1] = {};
    ULONG output[1] = {};
    DWORD trasfered = 0;

    // Prepare the User Index for sending
    buffer[0] = dwUserIndex + 1;

    auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_PROC_ID, buffer, _countof(buffer), output, 4, &trasfered, nullptr);

    if (DEVICE_IO_CONTROL_FAILED(retval) || *output == 0)
    {
        return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
    }

    if (Owned != nullptr)
    {
        *Owned = (GetCurrentProcessId() == *output) ? TRUE : FALSE;
    }

    return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Get the Virtual bus' version number. </summary>
///
///
///  <param name="Owned">	Pointer to DWORD holding the Bus Version Number. </param>
///
/// <remarks>	Shaul, 029.04.2016. </remarks>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------

DWORD XOutputGetBusVersion(PDWORD Version)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return XOUTPUT_VBUS_NOT_CONNECTED;
	}

	DWORD output[1] = {0};
	DWORD trasfered = 0;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_VERSION, nullptr, 0, output, 4, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
	}

	if (Version != nullptr)
	{
		*Version = *output;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Initialize all structures holding gamepad status </summary>
///
/// <remarks>
/// Called once from within Initialize()
/// </remarks>
///-------------------------------------------------------------------------------------------------
void InitAllGamePads(void)
{
	for (auto pad : g_Gamepad)
	{
		pad.bLeftTrigger = 0;
		pad.bRightTrigger = 0;
		pad.sThumbLX = 0;
		pad.sThumbLY = 0;
		pad.sThumbRX = 0;
		pad.sThumbRY = 0;
		pad.wButtons = 0;
	}
}

DWORD XOutputSetGetState(DWORD dwUserIndex, XINPUT_GAMEPAD * pGamepad, PBYTE bVibrate, PBYTE bLargeMotor, PBYTE bSmallMotor, PBYTE bLed)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return XOUTPUT_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
	}

	if (pGamepad == nullptr)
	{
		return XOUTPUT_VBUS_INVALID_STATE_INFO;
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
	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_REPORT_HARDWARE, buffer, _countof(buffer), output, FEEDBACK_BUFFER_LENGTH, &trasfered, nullptr);
	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
	}

	// Feedback
	if (bVibrate != nullptr)
	{
		*bVibrate = (output[1] == 0x08) ? 0x01 : 0x00;
	}

	if (bLargeMotor != nullptr)
	{
		*bLargeMotor = output[3];
	}

	if (bSmallMotor != nullptr)
	{
		*bSmallMotor = output[4];
	}

	if (bLed != nullptr)
	{
		*bLed = output[8];
	}

	// Test if device has started
	if (!output[9])
		return XOUTPUT_VBUS_DEVICE_NOT_READY;

	return ERROR_SUCCESS;
}

DWORD XOutputUnPlug_Internal(DWORD dwUserIndex, BOOL bForce)
{
	Initialize();

	if (VBUS_NOT_INITIALIZED())
	{
		return XOUTPUT_VBUS_NOT_CONNECTED;
	}

	if (USER_INDEX_OUT_OF_RANGE(dwUserIndex))
	{
		return XOUTPUT_VBUS_INDEX_OUT_OF_RANGE;
	}

	DWORD trasfered = 0;
	BUSENUM_UNPLUG_HARDWARE buffer = {};
	auto busIndex = dwUserIndex + 1;

	buffer.Size = sizeof(BUSENUM_UNPLUG_HARDWARE);
	buffer.SerialNo = busIndex;

	if (bForce)
		buffer.Flags = 0x0001;
	else
		buffer.Flags = 0x0000;

	auto retval = DeviceIoControl(g_hScpVBus, IOCTL_BUSENUM_UNPLUG_HARDWARE, static_cast<LPVOID>(&buffer), buffer.Size, nullptr, 0, &trasfered, nullptr);

	if (DEVICE_IO_CONTROL_FAILED(retval))
	{
		return XOUTPUT_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}
