#include "SerialInterface.h"

SerialInterface::SerialInterface()
{

}

void SerialInterface::write(uint8_t c)
{
    write(&c, 1);
}

void SerialInterface::write(const char *str)
{
    uint16_t len = strlen(str);
    if (len > 0)
        write((const uint8_t *)str, len);
}

void SerialInterface::writeBuffer(uint8_t c)
{
    writeBuffer(&c, 1);
}

void SerialInterface::writeBuffer(const char *str)
{
    uint16_t len = strlen(str);
    if (len > 0)
        writeBuffer((const uint8_t *)str, len);
}

void SerialInterface::print(const char *str)
{
    writeBuffer(str);
}

void SerialInterface::print(char c, int base)
{
    print((long) c, base);
}

void SerialInterface::print(unsigned char b, int base)
{
    print((unsigned long) b, base);
}

void SerialInterface::print(int n, int base)
{
    print((long) n, base);
}

void SerialInterface::print(unsigned int n, int base)
{
    print((unsigned long) n, base);
}

void SerialInterface::print(long n, int base)
{
    if (base == 0) {
        writeBuffer(n);
    }
    else if (base == 10) {
        if (n < 0) {
            print('-');
            n = -n;
        }
        printNumber(n, 10);
    }
    else {
        printNumber(n, base);
    }
}

void SerialInterface::print(unsigned long n, int base)
{
    if (base == 0) writeBuffer(n);
    else printNumber(n, base);
}

void SerialInterface::print(double n, int digits)
{
    printFloat(n, digits);
}

void SerialInterface::print(float n, int digits)
{
    printFloat(n, digits);
}

void SerialInterface::printNumber(unsigned long n, uint8_t base)
{
    unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
    unsigned long i = 0;

    if (n == 0) {
        print('0');
        return;
    }

    while (n > 0) {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
        print((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}

void SerialInterface::printFloat(double number, uint8_t digits)
{
    // Handle negative numbers
    if (number < 0.0){
        print('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i=0; i<digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
        print(".");

    // Extract digits from the remainder one at a time
    while (digits-- > 0){
        remainder *= 10.0;
        int toPrint = int(remainder);
        print(toPrint);
        remainder -= toPrint;
    }
}

