﻿/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.
 
 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        https://github.com/libusb/hidapi .
********************************************************/

#include <windows.h>

#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif

#ifdef __MINGW32__
#include <ntdef.h>
#include <winbase.h>
#endif

#ifdef __CYGWIN__
#include <ntdef.h>
#define _wcsdup wcsdup
#endif

/* The maximum number of characters that can be passed into the
   HidD_Get*String() functions without it failing.*/
#define MAX_STRING_WCHARS 0xFFF

/*#define HIDAPI_USE_DDK*/

#ifdef __cplusplus
extern "C" {
#endif
	#include <setupapi.h>
	#include <winioctl.h>
	#ifdef HIDAPI_USE_DDK
		#include <hidsdi.h>
	#endif

	/* Copied from inc/ddk/hidclass.h, part of the Windows DDK. */
	#define HID_OUT_CTL_CODE(id)  \
		CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
	#define IOCTL_HID_GET_FEATURE                   HID_OUT_CTL_CODE(100)
	#define IOCTL_HID_GET_INPUT_REPORT              HID_OUT_CTL_CODE(104)

#ifdef __cplusplus
} /* extern "C" */
#endif

#include <stdio.h>
#include <stdlib.h>


#include "hidapi.h"

#undef MIN
#define MIN(x,y) ((x) < (y)? (x): (y))

#ifdef _MSC_VER
	/* Thanks Microsoft, but I know how to use strncpy(). */
	#pragma warning(disable:4996)
#endif

