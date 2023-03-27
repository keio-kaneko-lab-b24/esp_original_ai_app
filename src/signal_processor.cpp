#include <Arduino.h>

#include "signal_processor.h"
#include "model_param.h"
#include "emg.h"

char sp_s[64];

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

    // 正規化(平均)
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

    // 移動平均
    RollingAverage(
        n_extensor_values,
        n_flexor_values,
        ra_extensor_values,
        ra_flexor_values,
        RAW_EMG_LENGTH);

    // ダウンサンプリング
    const int step = 10;
    DownSample(
        ra_extensor_values,
        ra_flexor_values,
        d_extensor_values,
        d_flexor_values,
        RAW_EMG_LENGTH,
        MODEL_INPUT_WIDTH,
        step);

    unsigned long currentMillis = xTaskGetTickCount();
    sprintf(sp_s, "time: %lu\ne_sp: %f\nf_sp: %f", currentMillis, d_extensor_values[MODEL_INPUT_WIDTH - 1], d_flexor_values[MODEL_INPUT_WIDTH - 1]);
    Serial.println(sp_s);

    // 正規化(0-1)
    for (int i = 0; i < MODEL_INPUT_WIDTH; ++i)
    {
        d_extensor_values[i] = _NormalizationZeroOne(d_extensor_values[i]);
        d_flexor_values[i] = _NormalizationZeroOne(d_flexor_values[i]);
    }

    // カテゴリ化
    Categorize(
        d_extensor_values,
        d_flexor_values,
        buffer_input,
        MODEL_INPUT_WIDTH,
        MODEL_INPUT_HEIGHT);
}

/**
 * リングバッファの整列
 */
void ArrangeArray(
    volatile int raw_extensor_values[],
    volatile int raw_flexor_values[],
    volatile int ar_extensor_values[],
    volatile int ar_flexor_values[],
    volatile int begin_index,
    const int RAW_LENGTH)
{
    for (int i = 0; i < RAW_LENGTH; ++i)
    {
        int ring_array_index = begin_index + i;
        if (ring_array_index >= RAW_LENGTH)
        {
            ring_array_index -= RAW_LENGTH;
        }
        ar_extensor_values[i] = raw_extensor_values[ring_array_index];
        ar_flexor_values[i] = raw_flexor_values[ring_array_index];
    }
}

/**
 * 正規化（平均）
 */
void Normalization(
    volatile int ar_extensor_data[],
    volatile int ar_flexor_data[],
    volatile float n_extensor_data[],
    volatile float n_flexor_data[],
    float extensor_mean,
    float flexor_mean,
    const int RAW_LENGTH)
{
    for (int i = 0; i < RAW_LENGTH; ++i)
    {

        float f_extensor = (float)ar_extensor_data[i];
        float f_flexor = (float)ar_flexor_data[i];

        // 正規化
        n_extensor_data[i] = abs(f_extensor - extensor_mean);
        n_flexor_data[i] = abs(f_flexor - flexor_mean);
    }
}

/**
 * 正規化（0-1）（1つのみ）
 */
float _NormalizationZeroOne(float value)
{
    float n_value = (value - NORMALIZE_MIN) / (NORMALIZE_MAX - NORMALIZE_MIN);
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
 * 移動平均
 * {WINDOW_SIZE}に満たない前後半の{WINDOW_SIZE/2}は計算から除外しているため、
 * m_extensor(flexor)_dataの後半{WINDOW_SIZE}分は必ず0になる。
 */
void RollingAverage(
    volatile float n_extensor_data[],
    volatile float n_flexor_data[],
    volatile float ra_extensor_data[],
    volatile float ra_flexor_data[],
    const int RAW_LENGTH)
{
    float extensor_sum = 0;
    float flexor_sum = 0;
    int m_i = 0;

    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        extensor_sum += n_extensor_data[i];
        flexor_sum += n_flexor_data[i];
    }

    for (int i = WINDOW_SIZE; i < RAW_LENGTH; i++)
    {
        extensor_sum -= n_extensor_data[i - WINDOW_SIZE];
        extensor_sum += n_extensor_data[i];
        ra_extensor_data[m_i] = extensor_sum / WINDOW_SIZE;
        flexor_sum -= n_flexor_data[i - WINDOW_SIZE];
        flexor_sum += n_flexor_data[i];
        ra_flexor_data[m_i] = flexor_sum / WINDOW_SIZE;
        m_i += 1;
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
    const int sampled_length,
    const int step)
{
    int j = 0;
    for (int i = 0; i < (step * sampled_length); i += step)
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
        buffer_input[i] = 0.0;
    }

    for (int i = 0; i < input_width; ++i)
    {
        int e_index_ = _CategorizeIndex(d_extensor_values[i]);
        int f_index_ = _CategorizeIndex(d_flexor_values[i]);
        int base_index = i * input_height * 2;
        int e_index = base_index + (e_index_ * 2);
        int f_index = base_index + (f_index_ * 2) + 1;
        // sprintf(sp_s, "cnn_index: %f -> %d , %f -> %d", d_extensor_values[i], e_index, d_flexor_values[i], f_index);
        // Serial.println(sp_s);
        buffer_input[e_index] = 1.0;
        buffer_input[f_index] = 1.0;
    }
}

/**
 * カテゴライズIndexの取得
 */
int _CategorizeIndex(float value)
{
    int index = floor(value * MODEL_INPUT_HEIGHT);
    if (index >= MODEL_INPUT_HEIGHT)
    {
        index = MODEL_INPUT_HEIGHT - 1;
    }
    return index;
}

float Mean(
    volatile int data[],
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