#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <time.h>

#include "defines.hpp"
#include "rpi_gpio.h"
#include "time_utils.h"


// GPIO 27, 26, 25, 33. leds
#define LED1_PIN        GPIO_NUM_27
#define LED2_PIN        GPIO_NUM_26
#define LED3_PIN        GPIO_NUM_25
#define LED4_PIN        GPIO_NUM_33

// GPIO 27, button
#define BUTTON_PIN      GPIO_NUM_16


#define WATER_REFULING_TIME     100


volatile unsigned int button_pressed = 0;

static f_callback p_callback = NULL;

static void IRAM_ATTR gpio_isr_handler (
        void* arg)
{
    button_pressed = (uint32_t) arg;
}

static void gpio_task_worker (
        void* arg)
{
    for (;;)
    {
        if (button_pressed)
        {
            int liquid_level = get_liquid_level ();
            LOG_INFO ("fuiling start. level %d,%d %%", liquid_level / 10, liquid_level % 10);

            bool toggle = true;

            time_t task_curr_time = time (NULL);
            time_t task_stop_time = task_curr_time + WATER_REFULING_TIME;
            while (task_stop_time > task_curr_time)
            {
                liquid_level = get_liquid_level () / 10;

                if (toggle)
                {
                    set_pin_voltage (LED1_PIN, HIGH);
                    set_pin_voltage (LED2_PIN, HIGH);
                    set_pin_voltage (LED3_PIN, HIGH);
                    set_pin_voltage (LED4_PIN, HIGH);

                    toggle = false;
                }
                else
                {
                    if (liquid_level < 95)
                        set_pin_voltage (LED4_PIN, LOW);
                    if (liquid_level < 70)
                        set_pin_voltage (LED3_PIN, LOW);
                    if (liquid_level < 45)
                        set_pin_voltage (LED2_PIN, LOW);
                    if (liquid_level < 20)
                        set_pin_voltage (LED1_PIN, LOW);

                    toggle = true;
                }

                sleep_milliseconds (300);
                task_curr_time = time (NULL);
            }

            liquid_level = get_liquid_level ();
            LOG_INFO ("fuiling finished. level %d,%d %%", liquid_level / 10, liquid_level % 10);

            if (p_callback != NULL)
            {
                std::vector <std::string> data;
                data.push_back ("\"level\" : " + std::to_string (liquid_level));
                p_callback ("fueling", 1, data);
            }

            button_pressed = 0;
        }

        sleep_milliseconds (100);
    }
}


// button
void setup_button (
        const f_callback f)
{
    p_callback = f;

    // leds
    set_pin_direction (LED1_PIN, OUTPUT);
    set_pin_direction (LED2_PIN, OUTPUT);
    set_pin_direction (LED3_PIN, OUTPUT);
    set_pin_direction (LED4_PIN, OUTPUT);

    // change gpio intrrupt type for one pin
    set_pin_direction (BUTTON_PIN, INPUT);
    gpio_set_intr_type (BUTTON_PIN, GPIO_INTR_ANYEDGE);

    // start gpio task
    xTaskCreate (gpio_task_worker, "button_task", 4096, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service (0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add (BUTTON_PIN, gpio_isr_handler, (void*) BUTTON_PIN);
}
