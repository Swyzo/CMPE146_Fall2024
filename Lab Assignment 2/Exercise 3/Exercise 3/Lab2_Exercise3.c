/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//![Simple GPIO Config]
int main(void) {

    volatile uint32_t ii;

    /* Halting the Watchdog */
    MAP_WDT_A_holdTimer();

    /* Configuring P1.0 as output */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    // Create a 32-bit counter with the DriverLib functions Timer32
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);
    Timer32_startTimer(TIMER32_0_BASE, 0);
    uint32_t clockFreq = CS_getMCLK();

    //Variables for the count calculation for frequency
    uint32_t startCounter, endCounter, counterDiff;
    float duration, blinkFreq;


    while (true) {
        //Start counter for frequency calculation
        startCounter = Timer32_getValue(TIMER32_0_BASE);

        /* Delay Loop */
        for(ii=0;ii<100000;ii++){}

        //Toggles Red LED to blink
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //End counter for frequency calculation
        endCounter = Timer32_getValue(TIMER32_0_BASE);

        //Get difference between start and end
        counterDiff = startCounter - endCounter;

        //Calculate duration in ms
        duration = (counterDiff / (float)clockFreq) * 1000;

        //Calculate blink frequency (Hz) using the duration (ms)
        blinkFreq = 1000 / (2 * duration);

        //Print statement for important details
        printf("Counter Difference: %u, Duration (ms): %.2f, Blinking Frequency: %.2f Hz\n", counterDiff, duration, blinkFreq);
    }
}
//![Simple GPIO Config]


