#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdlib.h>
#include <stdio.h>

#define CRC32_POLY              0xEDB88320
#define CRC32_INIT              0xFFFFFFFF

static uint32_t calculateCRC32(uint8_t* data, uint32_t length);
static uint32_t compute_simple_checksum(uint8_t* data, uint32_t length);

//![Simple CRC32 Example] 
int main(void) {
    //32-bit variables to hold calculated CRC hex values
    uint32_t hwCalculatedCRC, swCalculatedCRC, simpleCRC;

    static uint8_t myData[10240] = {};

    //Initialize Clk
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);
    MAP_Timer32_startTimer(TIMER32_0_BASE, 0);
    printf("Master Clock Frequency: %u Hz\n\n", MAP_CS_getMCLK());  //Master Clock Frequency for Reference

    //32-Bit variables for time calculations
    uint32_t hardwareCRCStartTime, hardwareCRCEndTime, hardwareCRCTime, softwareCRCStartTime, softwareCRCEndTime, softwareCRCTime, simpleCRCStartTime, simpleCRCEndTime, simpleCRCTime;

    //Iterator variable for hardware CRC function
    uint32_t ii;

    /* Stop WDT */
    MAP_WDT_A_holdTimer();

    //Initialize CRC32 seed
    MAP_CRC32_setSeed(CRC32_INIT, CRC32_MODE);

    /* Calculating CRC32 checksum via simple_checksum */
    simpleCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);                      //Start timer simpleCRC
    simpleCRC = compute_simple_checksum(myData, 10240);
    // Print out some data values to verify correct data is loaded
    printf("First 4 bytes from myData (simpleCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", myData[0], myData[1], myData[2], myData[3]);
    printf("Last 4 bytes from myData (simpleCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n", myData[10236], myData[10237], myData[10238], myData[10239]);
    simpleCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);                        //End timer simpleCRC
    simpleCRCTime = (simpleCRCEndTime - simpleCRCStartTime) * 1000 / 3000000;       //Calculate time elapsed to microseconds


    hardwareCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);                    //Start timer hardwareCRC
    for (ii = 0; ii < 10240; ii++) {
        MAP_CRC32_set8BitData(myData[ii], CRC32_MODE);                              //Iterate through each byte of myData
    }
    // Print out some data values to verify correct data is loaded
    printf("First 4 bytes from myData (hardwareCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", myData[0], myData[1], myData[2], myData[3]);
    printf("Last 4 bytes from myData (hardwareCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n", myData[10236], myData[10237], myData[10238], myData[10239]);
    /* Getting the result from the hardware module */
    hwCalculatedCRC = MAP_CRC32_getResultReversed(CRC32_MODE) ^ 0xFFFFFFFF;         //Get reversed hardwareCRC and reverse the bits
    hardwareCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);                      //End timer hardwareCRC
    hardwareCRCTime = (hardwareCRCEndTime - hardwareCRCStartTime) * 1000 / 3000000; //Calculate time elapsed to microseconds

    /* Calculating the CRC32 checksum through software */
    softwareCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);                    //Start timer softwareCRC
    swCalculatedCRC = calculateCRC32((uint8_t*) myData, 10240);                     //Calculate CRC
    // Print out some data values to verify correct data is loaded
    printf("First 4 bytes from myData (softwareCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", myData[0], myData[1], myData[2], myData[3]);
    printf("Last 4 bytes from myData (softwareCRC): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n", myData[10236], myData[10237], myData[10238], myData[10239]);
    softwareCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);                      //End timer softwareCRC
    softwareCRCTime = (softwareCRCEndTime - softwareCRCStartTime) * 1000 / 3000000; //Calculate time elapsed to microseconds

    /* Print out hardware CRC value and time elapsed in microseconds */
    printf("Hardware CRC: 0x%08X\n", hwCalculatedCRC);
    printf("Hardware CRC Time: %u us\n\n", hardwareCRCTime);

    /* Print out software CRC value and time elapsed in microseconds */
    printf("Software CRC: 0x%08X\n", swCalculatedCRC);
    printf("Software CRC Time: %u us\n\n", softwareCRCTime);

    /* Print out simple CRC value and time elapsed in microseconds */
    printf("Simple CRC: 0x%08X\n", simpleCRC);
    printf("Simple CRC Time: %u us\n\n", simpleCRCTime);

    printf("Now Ending\n");


    /* Pause for the debugger */
    __no_operation();
}
//![Simple CRC32 Example] 

/* Standard software calculation of CRC32 */
static uint32_t calculateCRC32(uint8_t* data, uint32_t length)
{
    uint32_t ii, jj, byte, crc, mask;;

    crc = 0xFFFFFFFF;

    for(ii=0;ii<length;ii++)
    {
        byte = data[ii];
        crc = crc ^ byte;

        for (jj = 0; jj < 8; jj++)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLY & mask);
        }

    }

    return ~crc;
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
