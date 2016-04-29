/***************************************************************************
*                                                                          *
*   XInput.h -- This module defines Xbox 360 Virtual Controller APIs       *
*               and constants for the Windows platform.                    *
*                                                                          *
*   Copyright (c) Benjamin Höglinger, Shaul Eizikovich                     *
*                                                                          *
***************************************************************************/

#ifdef _MSC_VER
#pragma once
#endif

#include <Windows.h>
#include <Xinput.h>

#ifndef DLL_EXPORT
#if XOUTPUT_EXPORTS
#define XOUTPUT_API __declspec(dllexport)
#else
#define XOUTPUT_API __declspec(dllimport)
#endif
#endif

/// 
/// XOutput error codes
/// 
#define XOUTPUT_VBUS_NOT_CONNECTED          0x90000
#define XOUTPUT_VBUS_INDEX_OUT_OF_RANGE     0x90001
#define XOUTPUT_VBUS_IOCTL_REQUEST_FAILED   0x90002
#define XOUTPUT_VBUS_INVALID_STATE_INFO     0x90003


/// 
/// XOutput API
/// 
#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
    // used by C++ source code
#endif

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Sends state information to a connected virtual controller. </summary>
    ///
    /// <remarks>
    /// This function fails if the supplied user index represents an unplugged device or a device
    /// owned by another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    /// <param name="pGamepad">     [in,out] Pointer to an XINPUT_GAMEPAD structure containing the
    ///                             state information to send to the virtual controller. </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputSetState(
        _In_  DWORD dwUserIndex,
              _Out_ XINPUT_GAMEPAD* pGamepad
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Retrieves the current vibration state of the specified virtual controller. </summary>
    ///
    /// <remarks>
    /// The values retrieved by this function are only updated after a preceding call of the
    /// XOutputSetState function. This function fails if the supplied user index represents an
    /// unplugged device or a device owned by another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    /// <param name="bVibrate">     This value is set to non-zero if vibration information is available,
    ///                             otherwise it is set to 0x00. </param>
    /// <param name="bLargeMotor">  The intensity of the large motor (0-255). </param>
    /// <param name="bSmallMotor">  The intensity of the small motor (0-255). </param>
    /// <param name="bLed">			The LED that represent this device (0-3). </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputGetState(
        _In_    DWORD dwUserIndex,
                _Out_ PBYTE bVibrate,
                _Out_ PBYTE bLargeMotor,
                _Out_ PBYTE bSmallMotor,
                _Out_ PBYTE bLed
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>
    /// Retrieves the assigned XInput slot index of the specified virtual controller.
    /// </summary>
    ///
    /// <remarks>
    /// The device index used internally on the virtual bus does not reflect the index used by the
    /// XInput API. The "real" index is based on the connection order of any XInput-compatible
    /// controllers plugged into the system and can not be enforced. The values retrieved by this
    /// function are only updated after a preceding call of the XOutputSetState function. This
    /// function fails if the supplied user index represents an unplugged device or a device owned by
    /// another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    /// <param name="dwRealIndex">  [in,out] Pointer to a DWORD value receiving the assigned XInput
    ///                             slot index. </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputGetRealUserIndex(
        _In_ DWORD dwUserIndex,
             _Out_ DWORD* dwRealIndex
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Requests the bus driver to attach a virtual controller. </summary>
    ///
    /// <remarks>
    /// This function fails if the supplied user index represents an already plugged in device or a
    /// device owned by another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputPlugIn(
        _In_ DWORD dwUserIndex
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Requests the bus driver to detach a virtual controller. </summary>
    ///
    /// <remarks>
    /// This function fails if the supplied user index represents an unplugged device or a device
    /// owned by another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputUnPlug(
        _In_ DWORD dwUserIndex
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Requests the bus driver to detach a virtual controller. </summary>
    ///
    /// <remarks>
    /// This function fails if the supplied user index represents an unplugged device.
    /// This function can remove a device owned by another process.
    /// </remarks>
    ///
    /// <param name="dwUserIndex">  Index of the virtual controller. Can be a value from 0 to 3. </param>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputUnPlugForce(
        _In_ DWORD dwUserIndex
    );

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Requests the bus driver to detach all owned virtual controllers. </summary>
    ///
    /// <remarks>   This function can not unplug devices owned by other processes. </remarks>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputUnPlugAll();

    ///-------------------------------------------------------------------------------------------------
    /// <summary>   Requests the bus driver to detach all virtual controllers. </summary>
    ///
    /// <remarks>   This function can unplug devices owned by other processes. </remarks>
    ///
    /// <returns>
    /// If the function succeeds, the return value is ERROR_SUCCESS.
    /// 
    /// If the function fails, the return value is an error code defined in XOutput.h. The function
    /// does not use SetLastError to set the calling thread's last-error code.
    /// </returns>
    ///-------------------------------------------------------------------------------------------------
    XOUTPUT_API DWORD XOutputUnPlugAllForce();

    XOUTPUT_API DWORD XOutputIsPluggedIn(
        _In_    DWORD dwUserIndex,
                _Out_	PBOOL Exist
    );

    XOUTPUT_API DWORD XOutputGetFreeSlots(
        _In_    DWORD dwUserIndex,
                _Out_	PUCHAR nSlots
    );

    XOUTPUT_API DWORD XOutputIsOwned(
        _In_    DWORD dwUserIndex,
                _Out_	PBOOL Owned
    );

#ifdef __cplusplus
}
#endif

