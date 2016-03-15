// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "XOutput.h"
#include <stdlib.h>

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
