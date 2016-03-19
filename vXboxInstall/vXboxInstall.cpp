// vXboxInstall.cpp : Defines the entry point for the console application.
//
// Plug-in/Unplug a vXbox device

// Sintax:
// xBoxInstall [P[lug]{1..4}][U[nplug]{1..4}]

#include "stdafx.h"
#include "vXboxInstall.h"

#pragma comment( lib, "Setupapi.lib" )

int main(int argc, char *argv[])
{


	int n, res;
	int Devices[4] = { 0 };

	n = DeviceToPlug(argc, argv, Devices);
	for (int i = 0; i < n;i++)
	{
		res = PlugIn_vBox(Devices[i]);
		if (res)
			printf("Plug-in device %d - Failed\n", Devices[i]);
		else
			printf("Plug-in device %d - OK\n", Devices[i]);
	}
		

	n = DeviceToUnPlug(argc, argv, Devices);
	for (int i = 0; i < n;i++)
	{
		res = UnPlug_vBox(Devices[i]);
		if (res)
			printf("UnPlug device %d - Failed\n", Devices[i]);
		else
			printf("UnPlug device %d - OK\n", Devices[i]);
	}
		

	return 0;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Parse Command Line: Get the devices to Plug-in. </summary>
///
/// <remarks>	Shaul, 19.03.2016. </remarks>
///
/// <param name="argc">	Number of Command line parameters. </param>
/// <param name="argv">	Array of Command line parameters. </param>
/// <param name="Devices">	Array of numbers of devices to Plugin (Out). </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
int DeviceToPlug(int argc, char *argv[], int *Devices)
{

	std::string param;
	bool Found = false;
	int Count = 0;
	int Devs[4] = { 0 };


	// Loop on all parameters
	for (int i = 1; i < argc; i++)
	{
		param = argv[i];

		// Search for parameter that starts with 'P'/'p'
		// The following numbers refer to devices to plug in
		if ((param[0] == 'p') || (param[0] == 'P'))
		{
			Found = true;
			continue;
		};

		// If 'p' found and the parameter is NOT a digit then this and the following
		// are NOT numbers that refer to devices to plug in
		if (Found && !isdigit(param[0]))
		{
			Found = false;
			continue;
		}

		// If 'p' found and the parameter is IS a digit then this number refer to devices to plug in
		if (Found && isdigit(param[0]))
		{
			Devices[Count] = stoi(param);
			Count++;
			continue;
		}
	}

	return Count;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Parse Command Line: Get the devices to UnPlug. </summary>
///
/// <remarks>	Shaul, 19.03.2016. </remarks>
///
/// <param name="argc">	Number of Command line parameters. </param>
/// <param name="argv">	Array of Command line parameters. </param>
/// <param name="Devices">	Array of numbers of devices to Unplug (Out). </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
int DeviceToUnPlug(int argc, char *argv[], int *Devices)
{

	std::string param;
	bool Found = false;
	int Count = 0;
	int Devs[4] = { 0 };


	// Loop on all parameters
	for (int i = 1; i < argc; i++)
	{
		param = argv[i];

		// Search for parameter that starts with 'P'/'p'
		// The following numbers refer to devices to plug in
		if ((param[0] == 'u') || (param[0] == 'U'))
		{
			Found = true;
			continue;
		};

		// If 'p' found and the parameter is NOT a digit then this and the following
		// are NOT numbers that refer to devices to plug in
		if (Found && !isdigit(param[0]))
		{
			Found = false;
			continue;
		}

		// If 'p' found and the parameter is IS a digit then this number refer to devices to plug in
		if (Found && isdigit(param[0]))
		{
			Devices[Count] = stoi(param);
			Count++;
			continue;
		}
	}

	return Count;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output un plug. </summary>
///
/// <remarks>	Shaul, 18.03.2016. </remarks>
///
/// <param name="iDev">	One-based index of the user. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputUnPlug(HANDLE hScpVBus, int iDev)
{
	if (hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	UCHAR buffer[16] = {};

	buffer[0] = 0x10;

	buffer[4] = ((iDev >> 0) & 0xFF);
	buffer[5] = ((iDev >> 8) & 0xFF);
	buffer[6] = ((iDev >> 16) & 0xFF);
	buffer[8] = ((iDev >> 24) & 0xFF);

	if (!DeviceIoControl(hScpVBus, 0x2A4004, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
	{
		return ERROR_NOT_CONNECTED;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output plug in. </summary>
///
/// <remarks>	Shaul, 17.03.2016. </remarks>
///
/// <param name="iDev">	One-based index of the user. </param>
/// <param name="hScpVBus">	Handle to the virtual bus. </param>
///
/// <returns>	A DWORD. </returns>
///-------------------------------------------------------------------------------------------------
DWORD XOutputPlugIn(HANDLE hScpVBus, int iDev)
{
	if (hScpVBus == INVALID_HANDLE_VALUE)
	{
		return ERROR_NOT_CONNECTED;
	}

	DWORD trasfered = 0;
	UCHAR buffer[16] = {};

	buffer[0] = 0x10;

	buffer[4] = ((iDev >> 0) & 0xFF);
	buffer[5] = ((iDev >> 8) & 0xFF);
	buffer[6] = ((iDev >> 16) & 0xFF);
	buffer[8] = ((iDev >> 24) & 0xFF);

	if (!DeviceIoControl(hScpVBus, 0x2A4000, buffer, _countof(buffer), nullptr, 0, &trasfered, nullptr))
	{
		return ERROR_NOT_CONNECTED;
	}

	return ERROR_SUCCESS;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Output plug in. </summary>
///
/// <remarks>	Shaul, 17.03.2016. </remarks>
///
/// <param name="iDev">	1-based index of the user. </param>
///
/// <returns>	An int. </returns>
///-------------------------------------------------------------------------------------------------
int PlugIn_vBox(int iDev)
{
	WCHAR path[MAX_PATH];
	// Test validity of input
	if (iDev < 1 || iDev > 4)
		return -2;

	// Get Path to the Bus - check output
	int n = GetBusPath(path, MAX_PATH);
	if (n <= 0)
		return -1;


	// bus found, open it and obtain handle
	HANDLE hScpVBus = CreateFile(path,
								 GENERIC_READ | GENERIC_WRITE,
								 FILE_SHARE_READ | FILE_SHARE_WRITE,
								 nullptr,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 nullptr);

	// Plug-in device to the virtualbus
	DWORD res = XOutputPlugIn(hScpVBus, iDev);

	// Clean-up
	CloseHandle(hScpVBus);

	if (ERROR_SUCCESS == res)
		return 0;
	else
		return -3;

}
///-------------------------------------------------------------------------------------------------
/// <summary>	Output UnPlug. </summary>
///
/// <remarks>	Shaul, 18.03.2016. </remarks>
///
/// <param name="iDev">	1-based index of the user. </param>
///
/// <returns>	An int. </returns>
///-------------------------------------------------------------------------------------------------
int UnPlug_vBox(int iDev)
{
	WCHAR path[MAX_PATH];
	// Test validity of input
	if (iDev < 1 || iDev > 4)
		return -2;

	// Get Path to the Bus - check output
	int n = GetBusPath(path, MAX_PATH);
	if (n <= 0)
		return -1;


	// bus found, open it and obtain handle
	HANDLE hScpVBus = CreateFile(path,
								 GENERIC_READ | GENERIC_WRITE,
								 FILE_SHARE_READ | FILE_SHARE_WRITE,
								 nullptr,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 nullptr);

	// Plug-in device to the virtualbus
	DWORD res = XOutputUnPlug(hScpVBus, iDev);

	// Clean-up
	CloseHandle(hScpVBus);

	if (ERROR_SUCCESS == res)
		return 0;
	else
		return -3;

}

///-------------------------------------------------------------------------------------------------
/// <summary>	Get Device Interface path. </summary>
///
/// <remarks>	Shaul, 17.03.2016. </remarks>
///
/// <param name="path">	Pointer to the output buffer holding the Device Interface path string. </param>
/// <param name="size">	Size of the output buffer in WCHARs.</param>
///
/// <returns>	If sucessful: Size of the the path in WCHARs
///             Negative number indicates failure.</returns>
/// 
///-------------------------------------------------------------------------------------------------
int GetBusPath(LPCTSTR path, UINT size)
{
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
	deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
	GUID deviceClassGuid = { 0xF679F562, 0x3164, 0x42CE, { 0xA4, 0xDB, 0xE7 ,0xDD ,0xBE ,0x72 ,0x39 ,0x09 } };
	DWORD memberIndex = 0;
	DWORD requiredSize = 0;

	auto deviceInfoSet = SetupDiGetClassDevs(&deviceClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &deviceClassGuid, memberIndex, &deviceInterfaceData))
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
			return -1;
		}

		// Copy	the path to output buffer
		memcpy((void *)path, detailDataBuffer->DevicePath, requiredSize*sizeof(WCHAR));

		// Cleanup
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
		free(detailDataBuffer);
	}
	else
		return -1;

	return requiredSize;
}


