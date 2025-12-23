#include <windows.h>
#include <stdint.h>
#include <stdio.h>

/**
 *
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  start;
    uint8_t  id;
    uint8_t  len;
    uint32_t value;
    uint8_t  crc;
} Frame;
#pragma pack(pop)

/**
 * Calculate CRC
 * @param data
 * @param len
 * @return
 */
uint8_t calc_crc(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t i;

    for (i = 0; i < len; i++)
        crc ^= data[i];

    return crc;
}

/**
 *
 * @return
 */
int main(void)
{
    HANDLE hSerial;
    DCB dcb;
    COMMTIMEOUTS timeouts;

    DWORD bytesRead;
    uint8_t rx;
    Frame frame;

    hSerial = CreateFile(
        "\\\\.\\COM9",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Blad otwarcia portu COM\n");
        return 1;
    }

    ZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    GetCommState(hSerial, &dcb);

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    SetCommState(hSerial, &dcb);

    ZeroMemory(&timeouts, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    printf("Nasluchiwanie...\n");

    while (1)
    {
        if (!ReadFile(hSerial, &rx, 1, &bytesRead, NULL) || bytesRead == 0)
            continue;

        if (rx != 0xAA)
            continue;

        frame.start = rx;

        if (!ReadFile(hSerial,
                      ((uint8_t*)&frame) + 1,
                      sizeof(Frame) - 1,
                      &bytesRead,
                      NULL))
            continue;

        if (bytesRead != sizeof(Frame) - 1)
            continue;

        if (calc_crc((uint8_t*)&frame, sizeof(Frame) - 1) != frame.crc)
        {
            printf("CRC ERROR\n");
            continue;
        }

        printf("ID: %u | ADC: %lu\n",
               frame.id,
               (unsigned long)frame.value);
    }

    CloseHandle(hSerial);
    return 0;
}