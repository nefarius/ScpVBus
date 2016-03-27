#pragma once
#include "stdafx.h"
#include <Xinput.h>

#ifndef DLL_EXPORT
#if XOUTPUT_EXPORTS
#define XOUTPUT_API __declspec(dllexport)
#else
#define XOUTPUT_API __declspec(dllimport)
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

	XOUTPUT_API DWORD XOutputSetState(
		_In_  DWORD dwUserIndex,
		      _Out_ XINPUT_GAMEPAD* pGamepad
	);

	XOUTPUT_API DWORD XOutputGetState(
		_In_    DWORD dwUserIndex,
		        _Out_ PBYTE bVibrate,
		        _Out_ PBYTE bLargeMotor,
		        _Out_ PBYTE bSmallMotor
	);

	XOUTPUT_API DWORD XOutputGetRealUserIndex(
		_In_ DWORD dwUserIndex,
		     _Out_ DWORD* dwRealIndex
	);

	XOUTPUT_API DWORD XOutputPlugIn(
		_In_ DWORD dwUserIndex
	);

	XOUTPUT_API DWORD XOutputUnPlug(
		_In_ DWORD dwUserIndex
	);

<<<<<<< HEAD
	DLL_EXPORT void Initialize(
		);
=======
	XOUTPUT_API DWORD XOutputUnPlugAll();
>>>>>>> 59ff1f7965c08ec22b31f8f8cd402d59cb5605d6

#ifdef __cplusplus
}
#endif

