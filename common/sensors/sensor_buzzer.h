
#ifndef SENSOR_BUZZER_H
#define SENSOR_BUZZER_H


#ifdef __cplusplus
extern "C" {
#endif

#define BUZZER_SHORT_BEEP       0
#define BUZZER_LONG_BEEP        1
#define BUZZER_TWO_SHORT_BEEPS  2

void buzzer_play_sound (
        const unsigned int gpio_num,
        const int sound_id);

#ifdef __cplusplus
}
#endif

#endif // ifndef SENSOR_BUZZER_H
