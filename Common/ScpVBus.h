//{F679F562-3164-42CE-A4DB-E7DDBE723909}  
DEFINE_GUID (GUID_DEVINTERFACE_SCPVBUS, 0xf679f562, 0x3164, 0x42ce, 0xa4, 0xdb, 0xe7, 0xdd, 0xbe, 0x72, 0x39, 0x9);

#ifndef __SCPVBUS_H
#define __SCPVBUS_H

// Bus Version
#define VER_L_ 0x02
#define VER_M_ 0x01
#define VER_H_ 0x07
#define VER_X_ 0x01

// Bus Version Number
#define	BUS_VERSION	(VER_L_ + 0x10*VER_M_ + 0x100*VER_H_ + 0x1000*VER_X_)

// Bus Version String
#define STRINGIFY_1(x)   #x
#define STRINGIFY(x)     STRINGIFY_1(x)
#define PASTE(x, y) x##y
#define MAKEWIDE(x) PASTE(L,x)
#define	BUS_VERSION_STR	MAKEWIDE(STRINGIFY(VER_H_)) L"." MAKEWIDE(STRINGIFY(VER_M_)) L"."  MAKEWIDE(STRINGIFY(VER_L_))


// HID Report size
#define REPORT_SIZE	20

//
//  Data structures used in User IoCtls
//

typedef struct _BUSENUM_PLUGIN_HARDWARE {

    __in ULONG Size;                          
  
    __in ULONG SerialNo;

    ULONG Reserved[2];    
                                                                        
} BUSENUM_PLUGIN_HARDWARE, *PBUSENUM_PLUGIN_HARDWARE;

typedef struct _BUSENUM_UNPLUG_HARDWARE {

    __in ULONG Size;                                    

	__in ULONG SerialNo;

	__in ULONG Flags;

    ULONG Reserved[1];    

} BUSENUM_UNPLUG_HARDWARE, *PBUSENUM_UNPLUG_HARDWARE;

typedef struct _BUSENUM_EJECT_HARDWARE {

    __in ULONG Size;                                    

    __in ULONG SerialNo;
    
    ULONG Reserved[2];    

} BUSENUM_EJECT_HARDWARE, *PBUSENUM_EJECT_HARDWARE;

typedef struct _BUSENUM_REPORT_HARDWARE {

    __in ULONG Size;                                    

    __in ULONG SerialNo;
    
	// HID Report buffer
    UCHAR Data[REPORT_SIZE]; 

} BUSENUM_REPORT_HARDWARE, *PBUSENUM_REPORT_HARDWARE;

#endif
