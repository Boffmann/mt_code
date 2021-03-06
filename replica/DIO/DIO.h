#ifndef __REVPI_DIO_H__
#define __REVPI_DIO_H__

#include <stdint.h>
#include <stdbool.h>

#define PICONTROL_DEVICE    "/dev/piControl0"
#define KB_IOC_MAGIC 'K'
#define KB_SET_VALUE        _IO(KB_IOC_MAGIC, 16)   ///< set the value of one bit in the process image
#define KB_FIND_VARIABLE    _IO(KB_IOC_MAGIC, 17)   ///< find a variable defined in piCtory

// Mostly taken from demo code on RevPi

typedef struct {
    uint16_t    i16uAddress;     ///< Address of the byte in the process image
    uint8_t     i8uBit;          ///< 0-7 bit position, >= 8 whole byte
    uint8_t     i8uValue;        ///< Value: 0/1 for bit access, whole byte otherwise
} SPIValue;

typedef struct {
    char        strVarName[32]; ///< Variable name
    uint16_t    i16uAddress;    ///< Address of the byte in the process image
    uint8_t     i8uBit;         /// 0-7 bit position, >=8 whole byte
    uint16_t    i16uLength;     ///< length of the variable in bits. Possible valures are 1, 8, 16, 32
} SPIVariable;

void digital_write(char* port_name, const bool value);

#endif