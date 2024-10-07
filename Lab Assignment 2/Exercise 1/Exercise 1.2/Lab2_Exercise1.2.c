/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

//![Simple GPIO Config]

// Define base addresses and register addresses based on the provided assignments
#define P1INREGADDR      (*((volatile uint8_t *) 0x40004C00))  // Input register for P1
#define P1OUTREGADDR     (*((volatile uint8_t *) 0x40004C02))  // Output register for P1
#define P1DIRREGADDR     (*((volatile uint8_t *) 0x40004C04))  // Direction register for P1
#define P1RENREGADDR     (*((volatile uint8_t *) 0x40004C06))  // Resistor enable register for P1

#define P2OUTREGADDR     (*((volatile uint8_t *) 0x40004C03))  // Output register for P2
#define P2DIRREGADDR     (*((volatile uint8_t *) 0x40004C05))  // Direction register for P2

// Pin definitions
#define P1_1      (1 << 1)   // S1 button is connected to P1.1
#define P2_2      (1 << 2)   // Blue LED is connected to P2.2

int main(void)
{
    // Stop watchdog timer
    WDT_A_holdTimer();

    // Initialize output pins for Red, Green, and Blue LEDs
    P2DIRREGADDR = ((1 << 0) | (1 << 1) | P2_2); // Set P2.0, P2.1, and P2.2 as outputs

    // Initialize input pin for button S1
    P1DIRREGADDR &= ~P1_1;  // Set P1.1 as input
    P1RENREGADDR |= P1_1;   // Enable pull-up/pull-down resistor on P1.1
    P1OUTREGADDR |= P1_1;   // Set pull-up resistor on P1.1

    // Turn off all LEDs initially (Set all LED pins low)
    P2OUTREGADDR &= ~((1 << 0) | (1 << 1) | P2_2); // Turn off all LEDs

    // Variable to keep track of button press and LED state
    uint8_t buttonWasPressed = 0;

    while (true)
    {
        // Check if the button is pressed (Low when pressed)
        if ((P1INREGADDR & P1_1) == 0)
        {
            // Toggle the Blue LED only if the button was not already pressed
            if (!buttonWasPressed)
            {
                // Toggle Blue LED
                P2OUTREGADDR ^= P2_2;

                // Set the flag to indicate the button is now pressed
                buttonWasPressed = 1;
            }
        }
        else
        {
            // Reset the button pressed flag when the button is released
            buttonWasPressed = 0;
        }
    }
}






//![Simple GPIO Config]