#ifdef __cplusplus
extern "C" {
#endif

static struct hid_api_version api_version = {
	.major = HID_API_VERSION_MAJOR,
	.minor = HID_API_VERSION_MINOR,
	.patch = HID_API_VERSION_PATCH
};

#ifndef HIDAPI_USE_DDK
	/* Since we're not building with the DDK, and the HID header
	   files aren't part of the SDK, we have to define all this
	   stuff here. In lookup_functions(), the function pointers
	   defined below are set. */
	typedef struct _HIDD_ATTRIBUTES{
		ULONG Size;
		USHORT VendorID;
		USHORT ProductID;
		USHORT VersionNumber;
	} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

	typedef USHORT USAGE;
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
	typedef void* PHIDP_PREPARSED_DATA;
	typedef struct _HIDP_LINK_COLLECTION_NODE {
		USAGE  LinkUsage;
		USAGE  LinkUsagePage;
		USHORT Parent;
		USHORT NumberOfChildren;
		USHORT NextSibling;
		USHORT FirstChild;
		ULONG  CollectionType : 8;
		ULONG  IsAlias : 1;
		ULONG  Reserved : 23;
		PVOID  UserContext;
	} HIDP_LINK_COLLECTION_NODE, * PHIDP_LINK_COLLECTION_NODE;
	typedef struct _HIDP_BUTTON_CAPS {
		USAGE   UsagePage;
		UCHAR   ReportID;
		BOOLEAN IsAlias;
		USHORT  BitField;
		USHORT  LinkCollection;
		USAGE   LinkUsage;
		USAGE   LinkUsagePage;
		BOOLEAN IsRange;
		BOOLEAN IsStringRange;
		BOOLEAN IsDesignatorRange;
		BOOLEAN IsAbsolute;
		ULONG   Reserved[10];
		union {
			struct {
				USAGE  UsageMin;
				USAGE  UsageMax;
				USHORT StringMin;
				USHORT StringMax;
				USHORT DesignatorMin;
				USHORT DesignatorMax;
				USHORT DataIndexMin;
				USHORT DataIndexMax;
			} Range;
			struct {
				USAGE  Usage;
				USAGE  Reserved1;
				USHORT StringIndex;
				USHORT Reserved2;
				USHORT DesignatorIndex;
				USHORT Reserved3;
				USHORT DataIndex;
				USHORT Reserved4;
			} NotRange;
		};
	} HIDP_BUTTON_CAPS, * PHIDP_BUTTON_CAPS;
	typedef struct _HIDP_VALUE_CAPS {
		USAGE   UsagePage;
		UCHAR   ReportID;
		BOOLEAN IsAlias;
		USHORT  BitField;
		USHORT  LinkCollection;
		USAGE   LinkUsage;
		USAGE   LinkUsagePage;
		BOOLEAN IsRange;
		BOOLEAN IsStringRange;
		BOOLEAN IsDesignatorRange;
		BOOLEAN IsAbsolute;
		BOOLEAN HasNull;
		UCHAR   Reserved;
		USHORT  BitSize;
		USHORT  ReportCount;
		USHORT  Reserved2[5];
		ULONG   UnitsExp;
		ULONG   Units;
		LONG    LogicalMin;
		LONG    LogicalMax;
		LONG    PhysicalMin;
		LONG    PhysicalMax;
		union {
			struct {
				USAGE  UsageMin;
				USAGE  UsageMax;
				USHORT StringMin;
				USHORT StringMax;
				USHORT DesignatorMin;
				USHORT DesignatorMax;
				USHORT DataIndexMin;
				USHORT DataIndexMax;
			} Range;
			struct {
				USAGE  Usage;
				USAGE  Reserved1;
				USHORT StringIndex;
				USHORT Reserved2;
				USHORT DesignatorIndex;
				USHORT Reserved3;
				USHORT DataIndex;
				USHORT Reserved4;
			} NotRange;
		};
	} HIDP_VALUE_CAPS, * PHIDP_VALUE_CAPS;
	typedef enum _HIDP_REPORT_TYPE {
		HidP_Input,
		HidP_Output,
		HidP_Feature,
		NUM_OF_HIDP_REPORT_TYPES
	} HIDP_REPORT_TYPE;

	#define HIDP_STATUS_SUCCESS 0x110000

	typedef BOOLEAN (__stdcall *HidD_GetAttributes_)(HANDLE device, PHIDD_ATTRIBUTES attrib);
	typedef BOOLEAN (__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
	typedef BOOLEAN (__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
	typedef BOOLEAN (__stdcall *HidD_GetInputReport_)(HANDLE handle, PVOID data, ULONG length);
	typedef BOOLEAN (__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
	typedef BOOLEAN (__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
	typedef BOOLEAN (__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
	typedef NTSTATUS (__stdcall* HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS *caps);
	typedef BOOLEAN (__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);
	typedef NTSTATUS (__stdcall* HidP_GetLinkCollectionNodes_)(PHIDP_LINK_COLLECTION_NODE link_collection_nodes, PULONG link_collection_nodes_length, PHIDP_PREPARSED_DATA preparsed_data);
	typedef NTSTATUS (__stdcall* HidP_GetButtonCaps_)(HIDP_REPORT_TYPE report_type, PHIDP_BUTTON_CAPS button_caps, PUSHORT button_caps_length, PHIDP_PREPARSED_DATA preparsed_data);
	typedef NTSTATUS(__stdcall* HidP_GetValueCaps_)(HIDP_REPORT_TYPE report_type, PHIDP_VALUE_CAPS value_caps,	PUSHORT value_caps_length, PHIDP_PREPARSED_DATA preparsed_data);
	typedef ULONG(__stdcall* HidP_MaxDataListLength_)(HIDP_REPORT_TYPE report_type, PHIDP_PREPARSED_DATA preparsed_data);

	static HidD_GetAttributes_ HidD_GetAttributes;
	static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
	static HidD_GetManufacturerString_ HidD_GetManufacturerString;
	static HidD_GetProductString_ HidD_GetProductString;
	static HidD_SetFeature_ HidD_SetFeature;
	static HidD_GetFeature_ HidD_GetFeature;
	static HidD_GetInputReport_ HidD_GetInputReport;
	static HidD_GetIndexedString_ HidD_GetIndexedString;
	static HidD_GetPreparsedData_ HidD_GetPreparsedData;
	static HidD_FreePreparsedData_ HidD_FreePreparsedData;
	static HidP_GetCaps_ HidP_GetCaps;
	static HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;
	static HidP_GetLinkCollectionNodes_ HidP_GetLinkCollectionNodes;
	static HidP_GetButtonCaps_ HidP_GetButtonCaps;
	static HidP_GetValueCaps_ HidP_GetValueCaps;
	static HidP_MaxDataListLength_ HidP_MaxDataListLength;

	static HMODULE lib_handle = NULL;
	static BOOLEAN initialized = FALSE;
#endif /* HIDAPI_USE_DDK */


struct hid_device_ {
		HANDLE device_handle;
		BOOL blocking;
		USHORT output_report_length;
		unsigned char *write_buf;
		size_t input_report_length;
		USHORT feature_report_length;
		unsigned char *feature_buf;
		void *last_error_str;
		DWORD last_error_num;
		BOOL read_pending;
		char *read_buf;
		OVERLAPPED ol;
		OVERLAPPED write_ol;			  
};

static hid_device *new_hid_device()
{
	hid_device *dev = (hid_device*) calloc(1, sizeof(hid_device));
	dev->device_handle = INVALID_HANDLE_VALUE;
	dev->blocking = TRUE;
	dev->output_report_length = 0;
	dev->write_buf = NULL;
	dev->input_report_length = 0;
	dev->feature_report_length = 0;
	dev->feature_buf = NULL;
	dev->last_error_str = NULL;
	dev->last_error_num = 0;
	dev->read_pending = FALSE;
	dev->read_buf = NULL;
	memset(&dev->ol, 0, sizeof(dev->ol));
	dev->ol.hEvent = CreateEvent(NULL, FALSE, FALSE /*initial state f=nonsignaled*/, NULL);
	memset(&dev->write_ol, 0, sizeof(dev->write_ol));
	dev->write_ol.hEvent = CreateEvent(NULL, FALSE, FALSE /*inital state f=nonsignaled*/, NULL);											  

	return dev;
}

static void free_hid_device(hid_device *dev)
{
	CloseHandle(dev->ol.hEvent);
	CloseHandle(dev->write_ol.hEvent);							   
	CloseHandle(dev->device_handle);
	LocalFree(dev->last_error_str);
	free(dev->write_buf);
	free(dev->feature_buf);
	free(dev->read_buf);
	free(dev);
}

static void register_error(hid_device *dev, const char *op)
{
	WCHAR *ptr, *msg;
	(void)op; // unreferenced  param
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msg, 0/*sz*/,
		NULL);
	
	/* Get rid of the CR and LF that FormatMessage() sticks at the
	   end of the message. Thanks Microsoft! */
	ptr = msg;
	while (*ptr) {
		if (*ptr == '\r') {
			*ptr = 0x0000;
			break;
		}
		ptr++;
	}

	/* Store the message off in the Device entry so that
	   the hid_error() function can pick it up. */
	LocalFree(dev->last_error_str);
	dev->last_error_str = msg;
}

#ifndef HIDAPI_USE_DDK
static int lookup_functions()
{
	lib_handle = LoadLibraryA("hid.dll");
	if (lib_handle) {
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) return -1;
		RESOLVE(HidD_GetAttributes);
		RESOLVE(HidD_GetSerialNumberString);
		RESOLVE(HidD_GetManufacturerString);
		RESOLVE(HidD_GetProductString);
		RESOLVE(HidD_SetFeature);
		RESOLVE(HidD_GetFeature);
		RESOLVE(HidD_GetInputReport);
		RESOLVE(HidD_GetIndexedString);
		RESOLVE(HidD_GetPreparsedData);
		RESOLVE(HidD_FreePreparsedData);
		RESOLVE(HidP_GetCaps);
		RESOLVE(HidD_SetNumInputBuffers);
		RESOLVE(HidP_GetLinkCollectionNodes);
		RESOLVE(HidP_GetButtonCaps);
		RESOLVE(HidP_GetValueCaps);
		RESOLVE(HidP_MaxDataListLength);
#undef RESOLVE
#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif
	}
	else
		return -1;

	return 0;
}
#endif


typedef enum rd_items_ {
	rd_main_input =			      0x80, ///< 1000 00 nn
	rd_main_output =			  0x81, ///< 1001 00 nn
	rd_main_feature =			  0xB0, ///< 1011 00 nn
	rd_main_collection =		  0xA0, ///< 1010 00 nn
	rd_main_collection_end =	  0xC0, ///< 1100 00 nn
	rd_global_usage_page =		  0x04, ///< 0000 01 nn
	rd_global_logical_minimum =   0x14, ///< 0001 01 nn
	rd_global_logical_maximum =   0x24, ///< 0010 01 nn
	rd_global_physical_minimum =  0x34, ///< 0011 01 nn
	rd_global_physical_maximum =  0x44, ///< 0100 01 nn
	rd_global_unit_exponent =     0x54, ///< 0101 01 nn
	rd_global_unit =              0x64, ///< 0110 01 nn
	rd_global_report_size =       0x74, ///< 0111 01 nn
	rd_global_report_id =         0x84, ///< 1000 01 nn
	rd_global_report_count =      0x94, ///< 1001 01 nn
	rd_global_push =              0xA4, ///< 1010 01 nn
	rd_global_pop =               0xB4, ///< 1011 01 nn
	rd_local_usage =              0x08, ///< 0000 10 nn
	rd_local_usage_minimum =      0x18, ///< 0001 10 nn
	rd_local_usage_maximum =      0x28, ///< 0010 10 nn
	rd_local_designator_index =   0x38, ///< 0011 10 nn
	rd_local_designator_minimum = 0x48, ///< 0100 10 nn
	rd_local_designator_maximum = 0x58, ///< 0101 10 nn
	rd_local_string =             0x78, ///< 0111 10 nn
	rd_local_string_minimum =     0x88, ///< 1000 10 nn
	rd_local_string_maximum =     0x98, ///< 1001 10 nn
	rd_local_delimiter =          0xA8  ///< 1010 10 nn
} RD_ITEMS;


/// <summary>
///  Writes a short report descriptor item according USB HID spec 1.11 chapter 6.2.2.2
/// </summary>
/// <param name="data">Optional data  (NULL if bSize is 0)</param>
/// <param name="rd_item">Enumeration identifying type (Main, Global, Local) and function (e.g Usage or Report Count) of the item.</param>
/// <param name="bSize">Numeric expression specifying size of data (range 0-3 for 0,1,2 or 4Bytes)</param>
/// <returns></returns>
static int rd_write_short_item(enum RD_ITEMS rd_item, LONG64 data) {
	if (rd_item & 0x03) {
		return -1; // Invaid input data
	}
	if (rd_item == rd_main_collection_end) {
		// Item without data
		unsigned char oneBytePrefix = rd_item + 0x00;
		printf("%02X ", oneBytePrefix);
	} else if ((rd_item == rd_global_logical_minimum) ||
			   (rd_item == rd_global_logical_maximum) ||
	      	   (rd_item == rd_global_physical_minimum) ||
		       (rd_item == rd_global_physical_maximum)) {
		// Item with signed integer data
		if ((data >= -128) && (data <= 127)) {
			unsigned char oneBytePrefix = rd_item + 0x01;
			char localData = (char)data;
			printf("%02X %02X ", oneBytePrefix, localData & 0xFF);
		}
		else if ((data >= -32768) && (data <= 32767)) {
			unsigned char oneBytePrefix = rd_item + 0x02;
			INT16 localData = (INT16)data;
			printf("%02X %02X %02X ", oneBytePrefix, localData & 0xFF, localData >> 8 & 0xFF);
		}
		else if ((data >= -2147483648LL) && (data <= 2147483647)) {
			unsigned char oneBytePrefix = rd_item + 0x03;
			INT32 localData = (INT32)data;
			printf("%02X %02X %02X %02X %02X ", oneBytePrefix, localData & 0xFF, localData >> 8 & 0xFF, localData >> 16 & 0xFF, localData >> 24 & 0xFF);
		} else {
			// Error data out of range
			return -1;
		}
	} else {
		// Item with unsigned integer data
		if ((data >= 0) && (data <= 0xFF)) {
			unsigned char oneBytePrefix = rd_item + 0x01;
			unsigned char localData = (unsigned char)data;
			printf("%02X %02X ", oneBytePrefix, localData & 0xFF);
		}
		else if ((data >= 0) && (data <= 0xFFFF)) {
			unsigned char oneBytePrefix = rd_item + 0x02;
			UINT16 localData = (UINT16)data;
			printf("%02X %02X %02X ", oneBytePrefix, localData & 0xFF, localData >> 8 & 0xFF);
		}
		else if ((data >= 0) && (data <= 0xFFFFFFFF)) {
			unsigned char oneBytePrefix = rd_item + 0x03;
			UINT32 localData = (UINT32)data;
			printf("%02X %02X %02X %02X %02X ", oneBytePrefix, localData & 0xFF, localData >> 8 & 0xFF, localData >> 16 & 0xFF, localData >> 24 & 0xFF);
		} else {
			// Error data out of range
			return -1;
		}
	}
	return 0;
}

/// <summary>
/// Writes a long report descriptor item according USB HID spec 1.11 chapter 6.2.2.3
/// </summary>
/// <param name="data">Optional data items (NULL if bDataSize is 0)</param>
/// <param name="bLongItemTag">Long item tag (8 bits)</param>
/// <param name="bDataSize">Size of long item data (range 0-255 in Bytes)</param>
/// <returns></returns>
static int rd_write_long_item(unsigned char* data, unsigned char bLongItemTag, unsigned char bDataSize) {

}

typedef struct _RD_BUTTON_VALUE_CAP {
	int Button;
	int Value;
} RD_BUTTON_VALUE_CAP;

static int parse_win32_report_description(PHIDP_LINK_COLLECTION_NODE link_collection_nodes, ULONG link_collection_nodes_len, PHIDP_BUTTON_CAPS* button_caps, USHORT* button_caps_len, PHIDP_VALUE_CAPS* value_caps, USHORT* value_caps_len, ULONG* max_datalist_len) {
	
	// Create lookup tables for capability indices [COLLECTION_INDEX][INPUT/OUTPUT/FEATURE][DATA_INDEX]
	RD_BUTTON_VALUE_CAP*** dataindex_lut;


	dataindex_lut = malloc(link_collection_nodes_len * sizeof(*dataindex_lut));
	for (USHORT collection_node_idx = 0; collection_node_idx < link_collection_nodes_len; collection_node_idx++) {
		dataindex_lut[collection_node_idx] = malloc(NUM_OF_HIDP_REPORT_TYPES * sizeof(dataindex_lut[0]));
		for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
			dataindex_lut[collection_node_idx][rt_idx] = malloc(max_datalist_len[rt_idx] * sizeof(RD_BUTTON_VALUE_CAP));
		}
	}

	// Initialize with -1
	for (USHORT collection_node_idx = 0; collection_node_idx < link_collection_nodes_len; collection_node_idx++) {
		for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
			for (int data_idx = 0; data_idx < max_datalist_len[rt_idx]; data_idx++) {
				dataindex_lut[collection_node_idx][rt_idx][data_idx].Button = -1;
				dataindex_lut[collection_node_idx][rt_idx][data_idx].Value = -1;
			}
		}
	}
	
	for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
		for (USHORT caps_idx = 0; caps_idx < button_caps_len[rt_idx]; caps_idx++) {
			dataindex_lut[button_caps[rt_idx][caps_idx].LinkCollection][rt_idx][button_caps[rt_idx][caps_idx].NotRange.DataIndex].Button = caps_idx;
		}
		for (USHORT caps_idx = 0; caps_idx < value_caps_len[rt_idx]; caps_idx++) {
			dataindex_lut[value_caps[rt_idx][caps_idx].LinkCollection][rt_idx][value_caps[rt_idx][caps_idx].NotRange.DataIndex].Value = caps_idx;
		}
	}

	for (USHORT collection_node_idx = 0; collection_node_idx < link_collection_nodes_len; collection_node_idx++) {
		rd_write_short_item(rd_global_usage_page, link_collection_nodes[collection_node_idx].LinkUsagePage);
		printf("Usage Page (%d)\n", link_collection_nodes[collection_node_idx].LinkUsagePage);
		rd_write_short_item(rd_local_usage, link_collection_nodes[collection_node_idx].LinkUsage);
		printf("Usage  (%d)\n", link_collection_nodes[collection_node_idx].LinkUsage);
		if (link_collection_nodes[collection_node_idx].CollectionType == 0) {
			rd_write_short_item(rd_main_collection, 0x00);
			printf("Collection (Physical)\n");
		}
		else if(link_collection_nodes[collection_node_idx].CollectionType == 1) {
			rd_write_short_item(rd_main_collection, 0x01);
			printf("Collection (Application)\n");
		}
		else if (link_collection_nodes[collection_node_idx].CollectionType == 2) {
			rd_write_short_item(rd_main_collection, 0x02);
			printf("Collection (Logical)\n");
		}
		else {
			printf("Collection (nnn)\n");
		}
		for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
			for (int data_idx = 0; data_idx < max_datalist_len[rt_idx]; data_idx++) {
				int caps_idx = dataindex_lut[collection_node_idx][rt_idx][data_idx].Button;
				if (caps_idx != -1) {
					if (rt_idx == HidP_Input) {
						printf("Input\n");
					}
					else if (rt_idx == HidP_Output) {
						printf("Output\n");
					}
					else if (rt_idx == HidP_Feature) {
						printf("Feature\n");
					}
				}
				caps_idx = dataindex_lut[collection_node_idx][rt_idx][data_idx].Value;
				if (caps_idx != -1) {
					rd_write_short_item(rd_global_report_id, value_caps[rt_idx][caps_idx].ReportID);
					printf("Report ID (%d)\n", value_caps[rt_idx][caps_idx].ReportID);
					if (value_caps[rt_idx][caps_idx].IsRange) {
						rd_write_short_item(rd_local_usage_minimum, value_caps[rt_idx][caps_idx].Range.UsageMin);
						printf("Usage Minimum (%d)\n", value_caps[rt_idx][caps_idx].Range.UsageMin);
						rd_write_short_item(rd_local_usage_maximum, value_caps[rt_idx][caps_idx].Range.UsageMax);
						printf("Usage Maximum (%d)\n", value_caps[rt_idx][caps_idx].Range.UsageMax);
					} else {
						rd_write_short_item(rd_local_usage, value_caps[rt_idx][caps_idx].NotRange.Usage);
						printf("Usage  (%d)\n", value_caps[rt_idx][caps_idx].NotRange.Usage);
					}

					rd_write_short_item(rd_global_logical_minimum, value_caps[rt_idx][caps_idx].LogicalMin);
					printf("Logical Minimum (%d)\n", value_caps[rt_idx][caps_idx].LogicalMin);

					rd_write_short_item(rd_global_logical_maximum, value_caps[rt_idx][caps_idx].LogicalMax);
					printf("Logical Maximum (%d)\n", value_caps[rt_idx][caps_idx].LogicalMax);

					rd_write_short_item(rd_global_report_size, value_caps[rt_idx][caps_idx].BitSize);
					printf("Report Size (%d)\n", value_caps[rt_idx][caps_idx].BitSize);
					rd_write_short_item(rd_global_report_count, value_caps[rt_idx][caps_idx].ReportCount);
					printf("Report Count (%d)\n", value_caps[rt_idx][caps_idx].ReportCount);

					if (rt_idx == HidP_Input) {
						rd_write_short_item(rd_main_input, value_caps[rt_idx][caps_idx].BitField);
						printf("Input (0x%02X)\n", value_caps[rt_idx][caps_idx].BitField);
					}
					else if (rt_idx == HidP_Output) {
						rd_write_short_item(rd_main_output, value_caps[rt_idx][caps_idx].BitField);
						printf("Output (0x%02X)\n", value_caps[rt_idx][caps_idx].BitField);
					}
					else if (rt_idx == HidP_Feature) {
						rd_write_short_item(rd_main_feature, value_caps[rt_idx][caps_idx].BitField);
						printf("Feature (0x%02X)\n", value_caps[rt_idx][caps_idx].BitField);
					}
				}
			}
					
		}
		rd_write_short_item(rd_main_collection_end, 0);
		printf("End Collection\n");
	}
	for (USHORT collection_node_idx = 0; collection_node_idx < link_collection_nodes_len; collection_node_idx++) {
		for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
			free(dataindex_lut[collection_node_idx][rt_idx]);
		}
		free(dataindex_lut[collection_node_idx]);
	}
	free(dataindex_lut);
	return 0;
}

