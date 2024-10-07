/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define SIZE 10240

// Compute simple checksum function
uint32_t compute_simple_checksum(uint8_t* data, uint32_t length);

int main(void) {
    static uint8_t myData[SIZE];

    // Call the checksum function with size 10240
    uint32_t checksum = compute_simple_checksum(myData, SIZE);

    // Print out some data values to verify correct data is loaded
    printf("First 4 bytes: 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", myData[0], myData[1], myData[2], myData[3]);
    printf("Last 4 bytes: 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", myData[10236], myData[10237], myData[10238], myData[10239]);

    // Print the checksum in 8-digit hexadecimal format
    printf("Checksum: 0x%08X\n", checksum);

    return 0;
}

// Compute simple checksum function
uint32_t compute_simple_checksum(uint8_t* myData, uint32_t length) {
    uint32_t checksumCalc = 0;
    uint32_t i = 0;

    // Iterate through each byte of the data
    for (; i < length; i++) {
        // Determine the byte position (0 to 3) and shift the byte into place
        uint32_t shiftByte = (i % 4) * 8;
        checksumCalc += (myData[i] << shiftByte);
    }

    // Reverse all bits of the checksum
    checksumCalc = ~checksumCalc;

    return checksumCalc;
}
