#ifndef EMG_H_
#define EMG_H_

#include "param.h"

// サンプリング周波数（1秒間に何回筋電位を取得するか）
constexpr int TARGET_HZ = 1000;
// 判定頻度（1秒間に何回判定するか）
constexpr int PREDICT_HZ = 10;
// 入力配列の時間（何秒間分を判定に使用するか）
const float NEEDS_TIME_SEC = 0.25;
// 入力配列の要素数（直近{RAW_LENGTH}個分の筋電を常に保持しておく）
const int RAW_EMG_LENGTH = (int)(TARGET_HZ * NEEDS_TIME_SEC);

// 移動平均に使用する窓枠サイズ
// 注意：{RAW_LENGTH}より小さい必要がある
const int WINDOW_SIZE = 100;

// ML入力の要素数
const int BUFFER_SIZE = kChannleNumber * kModelInputHeight * kModelInputWidth;

// 共通：信号処理後の値
extern volatile float extensor_value;
extern volatile float flexor_value;

extern volatile int begin_index;

// 生筋電
extern volatile int raw_extensor_values[RAW_EMG_LENGTH];
extern volatile int raw_flexor_values[RAW_EMG_LENGTH];

// 整列後の生筋電
extern volatile int ar_extensor_values[RAW_EMG_LENGTH];
extern volatile int ar_flexor_values[RAW_EMG_LENGTH];

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