#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdlib.h>
#include <stdio.h>

#define CRC32_POLY              0xEDB88320
#define CRC32_INIT              0xFFFFFFFF

static uint32_t compute_simple_checksum(uint8_t *data, uint32_t length);

//![Simple CRC32 Example] 
int main(void)
{
    //32-bit variables to hold calculated CRC hex values
    uint32_t hwCalculatedCRCNormal, simpleCRCNormal;
    uint32_t hwCalculatedCRC1BitCorrupted, simpleCRC1BitCorrupted;
    uint32_t hwCalculatedCRC2BitCorrupted, simpleCRC2BitCorrupted;

    static uint8_t myData[10240] = { };

    //Iterator variable for hardware CRC function
    uint32_t ii;

    /* Stop WDT */
    MAP_WDT_A_holdTimer();

// Part 1: Normal CRC Calculation
    //Initialize CRC32 seed
    MAP_CRC32_setSeed(CRC32_INIT, CRC32_MODE);

    /* Calculating CRC32 checksum via simple_checksum */
    simpleCRCNormal = compute_simple_checksum(myData, 10240);

    for (ii = 0; ii < 10240; ii++) {
        MAP_CRC32_set8BitData(myData[ii], CRC32_MODE);                                      //Iterate through each byte of myData
    }
    /* Getting the result from the hardware module */
    hwCalculatedCRCNormal = MAP_CRC32_getResultReversed(CRC32_MODE) ^ 0xFFFFFFFF;           //Get reversed hardwareCRC and reverse the bits

//Part 2: Reverse Bit 7 of myData[8000]
    printf("\nCalculating checksums after corrupting myData[8000] (bit 7)...\n");
    myData[8000] ^= (1 << 7);  // Flip bit 7

    MAP_CRC32_setSeed(CRC32_INIT, CRC32_MODE);

    simpleCRC1BitCorrupted = compute_simple_checksum(myData, 10240);

    for (ii = 0; ii < 10240; ii++) {
        MAP_CRC32_set8BitData(myData[ii], CRC32_MODE);                                      //Iterate through each byte of myData
    }
    /* Getting the result from the hardware module */
    hwCalculatedCRC1BitCorrupted = MAP_CRC32_getResultReversed(CRC32_MODE) ^ 0xFFFFFFFF;    //Get reversed hardwareCRC and reverse the bits

//Part 3: Reverse Bit 7 of myData[9016]
    printf("Calculating checksums after corrupting myData[9016] (bit 7)...\n\n");
    myData[9016] ^= (1 << 7);  // Flip bit 7

    MAP_CRC32_setSeed(CRC32_INIT, CRC32_MODE);

    simpleCRC2BitCorrupted = compute_simple_checksum(myData, 10240);

    for (ii = 0; ii < 10240; ii++) {
        MAP_CRC32_set8BitData(myData[ii], CRC32_MODE);                                      //Iterate through each byte of myData
    }
    /* Getting the result from the hardware module */
    hwCalculatedCRC2BitCorrupted = MAP_CRC32_getResultReversed(CRC32_MODE) ^ 0xFFFFFFFF;    //Get reversed hardwareCRC and reverse the bits

//Part 4: Printing Everything
    /* Print out normal hardware CRC value */
    printf("Hardware CRC (Normal): 0x%08X\n", hwCalculatedCRCNormal);
    /* Print out 1 corrupted bit hardware CRC value */
    printf("Hardware CRC (1 Corrupted Bit): 0x%08X\n", hwCalculatedCRC1BitCorrupted);
    /* Print out 2 bit corrupted hardware CRC values */
    printf("Hardware CRC (2 Corrupted Bits): 0x%08X\n\n", hwCalculatedCRC2BitCorrupted);

    /* Print out normal simple CRC value */
    printf("Simple CRC (Normal): 0x%08X\n", simpleCRCNormal);
    /* Print out 1 corrupted bit simple CRC value */
    printf("Simple CRC (1 Corrupted Bit): 0x%08X\n", simpleCRC1BitCorrupted);
    /* Print out 2 corrupted bit simple CRC value */
    printf("Simple CRC (2 Corrupted Bits): 0x%08X\n\n", simpleCRC2BitCorrupted);

    /* For Debugging Purposes */
    printf("Now Ending\n");

    /* Pause for the debugger */
    __no_operation();
}
//![Simple CRC32 Example] 

// Compute simple checksum function
uint32_t compute_simple_checksum(uint8_t *myData, uint32_t length)
{
    uint32_t checksumCalc = 0;
    uint32_t i = 0;

    // Iterate through each byte of the data
    for (; i < length; i++)
    {
        // Determine the byte position (0 to 3) and shift the byte into place
        uint32_t shiftByte = (i % 4) * 8;
        checksumCalc += (myData[i] << shiftByte);
    }

    // Reverse all bits of the checksum
    checksumCalc = ~checksumCalc;

    return checksumCalc;
}
