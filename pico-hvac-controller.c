#include "pico/stdlib.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef GPIO_COOL
#define GPIO_COOL 2
#endif

#ifndef GPIO_COOL_DIRECTION
#define GPIO_COOL_DIRECTION GPIO_IN
#endif

#ifndef GPIO_FREEZE
#define GPIO_FREEZE 3
#endif

#ifndef GPIO_FREEZE_DIRECTION
#define GPIO_FREEZE_DIRECTION GPIO_IN
#endif

#ifndef GPIO_COMPRESSOR
#define GPIO_COMPRESSOR 4
#endif

#ifndef GPIO_COMPRESSOR_DIRECTION
#define GPIO_COMPRESSOR_DIRECTION GPIO_OUT
#endif

#ifndef GPIO_FAN
#define GPIO_FAN 5
#endif

#ifndef GPIO_FAN_DIRECTION
#define GPIO_FAN_DIRECTION GPIO_OUT
#endif

#ifndef GPIO_REFERENCE
#define GPIO_REFERENCE 6
#endif

#ifndef GPIO_REFERENCE_DIRECTION
#define GPIO_REFERENCE_DIRECTION GPIO_OUT
#endif

#ifndef GPIO_REFERENCE_STATE
#define GPIO_REFERENCE_STATE true
#endif

 // Min Runtime - 3 Minutes
#ifndef COMPRESSOR_MIN_RUNTIME
#define COMPRESSOR_MIN_RUNTIME 180000
#endif

// Min Rest Time - 5 Minutes
#ifndef COMPRESSOR_MIN_OFFTIME
#define COMPRESSOR_MIN_OFFTIME 300000
#endif

// Delay Freeze Handling - 60 Minutes
#ifndef COMPRESSOR_FREEZE_DELAY
#define COMPRESSOR_FREEZE_DELAY 3600000
#endif

// GPIO Polling Rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 1000
#endif

#ifndef ON
#define ON true
#endif

#ifndef OFF
#define OFF false
#endif

// Perform initialisation
int pico_gpio_init(void) {
    #if defined(CYW43_WL_GPIO_LED_PIN)
        // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
        // so we can use normal GPIO functionality to turn the led on and off
        // For Pico W devices we need to initialise the driver etc
        cyw43_arch_init();
    #endif
    gpio_init(GPIO_COOL);
    gpio_set_dir(GPIO_COOL, GPIO_COOL_DIRECTION);
    gpio_init(GPIO_FREEZE);
    gpio_set_dir(GPIO_FREEZE, GPIO_FREEZE_DIRECTION);
    gpio_init(GPIO_COMPRESSOR);
    gpio_set_dir(GPIO_COMPRESSOR, GPIO_COMPRESSOR_DIRECTION);
    gpio_init(GPIO_FAN);
    gpio_set_dir(GPIO_FAN, GPIO_FAN_DIRECTION);
    gpio_init(GPIO_REFERENCE);
    gpio_set_dir(GPIO_REFERENCE, GPIO_REFERENCE_DIRECTION);
    return PICO_OK;
}

// Turn a GPIO on or off
void pico_set_output(int pin, bool state) {
    gpio_put(pin, state);
}

// Ask the wifi "driver" to set the GPIO on or off
void pico_set_led(bool state) {
    #if defined(CYW43_WL_GPIO_LED_PIN)
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
    #endif
}

void pico_blink_led(int ontime, int offtime) {
        pico_set_led(ON);
        sleep_ms(ontime);
        pico_set_led(OFF);
        sleep_ms(offtime);
}

void blink_message(int message) {
    switch (message)
    {
    case 0:
        pico_set_led(OFF);
        break;
    case 1:
        pico_set_led(ON);
        break;
    case 2:
        pico_blink_led(100, 100);
        break;
    case 3:
        for (int i = 0; i < 5; i++){
            pico_blink_led(100, 100);
        }
        break;
    default:
        break;
    }
}

