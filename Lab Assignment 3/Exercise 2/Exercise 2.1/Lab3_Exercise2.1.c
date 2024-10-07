/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

void PORT1_IRQHandler(void);

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    // Set Green LED as Output (Port 2, Pin 1)
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);

    // Ensure the Green LED starts OFF
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

    // Set S1 Button (Port 1, Pin 1) as input with a pull-up resistor
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // Configure GPIO interrupt for High-to-Low transition (button press)
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    // Enable Port 1 interrupt in the NVIC
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    // Enable global interrupts
    MAP_Interrupt_enableMaster();

    // Control Loop (NO FUNCTIONALITY)
    while (1)
    {
        // Empty loop
    }

}

// ISR for Port 1 interrupts
void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);

    // Check if the interrupt was caused by S1 button (Port 1, Pin 1)
    if (status & GPIO_PIN1) {
        // If button is pressed (High-to-Low transition)
        if (MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == GPIO_INPUT_PIN_LOW) {
            // Then toggle the Green LED (Port 2, Pin 1)
            MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
        }

        // Clear the interrupt flag for S1 (Port 1, Pin 2)
        MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    }
}
