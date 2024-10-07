/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

int main(void) {
    /* Halting the Watchdog */
    MAP_WDT_A_holdTimer();

    /* Configuring P2.0, P2.1, P2.2 as outputs */
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);

    /* Set LEDs to low */
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0); // red
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1); // green
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2); // blue

    /* Configure P1.1 (S1) as input with pull-up resistor */
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // Assign alias region addresses for calculation
    unsigned int alias_region_addr = 0x42000000;
    unsigned int peripheral_region_addr = 0x40000000;

    // Create an unint8_t pointer for the Blue LED alias address
    uint8_t* led_alias_addr;
    unsigned int led_port_addr = 0x40004C03;
    unsigned int bit_pos_led = 0x2;

    // Perform alias address calculation for the Blue LED
    led_alias_addr = (led_port_addr - peripheral_region_addr) * 32 + bit_pos_led * 4 + alias_region_addr;
    printf("Blue LED Alias Address: %p\n", led_alias_addr);

    // Create an unint8_t pointer for the button S1 alias address
    uint8_t *s1_alias_addr;
    unsigned int s1_port_addr = 0x40004C00;
    unsigned int bit_pos_s1 = 0x1;

    // Perform alias address calculation for the button S1
    s1_alias_addr = (s1_port_addr - peripheral_region_addr) * 32 + bit_pos_s1 * 4 + alias_region_addr;
    printf("S1 Alias Address: %p\n", s1_alias_addr);

    // Button press status bool
    bool buttonWasPressedBefore = false;

    // Blue LED status bool
    bool LEDOn = false;

    while (true) {
        // If the button S1 is pressed
        if (*s1_alias_addr == 0) {
            // And if the button S1 was not pressed before
            if (!buttonWasPressedBefore) {
                // Indicate that the button S1 was pressed before
                buttonWasPressedBefore = true;

                //If the LED is currently on
                if (LEDOn == true) {
                    //Then turn off the LED and indicate that it is off
                    *led_alias_addr = 0x0;
                    LEDOn = false;
                }
                else {
                    //Else turn the LED on and indicate that it is on
                    *led_alias_addr = 0x1;
                    LEDOn = true;
                }
            } else {
                //Otherwise, indicate that the button S1 was not pressed before
                buttonWasPressedBefore = false;
            }
        }
    }
}
