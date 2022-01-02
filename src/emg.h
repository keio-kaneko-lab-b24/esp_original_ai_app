#ifndef EMG_H_
#define EMG_H_

const int EMG_LENGTH = 20;

// 共通：信号処理後の値
extern volatile float extensor_value;
extern volatile float flexor_value;

extern volatile int begin_index;

// Delsysから取得したRMS（約1回/150ms）
extern volatile float extensor_values[EMG_LENGTH];
extern volatile float flexor_values[EMG_LENGTH];

// 整列後のRMS（約1回/150ms）
extern volatile float ar_extensor_values[EMG_LENGTH];
extern volatile float ar_flexor_values[EMG_LENGTH];

// 信号処理後のRMS（約1回/150ms）
extern volatile float s_extensor_values[EMG_LENGTH];
extern volatile float s_flexor_values[EMG_LENGTH];

extern void updataRMSFromString(
    std::string value);

#endif // EMG_H_