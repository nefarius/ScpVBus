// TestStatic.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "..\XOutput.h"

#pragma comment(lib, "C:\\Users\\Shaul\\Documents\\GitHubVisualStudio\\ScpVBus\\x64\\Debug\\StaticLib.lib")

int main()
{

	DWORDLONG Package = 0;
	DWORD res = XOutputGetDriverPackageVersion(&Package);
	return 0;
}

