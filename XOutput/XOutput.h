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

#define ERROR_VBUS_NOT_CONNECTED			0x90000
#define ERROR_VBUS_INDEX_OUT_OF_RANGE		0x90001
#define ERROR_VBUS_IOCTL_REQUEST_FAILED		0x90002
#define ERROR_VBUS_INVALID_STATE_INFO		0x90003

#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
	// used by C++ source code
#endif

	DLL_EXPORT DWORD XOutputSetState(
		_In_  DWORD dwUserIndex,
		      _Out_ XINPUT_GAMEPAD* pGamepad
	);

	DLL_EXPORT DWORD XOutputGetState(
		_In_    DWORD dwUserIndex,
		        _Out_ BYTE* bLargeMotor,
		        _Out_ BYTE* bSmallMotor
	);

	DLL_EXPORT DWORD XOutputGetRealUserIndex(
		_In_ DWORD dwUserIndex,
		     _Out_ DWORD* dwRealIndex
	);

	DLL_EXPORT DWORD XOutputPlugIn(
		_In_ DWORD dwUserIndex
	);

	DLL_EXPORT DWORD XOutputUnPlug(
		_In_ DWORD dwUserIndex
	);

#ifdef __cplusplus
}
#endif

