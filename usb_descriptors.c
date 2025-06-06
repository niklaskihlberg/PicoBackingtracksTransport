#include "tusb.h"

#ifndef USBD_VID
#define USBD_VID (0x2E8A)  // Raspberry Pi
#endif

#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USBD_PID                                                     \
  (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
   _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))
#ifndef USBD_MANUFACTURER
#define USBD_MANUFACTURER "Raspberry Pi"
#endif

#ifndef USBD_PRODUCT
#define USBD_PRODUCT "Pico"
#endif

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN + TUD_CDC_DESC_LEN)
#define TUD_RPI_RESET_DESC_LEN 9
#if !PICO_STDIO_USB_DEVICE_SELF_POWERED
#define USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE (0)
#define USBD_MAX_POWER_MA (250)
#else
#define USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE TUSB_DESC_CONFIG_ATT_SELF_POWERED
#define USBD_MAX_POWER_MA (1)
#endif

#define USBD_ITF_MIDI (0)  // needs 2 interfaces
#define USBD_ITF_CDC (2)   // needs 2 interfaces
#define USBD_ITF_MAX (4)

#define USBD_CDC_EP_CMD (0x81)

#define USBD_CDC_CMD_MAX_SIZE (8)
#define USBD_CDC_IN_OUT_MAX_SIZE (64)

#define USBD_CDC_EP_OUT (0x03)
#define USBD_CDC_EP_IN (0x83)

#define USBD_MIDI_EP_OUT (0x02)
#define USBD_MIDI_EP_IN (0x82)

#define USBD_MIDI_IN_OUT_MAX_SIZE (64)

#define USBD_STR_0 (0x00)
#define USBD_STR_MANUF (0x01)
#define USBD_STR_PRODUCT (0x02)
#define USBD_STR_SERIAL (0x03)
#define USBD_STR_RPI_RESET (0x06)

tusb_desc_device_t const usbd_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xCafe,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};
static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
  TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA),  
  TUD_MIDI_DESCRIPTOR(USBD_ITF_MIDI, USBD_STR_0, USBD_MIDI_EP_OUT, USBD_MIDI_EP_IN, USBD_MIDI_IN_OUT_MAX_SIZE),
  TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_0, USBD_CDC_EP_CMD, USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),
};

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

