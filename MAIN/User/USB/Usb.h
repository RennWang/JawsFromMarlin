#ifndef __USB_H_INCLUDED
#define __USB_H_INCLUDED

#include "SerialInterface.h"

class UsbSerial:public SerialInterface
{
public:
    UsbSerial();

    void init(void);
    int peek(void);
    int read(void);
    void flush(void);

    void write(const uint8_t *buffer, uint16_t bufsize);
    void writeBuffer(const uint8_t *buffer, uint16_t bufsize);

    void transmit(void);
    int available(void);

};

extern UsbSerial usbSerial;

#endif /* __USB_H_INCLUDED */
