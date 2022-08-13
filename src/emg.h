#ifndef EMG_H_
#define EMG_H_

#include "param.h"

const int RAW_EMG_LENGTH = (int)(1000 * 0.25);
const int BUFFER_SIZE = kChannleNumber * kModelInputHeight * kModelInputWidth;

// 共通：信号処理後の値
extern volatile float extensor_value;
extern volatile float flexor_value;

extern volatile int begin_index;

// 生筋電
extern volatile float raw_extensor_values[RAW_EMG_LENGTH];
extern volatile float raw_flexor_values[RAW_EMG_LENGTH];

// 整列後の生筋電
extern volatile float ar_extensor_values[RAW_EMG_LENGTH];
extern volatile float ar_flexor_values[RAW_EMG_LENGTH];

// 正規化後の生筋電
extern volatile float n_extensor_values[RAW_EMG_LENGTH];
extern volatile float n_flexor_values[RAW_EMG_LENGTH];

// 移動平均後の生筋電（=RMS）
extern volatile float ra_extensor_values[RAW_EMG_LENGTH];
extern volatile float ra_flexor_values[RAW_EMG_LENGTH];

// ダウンサンプリング後のRMS
extern volatile float d_extensor_values[kModelInputWidth];
extern volatile float d_flexor_values[kModelInputWidth];

// カテゴライズ後のバッファ
extern volatile float buffer_input[BUFFER_SIZE];

#endif // EMG_H_