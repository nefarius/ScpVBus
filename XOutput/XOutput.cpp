// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "XOutput.h"
#include <stdlib.h>

#define FEEDBACK_BUFFER_LENGTH 9
static BYTE g_Feedback[XUSER_MAX_COUNT][FEEDBACK_BUFFER_LENGTH] = {};

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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (dwUserIndex < 0 || dwUserIndex > 3)
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
	if (!DeviceIoControl(g_hScpVBus, 0x2A400C, buffer, _countof(buffer), output, FEEDBACK_BUFFER_LENGTH, &trasfered, nullptr))
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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (dwUserIndex < 0 || dwUserIndex > 3)
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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (dwUserIndex < 0 || dwUserIndex > 3)
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

	if (!DeviceIoControl(g_hScpVBus, 0x2A4000, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	if (dwUserIndex < 0 || dwUserIndex > 3)
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

	if (!DeviceIoControl(g_hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
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
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_VBUS_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	BYTE buffer[16] = {};

	buffer[0] = 0x10;

	if (!DeviceIoControl(g_hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
	{
		return ERROR_VBUS_IOCTL_REQUEST_FAILED;
	}

	return ERROR_SUCCESS;
}
