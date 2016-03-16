#pragma once
#include "stdafx.h"
#include <Xinput.h>

#ifndef DLL_EXPORT
  #if XOUTPUT_EXPORTS
    #define DLL_EXPORT __declspec(dllexport)
  #else
    #define DLL_EXPORT __declspec(dllimport)
  #endif
#endif

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
			  // used by C++ source code
#endif

	DLL_EXPORT DWORD XOutputSetState(
		_In_  DWORD        dwUserIndex,
		_Out_ XINPUT_STATE *pState
		);

	DLL_EXPORT DWORD XOutputGetState(
		_In_    DWORD            dwUserIndex,
		_Inout_ XINPUT_VIBRATION *pVibration
		);

	DLL_EXPORT DWORD XOutputGetRealUserIndex(
		_In_    DWORD            dwUserIndex
		);

	DLL_EXPORT DWORD XOutputPlugIn(
		_In_    DWORD            dwUserIndex
		);

	DLL_EXPORT DWORD XOutputUnPlug(
		_In_    DWORD            dwUserIndex
		);

#ifdef __cplusplus
}
#endif