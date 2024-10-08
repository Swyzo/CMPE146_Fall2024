/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Statics */
static volatile uint32_t crcSignature;

/* Definitions */
#define SIZE 1024

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(controlTable, SIZE)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (SIZE)))
#elif defined(__CC_ARM)
__align(SIZE)
#endif
uint8_t controlTable[SIZE];

#define CRC32_SEED 0xFFFFFFFF

uint8_t data_array[SIZE];

// Bool for DMA completion
volatile int dma_done;

// ex 3
int size_array[] = {2, 4, 16, 32, 64, 128, 256, 786, 1024};

int main(void)
{
    /* Halting Watchdog */
    MAP_WDT_A_holdTimer();

    // Timer Setup
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);
    MAP_Timer32_startTimer(TIMER32_0_BASE, 0);
    uint32_t clkFreq = MAP_CS_getMCLK();

    // setup DMA externally from for loop
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(controlTable);

    // Iterator variable for big for-loop
    int ii;

    /* Variables for Hardware Method Calculations */
    uint32_t hwCalculatedCRC, hardwareCRCStartTime, hardwareCRCEndTime, hardwareCRCTime;
    uint32_t dmaCRCStartTime, dmaCRCEndTime, dmaCRCTime;

    for (ii = 0; ii < (sizeof(size_array)/sizeof(size_array[0])); ii++) {

        int blockSize = size_array[ii];
        printf("Block size = %i\n", blockSize);

        // Hardware Method Start
        hardwareCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);

        // Init CRC32 Seed for Hardware Method
        MAP_CRC32_setSeed(CRC32_SEED, CRC32_MODE);


        // Iterator variable for blockSize
        int jj;
        for (jj = 0; jj < blockSize; jj++) {
            MAP_CRC32_set8BitData(data_array[jj], CRC32_MODE);
        }
        hwCalculatedCRC = MAP_CRC32_getResult(CRC32_MODE);

        // Hardware Method Stop
        hardwareCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);
        hardwareCRCTime = (hardwareCRCStartTime - hardwareCRCEndTime) / 3;

        printf("Hardware CRC = %08x\n", hwCalculatedCRC);
        printf("Hardware Method duration = %u us\n", hardwareCRCTime);

        // DMA Method Start
        dmaCRCStartTime = MAP_Timer32_getValue(TIMER32_0_BASE);

        // Before DMA transfer, set dma_done bool flag to 0
        dma_done = 0;

        // DMA Method Setup
        MAP_DMA_setChannelControl(UDMA_PRI_SELECT,
                                  UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1024);
        MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT,
                                   UDMA_MODE_AUTO, data_array,
                                   (void*) (&CRC32->DI32), blockSize);

        MAP_DMA_assignInterrupt(DMA_INT1, 0);
        MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
        MAP_Interrupt_enableMaster();
        MAP_DMA_enableChannel(0);

        // Init CRC32 Seed for DMA Method
        MAP_CRC32_setSeed(CRC32_SEED, CRC32_MODE);

        MAP_DMA_requestSoftwareTransfer(0);

        while(!dma_done) {}

        // DMA Method End
        dmaCRCEndTime = MAP_Timer32_getValue(TIMER32_0_BASE);
        dmaCRCTime = (dmaCRCStartTime - dmaCRCEndTime) / 3;

        // print DMA CRC
        printf("DMA CRC = %08x\n", crcSignature);
        printf("DMA Method Duration = %u us\n", dmaCRCTime);

        /* Verify CRC results are identical */
        if (hwCalculatedCRC != crcSignature) {
            printf("ERROR: CRC value mismatch!\n");
        }
        else {
            printf("CRC value match\n");
        }


        // Speedup Calc
        uint32_t speedupDMA = hardwareCRCTime / dmaCRCTime;
        uint32_t speedupHW = dmaCRCTime / hardwareCRCTime;

        if (speedupDMA > 0 && hardwareCRCTime > dmaCRCTime) {
            printf("Speedup of DMA over hw is about %ux faster\n\n", speedupDMA);
        } else if (speedupHW > 0 && dmaCRCTime > hardwareCRCTime) {
            printf("Speedup of hw over DMA is about %ux faster\n\n", speedupHW);
        }
    }

}

/* Completion interrupt for DMA */
void DMA_INT1_IRQHandler(void)
{
    MAP_DMA_disableChannel(0);
    crcSignature = MAP_CRC32_getResult(CRC32_MODE);

    // indicate completion of transfer
    dma_done = 1;
}
