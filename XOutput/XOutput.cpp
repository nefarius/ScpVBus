// XOutput.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "XOutput.h"

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
	return 0;
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
	return 0;
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
	return 0;
}