static HANDLE open_device(const char *path, BOOL open_rw)
{
	HANDLE handle;
	DWORD desired_access = (open_rw)? (GENERIC_WRITE | GENERIC_READ): 0;
	DWORD share_mode = FILE_SHARE_READ|FILE_SHARE_WRITE;

	handle = CreateFileA(path,
		desired_access,
		share_mode,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
		0);

	return handle;
}

HID_API_EXPORT const struct hid_api_version* HID_API_CALL hid_version()
{
	return &api_version;
}

HID_API_EXPORT const char* HID_API_CALL hid_version_str()
{
	return HID_API_VERSION_STR;
}

int HID_API_EXPORT hid_init(void)
{
#ifndef HIDAPI_USE_DDK
	if (!initialized) {
		if (lookup_functions() < 0) {
			hid_exit();
			return -1;
		}
		initialized = TRUE;
	}
#endif
	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
#ifndef HIDAPI_USE_DDK
	if (lib_handle)
		FreeLibrary(lib_handle);
	lib_handle = NULL;
	initialized = FALSE;
#endif
	return 0;
}

struct hid_device_info HID_API_EXPORT * HID_API_CALL hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	BOOL res;
	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;

	/* Hard-coded GUID retreived by HidD_GetHidGuid */
	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

	/* Windows objects for interacting with the driver. */
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	SP_DEVICE_INTERFACE_DETAIL_DATA_A *device_interface_detail_data = NULL;
	HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
	char driver_name[256];
	int device_index = 0;

	if (hid_init() < 0)
		return NULL;

	/* Initialize the Windows objects. */
	memset(&devinfo_data, 0x0, sizeof(devinfo_data));
	devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	/* Get information for all the devices belonging to the HID class. */
	device_info_set = SetupDiGetClassDevsA(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	
	/* Iterate over each device in the HID class, looking for the right one. */
	
	for (;;) {
		HANDLE write_handle = INVALID_HANDLE_VALUE;
		DWORD required_size = 0;
		HIDD_ATTRIBUTES attrib;

		res = SetupDiEnumDeviceInterfaces(device_info_set,
			NULL,
			&InterfaceClassGuid,
			device_index,
			&device_interface_data);
		
		if (!res) {
			/* A return of FALSE from this function means that
			   there are no more devices. */
			break;
		}

		/* Call with 0-sized detail size, and let the function
		   tell us how long the detail struct needs to be. The
		   size is put in &required_size. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,
			&device_interface_data,
			NULL,
			0,
			&required_size,
			NULL);

		/* Allocate a long enough structure for device_interface_detail_data. */
		device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*) malloc(required_size);
		device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

		/* Get the detailed data for this device. The detail data gives us
		   the device path for this device, which is then passed into
		   CreateFile() to get a handle to the device. */
		res = SetupDiGetDeviceInterfaceDetailA(device_info_set,
			&device_interface_data,
			device_interface_detail_data,
			required_size,
			NULL,
			NULL);

		if (!res) {
			/* register_error(dev, "Unable to call SetupDiGetDeviceInterfaceDetail");
			   Continue to the next device. */
			goto cont;
		}

		/* Populate devinfo_data. This function will return failure
		   when the device with such index doesn't exist. We've already checked it does. */
		res = SetupDiEnumDeviceInfo(device_info_set, device_index, &devinfo_data);
		if (!res)
			goto cont;


		/* Make sure this device has a driver bound to it. */
		res = SetupDiGetDeviceRegistryPropertyA(device_info_set, &devinfo_data,
			   SPDRP_DRIVER, NULL, (PBYTE)driver_name, sizeof(driver_name), NULL);
		if (!res)
			goto cont;

		//wprintf(L"HandleName: %s\n", device_interface_detail_data->DevicePath);

		/* Open a handle to the device */
		write_handle = open_device(device_interface_detail_data->DevicePath, FALSE);

		/* Check validity of write_handle. */
		if (write_handle == INVALID_HANDLE_VALUE) {
			/* Unable to open the device. */
			//register_error(dev, "CreateFile");
			goto cont_close;
		}		


		/* Get the Vendor ID and Product ID for this device. */
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		HidD_GetAttributes(write_handle, &attrib);
		//wprintf(L"Product/Vendor: %x %x\n", attrib.ProductID, attrib.VendorID);

		/* Check the VID/PID to see if we should add this
		   device to the enumeration list. */
		if ((vendor_id == 0x0 || attrib.VendorID == vendor_id) &&
		    (product_id == 0x0 || attrib.ProductID == product_id)) {

			#define WSTR_LEN 512
			const char *str;
			struct hid_device_info *tmp;
			PHIDP_PREPARSED_DATA pp_data = NULL;
			HIDP_CAPS caps;
			NTSTATUS nt_res;
			wchar_t wstr[WSTR_LEN]; /* TODO: Determine Size */
			size_t len;

			/* VID/PID match. Create the record. */
			tmp = (struct hid_device_info*) calloc(1, sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			cur_dev = tmp;

			/* Get the Usage Page and Usage for this device. */
			res = HidD_GetPreparsedData(write_handle, &pp_data);
			if (res) {
				nt_res = HidP_GetCaps(pp_data, &caps);
				if (nt_res == HIDP_STATUS_SUCCESS) {
					cur_dev->usage_page = caps.UsagePage;
					cur_dev->usage = caps.Usage;
				}

				// Experimental Report Descriptor code starts here
				// See: https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/link-collections#ddk-link-collection-array-kg

				PHIDP_LINK_COLLECTION_NODE link_collection_nodes;
				link_collection_nodes = (PHIDP_LINK_COLLECTION_NODE)malloc(caps.NumberLinkCollectionNodes * sizeof(HIDP_LINK_COLLECTION_NODE));
				ULONG                     link_collection_nodes_len = caps.NumberLinkCollectionNodes;

				PHIDP_BUTTON_CAPS button_caps[NUM_OF_HIDP_REPORT_TYPES];
				USHORT button_caps_len[NUM_OF_HIDP_REPORT_TYPES];

				button_caps[HidP_Input] = (PHIDP_BUTTON_CAPS)malloc(caps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
				button_caps_len[HidP_Input] = caps.NumberInputButtonCaps;
				button_caps[HidP_Output] = (PHIDP_BUTTON_CAPS)malloc(caps.NumberOutputButtonCaps * sizeof(HIDP_BUTTON_CAPS));
				button_caps_len[HidP_Output] = caps.NumberOutputButtonCaps;
				button_caps[HidP_Feature] = (PHIDP_BUTTON_CAPS)malloc(caps.NumberFeatureButtonCaps * sizeof(HIDP_BUTTON_CAPS));
				button_caps_len[HidP_Feature] = caps.NumberFeatureButtonCaps;
				
				
			    PHIDP_VALUE_CAPS value_caps[NUM_OF_HIDP_REPORT_TYPES];
				USHORT value_caps_len[NUM_OF_HIDP_REPORT_TYPES];

				value_caps[HidP_Input] = (PHIDP_VALUE_CAPS)malloc(caps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));
				value_caps_len[HidP_Input] = caps.NumberInputValueCaps;
				value_caps[HidP_Output] = (PHIDP_VALUE_CAPS)malloc(caps.NumberOutputValueCaps * sizeof(HIDP_VALUE_CAPS));
				value_caps_len[HidP_Output] = caps.NumberOutputValueCaps;
				value_caps[HidP_Feature] = (PHIDP_VALUE_CAPS)malloc(caps.NumberFeatureValueCaps * sizeof(HIDP_VALUE_CAPS));
				value_caps_len[HidP_Feature] = caps.NumberFeatureValueCaps;

				ULONG max_datalist_len[NUM_OF_HIDP_REPORT_TYPES];

				if (HidP_GetLinkCollectionNodes(link_collection_nodes, &link_collection_nodes_len, pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetLinkCollectionNodes: Buffer to small");
				}
				else if ((button_caps_len[HidP_Input] != 0) && HidP_GetButtonCaps(HidP_Input, button_caps[HidP_Input], &button_caps_len[HidP_Input], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetButtonCaps: HidP_Input: The preparsed data is not valid. ");
				}
				else if ((button_caps_len[HidP_Output] != 0) && HidP_GetButtonCaps(HidP_Output, button_caps[HidP_Output], &button_caps_len[HidP_Output], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetButtonCaps: HidP_Output: The preparsed data is not valid. ");
				}
				else if ((button_caps_len[HidP_Feature] != 0) && HidP_GetButtonCaps(HidP_Feature, button_caps[HidP_Feature], &button_caps_len[HidP_Feature], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetButtonCaps: HidP_Feature: The preparsed data is not valid. ");
				}
				else if ((value_caps_len[HidP_Input] != 0) && HidP_GetValueCaps(HidP_Input, value_caps[HidP_Input], &value_caps_len[HidP_Input], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetValueCaps: HidP_Input: The preparsed data is not valid. ");
				}
				else if ((value_caps_len[HidP_Output] != 0) && HidP_GetValueCaps(HidP_Output, value_caps[HidP_Output], &value_caps_len[HidP_Output], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetValueCaps: HidP_Output: The preparsed data is not valid. ");
				}
				else if ((value_caps_len[HidP_Feature] != 0) && HidP_GetValueCaps(HidP_Feature, value_caps[HidP_Feature], &value_caps_len[HidP_Feature], pp_data) != HIDP_STATUS_SUCCESS) {
					//register_error(dev, "HidP_GetValueCaps: HidP_Feature: The preparsed data is not valid. ");
				}
				else {
					// All data read successfull
					max_datalist_len[HidP_Input] = HidP_MaxDataListLength(HidP_Input, pp_data);
					max_datalist_len[HidP_Output] = HidP_MaxDataListLength(HidP_Output, pp_data);
					max_datalist_len[HidP_Feature] = HidP_MaxDataListLength(HidP_Feature, pp_data);
					parse_win32_report_description(link_collection_nodes, link_collection_nodes_len, button_caps, button_caps_len, value_caps, value_caps_len, max_datalist_len);

				}

				// Free allocated memory
				for (int rt_idx = 0; rt_idx < NUM_OF_HIDP_REPORT_TYPES; rt_idx++) {
					free(button_caps[rt_idx]);
					free(value_caps[rt_idx]);
				}
				free(link_collection_nodes);
				// Experimental Report Descriptor code ends here
				HidD_FreePreparsedData(pp_data);
			}
			
			/* Fill out the record */
			cur_dev->next = NULL;
			str = device_interface_detail_data->DevicePath;
			if (str) {
				len = strlen(str);
				cur_dev->path = (char*) calloc(len+1, sizeof(char));
				strncpy(cur_dev->path, str, len+1);
				cur_dev->path[len] = '\0';
			}
			else
				cur_dev->path = NULL;

			/* Serial Number */
			wstr[0]= 0x0000;
			res = HidD_GetSerialNumberString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->serial_number = _wcsdup(wstr);
			}

			/* Manufacturer String */
			wstr[0]= 0x0000;
			res = HidD_GetManufacturerString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->manufacturer_string = _wcsdup(wstr);
			}

			/* Product String */
			wstr[0]= 0x0000;
			res = HidD_GetProductString(write_handle, wstr, sizeof(wstr));
			wstr[WSTR_LEN-1] = 0x0000;
			if (res) {
				cur_dev->product_string = _wcsdup(wstr);
			}

			/* VID/PID */
			cur_dev->vendor_id = attrib.VendorID;
			cur_dev->product_id = attrib.ProductID;

			/* Release Number */
			cur_dev->release_number = attrib.VersionNumber;

			/* Interface Number. It can sometimes be parsed out of the path
			   on Windows if a device has multiple interfaces. See
			   http://msdn.microsoft.com/en-us/windows/hardware/gg487473 or
			   search for "Hardware IDs for HID Devices" at MSDN. If it's not
			   in the path, it's set to -1. */
			cur_dev->interface_number = -1;
			if (cur_dev->path) {
				char *interface_component = strstr(cur_dev->path, "&mi_");
				if (interface_component) {
					char *hex_str = interface_component + 4;
					char *endptr = NULL;
					cur_dev->interface_number = strtol(hex_str, &endptr, 16);
					if (endptr == hex_str) {
						/* The parsing failed. Set interface_number to -1. */
						cur_dev->interface_number = -1;
					}
				}
			}
		}

cont_close:
		CloseHandle(write_handle);
cont:
		/* We no longer need the detail data. It can be freed */
		free(device_interface_detail_data);

		device_index++;

	}

	/* Close the device information handle. */
	SetupDiDestroyDeviceInfoList(device_info_set);

	return root;

}

void  HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info *devs)
{
	/* TODO: Merge this with the Linux version. This function is platform-independent. */
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

HID_API_EXPORT hid_device * HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	/* TODO: Merge this functions with the Linux version. This function should be platform independent. */
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;
	
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (cur_dev->serial_number && wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);
	
	return handle;
}

HID_API_EXPORT hid_device * HID_API_CALL hid_open_path(const char *path)
{
	hid_device *dev;
	HIDP_CAPS caps;
	PHIDP_PREPARSED_DATA pp_data = NULL;
	BOOLEAN res;
	NTSTATUS nt_res;

	if (hid_init() < 0) {
		return NULL;
	}

	dev = new_hid_device();

	/* Open a handle to the device */
	dev->device_handle = open_device(path, TRUE);

	/* Check validity of write_handle. */
	if (dev->device_handle == INVALID_HANDLE_VALUE) {
		/* System devices, such as keyboards and mice, cannot be opened in
		   read-write mode, because the system takes exclusive control over
		   them.  This is to prevent keyloggers.  However, feature reports
		   can still be sent and received.  Retry opening the device, but
		   without read/write access. */
		dev->device_handle = open_device(path, FALSE);

		/* Check the validity of the limited device_handle. */
		if (dev->device_handle == INVALID_HANDLE_VALUE) {
			/* Unable to open the device, even without read-write mode. */
			register_error(dev, "CreateFile");
			goto err;
		}
	}

	/* Set the Input Report buffer size to 64 reports. */
	res = HidD_SetNumInputBuffers(dev->device_handle, 64);
	if (!res) {
		register_error(dev, "HidD_SetNumInputBuffers");
		goto err;
	}

	/* Get the Input Report length for the device. */
	res = HidD_GetPreparsedData(dev->device_handle, &pp_data);
	if (!res) {
		register_error(dev, "HidD_GetPreparsedData");
		goto err;
	}
	nt_res = HidP_GetCaps(pp_data, &caps);
	if (nt_res != HIDP_STATUS_SUCCESS) {
		register_error(dev, "HidP_GetCaps");	
		goto err_pp_data;
	}
	dev->output_report_length = caps.OutputReportByteLength;
	dev->input_report_length = caps.InputReportByteLength;
	dev->feature_report_length = caps.FeatureReportByteLength;
	HidD_FreePreparsedData(pp_data);

	dev->read_buf = (char*) malloc(dev->input_report_length);

	return dev;

err_pp_data:
		HidD_FreePreparsedData(pp_data);
err:	
		free_hid_device(dev);
		return NULL;
}

int HID_API_EXPORT HID_API_CALL hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	DWORD bytes_written = 0;
	int function_result = -1;
	BOOL res;
	BOOL overlapped = FALSE;

	unsigned char *buf;

	/* Make sure the right number of bytes are passed to WriteFile. Windows
	   expects the number of bytes which are in the _longest_ report (plus
	   one for the report number) bytes even if the data is a report
	   which is shorter than that. Windows gives us this value in
	   caps.OutputReportByteLength. If a user passes in fewer bytes than this,
	   use cached temporary buffer which is the proper size. */
	if (length >= dev->output_report_length) {
		/* The user passed the right number of bytes. Use the buffer as-is. */
		buf = (unsigned char *) data;
	} else {
		if (dev->write_buf == NULL)
			dev->write_buf = (unsigned char *) malloc(dev->output_report_length);
		buf = dev->write_buf;
		memcpy(buf, data, length);
		memset(buf + length, 0, dev->output_report_length - length);
		length = dev->output_report_length;
	}

	res = WriteFile(dev->device_handle, buf, (DWORD) length, NULL, &dev->write_ol);
	
	if (!res) {
		if (GetLastError() != ERROR_IO_PENDING) {
			/* WriteFile() failed. Return error. */
			register_error(dev, "WriteFile");
			goto end_of_function;
		}
		overlapped = TRUE;
	}

	if (overlapped) {
		/* Wait for the transaction to complete. This makes
		   hid_write() synchronous. */
		res = WaitForSingleObject(dev->write_ol.hEvent, 1000);
		if (res != WAIT_OBJECT_0) {
			/* There was a Timeout. */
			register_error(dev, "WriteFile/WaitForSingleObject Timeout");
			goto end_of_function;
		}

		/* Get the result. */
		res = GetOverlappedResult(dev->device_handle, &dev->write_ol, &bytes_written, FALSE/*wait*/);
		if (res) {
			function_result = bytes_written;
		}
		else {
			/* The Write operation failed. */
			register_error(dev, "WriteFile");
			goto end_of_function;
		}
	}

end_of_function:
	return function_result;
}


int HID_API_EXPORT HID_API_CALL hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	DWORD bytes_read = 0;
	size_t copy_len = 0;
	BOOL res = FALSE;
	BOOL overlapped = FALSE;

	/* Copy the handle for convenience. */
	HANDLE ev = dev->ol.hEvent;

	if (!dev->read_pending) {
		/* Start an Overlapped I/O read. */
		dev->read_pending = TRUE;
		memset(dev->read_buf, 0, dev->input_report_length);
		ResetEvent(ev);
		res = ReadFile(dev->device_handle, dev->read_buf, (DWORD) dev->input_report_length, &bytes_read, &dev->ol);
		
		if (!res) {
			if (GetLastError() != ERROR_IO_PENDING) {
				/* ReadFile() has failed.
				   Clean up and return error. */
				CancelIo(dev->device_handle);
				dev->read_pending = FALSE;
				goto end_of_function;
			}
			overlapped = TRUE;	   
		}																		   
	}
	else {
		overlapped = TRUE;	
	}

	if (overlapped) {
		if (milliseconds >= 0) {
			/* See if there is any data yet. */
			res = WaitForSingleObject(ev, milliseconds);
			if (res != WAIT_OBJECT_0) {
				/* There was no data this time. Return zero bytes available,
				   but leave the Overlapped I/O running. */
				return 0;
			}
		}

		/* Either WaitForSingleObject() told us that ReadFile has completed, or
		   we are in non-blocking mode. Get the number of bytes read. The actual
		   data has been copied to the data[] array which was passed to ReadFile(). */
		res = GetOverlappedResult(dev->device_handle, &dev->ol, &bytes_read, TRUE/*wait*/);
	}
	/* Set pending back to false, even if GetOverlappedResult() returned error. */
	dev->read_pending = FALSE;

	if (res && bytes_read > 0) {
		if (dev->read_buf[0] == 0x0) {
			/* If report numbers aren't being used, but Windows sticks a report
			   number (0x0) on the beginning of the report anyway. To make this
			   work like the other platforms, and to make it work more like the
			   HID spec, we'll skip over this byte. */
			bytes_read--;
			copy_len = length > bytes_read ? bytes_read : length;
			memcpy(data, dev->read_buf+1, copy_len);
		}
		else {
			/* Copy the whole buffer, report number and all. */
			copy_len = length > bytes_read ? bytes_read : length;
			memcpy(data, dev->read_buf, copy_len);
		}
	}
	
end_of_function:
	if (!res) {
		register_error(dev, "GetOverlappedResult");
		return -1;
	}
	
	return (int) copy_len;
}

int HID_API_EXPORT HID_API_CALL hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, (dev->blocking)? -1: 0);
}

