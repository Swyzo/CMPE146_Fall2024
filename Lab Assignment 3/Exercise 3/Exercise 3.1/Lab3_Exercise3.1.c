/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

/* Definitions */
#define CRC32_SEED 0xFFFFFFFF
#define SIZE 1024

/* Statics */
static volatile uint32_t crcSignature;
static volatile int dma_done;

static volatile uint32_t dmaCRCStartTime, dmaCRCEndTime, dmaCRCTime;

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(controlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (SIZE)))
#elif defined(__CC_ARM)
__align(SIZE)
#endif
uint8_t controlTable[SIZE];

/* Data Array */
uint8_t data_array[SIZE];

/* Function prototypes */
void calculate_crc_hardware(uint32_t *checksum);

int main(void)
{
    /* Halting Watchdog */
    MAP_WDT_A_holdTimer();

    /* Initialize CRC Seed */
    MAP_CRC32_setSeed(CRC32_SEED, CRC32_MODE); // Set seed before hardware calculation
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);

    /* Variable for storing timing */
    uint32_t hwCalculatedCRC, hardwareCRCStartTime, hardwareCRCEndTime, hardwareCRCTime, mclkFrequency;
    mclkFrequency = MAP_CS_getMCLK();

    // Set CRC seed again before hardware calculation
    MAP_CRC32_setSeed(CRC32_SEED, CRC32_MODE);

    MAP_Timer32_startTimer(TIMER32_0_BASE, 0);
    int ii;
    hardwareCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE); // Start timer hardwareCRC
    for (ii = 0; ii < SIZE; ++ii)
    {
        MAP_CRC32_set8BitData(data_array[ii], CRC32_MODE); // Iterate through each byte of myData
    }

    /* Getting the result from the hardware module */
    hwCalculatedCRC = MAP_CRC32_getResult(CRC32_MODE); // Get reversed hardwareCRC
    hardwareCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE); // End timer hardwareCRC
    hardwareCRCTime = (hardwareCRCEndTime - hardwareCRCStartTime) * 1000 / mclkFrequency; // Calculate time
    printf("\nHardware CRC: 0x%08X, Time: %lu us\n", hwCalculatedCRC, hardwareCRCTime);

    /* Configure and start DMA for checksum calculation */
    dma_done = 0; // Reset DMA done flag

    /* Configuring DMA module */
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(controlTable);

    /* Setting Control Indexes for DMA transfer */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT, UDMA_SIZE_8 | UDMA_SRC_INC_8 |
    UDMA_DST_INC_NONE | UDMA_ARB_1024);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT, UDMA_MODE_AUTO, data_array, (void*)
    (&CRC32->DI32), SIZE);
    MAP_DMA_assignInterrupt(DMA_INT1, 0);

    /* Assigning/Enabling Interrupts */
    //MAP_DMA_assignInterrupt(DMA_INT1, 0);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_Interrupt_enableMaster();

    /* Enabling DMA Channel 0 */
    MAP_DMA_enableChannel(0);

    /* Set CRC seed before DMA calculation */
    MAP_CRC32_setSeed(CRC32_SEED, CRC32_MODE);

    dmaCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);

    /* Forcing a software transfer on DMA Channel 0 */
    MAP_DMA_requestSoftwareTransfer(0);

    /* Wait for DMA transfer to complete */
    while (!dma_done);

    dmaCRCTime = (dmaCRCEndTime - dmaCRCStartTime) * 1000 / mclkFrequency; // Calculate time
    uint32_t speedUp = 0;
    printf("DMA CRC: 0x%08X, Time: %lu us\n", crcSignature, dmaCRCTime);
    if (dmaCRCTime > hardwareCRCTime) {
        speedUp = dmaCRCTime / hardwareCRCTime;
        printf("Speedup (DMA Faster): %lu us\n", speedUp);

    }
    else {
        speedUp = hardwareCRCTime / dmaCRCTime;
        printf("Speedup (Hardware Faster): %lu us\n", speedUp);
    }

    //Main control loop; does nothing
    while (1) {}
}


/* Completion interrupt for DMA */
void DMA_INT1_IRQHandler(void)
{
    /* Disable the DMA channel and read the CRC result */
    MAP_DMA_disableChannel(0);
    crcSignature = MAP_CRC32_getResult(CRC32_MODE);
    dma_done = 1; // Set flag to indicate DMA is done
    dmaCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);
}
