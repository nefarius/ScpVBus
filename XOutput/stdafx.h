// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
void Initialize();

extern HANDLE g_hScpVBus;

#define DEVICE_IO_CONTROL_FAILED(retval) ((retval == FALSE) && (GetLastError() != ERROR_SUCCESS))
#define USER_INDEX_OUT_OF_RANGE(dwUserIndex) ((dwUserIndex < 0) || (dwUserIndex > 3))
#define VBUS_NOT_INITIALIZED() ((g_hScpVBus == INVALID_HANDLE_VALUE))