char const *usbd_string_desc[] = {
  (const char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
  "Niklas Kihlberg",              // 1: Manufacturer
  "Niklas BTC",          // 2: Product
  "123456",                    // 3: Serials, should use chip ID
};

static uint16_t usbd_string_desc_len[32];

const uint8_t *tud_descriptor_device_cb(void) {
  return (const uint8_t *)&usbd_desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;  // for multiple configurations

  return usbd_desc_cfg;
}

#define USBDusbd_string_desc_len_MAX 20

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  uint8_t chr_count;

  if (index == 0) {
    memcpy(&usbd_string_desc_len[1], usbd_string_desc[0], 2);
    chr_count = 1;
  } else {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(usbd_string_desc) / sizeof(usbd_string_desc[0])))
      return NULL;

    const char *str = usbd_string_desc[index];

    // Cap at max char
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
      usbd_string_desc_len[1 + i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  usbd_string_desc_len[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

  return usbd_string_desc_len;
}





// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// // #include "tusb_config.h"

// #include "bsp/board_api.h"
// #include "tusb.h"


// // #include "class/cdc/cdc_device.h" 
// // #include "class/midi/midi_device.h"

// #define USB_VID 0xCafe

// #define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
// // #define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) |  _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )
// #define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

// #define USB_BCD 0x0100

// /* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
//  * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
//  *
//  * Auto ProductID layout's Bitmap:
//  *   [MSB]         HID | MSC | CDC          [LSB]
//  */
// // #define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
// // #define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

// //--------------------------------------------------------------------+
// // Device Descriptors
// //--------------------------------------------------------------------+
// tusb_desc_device_t const desc_device = {
//   .bLength            = sizeof(tusb_desc_device_t),
//   .bDescriptorType    = TUSB_DESC_DEVICE,
//   .bcdUSB             = 0x0200,
//   .bDeviceClass       = 0x00,
//   .bDeviceSubClass    = 0x00,
//   .bDeviceProtocol    = 0x00,
//   .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

//   .idVendor           = USB_VID,
//   .idProduct          = USB_PID,
//   .bcdDevice          = USB_BCD,

//   .iManufacturer      = 0x01,
//   .iProduct           = 0x02,
//   .iSerialNumber      = 0x03,

//   .bNumConfigurations = 0x01
// };

// // Invoked when received GET DEVICE DESCRIPTOR
// // Application return pointer to descriptor
// uint8_t const * tud_descriptor_device_cb(void) {
//   return (uint8_t const *) &desc_device;
// }


// //--------------------------------------------------------------------+
// // Configuration Descriptor
// //--------------------------------------------------------------------+
// #define ITF_NUM_CDC         0
// #define ITF_NUM_CDC_DATA    1
// #define ITF_NUM_MIDI        2
// #define ITF_NUM_MIDI_STREAM 3
// #define ITF_NUM_TOTAL       4

// #define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)





// #if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
//   // LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
//   // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
//   #define EPNUM_MIDI_OUT  0x02
//   #define EPNUM_MIDI_IN   0x82

// #elif CFG_TUSB_MCU == OPT_MCU_CXD56
//   // CXD56 USB driver has fixed endpoint type (bulk/interrupt/iso) and direction (IN/OUT) by its number
//   // 0 control (IN/OUT), 1 Bulk (IN), 2 Bulk (OUT), 3 In (IN), 4 Bulk (IN), 5 Bulk (OUT), 6 In (IN)
//   #define EPNUM_MIDI_OUT  0x02
//   #define EPNUM_MIDI_IN   0x81

// #elif defined(TUD_ENDPOINT_ONE_DIRECTION_ONLY)
//   // MCUs that don't support a same endpoint number with different direction IN and OUT defined in tusb_mcu.h
//   //    e.g EP1 OUT & EP1 IN cannot exist together
//   #define EPNUM_MIDI_OUT  0x01
//   #define EPNUM_MIDI_IN   0x82

// #else
//   #define EPNUM_MIDI_OUT  0x01
//   #define EPNUM_MIDI_IN   0x81
// #endif

// uint8_t const desc_fs_configuration[] = {
//   // Config number, interface count, string index, total length, attribute, power in mA
//   TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

//   // Interface number, string index, EP Out & EP In address, EP size
//   TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), 64)
// };

// #if TUD_OPT_HIGH_SPEED
// uint8_t const desc_hs_configuration[] = {
//   // Config number, interface count, string index, total length, attribute, power in mA
//   TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

//   // Interface number, string index, EP Out & EP In address, EP size
//   TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), 512)
// };
// #endif

















// uint8_t const desc_configuration[] = {
  
//   // Config number, interface count, string index, total length, attribute, power in mA
//   TUD_CONFIG_DESCRIPTOR(1, 4, 0, CONFIG_TOTAL_LEN, 0x80, 100), // 4 interfaces: CDC (2) + MIDI (2)

//   // CDC: Interface number, string index, EP notification, EP data out, EP data in, EP size
//   TUD_CDC_DESCRIPTOR(0, 0, 0x81, 8, 0x02, 0x82, 64),           // CDC: itf 0/1

//   // MIDI: Interface number, string index, EP Out & In address, EP size
//   TUD_MIDI_DESCRIPTOR(2, 0, 0x03, 0x83, 64)                    // MIDI: itf 2

// };


// // // Invoked when received GET CONFIGURATION DESCRIPTOR
// // // Application return pointer to descriptor
// // // Descriptor contents must exist long enough for transfer to complete
// // uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
// //   (void) index; // for multiple configurations

// // #if TUD_OPT_HIGH_SPEED
// //   // Although we are highspeed, host may be fullspeed.
// //   return (tud_speed_get() == TUSB_SPEED_HIGH) ?  desc_hs_configuration : desc_fs_configuration;
// // #else
// //   return desc_fs_configuration;
// // #endif
// // }








// // Invoked when received GET CONFIGURATION DESCRIPTOR
// // Application return pointer to descriptor
// // Descriptor contents must exist long enough for transfer to complete
// const uint8_t* tud_descriptor_configuration_cb(uint8_t index) {
//   (void)index;
//   return desc_configuration;
// }

// static const char* string_desc_arr[] = {
//   (const char[]){0x09, 0x04},
//   "Niklas Kihlberg", // Manufacturer
//   "Niklas BT Control", // Product
//   "123456", // Serial
// };

// static uint16_t _desc_str[32];

// const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
//     (void)langid;
//     uint8_t chr_count = 0;
//     if (index == 0) {
//         _desc_str[1] = (string_desc_arr[0][1] << 8) | string_desc_arr[0][0];
//         chr_count = 1;
//     } else {
//         const char* str = string_desc_arr[index];
//         chr_count = strlen(str);
//         for (uint8_t i = 0; i < chr_count; i++) {
//             _desc_str[1 + i] = str[i];
//         }
//     }
//     _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
//     return _desc_str;
// }
// 
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████

