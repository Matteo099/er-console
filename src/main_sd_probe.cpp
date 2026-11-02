#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <cstring>
#include "config/BoardConfig.h"

// Standalone, read-only probe; selected only by the sd-probe environment.
static bool bulkCommands = false;
static bool idleClocks = true;
static unsigned maxResponseClocks = 0;
static uint32_t suspectSector = UINT32_MAX;
static uint32_t rawSector = 0;

static uint16_t little16(const uint8_t *p)
{
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

static uint32_t little32(const uint8_t *p)
{
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

static bool readSector(uint32_t lba, uint8_t *buffer, const char *label)
{
    if (lba >= SD.numSectors())
    {
        Serial.printf("FAT PROBE invalid address %s LBA=%lu card_sectors=%lu\n",
                      label, (unsigned long)lba, (unsigned long)SD.numSectors());
        return false;
    }
    const bool ok = SD.readRAW(buffer, lba);
    Serial.printf("FAT PROBE %s LBA=%lu read=%s\n", label, (unsigned long)lba, ok ? "OK" : "FAILED");
    if (!ok && suspectSector == UINT32_MAX)
        suspectSector = lba;
    return ok;
}

static uint32_t findDirectory(uint32_t lba, uint8_t sectors, const char *name)
{
    uint8_t buffer[512];
    for (unsigned i = 0; i < sectors; ++i)
    {
        if (!readSector(lba + i, buffer, "directory"))
            return 0;
        for (unsigned offset = 0; offset < sizeof(buffer); offset += 32)
        {
            const uint8_t *entry = buffer + offset;
            if (entry[0] == 0)
                return 0;
            if (entry[0] == 0xe5 || entry[11] == 0x0f || !(entry[11] & 0x10))
                continue;
            if (std::memcmp(entry, name, 11) == 0)
                return (uint32_t(little16(entry + 20)) << 16) | little16(entry + 26);
        }
    }
    return 0;
}

static void checkGameDirectorySectors()
{
    uint8_t buffer[512];
    if (!readSector(0, buffer, "MBR"))
        return;
    const uint32_t partition = little32(buffer + 446 + 8);
    if (!partition || !readSector(partition, buffer, "partition boot"))
        return;
    if (little16(buffer + 11) != 512 || little16(buffer + 17) != 0 || little16(buffer + 22) != 0)
    {
        Serial.println("FAT PROBE: layout is not FAT32/512; skipping directory-sector probe");
        return;
    }
    const uint8_t sectors = buffer[13];
    const uint32_t fat = partition + little16(buffer + 14);
    const uint32_t data = fat + buffer[16] * little32(buffer + 36);
    const uint32_t root = little32(buffer + 44);
    if (!sectors || root < 2)
        return;
    const uint32_t games = findDirectory(data + (root - 2) * sectors, sectors, "GAMES      ");
    if (games < 2)
        return;
    const uint32_t lab = findDirectory(data + (games - 2) * sectors, sectors, "1-LAB      ");
    Serial.printf("FAT PROBE games_cluster=%lu lab_cluster=%lu sectors_per_cluster=%u\n",
                  (unsigned long)games, (unsigned long)lab, sectors);
    if (lab < 2)
        return;
    const uint32_t first = data + (lab - 2) * sectors;
    if (readSector(first, buffer, "/games/1-lab first sector"))
    {
        for (unsigned offset = 0; offset < sizeof(buffer) && buffer[offset]; offset += 32)
            if (buffer[offset] != 0xe5 && buffer[offset + 11] != 0x0f)
                Serial.printf("FAT PROBE lab entry: %.11s attr=%02x\n", buffer + offset, buffer[offset + 11]);
    }
    if (readSector(fat + lab / 128, buffer, "lab FAT link"))
        Serial.printf("FAT PROBE lab_next_cluster=%08lx\n", (unsigned long)(little32(buffer + (lab % 128) * 4) & 0x0fffffff));
}
static uint8_t commandCrc(const uint8_t *data)
{
    uint8_t crc = 0;
    for (unsigned i = 0; i < 5; ++i)
    {
        uint8_t value = data[i];
        for (unsigned bit = 0; bit < 8; ++bit)
        {
            crc <<= 1;
            if ((value ^ crc) & 0x80)
                crc ^= 0x09;
            value <<= 1;
        }
    }
    return (crc << 1) | 1;
}

static uint8_t sendCommand(SPIClass &bus, uint8_t cmd, uint32_t arg)
{
    uint8_t packet[] = {uint8_t(0x40 | cmd), uint8_t(arg >> 24),
                        uint8_t(arg >> 16), uint8_t(arg >> 8), uint8_t(arg), 0};
    packet[5] = commandCrc(packet);
    bus.transfer(0xff);
    if (bulkCommands)
        bus.writeBytes(packet, sizeof(packet));
    else
        for (uint8_t value : packet)
            bus.transfer(value);
    uint8_t response = 0xff;
    for (unsigned i = 0; i < 32; ++i)
    {
        response = bus.transfer(0xff);
        if (!(response & 0x80))
        {
            maxResponseClocks = max(maxResponseClocks, i + 1);
            break;
        }
    }
    return response;
}

static void deselect(SPIClass &bus)
{
    digitalWrite(SD_CS_PIN, HIGH);
    if (idleClocks)
        bus.transfer(0xff);
    bus.endTransaction();
}

static uint8_t rawCommand(SPIClass &bus, uint8_t cmd, uint32_t arg, uint32_t *extra = nullptr)
{
    bus.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
    digitalWrite(SD_CS_PIN, LOW);
    const uint8_t response = sendCommand(bus, cmd, arg);
    if (extra && response <= 1)
    {
        *extra = 0;
        for (unsigned i = 0; i < 4; ++i)
            *extra = (*extra << 8) | bus.transfer(0xff);
    }
    deselect(bus);
    return response;
}

static void rawProbe(SPIClass &bus, bool crcEnabled)
{
    maxResponseClocks = 0;
    Serial.printf("RAW PROBE CRC=%s bulk=%u idle_clocks=%u CMD17 LBA=%lu\n",
                  crcEnabled ? "on" : "off", bulkCommands, idleClocks, (unsigned long)rawSector);
    bus.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
    digitalWrite(SD_CS_PIN, HIGH);
    for (unsigned i = 0; i < 20; ++i)
        bus.transfer(0xff);
    bus.endTransaction();

    const uint8_t idle = rawCommand(bus, 0, 0);
    uint32_t extra = 0;
    const uint8_t cond = rawCommand(bus, 8, 0x1aa, &extra);
    Serial.printf("RAW CMD0=%02x CMD8=%02x R7=%08lx\n", idle, cond, (unsigned long)extra);
    if (idle != 1 || cond != 1 || (extra & 0xfff) != 0x1aa)
        return;
    Serial.printf("RAW CMD59=%02x\n", rawCommand(bus, 59, crcEnabled ? 1 : 0));
    uint8_t ready = 1;
    const uint32_t start = millis();
    while (ready == 1 && millis() - start < 2000)
    {
        rawCommand(bus, 55, 0);
        ready = rawCommand(bus, 41, 0x40000000);
        delay(10);
    }
    const uint8_t ocr = rawCommand(bus, 58, 0, &extra);
    Serial.printf("RAW ACMD41=%02x CMD58=%02x OCR=%08lx\n", ready, ocr, (unsigned long)extra);
    if (ready != 0 || ocr != 0)
        return;
    if (!(extra & (1UL << 30)))
        rawCommand(bus, 16, 512);

    bus.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
    digitalWrite(SD_CS_PIN, LOW);
    const uint8_t response = sendCommand(bus, 17, (extra & (1UL << 30)) ? rawSector : rawSector * 512);
    uint8_t token = 0xff;
    const uint32_t waitStart = millis();
    if (response == 0)
        do { token = bus.transfer(0xff); } while (token == 0xff && millis() - waitStart < 1000);
    if (response == 0 && token == 0xfe)
    {
        uint8_t sector[512];
        uint16_t checksum = 0;
        for (unsigned i = 0; i < sizeof(sector); ++i)
        {
            sector[i] = bus.transfer(0xff);
            checksum ^= uint16_t(sector[i]) << 8;
            for (unsigned bit = 0; bit < 8; ++bit)
                checksum = (checksum & 0x8000) ? (checksum << 1) ^ 0x1021 : checksum << 1;
        }
        uint16_t received = uint16_t(bus.transfer(0xff)) << 8;
        received |= bus.transfer(0xff);
        Serial.printf("RAW READ %s signature=%02x%02x CRC=%04x calculated=%04x max_R1_bytes=%u\n",
                      received == checksum ? "OK" : "CRC_MISMATCH",
                      sector[510], sector[511], received, checksum, maxResponseClocks);
    }
    else
        Serial.printf("RAW READ FAILED CMD17=%02x token=%02x max_R1_bytes=%u\n",
                      response, token, maxResponseClocks);
    deselect(bus);
}

static void checkFile(const char *path)
{
    uint8_t buffer[512];
    for (unsigned pass = 1; pass <= 2; ++pass)
    {
        File file = SD.open(path, FILE_READ);
        if (!file)
        {
            Serial.printf("PROBE file missing/unreadable: %s\n", path);
            return;
        }
        const size_t expected = file.size();
        size_t total = 0;
        uint32_t hash = 2166136261u;
        while (total < expected)
        {
            const size_t chunk = min(sizeof(buffer), expected - total);
            const size_t count = file.read(buffer, chunk);
            if (!count || count > chunk)
                break;
            for (size_t i = 0; i < count; ++i)
                hash = (hash ^ buffer[i]) * 16777619u;
            total += count;
        }
        file.close();
        Serial.printf("PROBE %s pass=%u bytes=%u/%u fnv1a=%08lx\n",
                      path, pass, (unsigned)total, (unsigned)expected, (unsigned long)hash);
    }
}

static void checkHost(uint8_t host, const char *name)
{
    SPIClass bus(host);
    bus.begin(SPI3_SCLK_PIN, SPI3_MISO_PIN, SPI3_MOSI_PIN, -1);
    const uint32_t frequencies[] = {400000, 1000000, 4000000};
    for (uint32_t frequency : frequencies)
    {
        Serial.printf("\nPROBE host=%s frequency=%lu SCK=%d MISO=%d MOSI=%d CS=%d\n",
                      name, (unsigned long)frequency,
                      SPI3_SCLK_PIN, SPI3_MISO_PIN, SPI3_MOSI_PIN, SD_CS_PIN);
        // Never format or write files, including after a mount failure.
        if (!SD.begin(SD_CS_PIN, bus, frequency, "/sd", 5, false))
            Serial.println("PROBE MOUNT FAILED");
        else
        {
            Serial.printf("PROBE MOUNT OK type=%u bytes=%llu\n", SD.cardType(), SD.cardSize());
            uint8_t sector[512];
            if (SD.readRAW(sector, 0))
                Serial.printf("PROBE sector0 signature=%02x%02x\n", sector[510], sector[511]);
            else
                Serial.println("PROBE sector0 read FAILED");
            checkFile("/.core/images/brand_negative.png");
            checkFile("/.core/images/logo.png");
            if (frequency == 400000)
                checkGameDirectorySectors();
        }
        SD.end();
        delay(100);
        if (suspectSector != UINT32_MAX)
            break;
    }
    for (bool bulk : {false, true})
        for (bool clocks : {true, false})
        {
            bulkCommands = bulk;
            idleClocks = clocks;
            rawProbe(bus, true);
        }
    if (suspectSector != UINT32_MAX)
    {
        rawSector = suspectSector;
        bulkCommands = false;
        idleClocks = true;
        rawProbe(bus, true);
    }
    bus.end();
}

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.printf("\nSTANDALONE SD PROBE built %s %s\n", __DATE__, __TIME__);
    for (int pin : {SD_CS_PIN, TFT_CS_PIN, TOUCH_CS_PIN})
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    checkHost(HSPI, "SPI3/HSPI");
    Serial.println("PROBE COMPLETE; no SD writes performed");
}

void loop()
{
    delay(1000);
}
