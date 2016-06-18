#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "Jawsdef.h"
#include "Jaws_main.h"
#include "SerialInterface.h"


typedef enum
{
    SERIAL_COMMAND      =   0,
    SERIAL_DEBUG        =   1
}SerialPortType;

class SerialPort
{
private:
    RingBufferStruct rx_buffer;
    LineBufferStruct tx_buffer;

    SerialPortType  serial_port;

private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);

public:
    SerialPort(SerialPortType serial);

    void init(void);
    int peek(void);
    int read(void);
    void flush(void);

    void write(uint8_t c);
    void write(const char *str);
    void write(const uint8_t *buffer, uint16_t bufsize);

    void writeBuffer(uint8_t c);
    void writeBuffer(const char *str);
    void writeBuffer(const uint8_t *buffer, uint16_t bufsize);

    void transmit(void);

    FORCE_INLINE int available(void)
    {
        return (unsigned int)(RX_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RX_BUFFER_SIZE;
    }

    FORCE_INLINE void receive(uint8_t c)
    {
        int i = (unsigned int)(rx_buffer.head + 1) % RX_BUFFER_SIZE;

        if (i != rx_buffer.tail) {
            rx_buffer.buffer[rx_buffer.head] = c;
            rx_buffer.head = i;
        }
    }

    void print(const char *str)
    {
        writeBuffer(str);
    }

    void print(char, int = BYT);
    void print(unsigned char, int = BYT);
    void print(int, int = DEC);
    void print(unsigned int, int = DEC);
    void print(long, int = DEC);
    void print(unsigned long, int = DEC);
    void print(double, int = 2);
    void print(float, int = 2);

public:
    __IO uint8_t is_TxBusy;

};

extern SerialPort commdSerial;
extern SerialPort debugSerial;




#endif /* JAWSRINGBUFFER_H */
