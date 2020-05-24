
/*-
 * $Copyright$
-*/

#include <usb/UsbTypes.hpp>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

extern const ::usb::UsbDeviceDescriptor_t usbDeviceDescriptor __attribute__((aligned(4), section(".fixeddata"))) = {
    .m_bLength              = sizeof(::usb::UsbDeviceDescriptor_t),                     /* m_bLength */
    .m_bDescriptorType      = ::usb::UsbDescriptorTypeId_t::e_Device,                   /* m_bDescriptorType */
    .m_bcdUsb               = { 0x00, 0x02 },                                           /* m_bLength */
    .m_bDeviceClass         = ::usb::UsbInterfaceClass_e::e_UsbInterface_Misc_EFh,      /* m_bDeviceClass */
    .m_bDeviceSubClass      = 0x02,                                                     /* m_bDeviceSubClass */
    .m_bDeviceProtocol      = 0x01,                                                     /* m_bDeviceProtocol */
    .m_bMaxPacketSize0      = 64,                                                       /* m_bMaxPacketSize0 -- Should be 64 for a USB Full Speed Device. */
    .m_idVendor             = { 0xad, 0xde },                                           /* m_idVendor */
    .m_idProduct            = { 0xef, 0xbe },                                           /* m_idProduct */
    .m_bcdDevice            = { 0xfe, 0xca },                                           /* m_bcdDevice */
    .m_iManufacturer        = ::usb::UsbStringDescriptorId_t::e_StrDesc_Manufacturer,   /* e_iManufacturer */
    .m_iProduct             = ::usb::UsbStringDescriptorId_t::e_StrDesc_Product,        /* e_iProduct */
    .m_iSerialNumber        = ::usb::UsbStringDescriptorId_t::e_StrDesc_SerialNumber,   /* e_iSerialNumber */
    .m_bNumConfigurations   = 1                                                         /* e_bNumConfigurations */
};

/*******************************************************************************
 * USB Device String Descriptors
 ******************************************************************************/
static const ::usb::UsbLangId_t usbSupportedLanguageIds[] = { 0x0409, 0x0000 };

extern const ::usb::UsbStringDescriptors_t usbStringDescriptors __attribute__((aligned(4), section(".fixeddata"))) = {
        .m_stringDescriptorTable = {
                .m_languageIds      = ::usb::UsbLangIdStringDescriptor_t(usbSupportedLanguageIds),
                .m_manufacturer     = ::usb::UsbStringDescriptor("PhiSch.org"),
                .m_product          = ::usb::UsbStringDescriptor("PhiSch.org USB Virtual COM Port (VCP) Demo on STM32F4Discovery"),
                .m_serialNumber     = ::usb::UsbStringDescriptor("6B426366-3E64-4B20-A12D-B8AC744F8ED5"),
                .m_configuration    = ::usb::UsbStringDescriptor("PhiSch.org USB Virtual COM Port (VCP) Configuration"),
                .m_interface        = ::usb::UsbStringDescriptor("PhiSch.org USB Comm. Device Class (CDC) Interface")
        }
};

