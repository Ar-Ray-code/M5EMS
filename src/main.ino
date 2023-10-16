#include "M5Dial.h"

#define UNLOCK_REQUIRED_STEPS 1
bool emergency_flag = false;
int old_position = 0;
long newPosition = 0;

void emergencySound()
{
    M5Dial.Speaker.tone(10000, 20);
    delay(20);
    M5Dial.Speaker.tone(4000, 20);
    delay(20);
}

void showStatus(bool _emergency_flag)
{
    int text_offset = 0;
    int textsize = 1;
    int background_color;
    int text_color;
    String text;

    if (_emergency_flag)
    {
        background_color = TFT_BLACK;
        text_color = TFT_WHITE;
        text_offset = -80;
        text = "UNLOCK";
    }
    else
    {
        background_color = TFT_RED;
        text_color = TFT_WHITE;
        text_offset = -50;
        text = "STOP";
    }
    M5Dial.Display.fillScreen(background_color);
    M5Dial.Display.setTextColor(text_color);
    M5Dial.Display.setTextSize(textsize);
    M5Dial.Display.setTextDatum(MC_DATUM);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setCursor(M5Dial.Display.width() / 2 + text_offset, M5Dial.Display.height() / 2);
    M5Dial.Display.println(text);
}

void encReadTask(void *pvParameters)
{
    while (1)
    {
        newPosition = M5Dial.Encoder.read();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void outputStatusTask(void *pvParameters)
{
    while (1)
    {
        digitalWrite(GPIO_NUM_2, !emergency_flag);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void updateStatusTask(void *pvParameters)
{
    while (1)
    {
        M5Dial.update();
        // Encoder
        if (emergency_flag)
        {
            if (abs(newPosition - old_position) > UNLOCK_REQUIRED_STEPS)
            {
                emergency_flag = false;
                showStatus(emergency_flag);
            }
        }
        else
        {
            old_position = newPosition;
        }

        // Button
        if (M5Dial.BtnA.wasPressed())
        {
            emergency_flag = true;
            showStatus(emergency_flag);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void buzzerTask(void *pvParameters)
{
    while (1)
    {
        if (emergency_flag)
        {
            emergencySound();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, true);

    pinMode(GPIO_NUM_2, OUTPUT);
    showStatus(emergency_flag);

    xTaskCreatePinnedToCore(encReadTask, "encReadTask", 10000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(updateStatusTask, "updateStatusTask", 10000, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(outputStatusTask, "outputStatusTask", 10000, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(buzzerTask, "buzzerTask", 10000, NULL, 3, NULL, 1);
}

void loop() {}
