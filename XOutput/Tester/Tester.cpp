// Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\XOutput.h"


int main()
{
	DWORD result;
	printf("Testing Module XOutput\n\n");

	DWORDLONG Package = 0;
	if (ERROR_SUCCESS == XOutputGetDriverPackageVersion(&Package))
		printf("Driver Package Version is %d.%d.%d.%d\n", \
		(unsigned)(Package >> 48), \
			   (unsigned)((Package >> 32) & 0xffff), \
			   (unsigned)((Package >> 16) & 0xffff), \
			   (unsigned)((Package) & 0xffff) \
		);
	else
		printf("Driver Package Version is Unknown\n");
	DWORD ver;
	result = XOutputGetBusVersion(&ver);
	printf("XOutputGetBusVersion(): Return 0x%x ; Version=0x%x\n", result, ver);


	result = XOutputUnPlugAll();
	printf("XOutputUnPlugAll(): Return 0x%x\n", result);

	printf("Hit any key toXOutputUnPlugAllForce()\n\n");
	getchar();

	result = XOutputUnPlugAllForce();
	printf("XOutputUnPlugAllForce(): Return 0x%x\n", result);

	result = XOutputUnPlug(0);
	printf("XOutputUnPlug(0): Return 0x%x\n", result);

	printf("Hit any key to XOutputUnPlugForce(0)\n\n");
	getchar();

	result = XOutputUnPlugForce(0);
	printf("XOutputUnPlugForce(0): Return 0x%x\n", result);

	result = XOutputUnPlug(1);
	printf("XOutputUnPlug(1): Return 0x%x\n", result);

	result = XOutputUnPlug(2);
	printf("XOutputUnPlug(2): Return 0x%x\n", result);

	result = XOutputUnPlug(3);
	printf("XOutputUnPlug(3): Return 0x%x\n", result);

	result = XOutputUnPlug(4);
	printf("XOutputUnPlug(4): Return 0x%x\n", result);

	printf("Hit any key to plug in devices 0+2\n\n");
	getchar();

	result = XOutputPlugIn(2);
	printf("XOutputPlugIn(2): Return 0x%x\n", result);
	result = XOutputPlugIn(0);
	printf("XOutputPlugIn(0): Return 0x%x\n", result);

	printf("Hit any key to continue\n\n");
	getchar();

	XINPUT_GAMEPAD 	Gamepad = { 0 };
	Gamepad.wButtons = 0x1;
	XOutputSetState(2, &Gamepad);
	XOutputSetState(0, &Gamepad);

	BYTE bVibrate[4], bLargeMotor[4], bSmallMotor[4], bLed[4];
	XOutputGetState(0, &(bVibrate[0]), &(bLargeMotor[0]), &(bSmallMotor[0]), &(bLed[0]));
	XOutputGetState(2, &(bVibrate[2]), &(bLargeMotor[2]), &(bSmallMotor[2]), &(bLed[2]));

    return 0;
}