int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device *dev, int nonblock)
{
	dev->blocking = !nonblock;
	return 0; /* Success */
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	BOOL res = FALSE;
	unsigned char *buf;
	size_t length_to_send;

	/* Windows expects at least caps.FeatureReportByteLength bytes passed
	   to HidD_SetFeature(), even if the report is shorter. Any less sent and
	   the function fails with error ERROR_INVALID_PARAMETER set. Any more
	   and HidD_SetFeature() silently truncates the data sent in the report
	   to caps.FeatureReportByteLength. */
	if (length >= dev->feature_report_length) {
		buf = (unsigned char *) data;
		length_to_send = length;
	} else {
		if (dev->feature_buf == NULL)
			dev->feature_buf = (unsigned char *) malloc(dev->feature_report_length);
		buf = dev->feature_buf;
		memcpy(buf, data, length);
		memset(buf + length, 0, dev->feature_report_length - length);
		length_to_send = dev->feature_report_length;
	}

	res = HidD_SetFeature(dev->device_handle, (PVOID)buf, (DWORD) length_to_send);

	if (!res) {
		register_error(dev, "HidD_SetFeature");
		return -1;
	}

	return (int) length;
}


int HID_API_EXPORT HID_API_CALL hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	BOOL res;
#if 0
	res = HidD_GetFeature(dev->device_handle, data, length);
	if (!res) {
		register_error(dev, "HidD_GetFeature");
		return -1;
	}
	return 0; /* HidD_GetFeature() doesn't give us an actual length, unfortunately */
