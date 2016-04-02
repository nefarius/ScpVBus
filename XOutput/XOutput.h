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

#define IOCTL_BUSENUM_BASE 0x801
#define FILE_DEVICE_BUSENUM		FILE_DEVICE_BUS_EXTENDER
#define BUSENUM_IOCTL(_index_)	CTL_CODE(FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_READ_DATA)
#define BUSENUM_W_IOCTL(_index_)	CTL_CODE(FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_WRITE_DATA)
#define BUSENUM_R_IOCTL(_index_)	CTL_CODE(FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_READ_DATA)
#define BUSENUM_RW_IOCTL(_index_)	CTL_CODE(FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_WRITE_DATA | FILE_READ_DATA)

#define IOCTL_BUSENUM_PLUGIN_HARDWARE	BUSENUM_IOCTL(0x0)
#define IOCTL_BUSENUM_UNPLUG_HARDWARE	BUSENUM_IOCTL(0x1)
#define IOCTL_BUSENUM_EJECT_HARDWARE	BUSENUM_IOCTL(0x2)
#define IOCTL_BUSENUM_REPORT_HARDWARE	BUSENUM_IOCTL(0x3)


#define IOCTL_BUSENUM_ISDEVPLUGGED	BUSENUM_RW_IOCTL(IOCTL_BUSENUM_BASE+0x100)
#define IOCTL_BUSENUM_EMPTY_SLOTS	BUSENUM_RW_IOCTL(IOCTL_BUSENUM_BASE+0x101)


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

	XOUTPUT_API DWORD XOutputUnPlugAll();

	XOUTPUT_API DWORD XOutputIsCtrlExist(
		_In_    DWORD dwUserIndex,
		_Out_	PBOOL Exist
		);

	XOUTPUT_API DWORD  XOutputNumEmptyBusSlots(
		_In_    DWORD  dwUserIndex,
		_Out_	PUCHAR nSlots
		);


#ifdef __cplusplus
}
#endif

