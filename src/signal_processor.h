#ifndef SIGNAL_PROCESSOR_H_
#define SIGNAL_PROCESSOR_H_

extern void SignalProcess();

extern void ArrangeArray(
    volatile float raw_extensor_values[],
    volatile float raw_flexor_values[],
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile int begin_index,
    const int value_length);

extern void Normalization(
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile float n_extensor_values[],
    volatile float n_flexor_values[],
    float extensor_mean,
    float flexor_mean,
    const int r_length);

extern float _NormalizationZeroOne(float value);

extern void RollingAverage(
    volatile float n_extensor_values[],
    volatile float n_flexor_values[],
    volatile float ra_extensor_values[],
    volatile float ra_flexor_values[],
    const int r_length);

extern void DownSample(
    volatile float ra_extensor_values[],
    volatile float ra_flexor_values[],
    volatile float d_extensor_values[],
    volatile float d_flexor_values[],
    const int original_length,
    const int sampled_length);

extern void Categorize(
    volatile float d_extensor_values[],
    volatile float d_flexor_values[],
    volatile float buffer_input[],
    const int input_width,
    const int input_height);

extern int _CategorizeIndex(float value);

extern float Max(
    float data[],
    int length);

extern float Min(
    float data[],
    int length);

extern int Max(
    int data[],
    int length);

extern int Min(
    int data[],
    int length);

extern float Mean(
    volatile float data[],
    int length);

#endif // SIGNAL_PROCESSOR_H_