#else
	DWORD bytes_returned;

	OVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));

	res = DeviceIoControl(dev->device_handle,
		IOCTL_HID_GET_FEATURE,
		data, (DWORD) length,
		data, (DWORD) length,
		&bytes_returned, &ol);

	if (!res) {
		if (GetLastError() != ERROR_IO_PENDING) {
			/* DeviceIoControl() failed. Return error. */
			register_error(dev, "Send Feature Report DeviceIoControl");
			return -1;
		}
	}

	/* Wait here until the write is done. This makes
	   hid_get_feature_report() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_returned, TRUE/*wait*/);
	if (!res) {
		/* The operation failed. */
		register_error(dev, "Send Feature Report GetOverLappedResult");
		return -1;
	}

	/* bytes_returned does not include the first byte which contains the
	   report ID. The data buffer actually contains one more byte than
	   bytes_returned. */
	bytes_returned++;

	return bytes_returned;
#endif
}


int HID_API_EXPORT HID_API_CALL hid_get_input_report(hid_device *dev, unsigned char *data, size_t length)
{
	BOOL res;
#if 0
	res = HidD_GetInputReport(dev->device_handle, data, length);
	if (!res) {
		register_error(dev, "HidD_GetInputReport");
		return -1;
	}
	return length;
#else
	DWORD bytes_returned;

	OVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));

	res = DeviceIoControl(dev->device_handle,
		IOCTL_HID_GET_INPUT_REPORT,
		data, (DWORD) length,
		data, (DWORD) length,
		&bytes_returned, &ol);

	if (!res) {
		if (GetLastError() != ERROR_IO_PENDING) {
			/* DeviceIoControl() failed. Return error. */
			register_error(dev, "Send Input Report DeviceIoControl");
			return -1;
		}
	}

	/* Wait here until the write is done. This makes
	   hid_get_feature_report() synchronous. */
	res = GetOverlappedResult(dev->device_handle, &ol, &bytes_returned, TRUE/*wait*/);
	if (!res) {
		/* The operation failed. */
		register_error(dev, "Send Input Report GetOverLappedResult");
		return -1;
	}

	/* bytes_returned does not include the first byte which contains the
	   report ID. The data buffer actually contains one more byte than
	   bytes_returned. */
	bytes_returned++;

	return bytes_returned;
