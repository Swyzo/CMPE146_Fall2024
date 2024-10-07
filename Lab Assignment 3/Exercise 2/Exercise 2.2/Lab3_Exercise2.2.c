/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define QUEUE_SIZE 10

// Record structure for circular queue
struct record {
    uint32_t timestamp;
    uint8_t state;
};

// Circular queue variables
struct record queue[QUEUE_SIZE];
volatile int read_index = 0;
volatile int write_index = 0;
uint32_t previous_press_timestamp = 0;  // Store the last press event's timestamp

/* Function Prototypes */
void PORT1_IRQHandler(void);
void Timer32_init(void);
void process_queue(void);
uint32_t get_current_time(void);

int main(void)
{
    /* Stop Watchdog */
    MAP_WDT_A_holdTimer();

    // Set Green LED as Output (Port 2, Pin 1)
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);

    // Ensure the Green LED starts OFF
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

    // Set S1 Button (Port 1, Pin 1) as input with a pull-up resistor
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // Configure GPIO interrupt for both edges (press and release)
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION); // Initially detect press on S1
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    // Enable Port 1 interrupt in the NVIC
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    // Enable global interrupts
    MAP_Interrupt_enableMaster();

    // Initialize Timer32 for time measurement
    Timer32_init();

    // Control Loop
    while (1)
    {
        // Simulate CPU doing other tasks (delay)
        __delay_cycles(200 * 3000); // Delay 200 ms at 3 MHz

        // Process the queue and compute time duration
        process_queue();
    }
}

// ISR for Port 1 interrupts
void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);

    // Check if the interrupt was caused by S1 button (Port 1, Pin 1)
    if (status & GPIO_PIN1) {
        // Get the current timestamp
        uint32_t timestamp = get_current_time();

        // Get the button state (pressed = low, released = high)
        uint8_t state = MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1);

        // Store the record in the circular queue
        int next_write_index = (write_index + 1) % QUEUE_SIZE;

        if (next_write_index != read_index) { // Check if queue is not full
            queue[write_index].timestamp = timestamp;
            queue[write_index].state = state;
            write_index = next_write_index;

            // Print debug information
            printf("ISR: Button %s. Timestamp: %lu\n", (state == GPIO_INPUT_PIN_LOW) ? "Pressed" : "Released", timestamp);

            // Toggle LED on button press
            if (state == GPIO_INPUT_PIN_LOW) {
                MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
                previous_press_timestamp = timestamp; // Store the press timestamp
            }
        } else {
            // Queue full, print error
            printf("Queue full, dropping record!\n");
        }

        // Toggle edge detection for next interrupt (to detect release)
        MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, (state == GPIO_INPUT_PIN_LOW) ? GPIO_LOW_TO_HIGH_TRANSITION : GPIO_HIGH_TO_LOW_TRANSITION);

        // Clear the interrupt flag for S1
        MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    }
}

// Initialize Timer32 for free-running mode
void Timer32_init(void)
{
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);
    Timer32_startTimer(TIMER32_0_BASE, 0);
}

// Get the current time from Timer32
uint32_t get_current_time(void)
{
    return Timer32_getValue(TIMER32_0_BASE);
}

// Process the queue and compute button press duration
void process_queue(void)
{
    // Check if there is any new data in the queue
    if (read_index != write_index) {
        struct record rec = queue[read_index];

        // Advance read index (circular buffer)
        read_index = (read_index + 1) % QUEUE_SIZE;

        // Process the record only when the button is released
        if (rec.state == GPIO_INPUT_PIN_HIGH) {
            // Calculate the time duration considering Timer32 is a down-counter
            uint32_t duration = 0;

            if (rec.timestamp <= previous_press_timestamp) {
                // Normal case, no wrap-around
                duration = previous_press_timestamp - rec.timestamp;
            } else {
                // Handle timer overflow (wrap-around)
                duration = previous_press_timestamp + (0xFFFFFFFF - rec.timestamp) + 1;
            }

            // Convert duration from clock cycles to milliseconds (assuming 3 MHz clock)
            uint32_t duration_ms = duration / 3000;

            // Print the time duration and read_index
            printf("Button press duration: %lu ms, read_index: %d\n", duration_ms, read_index);
        }
    }
}
