#include "MAX98357Driver.h"
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config/BoardConfig.h"

MAX98357Driver::MAX98357Driver(int bclk, int lrck, int dout)
    : pinBCLK(bclk), pinLRCK(lrck), pinDOUT(dout)
{
}

bool MAX98357Driver::init()
{
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false};

    i2s_pin_config_t pins = {
        .bck_io_num = pinBCLK,
        .ws_io_num = pinLRCK,
        .data_out_num = pinDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};

    i2s_driver_install(I2S_PORT, &cfg, 0, nullptr);
    i2s_set_pin(I2S_PORT, &pins);

    return true;
}

void MAX98357Driver::playTone(uint16_t freq, uint16_t ms)
{
    const int samples = SAMPLE_RATE * ms / 1000;
    int16_t buffer[256];

    for (int i = 0; i < samples; i += 256)
    {
        for (int j = 0; j < 256; ++j)
        {
            float t = (float)(i + j) / SAMPLE_RATE;
            buffer[j] = (int16_t)(sinf(2 * M_PI * freq * t) * 20000);
        }

        size_t written;
        i2s_write(I2S_PORT, buffer, sizeof(buffer), &written, portMAX_DELAY);
    }
}

void MAX98357Driver::playWav(IFile* file)
{
    if (!file)
        return;

    file->seek(44); // WAV header

    int16_t buffer[256];

    while (file->available())
    {
        int read = file->read((uint8_t*)buffer, sizeof(buffer));
        if (read <= 0)
            break;

        applyVolume(buffer, read);
        i2s_write(I2S_PORT, buffer, read, nullptr, portMAX_DELAY);
    }

    file->close();
    delete file;
}

void MAX98357Driver::applyVolume(int16_t* buffer, size_t samples)
{
    if (!buffer || currentVolume == 100) return;  // full volume, no change

    float factor = currentVolume / 100.0f;

    for (size_t i = 0; i < samples; ++i)
    {
        // multiply sample by volume factor
        int32_t temp = static_cast<int32_t>(buffer[i] * factor);

        // clamp to int16_t range
        if (temp > 32767) temp = 32767;
        else if (temp < -32768) temp = -32768;

        buffer[i] = static_cast<int16_t>(temp);
    }
}

void MAX98357Driver::stop()
{
    i2s_zero_dma_buffer(I2S_PORT);
}
