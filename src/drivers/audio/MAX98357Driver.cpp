#include "MAX98357Driver.h"

#include <Arduino.h>

#include "config/BoardConfig.h"
#include "log/Logger.h"

MAX98357Driver::MAX98357Driver(int bclk, int lrck, int dout) : pinBCLK(bclk), pinLRCK(lrck), pinDOUT(dout) {}

bool MAX98357Driver::init(uint32_t sampleRate)
{
    pinMode(I2S_SHUTDOWN_PIN, OUTPUT);
    digitalWrite(I2S_SHUTDOWN_PIN, LOW);
    Log::info(Log::DRIVER_TAG, "Initializing MAX98357 (BCLK=%d LRCK=%d DOUT=%d SR=%lu)", pinBCLK, pinLRCK, pinDOUT, sampleRate);
    i2s_config_t config = {
        .mode = static_cast<i2s_mode_t>(
            I2S_MODE_MASTER | I2S_MODE_TX),

        .sample_rate = sampleRate,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_STAND_I2S,

        .intr_alloc_flags =
            ESP_INTR_FLAG_LEVEL1,

        .dma_buf_count = 8,

        .dma_buf_len = 64,

        .use_apll = false,

#if ESP_IDF_VERSION_MAJOR >= 4
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
#endif
    };

    i2s_pin_config_t pins = {
        .bck_io_num = pinBCLK,
        .ws_io_num = pinLRCK,
        .data_out_num = pinDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};
    esp_err_t err;
    err = i2s_driver_install(I2S_PORT, &config, 0, nullptr);
    if (err != ESP_OK)
    {
        Log::error(Log::DRIVER_TAG, "Failed installing I2S driver (%d)", err);
        return false;
    }
    err = i2s_set_pin(I2S_PORT, &pins);
    if (err != ESP_OK)
    {
        Log::error(Log::DRIVER_TAG, "Failed configuring I2S pins (%d)", err);
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }
    initialized = true;
    return true;
}

bool MAX98357Driver::write(const int16_t *samples, size_t sampleCount)
{
    if (!initialized)
        return false;
    if (!samples)
        return false;
    if (sampleCount == 0)
        return true;
    size_t bytesWritten = 0;
    esp_err_t err = i2s_write(I2S_PORT, samples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    return err == ESP_OK;
}

void MAX98357Driver::clear()
{
    if (!initialized)
        return;
    i2s_zero_dma_buffer(I2S_PORT);
}

void MAX98357Driver::setEnabled(bool enabled)
{
    digitalWrite(I2S_SHUTDOWN_PIN, enabled ? HIGH : LOW);
}