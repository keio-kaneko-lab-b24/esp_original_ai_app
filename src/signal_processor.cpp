#include <Arduino.h>

#include "signal_processor.h"
#include "param.h"
#include "emg.h"

char signal_process_s[100];

/**
 * 信号処理
 */
void SignalProcess()
{
    sprintf(signal_process_s, "normalize_max: %f, normalize_min: %f", normalize_max, normalize_min);
    Serial.println(signal_process_s);

    // Serial.println("before normalize");
    // for (int i = 0; i < EMG_LENGTH; ++i)
    // {
    //     sprintf(signal_process_s, "%d: %f, %f", i, extensor_values[i], flexor_values[i]);
    //     Serial.println(signal_process_s);
    // }

    // 整列
    ArrangeArray(
        extensor_values,
        flexor_values,
        ar_extensor_values,
        ar_flexor_values,
        begin_index,
        EMG_LENGTH);

    // 正規化
    Normalize(
        ar_extensor_values,
        ar_flexor_values,
        s_extensor_values,
        s_flexor_values,
        EMG_LENGTH);

    //     Serial.println("after normalize");
    //     for (int i = 0; i < EMG_LENGTH; ++i)
    //     {
    //         sprintf(signal_process_s, "%d: %f, %f", i, s_extensor_values[i], s_flexor_values[i]);
    //         Serial.println(signal_process_s);
    //     }
}

/**
 * 正規化
 */
void Normalize(
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile float s_extensor_values[],
    volatile float s_flexor_values[],
    const int value_length)
{
    for (int i = 0; i < EMG_LENGTH; ++i)
    {
        s_extensor_values[i] = _Normalize(ar_extensor_values[i]);
        s_flexor_values[i] = _Normalize(ar_flexor_values[i]);
    }
}

/**
 * 正規化（1つの値のみ）
 */
float _Normalize(float value)
{
    float n_value = (value - normalize_min) / (normalize_max - normalize_min);
    if (n_value >= 1)
    {
        return 1;
    }
    else if (n_value <= 0)
    {
        return 0;
    }
    return n_value;
}

/**
 * リングバッファの整列
 */
void ArrangeArray(
    volatile float extensor_values[],
    volatile float flexor_values[],
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile int begin_index,
    const int value_length)
{
    for (int i = 0; i < value_length; ++i)
    {
        int ring_array_index = begin_index + i - value_length + 1;
        if (ring_array_index < 0)
        {
            ring_array_index += value_length;
        }
        ar_extensor_values[i] = extensor_values[ring_array_index];
        ar_flexor_values[i] = flexor_values[ring_array_index];
    }
}