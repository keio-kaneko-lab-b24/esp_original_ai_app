#include <Arduino.h>

#include "input_handler.h"
#include "emg.h"

#define MD_IN1 34 // GPIO #34
#define MD_IN2 35 // GPIO #35

char handle_input_s[64];

bool HandleInput()
{

    int a, b;
    b = analogRead(MD_IN1);
    a = analogRead(MD_IN2);
    raw_extensor_values[begin_index] = a;
    raw_flexor_values[begin_index] = b;

    // if (begin_index % 100 == 0)
    // {
    //     unsigned long currentMillis = xTaskGetTickCount();
    //     sprintf(handle_input_s, "time: %lu\n", currentMillis);
    //     Serial.println(handle_input_s);
    //     sprintf(handle_input_s, "index: %d\ne: %d\nf: %d\n", begin_index, r_extensor_data[begin_index], r_flexor_data[begin_index]);
    //     Serial.println(handle_input_s);
    // }

    unsigned long currentMillis = xTaskGetTickCount();
    sprintf(handle_input_s, "time: %lu\n", currentMillis);
    Serial.println(handle_input_s);
    sprintf(handle_input_s, "index: %d\ne: %f\nf: %f\n", begin_index, raw_extensor_values[begin_index], raw_flexor_values[begin_index]);
    Serial.println(handle_input_s);

    return true;
}