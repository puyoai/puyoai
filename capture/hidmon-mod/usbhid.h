/* usbhid.h */

#define USAGE short
#include <SETUPAPI.H>

//Header file: hidsdi.h

typedef struct _HIDD_ATTRIBUTES {
    ULONG Size;                 // = sizeof (struct _HIDD_ATTRIBUTES)

    //
    // Vendor ids of this hid device
    //
    USHORT VendorID;
    USHORT ProductID;
    USHORT VersionNumber;

    //
    // Additional fields will be added to the end of this structure.
    //
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;


typedef BOOLEAN(__stdcall * _HidD_GetAttributes)
	(IN HANDLE HidDeviceObject, OUT PHIDD_ATTRIBUTES Attributes);

typedef void (__stdcall * _HidD_GetHidGuid) (OUT LPGUID HidGuid);


typedef struct _HIDP_PREPARSED_DATA *PHIDP_PREPARSED_DATA;

typedef BOOLEAN(__stdcall * _HidD_GetPreparsedData)
	(IN HANDLE HidDeviceObject, OUT PHIDP_PREPARSED_DATA * PreparsedData);


typedef BOOLEAN(__stdcall * _HidD_FreePreparsedData) (IN PHIDP_PREPARSED_DATA PreparsedData);


//Header file: hidpi.h

typedef LONG NTSTATUS, *PNTSTATUS;

typedef enum _HIDP_REPORT_TYPE {
    HidP_Input,
    HidP_Output,
    HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _HIDP_CAPS {
    USAGE Usage;
    USAGE UsagePage;
    USHORT InputReportByteLength;
    USHORT OutputReportByteLength;
    USHORT FeatureReportByteLength;
    USHORT Reserved[17];

    USHORT NumberLinkCollectionNodes;

    USHORT NumberInputButtonCaps;
    USHORT NumberInputValueCaps;
    USHORT NumberInputDataIndices;

    USHORT NumberOutputButtonCaps;
    USHORT NumberOutputValueCaps;
    USHORT NumberOutputDataIndices;

    USHORT NumberFeatureButtonCaps;
    USHORT NumberFeatureValueCaps;
    USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;


typedef struct _HIDP_VALUE_CAPS {
    USAGE UsagePage;
    UCHAR ReportID;
    BOOLEAN IsAlias;

    USHORT BitField;
    USHORT LinkCollection;      // A unique internal index pointer

    USAGE LinkUsage;
    USAGE LinkUsagePage;

    BOOLEAN IsRange;
    BOOLEAN IsStringRange;
    BOOLEAN IsDesignatorRange;
    BOOLEAN IsAbsolute;

    BOOLEAN HasNull;            // Does this channel have a null report   union
    UCHAR Reserved;
    USHORT BitSize;             // How many bits are devoted to this value?

    USHORT ReportCount;         // See Note below.  Usually set to 1.
    USHORT Reserved2[5];

    ULONG UnitsExp;
    ULONG Units;

    LONG LogicalMin, LogicalMax;
    LONG PhysicalMin, PhysicalMax;

    union {
        struct {
            USAGE UsageMin, UsageMax;
            USHORT StringMin, StringMax;
            USHORT DesignatorMin, DesignatorMax;
            USHORT DataIndexMin, DataIndexMax;
        } Range;

        struct {
            USAGE Usage, Reserved1;
            USHORT StringIndex, Reserved2;
            USHORT DesignatorIndex, Reserved3;
            USHORT DataIndex, Reserved4;
        } NotRange;
    };
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

typedef NTSTATUS(__stdcall * _HidP_GetCaps) (IN PHIDP_PREPARSED_DATA PreparsedData,
                                   OUT PHIDP_CAPS Capabilities);

typedef NTSTATUS(__stdcall * _HidP_GetValueCaps) (IN HIDP_REPORT_TYPE ReportType,
                                        OUT PHIDP_VALUE_CAPS ValueCaps,
                                        IN OUT PUSHORT ValueCapsLength,
                                        IN PHIDP_PREPARSED_DATA
                                        PreparsedData);


//
typedef NTSTATUS(__stdcall * _HidD_GetFeature) (IN HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
typedef NTSTATUS(__stdcall * _HidD_SetFeature) (IN HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
typedef NTSTATUS(__stdcall * _HidD_GetManufacturerString) (IN HANDLE device, OUT void *buffer, IN ULONG bufferLen);
typedef NTSTATUS(__stdcall * _HidD_GetProductString) (IN HANDLE device, OUT void *buffer, IN ULONG bufferLen);
typedef NTSTATUS(__stdcall * _HidD_GetSerialNumberString) (IN HANDLE device, OUT void *buffer, IN ULONG bufferLen);

/* usbhid.h */