extern const struct UsbConfigurationDescriptor_s {
    struct ::usb::UsbConfigurationDescriptorT<void *, 0>    m_configDescrHdr;
    struct ::usb::UsbInterfaceAssociationDescriptor_s       m_iad;
    struct ::usb::UsbInterfaceDescriptorT<0>                m_cdcInterface;
    struct ::usb::UsbCdc_FunctDescr_Header_s                m_cdcFunctDescr_Header;
    struct ::usb::UsbCdc_FunctDescr_CallMgmt_s              m_cdcFunctDescr_CallMgmt;
    struct ::usb::UsbCdc_FunctDescr_ACM_s                   m_cdcFunctDescr_ACM;
    struct ::usb::UsbCdc_FunctDescr_UnionT<1>               m_cdcFunctDescr_Union;
    struct ::usb::UsbEndpointDescriptor                     m_notificationEndpoint;
    struct ::usb::UsbInterfaceDescriptorT<2>                m_dataInterface;
} __attribute__((packed)) usbConfigurationDescriptor __attribute__((aligned(4), section(".fixeddata"))) = {
    .m_configDescrHdr = {
        .m_bLength                  = sizeof( decltype(usbConfigurationDescriptor.m_configDescrHdr)),
        .m_bDescriptorType          = ::usb::UsbDescriptorTypeId_e::e_Configuration,
        .m_wTotalLength = {
            .m_loByte               = (sizeof(decltype(usbConfigurationDescriptor)) >> 0),
            .m_hiByte               = (sizeof(decltype(usbConfigurationDescriptor)) >> 8)
        },
        .m_bNumInterfaces           = 2,
        .m_bConfigurationValue      = 1,
        .m_iConfiguration           = 4, /* Index of m_configuration within usbStringDescriptors.m_stringDescriptorTable */
        .m_bmAttributes             = 0x80      // USB 2.0 Spec demands this Bit to always be set
                                    | 0x40,     // Set to 0x40 for Self-powered Device, 0x00 for Bus-powered
        .m_bMaxPower                = 5,        // Power consumption in Units of 2mA
        .m_interfaces               = {}
    },
    .m_iad = {
        .m_bLength                  = sizeof(decltype(usbConfigurationDescriptor.m_iad)),
        .m_bDescriptorType          = 0x0B /* ::usb::UsbDescriptorTypeId_e::e_InterfaceAssociation */,
        .m_bFirstInterface          = 0 /* usbConfigurationDescriptor.m_cdcInterface.m_bInterfaceNumber */,
        .m_bInterfaceCount          = 2,
        .m_bFunctionClass           = ::usb::UsbInterfaceClass_e::e_UsbInterface_CommunicationDeviceClass,
        .m_bFunctionSubClass        = ::usb::UsbCdc_SubclassCode_e::e_UsbCdcSubclass_AbstractControl,
        .m_bFunctionProtocol        = ::usb::UsbCdc_ProtocolCode_e::e_UsbCdcProto_AT_V250,
        .m_iFunction                = 5 /* Index of m_interface within usbStringDescriptors.m_stringDescriptorTable */
    },
    .m_cdcInterface = {
        .m_bLength                  = sizeof(decltype(usbConfigurationDescriptor.m_cdcInterface)),
        .m_bDescriptorType          = ::usb::UsbDescriptorTypeId_e::e_Interface,
        .m_bInterfaceNumber         = 0,
        .m_bAlternateSetting        = 0,
        .m_bNumEndpoints            = 1,
        .m_bInterfaceClass          = ::usb::UsbInterfaceClass_e::e_UsbInterface_CommunicationDeviceClass,
        .m_bInterfaceSubClass       = ::usb::UsbCdc_SubclassCode_e::e_UsbCdcSubclass_AbstractControl,
        .m_bInterfaceProtocol       = ::usb::UsbCdc_ProtocolCode_e::e_UsbCdcProto_AT_V250,
        .m_iInterface               = 5, /* Index of m_interface within usbStringDescriptors.m_stringDescriptorTable */
        .m_endpoints                = {}
    },
    .m_cdcFunctDescr_Header = {
        .m_cdcFncDescr = {
            .m_bFunctionLength      = sizeof(decltype(usbConfigurationDescriptor.m_cdcFunctDescr_Header)),
            .m_bDescriptorType      = ::usb::UsbCdcFunctionalDescriptorType_e::e_UsbDec_DescrType_Interface,
            .m_bDescriptorSubtype   = ::usb::UsbCdcFunctionalDescriptorSubtype_e::e_UsbDec_DescrSubtype_Header
        },
        .m_bcdCDC = {
            .m_loByte               = 0x10,
            .m_hiByte               = 0x01
        }
    },
    .m_cdcFunctDescr_CallMgmt = {
        .m_cdcFncDescr = {
            .m_bFunctionLength      = sizeof(decltype(usbConfigurationDescriptor.m_cdcFunctDescr_CallMgmt)),
            .m_bDescriptorType      = ::usb::UsbCdcFunctionalDescriptorType_e::e_UsbDec_DescrType_Interface,
            .m_bDescriptorSubtype   = ::usb::UsbCdcFunctionalDescriptorSubtype_e::e_UsbDec_DescrSubtype_CallMgmt
        },
        .m_bmCapabilities           = 0,
        .m_bDataInterface           = 1 /* usbConfigurationDescriptor.m_dataInterface.m_bInterfaceNumber */
    },
    .m_cdcFunctDescr_ACM = {
        .m_cdcFncDescr = {
            .m_bFunctionLength      = sizeof(decltype(usbConfigurationDescriptor.m_cdcFunctDescr_ACM)),
            .m_bDescriptorType      = ::usb::UsbCdcFunctionalDescriptorType_e::e_UsbDec_DescrType_Interface,
            .m_bDescriptorSubtype   = ::usb::UsbCdcFunctionalDescriptorSubtype_e::e_UsbDec_DescrSubtype_AbstractControlMgmt
        },
        .m_bmCapabilities           = 0x2
    },
    .m_cdcFunctDescr_Union = {
        .m_cdcFncDescr = {
            .m_bFunctionLength      = sizeof(decltype(usbConfigurationDescriptor.m_cdcFunctDescr_Union)),
            .m_bDescriptorType      = ::usb::UsbCdcFunctionalDescriptorType_e::e_UsbDec_DescrType_Interface,
            .m_bDescriptorSubtype   = ::usb::UsbCdcFunctionalDescriptorSubtype_e::e_UsbDec_DescrSubtype_Union
        },
        .m_bControlInterface        = 0 /* usbConfigurationDescriptor.m_cdcInterface.m_bInterfaceNumber */,
        .m_subordinateInterface     = {
            1 /* usbConfigurationDescriptor.m_dataInterface.m_bInterfaceNumber */
        }
    },
    .m_notificationEndpoint = {
        .m_bLength                  = sizeof(decltype(usbConfigurationDescriptor.m_notificationEndpoint)),
        .m_bDescriptorType          = ::usb::UsbDescriptorTypeId_e::e_Endpoint,
        .m_bEndpointAddress         = 0x82,
        .m_bmAttributes             = 3,    // IRQ
        .m_wMaxPacketSize = {
            .m_loByte               = 8,
            .m_hiByte               = 0
        },
        .m_bInterval                = 0xFF
    },
    .m_dataInterface = {
        .m_bLength                  = sizeof(decltype(usbConfigurationDescriptor.m_dataInterface)) - sizeof(usbConfigurationDescriptor.m_dataInterface.m_endpoints),
        .m_bDescriptorType          = ::usb::UsbDescriptorTypeId_e::e_Interface,
        .m_bInterfaceNumber         = 1,
        .m_bAlternateSetting        = 0,
        .m_bNumEndpoints            = 2,
        .m_bInterfaceClass          = ::usb::UsbInterfaceClass_e::e_UsbInterface_CdcData,
        .m_bInterfaceSubClass       = 0,
        .m_bInterfaceProtocol       = 0,
        .m_iInterface               = 5, /* Index of m_interface within usbStringDescriptors.m_stringDescriptorTable */
        .m_endpoints                = {
            /* Index #0 */ {
                .m_bLength          = sizeof(decltype(usbConfigurationDescriptor.m_dataInterface.m_endpoints[0])),
                .m_bDescriptorType  = ::usb::UsbDescriptorTypeId_e::e_Endpoint,
                .m_bEndpointAddress = 0x01,
                .m_bmAttributes     = 2,    // Bulk
                .m_wMaxPacketSize = {
                    .m_loByte       = 64,
                    .m_hiByte       = 0
                },
                .m_bInterval        = 0
            },
            /* Index #1 */ {
                .m_bLength          = sizeof(decltype(usbConfigurationDescriptor.m_dataInterface.m_endpoints[1])),
                .m_bDescriptorType  = ::usb::UsbDescriptorTypeId_e::e_Endpoint,
                .m_bEndpointAddress = 0x81,
                .m_bmAttributes     = 2,    // Bulk
                .m_wMaxPacketSize = {
                    .m_loByte       = 64,
                    .m_hiByte       = 0
                },
                .m_bInterval        = 0
            }
        }
    }
};

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
