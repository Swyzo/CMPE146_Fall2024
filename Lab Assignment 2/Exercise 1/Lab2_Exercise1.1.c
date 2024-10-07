/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

//![Simple GPIO Config]
int main(void)
{
    // Stop the watchdog timer
    MAP_WDT_A_holdTimer();

    // Configure LED pins as output
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);  // Red LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);  // Green LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);  // Blue LED

    // Configure S1 push button as input with pull-up resistor
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // Turn off all LEDs by setting pins low
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);  // Red LED OFF
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);  // Green LED OFF
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);  // Blue LED OFF

    // Variable to track the previous button state (initialized to released state)
    bool buttonWasPressedBefore = false;

    // Main control loop
    while (true)
    {
        // Bool to check if the button is pressed (active low logic)
        bool buttonIsPressed = (MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == GPIO_INPUT_PIN_LOW);

        if (buttonIsPressed && !buttonWasPressedBefore)
        {
            // Toggle the blue LED when the button is pressed
            MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }

        // Update the button state
        buttonWasPressedBefore = buttonIsPressed;
    }
}

//![Simple GPIO Config]


