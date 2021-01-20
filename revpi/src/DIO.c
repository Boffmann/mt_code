#include "DIO.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int g_PiControlHandle = -1;

int piControlOpen(void) {
    if (g_PiControlHandle < 0) {
        g_PiControlHandle = open(PICONTROL_DEVICE, O_WRONLY);
    }

    return g_PiControlHandle;
}

int piControlGetVariableInfo(SPIVariable* spiVariable) {
    int control_handle = piControlOpen();

    if (control_handle < 0) {
        return -ENODEV;
    }

    if (ioctl(control_handle, KB_FIND_VARIABLE, spiVariable) < 0) {
        return -errno;
    }

    return 0;
}

int piControlSetBitValue(SPIValue * pSpiValue) {
    
    int control_handle = piControlOpen();

    if (control_handle < 0)
            return -ENODEV;

    pSpiValue->i16uAddress += pSpiValue->i8uBit / 8;
    pSpiValue->i8uBit %= 8;

    if (ioctl(control_handle, KB_SET_VALUE, pSpiValue) < 0)
            return -errno;

    return 0;
}

char *getWriteError(int error) {
    static char *WriteError[] = {
            "Cannot connect to control process",
            "Offset seek error",
            "Cannot write to control process",
            "Unknown error"
    };
    switch (error) {
    case -1:
            return WriteError[0];
            break;
    case -2:
            return WriteError[1];
            break;
    case -3:
            return WriteError[2];
            break;
    default:
            return WriteError[3];
            break;
    }
}

int piControlWrite(uint32_t Offset, uint32_t Length, uint8_t * pData)
{
    int BytesWritten = 0;

    int control_handle = piControlOpen();

    if (control_handle < 0)
            return -ENODEV;

    /* seek */
    if (lseek(control_handle, Offset, SEEK_SET) < 0) {
            return -errno;
    }

    /* Write */
    BytesWritten = write(control_handle, pData, Length);
    if (BytesWritten < 0) {
            return -errno;
    }

    return BytesWritten;
}

void digital_write(char* port_name, const bool value) {
    int rc;
    SPIVariable sPiVariable;
    SPIValue sPIValue;
    uint8_t i8uValue;
    uint16_t i16uValue;

    strncpy(sPiVariable.strVarName, port_name, sizeof(sPiVariable.strVarName));

    rc = piControlGetVariableInfo(&sPiVariable);
    if (rc < 0) {
            printf("Cannot find variable '%s'\n", port_name);
            return;
    }

    if (sPiVariable.i16uLength == 1) {
            sPIValue.i16uAddress = sPiVariable.i16uAddress;
            sPIValue.i8uBit = sPiVariable.i8uBit;
            sPIValue.i8uValue = value;
            rc = piControlSetBitValue(&sPIValue);
            if (rc < 0)
                    printf("Set bit error %s\n", getWriteError(rc));
            else
                    printf("Set bit %d on byte at offset %d. Value %d\n", sPIValue.i8uBit, sPIValue.i16uAddress,
                        sPIValue.i8uValue);
    } else if (sPiVariable.i16uLength == 8) {
            i8uValue = value;
            rc = piControlWrite(sPiVariable.i16uAddress, 1, (uint8_t *) & i8uValue);
            if (rc < 0)
                    printf("Write error %s\n", getWriteError(rc));
            else
                    printf("Write value %d dez (=%02x hex) to offset %d.\n", i8uValue, i8uValue,
                        sPiVariable.i16uAddress);
    } else if (sPiVariable.i16uLength == 16) {
            i16uValue = value;
            rc = piControlWrite(sPiVariable.i16uAddress, 2, (uint8_t *) & i16uValue);
            if (rc < 0)
                    printf("Write error %s\n", getWriteError(rc));
            else
                    printf("Write value %d dez (=%04x hex) to offset %d.\n", i16uValue, i16uValue,
                        sPiVariable.i16uAddress);
    } else if (sPiVariable.i16uLength == 32) {
            rc = piControlWrite(sPiVariable.i16uAddress, 4, (uint8_t *) & value);
            if (rc < 0)
                    printf("Write error %s\n", getWriteError(rc));
            else
                    printf("Write value %d dez (=%08x hex) to offset %d.\n", value, value,
                        sPiVariable.i16uAddress);
    }
}