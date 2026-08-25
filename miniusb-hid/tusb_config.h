#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// BOARD SPECIFIC CONFIGURATION
//--------------------------------------------------------------------+

// Chỉ định chip MCU là RP2040 (USB IP block trên RP2350 giống hệt RP2040)
#define CFG_TUSB_MCU             OPT_MCU_RP2040

// Kích hoạt chế độ USB Device
#define CFG_TUD_ENABLED          1

// Định nghĩa cổng USB Rhport 0 hoạt động ở chế độ Device
#define CFG_TUSB_RHPORT0_MODE    OPT_MODE_DEVICE

//--------------------------------------------------------------------+
// DEVICE CONFIGURATION
//--------------------------------------------------------------------+

#define CFG_TUD_ENDPOINT0_SIZE   64

// Chỉ kích hoạt chức năng HID (Keyboard)
#define CFG_TUD_HID              1
#define CFG_TUD_CDC              0
#define CFG_TUD_MSC              0
#define CFG_TUD_MIDI             0
#define CFG_TUD_VENDOR           0

// Cấu hình kích thước bộ đệm HID Endpoint
#define CFG_TUD_HID_EP_BUFSIZE   16

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
