
#include "sensor_buzzer.h"
#include "rpi_gpio.h"
#include "time_utils.h"
#include "log.h"


void play_sound (
        const unsigned int gpio_num,
        const int long_in_ms)
{
    for (int i = 0; i < long_in_ms; ++ i)
    {
        sleep_milliseconds (1);
        set_pin_voltage (gpio_num, HIGH);
        sleep_milliseconds (1);
        set_pin_voltage (gpio_num, LOW);
    }
}

void buzzer_play_sound (
        const unsigned int gpio_num,
        const int sound_id)
{
    set_pin_direction (gpio_num, OUTPUT);
    set_pin_voltage (gpio_num, LOW);

    switch (sound_id)
    {

        case BUZZER_SHORT_BEEP:
            play_sound (gpio_num, 80);
            break;

        case BUZZER_LONG_BEEP:
            play_sound (gpio_num, 200);
            break;

        case BUZZER_TWO_SHORT_BEEPS:
            play_sound (gpio_num, 80);
            sleep_milliseconds (100);
            play_sound (gpio_num, 80);
            break;

        default:
            break;
    }
}
