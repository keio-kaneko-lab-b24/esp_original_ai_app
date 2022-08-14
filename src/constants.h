#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// サンプリング周波数
constexpr int kTargetHz = 100;

// 推定頻度
constexpr int kPredictHz = 10;

// 配列長
const float kNeedsTimeSec = 2.5;                       // 入力配列の時間
const int r_length = (int)(kTargetHz * kNeedsTimeSec); // 入力配列の長さ

#endif // CONSTANTS_H_