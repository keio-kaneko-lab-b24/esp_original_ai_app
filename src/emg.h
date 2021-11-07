#ifndef EMG_H_
#define EMG_H_

// 共通：信号処理後の値
extern volatile float extensor_value;
extern volatile float flexor_value;

extern void updataRMSFromString(
    std::string value);

#endif // EMG_H_