// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "XOutput.h"
#include <stdlib.h>

#define FEEDBACK_LENGTH 9
static UCHAR g_Feedback[4][FEEDBACK_LENGTH] = {};

///-------------------------------------------------------------------------------------------------
/// <summary>	Output set state. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
/// <param name="pState">	  	[in,out] If non-null, the state. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputSetState(DWORD dwUserIndex, XINPUT_STATE * pState)
{
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_NOT_CONNECTED;
	}

	if(dwUserIndex < 1 || dwUserIndex > 3)
	{
		return ERROR_RANGE_NOT_FOUND;
	}

	DWORD trasfered = 0;
	UCHAR buffer[28] = {};

	buffer[0] = 0x1C;

	buffer[4] = ((dwUserIndex >> 0) & 0xFF);
	buffer[5] = ((dwUserIndex >> 8) & 0xFF);
	buffer[6] = ((dwUserIndex >> 16) & 0xFF);
	buffer[7] = ((dwUserIndex >> 24) & 0xFF);

	buffer[9] = 0x14;

	// buttons
	buffer[10] = ((pState->Gamepad.wButtons >> 0) & 0xFF);
	buffer[11] = ((pState->Gamepad.wButtons >> 8) & 0xFF);

	// left trigger
	buffer[12] = pState->Gamepad.bLeftTrigger;
	// right trigger
	buffer[13] = pState->Gamepad.bRightTrigger;

	// left thumb X
	buffer[14] = ((pState->Gamepad.sThumbLX >> 0) & 0xFF);
	buffer[15] = ((pState->Gamepad.sThumbLX >> 8) & 0xFF);

	// left thumb Y
	buffer[16] = ((pState->Gamepad.sThumbLY >> 0) & 0xFF);
	buffer[17] = ((pState->Gamepad.sThumbLY >> 8) & 0xFF);

	// right thumb X
	buffer[18] = ((pState->Gamepad.sThumbRX >> 0) & 0xFF);
	buffer[19] = ((pState->Gamepad.sThumbRX >> 8) & 0xFF);

	// right thumb Y
	buffer[20] = ((pState->Gamepad.sThumbRY >> 0) & 0xFF);
	buffer[21] = ((pState->Gamepad.sThumbRY >> 8) & 0xFF);

	UCHAR output[FEEDBACK_LENGTH] = {};
	
	if (!DeviceIoControl(g_hScpVBus, 0x2A400C, buffer, _countof(buffer), output, FEEDBACK_LENGTH, &trasfered, nullptr))
	{
		return ERROR_NOT_CONNECTED;
	}

	memcpy(g_Feedback[(dwUserIndex - 1)], output, FEEDBACK_LENGTH);

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output get state. </summary>
///
/// <remarks>	Benjamin, 15.03.2016. </remarks>
///
/// <param name="dwUserIndex">	Zero-based index of the user. </param>
/// <param name="pVibration"> 	[in,out] If non-null, the vibration. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputGetState(DWORD dwUserIndex, XINPUT_VIBRATION * pVibration)
{
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_NOT_CONNECTED;
	}

	if (dwUserIndex < 1 || dwUserIndex > 3)
	{
		return ERROR_RANGE_NOT_FOUND;
	}

	auto pad = g_Feedback[(dwUserIndex - 1)];

	

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
DWORD XOutputGetRealUserIndex(DWORD dwUserIndex)
{
	if (g_hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_NOT_CONNECTED;
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
		return ERROR_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	UCHAR buffer[16] = {};

	buffer[0] = 0x10;

	buffer[4] = ((dwUserIndex >> 0) & 0xFF);
	buffer[5] = ((dwUserIndex >> 8) & 0xFF);
	buffer[6] = ((dwUserIndex >> 16) & 0xFF);
	buffer[8] = ((dwUserIndex >> 24) & 0xFF);

	if (!DeviceIoControl(g_hScpVBus, 0x2A4000, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
	{
		return ERROR_NOT_CONNECTED;
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
		return ERROR_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	UCHAR buffer[16] = {};

	buffer[0] = 0x10;

	buffer[4] = ((dwUserIndex >> 0) & 0xFF);
	buffer[5] = ((dwUserIndex >> 8) & 0xFF);
	buffer[6] = ((dwUserIndex >> 16) & 0xFF);
	buffer[8] = ((dwUserIndex >> 24) & 0xFF);

	if (!DeviceIoControl(g_hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
	{
		return ERROR_NOT_CONNECTED;
	}

	return ERROR_SUCCESS;
}
