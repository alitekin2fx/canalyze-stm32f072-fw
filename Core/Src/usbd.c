#include "usbd.h"
#include "usbd_8dev.h"
#include "usbd_8dev_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

void usb_init() {
    // Init Device Library
    USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);

    // Add Supported Class
    USBD_RegisterClass(&hUsbDeviceFS, &usbd_8dev);

    // Add CDC Interface Class
    usbd_8dev_registerinterface(&hUsbDeviceFS, &usbd_8dev_fops);

    // Start Device Process
    USBD_Start(&hUsbDeviceFS);
}