#endif
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device *dev)
{
	if (!dev)
		return;
	CancelIo(dev->device_handle);
	free_hid_device(dev);
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetManufacturerString(dev->device_handle, string, sizeof(wchar_t) * (DWORD) MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetManufacturerString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetProductString(dev->device_handle, string, sizeof(wchar_t) * (DWORD) MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetProductString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetSerialNumberString(dev->device_handle, string, sizeof(wchar_t) * (DWORD) MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetSerialNumberString");
		return -1;
	}

	return 0;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	BOOL res;

	res = HidD_GetIndexedString(dev->device_handle, string_index, string, sizeof(wchar_t) * (DWORD) MIN(maxlen, MAX_STRING_WCHARS));
	if (!res) {
		register_error(dev, "HidD_GetIndexedString");
		return -1;
	}

	return 0;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	if (dev) {
		if (dev->last_error_str == NULL)
			return L"Success";
		return (wchar_t*)dev->last_error_str;
	}

	// Global error messages are not (yet) implemented on Windows.
	return L"hid_error for global errors is not implemented yet";
}


/*#define PICPGM*/
/*#define S11*/
#define P32
#ifdef S11 
  unsigned short VendorID = 0xa0a0;
	unsigned short ProductID = 0x0001;
#endif

#ifdef P32
  unsigned short VendorID = 0x04d8;
	unsigned short ProductID = 0x3f;
#endif


#ifdef PICPGM
  unsigned short VendorID = 0x04d8;
  unsigned short ProductID = 0x0033;
#endif


#if 0
int __cdecl main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	/* Set up the command buffer. */
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0;
	buf[1] = 0x81;
	

	/* Open the device. */
	int handle = open(VendorID, ProductID, L"12345");
	if (handle < 0)
		printf("unable to open device\n");


	/* Toggle LED (cmd 0x80) */
	buf[1] = 0x80;
	res = write(handle, buf, 65);
	if (res < 0)
		printf("Unable to write()\n");

	/* Request state (cmd 0x81) */
	buf[1] = 0x81;
	write(handle, buf, 65);
	if (res < 0)
		printf("Unable to write() (2)\n");

	/* Read requested state */
	read(handle, buf, 65);
	if (res < 0)
		printf("Unable to read()\n");

	/* Print out the returned buffer. */
	for (int i = 0; i < 4; i++)
		printf("buf[%d]: %d\n", i, buf[i]);

	return 0;
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
