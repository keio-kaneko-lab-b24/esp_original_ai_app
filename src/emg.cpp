#include <Arduino.h>

#include "emg.h"
#include "param_ml.h"

volatile float extensor_value = 0;
volatile float flexor_value = 0;

volatile int begin_index = 0;
volatile float raw_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float raw_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float ar_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float ar_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float n_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float n_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float ra_extensor_values[RAW_EMG_LENGTH] = {0};
volatile float ra_flexor_values[RAW_EMG_LENGTH] = {0};
volatile float d_extensor_values[kModelInputWidth] = {0};
volatile float d_flexor_values[kModelInputWidth] = {0};
volatile float buffer_input[kModelInputWidth * kModelInputHeight * kChannleNumber] = {0};