#ifndef SIGNAL_PROCESSOR_H_
#define SIGNAL_PROCESSOR_H_

extern void SignalProcess();

extern void Normalize(
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile float s_extensor_values[],
    volatile float s_flexor_values[],
    const int value_length);

extern float _Normalize(float value);

extern void ArrangeArray(
    volatile float extensor_values[],
    volatile float flexor_values[],
    volatile float ar_extensor_values[],
    volatile float ar_flexor_values[],
    volatile int begin_index,
    const int value_length);

#endif // SIGNAL_PROCESSOR_H_