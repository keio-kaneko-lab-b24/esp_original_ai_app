#include <Arduino.h>

#include "main.h"
#include "ble.h"
#include "output_handler.h"
#include "motion.h"
#include "predictor.h"
#include "emg.h"
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

// IOスレッド
void TaskIOcode(void *pvParameters)
{
  for (;;)
  {
    // 1000Hz
    if ((micros() - last_sample_micros) < 1 * 1000)
    {
      continue;
    }
    last_sample_micros = micros();

    UpdateBLEConnection();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // スレッドセーフな処理
    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      xSemaphoreGive(xMutex);
    }
  }
};

// Mainスレッド
void TaskMaincode(void *pvParameters)
{
  for (;;)
  {
    // 10Hz
    if ((micros() - last_process_micros) < 500 * 10000)
    {
      continue;
    }
    last_process_micros = micros();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // スレッドセーフな処理
    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      SignalProcess();

      xSemaphoreGive(xMutex);
    }

    // 閾値判定
    motion motion = NONE;
    motion = PredictThreshold(
        extensor_value,
        flexor_value,
        rock_flexor_lower_limit,
        rock_extensor_upper_limit,
        paper_extensor_lower_limit,
        paper_flexor_upper_limit);

    // ロボットへ出力
    HandleOutput(motion);

    // float buffer_input[BUFFER_SIZE] = {
    //     1., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.,
    //     0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0.,
    //     0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
    //     0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.};

    float *input_buffer = nn->getInputBuffer();
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      input_buffer[i] = buffer_input[i];
    }

    float *result = nn->predict();
    Serial.printf("%.2f, %.2f, %.2f\n", result[0], result[1], result[2]);
  }
};

void setup()
{
  delay(3000);
  Serial.begin(115200);

  SetUpBLE();

  OutputSetup();

  nn = new NeuralNetwork();

  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(TaskIOcode, "TaskIO", 4096, NULL, 2, &TaskIO, 0); //Task1実行
  delay(500);
  xTaskCreatePinnedToCore(TaskMaincode, "TaskMain", 4096, NULL, 2, &TaskMain, 1); //Task2実行
  delay(500);

  Serial.println("[setup] finished.");
}

void loop()
{
  // loopは使用しないので、削除する
  vTaskDelete(NULL);
}