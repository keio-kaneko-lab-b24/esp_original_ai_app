#include <Arduino.h>

#include "emg.h"

volatile float extensor_value = 0;
volatile float flexor_value = 0;

volatile int begin_index = 0;
volatile int raw_extensor_values[RAW_EMG_LENGTH] = {0};
volatile int raw_flexor_values[RAW_EMG_LENGTH] = {0};
volatile int ar_extensor_values[RAW_EMG_LENGTH] = {0};
volatile int ar_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float n_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float n_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float ra_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float ra_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float d_extensor_values[MODEL_INPUT_WIDTH] = {0};
volatile float d_flexor_values[MODEL_INPUT_WIDTH] = {0};
volatile float buffer_input[MODEL_INPUT_WIDTH * MODEL_INPUT_HEIGHT * CHANNEL_NUMBER] = {0};