/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//![Simple GPIO Config]
int main(void) {

    // Stop watchdog timer
    WDT_A_holdTimer();

    // Set the LED pins (P2.0, P2.1, P2.2) as output
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    // Set the S1 button (P1.1) as input with pull-up resistor
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // Turn off all LEDs by setting output pins low
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    // Create a 32-bit counter with the DriverLib functions Timer32
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);
    Timer32_startTimer(TIMER32_0_BASE, 0);

    // Get the master clock (MCLK) value for reference
    uint32_t clockFrequency = CS_getMCLK();


    // Variable to store previous state of S1
    uint8_t wasButtonPressedBefore = 0;

    // Variable to store LED state (on or off)
    uint8_t LEDOn = 0;

    // Variables to measure button press duration
    uint32_t pressStartTime = 0;
    uint32_t pressEndTime = 0;
    uint32_t pressDuration = 0;

    // Main control loop
    while (true) {
        // Read current button state
        uint8_t buttonPressedNow = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == GPIO_INPUT_PIN_LOW;

        if (buttonPressedNow && !wasButtonPressedBefore) {
            // Capture the timer value when the button is first pressed
            pressStartTime = Timer32_getValue(TIMER32_0_BASE);

            // Toggle the state of the blue LED
            if (LEDOn) {
                GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);  // Turn off blue LED
                LEDOn = 0;  // Update state
            }
            else {
                GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2); // Turn on blue LED
                LEDOn = 1;  // Update state
            }

            wasButtonPressedBefore = 1;  // Mark that the button is pressed
        }
        else if (!buttonPressedNow && wasButtonPressedBefore) {
            // Capture the timer value when the button is released
            pressEndTime = Timer32_getValue(TIMER32_0_BASE);

            // Calculate the press duration in counter ticks
            pressDuration = pressStartTime - pressEndTime;

            // Convert the press duration to milliseconds and calculate time
            float timeMs = (pressDuration * 1000.0f) / clockFrequency;

            // Print the duration in milliseconds
            printf("Button was pressed for %.2f ms\n", timeMs);

            wasButtonPressedBefore = 0;  // Reset button state for next detection
        }
    }
}
//![Simple GPIO Config]
