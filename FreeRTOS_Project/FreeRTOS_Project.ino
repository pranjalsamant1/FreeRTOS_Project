#include <Arduino.h>

// Define correct pins here.
#define LED_T1_PIN        4
#define FREQ_IN_T2_PIN    14
#define FREQ_IN_T3_PIN    25
#define LED_T4_PIN        2
#define POT_PIN           15
#define SWITCH_T6_PIN     27
#define LED_T6_PIN        26

// Data for RTOS (Shared)

typedef struct {
  float freq_t2_hz;
  float freq_t3_hz;
} SharedFreqs;

static SharedFreqs gFreqs = {0.0f, 0.0f};
static SemaphoreHandle_t gFreqMutex = nullptr;

static QueueHandle_t gBtnQueue = nullptr;

// Debug Helpers
static bool waitLevelTimeout(uint8_t pin, uint8_t level, uint32_t timeout_us) {
  const uint32_t t0 = micros();
  while (digitalRead(pin) != level) {
    if ((uint32_t)(micros() - t0) >= timeout_us) return false;
    taskYIELD();
  }
  return true;
}

static float measureFreqRisingToRising(uint8_t pin, uint32_t timeout_us) {
  if (!waitLevelTimeout(pin, LOW, timeout_us))  return 0.0f;
  if (!waitLevelTimeout(pin, HIGH, timeout_us)) return 0.0f;

  const uint32_t t1 = micros();

  if (!waitLevelTimeout(pin, LOW, timeout_us))  return 0.0f;
  if (!waitLevelTimeout(pin, HIGH, timeout_us)) return 0.0f;

  const uint32_t t2 = micros();
  const uint32_t dt = (uint32_t)(t2 - t1);
  if (dt == 0) return 0.0f;

  return 1000000.0f / (float)dt;
}

// Task1
void task1(void *pvParameters) {
  (void)pvParameters;

  while (1) {
    digitalWrite(LED_T1_PIN, HIGH);
    delayMicroseconds(200);

    digitalWrite(LED_T1_PIN, LOW);
    delayMicroseconds(50);

    digitalWrite(LED_T1_PIN, HIGH);
    delayMicroseconds(30);

    digitalWrite(LED_T1_PIN, LOW);

    vTaskDelay(pdMS_TO_TICKS(4));
  }
}

// Task2
void task2(void *pvParameters) {
  (void)pvParameters;

  while (1) {
    const float f = measureFreqRisingToRising(FREQ_IN_T2_PIN, 50000);

    xSemaphoreTake(gFreqMutex, portMAX_DELAY);
    gFreqs.freq_t2_hz = f;
    xSemaphoreGive(gFreqMutex);

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task3
void task3(void *pvParameters) {
  (void)pvParameters;

  while (1) {
    const float f = measureFreqRisingToRising(FREQ_IN_T3_PIN, 50000);

    xSemaphoreTake(gFreqMutex, portMAX_DELAY);
    gFreqs.freq_t3_hz = f;
    xSemaphoreGive(gFreqMutex);

    vTaskDelay(pdMS_TO_TICKS(8));
  }
}

// Task4
void task4(void *pvParameters) {
  (void)pvParameters;

  const float half = 3.3f * 0.5f;
  float z1 = 0, z2 = 0, z3 = 0, z4 = 0;

  while (1) {
    const int adc = analogRead(POT_PIN);
    const float v = (3.3f * (float)adc) / 4095.0f;

    z1 = z2; z2 = z3; z3 = z4; z4 = v;
    const float avg = (z1 + z2 + z3 + z4) * 0.25f;

    digitalWrite(LED_T4_PIN, (avg > half) ? HIGH : LOW);

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task5
void task5(void *pvParameters) {
  (void)pvParameters;

  while (1) {
    float f2, f3;

    xSemaphoreTake(gFreqMutex, portMAX_DELAY);
    f2 = gFreqs.freq_t2_hz;
    f3 = gFreqs.freq_t3_hz;
    xSemaphoreGive(gFreqMutex);

    
    float s2 = ((f2 - 333.0f) * 99.0f) / (1000.0f - 333.0f);
    float s3 = ((f3 - 500.0f) * 99.0f) / (1000.0f - 500.0f);

    int out2 = (int)s2;
    int out3 = (int)s3;

    if (out2 < 0) out2 = 0; else if (out2 > 99) out2 = 99;
    if (out3 < 0) out3 = 0; else if (out3 > 99) out3 = 99;

    Serial.printf("%d, %d\r\n", out2, out3);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task6
void task6(void *parameter) {
  (void)parameter;

  bool prev = digitalRead(SWITCH_T6_PIN);

  while (1) {
    bool cur = digitalRead(SWITCH_T6_PIN);

    if (cur != prev) {
      delay(5);
      cur = digitalRead(SWITCH_T6_PIN);
    }

    if (cur != prev) {
      prev = cur;
      if (cur == LOW) {
        uint8_t ev = 1;
        xQueueSend(gBtnQueue, &ev, portMAX_DELAY);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task7
void task7(void *parameter) {
  (void)parameter;

  uint8_t ev;
  bool ledOn = false;

  while (1) {
    if (xQueueReceive(gBtnQueue, &ev, portMAX_DELAY) == pdPASS) {
      ledOn = !ledOn;
      digitalWrite(LED_T6_PIN, ledOn ? HIGH : LOW);
    }
  }
}

// Steup and loop below.
void setup() {
  Serial.begin(9600);

  pinMode(LED_T1_PIN, OUTPUT);
  pinMode(FREQ_IN_T2_PIN, INPUT);
  pinMode(FREQ_IN_T3_PIN, INPUT);
  pinMode(LED_T4_PIN, OUTPUT);

  pinMode(SWITCH_T6_PIN, INPUT_PULLUP);
  pinMode(LED_T6_PIN, OUTPUT);

  gFreqMutex = xSemaphoreCreateMutex();
  gBtnQueue  = xQueueCreate(5, sizeof(uint8_t));

  xTaskCreatePinnedToCore(task1, "Task1", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task2, "Task2", 2048, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task3, "Task3", 2048, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task4, "Task4", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task5, "Task5", 2048, NULL, 0, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task6, "Task6", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(task7, "Task7", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
