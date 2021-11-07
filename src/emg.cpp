#include <Arduino.h>

#include "emg.h"
#include "param_ml.h"

volatile float extensor_value = 0;
volatile float flexor_value = 0;

// RMS値を更新する
// @value （例）"0.012345, F: 0.056789, E5"
void updataRMSFromString(
    std::string value)
{
    size_t pos = 0;
    std::string delimiter = ",";

    // TODO: もっとよい書き方
    // ExtensorのRMS値を取得
    while ((pos = value.find(delimiter)) != std::string::npos)
    {
        std::string e_token = value.substr(3, 100); // 多めに100文字目まで取得
        const char *e_str = e_token.c_str();
        extensor_value = atof(e_str);
        value.erase(0, pos + delimiter.length());
        break;
    }

    // FlexorのRMS値を取得
    while ((pos = value.find(delimiter)) != std::string::npos)
    {
        std::string f_token = value.substr(4, 100); // 多めに100文字目まで取得
        const char *f_str = f_token.c_str();
        flexor_value = atof(f_str);
        value.erase(0, pos + delimiter.length());
        break;
    }
}