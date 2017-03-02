/* DFU related USB Descriptors */
/* (C) 2006-2017 Harald Welte <hwelte@hmw-consulting.de> */

#include <unistd.h>

#include "board.h"
#include "utils.h"

#include <usb/include/USBDescriptors.h>

#include <usb/include/USBDDriver.h>

#include <usb/common/dfu/usb_dfu.h>
#include <usb/device/dfu/dfu.h>

enum {
	STR_MANUF	= 1,
	STR_PROD,
	STR_CONFIG,
	_STR_FIRST_ALT,
	STR_SERIAL	= (_STR_FIRST_ALT+BOARD_DFU_NUM_IF),
};

static const USBDeviceDescriptor fsDevice = {
	.bLength = 		sizeof(USBDeviceDescriptor),
	.bDescriptorType = 	USBGenericDescriptor_DEVICE,
	.bcdUSB = 		USBDeviceDescriptor_USB2_00,
	.bDeviceClass =		0,
	.bDeviceSubClass = 	0,
	.bDeviceProtocol = 	0,
	.bMaxPacketSize0 =	BOARD_USB_ENDPOINTS_MAXPACKETSIZE(0),
	.idVendor =		BOARD_USB_VENDOR,
	.idProduct = 		BOARD_USB_PRODUCT,
	.bcdDevice = 		BOARD_USB_RELEASE,
	.iManufacturer =	STR_MANUF,
	.iProduct =		STR_PROD,
#ifdef BOARD_USB_SERIAL
	.iSerialNumber =	STR_SERIAL,
#else
	.iSerialNumber =	0,
#endif
	.bNumConfigurations =	1,
};

/* Alternate Interface Descriptor, we use one per partition/memory type */
#define DFU_IF(ALT)								\
	{									\
		.bLength =		sizeof(USBInterfaceDescriptor),		\
		.bDescriptorType = 	USBGenericDescriptor_INTERFACE,		\
		.bInterfaceNumber =	0,					\
		.bAlternateSetting =	ALT,					\
		.bNumEndpoints =	0,					\
		.bInterfaceClass =	0xfe,					\
		.bInterfaceSubClass =	1,					\
		.iInterface =		(_STR_FIRST_ALT+ALT),			\
		.bInterfaceProtocol =	2,					\
	}

/* overall descriptor for the DFU configuration, including all
 * descriptors for alternate interfaces */
const struct dfu_desc dfu_cfg_descriptor = {
	.ucfg = {
		.bLength =	   sizeof(USBConfigurationDescriptor),
		.bDescriptorType = USBGenericDescriptor_CONFIGURATION,
		.wTotalLength =	   sizeof(struct dfu_desc),
		.bNumInterfaces =  1,
		.bConfigurationValue = 1,
		.iConfiguration =  STR_CONFIG,
		.bmAttributes =	   BOARD_USB_BMATTRIBUTES,
		.bMaxPower =	   100,
	},
	.uif[0] = DFU_IF(0),
#if BOARD_DFU_NUM_IF > 1
	.uif[1] = DFU_IF(1),
#endif
#if BOARD_DFU_NUM_IF > 2
	.uif[2] = DFU_IF(2),
#endif
#if BOARD_DFU_NUM_IF > 3
	.uif[3] = DFU_IF(3),
#endif
#if BOARD_DFU_NUM_IF > 4
	.uif[4] = DFU_IF(4),
#endif
	.func_dfu = DFU_FUNC_DESC
};

#if 0
#include "usb_strings.h"


static const unsigned char *usb_strings[] = {
	USB_STRINGS_GENERATED
#ifdef BOARD_USB_SERIAL
	NULL
#endif
};

void set_usb_serial_str(const uint8_t *serial_usbstr)
{
	usb_strings[STR_SERIAL] = serial_usbstr;
}
#else
static const unsigned char langDesc[] = {
	USBStringDescriptor_LENGTH(1),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_ENGLISH_US
};

static const unsigned char manufStringDescriptor[] = {
	USBStringDescriptor_LENGTH(24),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('s'),
	USBStringDescriptor_UNICODE('y'),
	USBStringDescriptor_UNICODE('s'),
	USBStringDescriptor_UNICODE('m'),
	USBStringDescriptor_UNICODE('o'),
	USBStringDescriptor_UNICODE('c'),
	USBStringDescriptor_UNICODE('o'),
	USBStringDescriptor_UNICODE('m'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('-'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('s'),
	USBStringDescriptor_UNICODE('.'),
	USBStringDescriptor_UNICODE('f'),
	USBStringDescriptor_UNICODE('.'),
	USBStringDescriptor_UNICODE('m'),
	USBStringDescriptor_UNICODE('.'),
	USBStringDescriptor_UNICODE('c'),
	USBStringDescriptor_UNICODE('.'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('G'),
	USBStringDescriptor_UNICODE('m'),
	USBStringDescriptor_UNICODE('b'),
	USBStringDescriptor_UNICODE('H'),
};

static const unsigned char productStringDescriptor[] = {
	USBStringDescriptor_LENGTH(10),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('S'),
	USBStringDescriptor_UNICODE('I'),
	USBStringDescriptor_UNICODE('M'),
	USBStringDescriptor_UNICODE('t'),
	USBStringDescriptor_UNICODE('r'),
	USBStringDescriptor_UNICODE('a'),
	USBStringDescriptor_UNICODE('c'),
	USBStringDescriptor_UNICODE('e'),
	USBStringDescriptor_UNICODE(' '),
	USBStringDescriptor_UNICODE('2'),
};


static const unsigned char configStringDescriptor[] = {
	USBStringDescriptor_LENGTH(3),
	USBGenericDescriptor_STRING,
	USBStringDescriptor_UNICODE('D'),
	USBStringDescriptor_UNICODE('F'),
	USBStringDescriptor_UNICODE('U'),
};

/** List of string descriptors used by the device */
static const unsigned char *usb_strings[] = {
	langDesc,
	[STR_MANUF] = manufStringDescriptor,
	[STR_PROD] = productStringDescriptor,
	[STR_CONFIG] = configStringDescriptor,
};
#endif

static const USBConfigurationDescriptor *conf_desc_arr[] = {
	&dfu_cfg_descriptor.ucfg,
};

const USBDDriverDescriptors dfu_descriptors = {
	.pFsDevice = &fsDevice,
	.pFsConfiguration = (const USBConfigurationDescriptor **)&conf_desc_arr,
	/* DFU only supports FS for now */
	.pFsQualifier = NULL,
	.pFsOtherSpeed = NULL,
	.pHsDevice = NULL,
	.pHsConfiguration = NULL,
	.pHsQualifier = NULL,
	.pHsOtherSpeed = NULL,
	.pStrings = usb_strings,
	.numStrings = ARRAY_SIZE(usb_strings),
};