// Compressor Startup Routine
void compressor_startup(int time) {
    // Turn compressor and fan on
    pico_set_output(GPIO_COMPRESSOR, ON);
    pico_set_output(GPIO_FAN, ON);
    // Run compressor and fan
    sleep_ms(time);
}

// Compressor Shutdown Routine
void compressor_shutdown(int time) {
    // Turn compressor off
    pico_set_output(GPIO_COMPRESSOR, OFF);
    // Run fan with compressor off
    pico_set_output(GPIO_FAN, ON);
    sleep_ms(time);
    // Turn fan off
    pico_set_output(GPIO_FAN, OFF);
}

int main() {
    int rc = pico_gpio_init();
    hard_assert(rc == PICO_OK);
    blink_message(3);

     // Set the reference output to the desired state
    gpio_put(GPIO_REFERENCE, GPIO_REFERENCE_STATE);

    // Power Loss Recovery Routine
    // When we first start up, we have no idea how long the compressor has been running or resting for
    // If we booted with thermostat calling for cool, there might have been a power blip or program error
    // In this case, assume we need to give the compressor a rest period to prevent short cycling
    if (gpio_get(GPIO_COOL)) {
        compressor_shutdown(COMPRESSOR_MIN_OFFTIME);
    }

    bool cool_mode = false;
    bool cool_call = false;
    bool freeze_in = false;

    // Main HVAC Monitoring Loop
    while (true) {
        //blink_message(2);
        cool_call = gpio_get(GPIO_COOL);
        freeze_in = gpio_get(GPIO_FREEZE);

        // The freeze stat input is triggered
        if(freeze_in){
            blink_message(1);
            bool starting_cool_call = cool_call;
            // A low temperature event can trigger some time before icing actually obstructs airflow
            // To maximize compressor runtime during this period, delay executing the compressor shutdown for some time
            // Don't bother running if the system isn't calling for cooling
            if(cool_call){
                for(int i=0; i<COMPRESSOR_FREEZE_DELAY/SAMPLE_RATE; i++) {
                    sleep_ms(SAMPLE_RATE);
                    // Check if the cooling call state changes during this delay period
                    // If it does, abandon the delay and skip right to the thaw routine
                    if(starting_cool_call != gpio_get(GPIO_COOL)) {
                        break;
                    }
                }
            }

            // Turn compressor off
            pico_set_output(GPIO_COMPRESSOR, OFF);
            // Turn fan on
            pico_set_output(GPIO_FAN, ON);

            // Wait until the freeze stat indicates a thawed state
            while(gpio_get(GPIO_FREEZE)) {
                sleep_ms(SAMPLE_RATE);
            }
            
            // Turn fan off
            pico_set_output(GPIO_FAN, OFF);
            
            cool_mode = false;
            blink_message(0);
        } else {
            if(cool_call == cool_mode) {
                // Call State Matches Current Mode
                sleep_ms(SAMPLE_RATE);
            } else if (cool_call && !cool_mode) {
                // Cooling Requested, Not Currently Cooling
                blink_message(1);
                compressor_startup(COMPRESSOR_MIN_RUNTIME);
                cool_mode = true;
                blink_message(0);
            } else {
                // Cooling Not Requested, Currently Cooling
                blink_message(1);
                compressor_shutdown(COMPRESSOR_MIN_OFFTIME);
                cool_mode = false;
                blink_message(0);
            }
        }
    }
}

/*
    freeze_mode can run independently of a cool_call, so it's a "global" mode
    kind of like if thermostat was calling for fan only
    So we need to do a freeze_mode check on every sample
    If not in freeze_mode, nothing changes, we run the routine as usual
    If we are in freeze_mode, run the compressor_pause() routine
    This will turn off the compressor and run the fan for a set period
    Then we need to check what to do, because some time has passed
    I think the easiest way is to reset cool_mode to false
    This will let the loop run without modification, so compressor_startup runs as usual
*/