#ifndef __SERIALINTERFACE_H_INCLUDED
#define __SERIALINTERFACE_H_INCLUDED

#include "Jaws_main.h"

#define RX_BUFFER_SIZE          256
#define TX_BUFFER_SIZE          512

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYT 0

typedef struct
{
    unsigned char buffer[RX_BUFFER_SIZE];
    int head;
    int tail;
}RingBufferStruct;

typedef struct
{
    unsigned char buffer[TX_BUFFER_SIZE];
    int datalen;
}LineBufferStruct;

class SerialInterface
{
public:
    SerialInterface();

private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);

public:
    virtual void init(void) = 0;
    virtual int peek(void) = 0;
    virtual int read(void) = 0;
    virtual void flush(void) = 0;
    virtual void write(const uint8_t *buffer, uint16_t bufsize) = 0;
    virtual void writeBuffer(const uint8_t *buffer, uint16_t bufsize) = 0;
    virtual void transmit(void) = 0;

    virtual void write(uint8_t c);
    virtual void write(const char *str);

    virtual void writeBuffer(uint8_t c);
    virtual void writeBuffer(const char *str);

    virtual void print(const char *str);
    virtual void print(char, int = BYT);
    virtual void print(unsigned char, int = BYT);
    virtual void print(int, int = DEC);
    virtual void print(unsigned int, int = DEC);
    virtual void print(long, int = DEC);
    virtual void print(unsigned long, int = DEC);
    virtual void print(double, int = 2);
    virtual void print(float, int = 2);
};



#endif /* __SERIALINTERFACE_H_INCLUDED */
