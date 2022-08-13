#include <Arduino.h>

#include "signal_processor.h"
#include "param.h"
#include "emg.h"

char sp_s[64];

char signal_process_s[100];
// RollingAverageのフィルタ幅
int filterWidth = 10;

/**
 * 信号処理
 */
void SignalProcess()
{
    // 整列
    ArrangeArray(
        raw_extensor_values,
        raw_flexor_values,
        ar_extensor_values,
        ar_flexor_values,
        begin_index,
        RAW_EMG_LENGTH);

    // 正規化
    float extensor_mean = Mean(ar_extensor_values, RAW_EMG_LENGTH);
    float flexor_mean = Mean(ar_flexor_values, RAW_EMG_LENGTH);
    Normalization(
        ar_extensor_values,
        ar_flexor_values,
        n_extensor_values,
        n_flexor_values,
        extensor_mean,
        flexor_mean,
        RAW_EMG_LENGTH);

    sprintf(sp_s, "e_norm: %f\nf_norm: %f", n_extensor_values[RAW_EMG_LENGTH - 1], n_flexor_values[RAW_EMG_LENGTH - 1]);
    Serial.println(sp_s);

    // 正規化
    RollingAverage(
        n_extensor_values,
        n_flexor_values,
        ra_extensor_values,
        ra_flexor_values,
        RAW_EMG_LENGTH);

    // ダウンサンプリング
    DownSample(
        ra_extensor_values,
        ra_flexor_values,
        d_extensor_values,
        d_flexor_values,
        RAW_EMG_LENGTH,
        kModelInputWidth);

    sprintf(sp_s, "e_sp: %f\nf_sp: %f", d_extensor_values[RAW_EMG_LENGTH - 1], d_flexor_values[RAW_EMG_LENGTH - 1]);
    Serial.println(sp_s);

    // カテゴリ化
    Categorize(
        d_extensor_values,
        d_flexor_values,
        buffer_input,
        kModelInputWidth,
        kModelInputHeight);
}

/**
 * リングバッファの整列
 */
void ArrangeArray(
    volatile float raw_extensor_values[],
    volatile float raw_flexor_values[],
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
        ar_extensor_values[i] = raw_extensor_values[ring_array_index];
        ar_flexor_values[i] = raw_flexor_values[ring_array_index];
    }
}

/**
 * フィルタ＋正規化
 */
void Normalization(
    volatile float ar_extensor_data[],
    volatile float ar_flexor_data[],
    volatile float n_extensor_data[],
    volatile float n_flexor_data[],
    float extensor_mean,
    float flexor_mean,
    const int r_length)
{
    for (int i = 0; i < r_length; ++i)
    {

        float f_extensor = (float)ar_extensor_data[i];
        float f_flexor = (float)ar_flexor_data[i];

        // 正規化
        n_extensor_data[i] = f_extensor;
        n_flexor_data[i] = f_flexor;
        n_extensor_data[i] = abs(f_extensor - extensor_mean);
        n_flexor_data[i] = abs(f_flexor - flexor_mean);
    }
}

/**
 * 移動平均
 */
void RollingAverage(
    volatile float n_extensor_data[],
    volatile float n_flexor_data[],
    volatile float ra_extensor_data[],
    volatile float ra_flexor_data[],
    const int r_length)
{
    float extensor_sum = 0;
    float flexor_sum = 0;
    int m_i = 0;

    for (int i = 0; i < filterWidth; i++)
    {
        extensor_sum += n_extensor_data[i];
        flexor_sum += n_flexor_data[i];
        if (i >= (filterWidth / 2) - 1)
        {
            ra_extensor_data[m_i] = extensor_sum / (i + 1);
            ra_flexor_data[m_i] = flexor_sum / (i + 1);
            m_i += 1;
        }
    }

    for (int i = filterWidth; i < r_length; i++)
    {
        extensor_sum -= n_extensor_data[i - filterWidth];
        extensor_sum += n_extensor_data[i];
        ra_extensor_data[m_i] = extensor_sum / filterWidth;
        flexor_sum -= n_flexor_data[i - filterWidth];
        flexor_sum += n_flexor_data[i];
        ra_flexor_data[m_i] = flexor_sum / filterWidth;
        m_i += 1;
    }

    for (int i = r_length; i < r_length + (filterWidth / 2); i++)
    {
        extensor_sum -= n_extensor_data[i - filterWidth];
        ra_extensor_data[m_i] = extensor_sum / (filterWidth - (i - r_length) + 1);
        flexor_sum -= n_flexor_data[i - filterWidth];
        ra_flexor_data[m_i] = flexor_sum / (filterWidth - (i - r_length) + 1);
        m_i += 1;
        if (m_i >= r_length)
        {
            break;
        }
    }
}

/**
 * ダウンサンプリング
 * ra_*（orignal長）をd_*（sampled長）にダウンサプリングする
 */
void DownSample(
    volatile float ra_extensor_data[],
    volatile float ra_flexor_data[],
    volatile float d_extensor_data[],
    volatile float d_flexor_data[],
    const int original_length,
    const int sampled_length)
{
    int step = original_length / sampled_length;
    // 初期化
    int j = 0;
    for (int i = 0; i < original_length; i += step)
    {
        d_extensor_data[j] = ra_extensor_data[i];
        d_flexor_data[j] = ra_flexor_data[i];
        j += 1;
    }
}

/**
 * カテゴライズ
 */
void Categorize(
    volatile float d_extensor_values[],
    volatile float d_flexor_values[],
    volatile float buffer_input[],
    const int input_width,
    const int input_height)
{

    // 初期化
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        buffer_input[0] = 0.0;
    }

    for (int i = 0; i < input_width; ++i)
    {
        int e_index_ = _CategorizeIndex(d_extensor_values[i]);
        int f_index_ = _CategorizeIndex(d_flexor_values[i]);
        int base_index = i * input_height * 2;
        int e_index = base_index + (e_index_ * 2);
        int f_index = base_index + (f_index_ * 2) + 1;
        // sprintf(signal_process_s, "index-> %d %d", e_index, f_index);
        // Serial.println(signal_process_s);
        buffer_input[e_index] = 1.0;
        buffer_input[f_index] = 1.0;
    }
}

/**
 * カテゴライズIndexの取得
 */
int _CategorizeIndex(float value)
{
    int index = floor(value * kModelInputHeight);
    if (index >= kModelInputHeight)
    {
        index = kModelInputHeight - 1;
    }
    return index;
}

float Mean(
    volatile float data[],
    int length)
{
    int total = 0;
    for (int i = 0; i < length; i++)
    {
        total += data[i];
    }
    float mean_value = (float)total / (float)length;
    return mean_value;
}