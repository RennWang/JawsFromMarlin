#include "Jaws_main.h"
#include "usbd_desc.h"
#include "usbd_cdc_interface.h"

#include "Usb.h"

UsbSerial usbSerial;

UsbSerial::UsbSerial()
{
    USBSerialFlush();
}

void UsbSerial::init(void)
{
    USBSerialInit();
}

int UsbSerial::peek(void)
{
    return USBSerialPeek();
}

int UsbSerial::read(void)
{
    return USBSerialRead();
}

void UsbSerial::flush(void)
{
    USBSerialFlush();
}

void UsbSerial::write(const uint8_t *buffer, uint16_t bufsize)
{
    USBSerialWrite(buffer, bufsize);
}

void UsbSerial::writeBuffer(const uint8_t *buffer, uint16_t bufsize)
{
    USBSerialWriteBuffer(buffer, bufsize);
}

void UsbSerial::transmit(void)
{
    USBSerialTransmit();
}

int UsbSerial::available(void)
{
    return USBSerialAvailable();
}

/**
  * @brief  This function handles USB Handler.
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd);
}
