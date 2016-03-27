// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <SetupAPI.h>
#include <stdlib.h>


HANDLE g_hScpVBus = INVALID_HANDLE_VALUE;

void Initialize();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	DisableThreadLibraryCalls(hModule);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// loader lock active, initialize in new thread
		return CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Initialize), nullptr, 0, nullptr) > nullptr;
	case DLL_PROCESS_DETACH:
		// close bus handle
		if (g_hScpVBus)
		{
			CloseHandle(g_hScpVBus);
		}
		break;
	}
	return TRUE;
}

void Initialize()
{
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
	deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
	GUID deviceClassGuid = { 0xF679F562, 0x3164, 0x42CE, {0xA4, 0xDB, 0xE7 ,0xDD ,0xBE ,0x72 ,0x39 ,0x09 } };
	DWORD memberIndex = 0;
	DWORD requiredSize = 0;

	auto deviceInfoSet = SetupDiGetClassDevs(&deviceClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	while (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &deviceClassGuid, memberIndex, &deviceInterfaceData))
	{
		// get required target buffer size
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);

		// allocate target buffer
		auto detailDataBuffer = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(requiredSize));
		detailDataBuffer->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// get detail buffer
		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailDataBuffer, requiredSize, &requiredSize, nullptr))
		{
			SetupDiDestroyDeviceInfoList(deviceInfoSet);
			free(detailDataBuffer);
			continue;
		}

		// bus found, open it
		g_hScpVBus = CreateFile(detailDataBuffer->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			nullptr);

		free(detailDataBuffer);
		break;
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
}
