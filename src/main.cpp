#include <Arduino.h>

#include "main.h"
#include "output_handler.h"
#include "motion.h"
#include "predictor.h"
#include "emg.h"
#include "input_handler.h"
#include "param_ml.h"
#include "signal_processor.h"
#include "model.h"
#include "param.h"
#include "NeuralNetwork.h"

NeuralNetwork *nn;

TaskHandle_t TaskIO;
TaskHandle_t TaskMain;

// セマフォ
SemaphoreHandle_t xMutex = NULL;

char main_s[64];
long last_sample_micros = 0;
long last_process_micros = 0;
long measured_time = 0;

// IOスレッド
void TaskIOcode(void *pvParameters)
{
  for (;;)
  {
    // 毎秒 {TARGET_HZ} 回実行される
    if ((micros() - last_sample_micros) < (1000 * 1000 / TARGET_HZ))
    {
      continue;
    }
    last_sample_micros = micros();

    // UpdateBLEConnection();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // ブロックが必要な処理
    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      begin_index += 1;
      if (begin_index >= RAW_EMG_LENGTH)
      {
        begin_index = 0;
      }

      HandleInput();

      xSemaphoreGive(xMutex);
    }
  }
};

// Mainスレッド
void TaskMaincode(void *pvParameters)
{
  for (;;)
  {
    // 毎秒 {PREDICT_HZ} 回実行される
    if ((micros() - last_process_micros) < (1000 * 1000 / PREDICT_HZ))
    {
      continue;
    }
    last_process_micros = micros();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // スレッドセーフな処理
    measured_time = micros();
    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      SignalProcess();

      xSemaphoreGive(xMutex);
    }

    // 直近のRMSが0.5以下の場合は、Restに判定
    float last_extensor = d_extensor_values[kModelInputWidth - 1];
    float last_flexor = d_flexor_values[kModelInputWidth - 1];
    if ((last_extensor <= 0.5) & (last_flexor <= 0.5))
    {
      motion motion = NONE;
      Serial.printf("threshold predict: paper\n");
      HandleOutput(motion);
      continue;
    }
    // 直近のRMSがExtensorだけ0.8を超えた場合は、Paperに判定
    if ((last_extensor >= 0.8) & (last_flexor < 0.5))
    {
      motion motion = PAPER;
      HandleOutput(motion);
      Serial.printf("threshold predict: paper\n");
      continue;
    }
    // 直近のRMSがFlexorだけ0.8を超えた場合は、Rockに判定
    if ((last_flexor >= 0.8) & (last_extensor < 0.5))
    {
      motion motion = ROCK;
      HandleOutput(motion);
      Serial.printf("threshold predict: rock\n");
      continue;
    }

    // 推論
    float *input_buffer = nn->getInputBuffer();
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      input_buffer[i] = buffer_input[i];
    }
    float *result = nn->predict();
    Serial.printf("%.2f, %.2f, %.2f\n", result[0], result[1], result[2]);

    // 判定
    motion motion = NONE;
    motion = PredictML(result[0], result[1], result[2]);

    // ロボットへ出力
    HandleOutput(motion);

    // 推論時間
    // Serial.printf("推論時間 = %ld micro sec\n", micros() - measured_time);
  }
};

void setup()
{
  delay(3000);
  Serial.begin(115200);

  OutputSetup();

  nn = new NeuralNetwork();

  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(TaskIOcode, "TaskIO", 4096, NULL, 2, &TaskIO, 0); // Task1実行
  delay(500);
  xTaskCreatePinnedToCore(TaskMaincode, "TaskMain", 4096, NULL, 2, &TaskMain, 1); // Task2実行
  delay(500);

  Serial.println("[setup] finished.");
}

void loop()
{
  // loopは使用しないので、削除する
  vTaskDelete(NULL);
}