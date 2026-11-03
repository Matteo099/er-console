#include "LVGLImageProvider.h"
#include "drivers/bus/SPIBusLock.h"
#include "log/Logger.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include <SD.h>

static StorageSystem *storageService()
{
    return static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
}

static void* fs_open(lv_fs_drv_t*, const char* path, lv_fs_mode_t mode)
{
    SPIBusLock::Guard guard;

    if (!SystemEventBus::systemEvents ||
        !(xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY))
        return nullptr;
    auto *storage = static_cast<StorageSystem *>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (!storage || !storage->isReady())
        return nullptr;

    Log::debug(Log::DISPLAY_TAG, "LVGL loading image: %s", path);
    const bool write = (mode & LV_FS_MODE_WR) != 0;
    File file = storage->openNative(path, write);
    if (!file)
    {
        Log::warn(Log::DISPLAY_TAG, "LVGL image open failed: %s", path);
        return nullptr;
    }

    return new File(file);
}

static lv_fs_res_t fs_close(lv_fs_drv_t*, void* file_p)
{
    SPIBusLock::Guard guard;

    if (!file_p)
        return LV_FS_RES_INV_PARAM;

    File* f = static_cast<File*>(file_p);
    if (*f)
        f->close();
    delete f;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(
    lv_fs_drv_t*, void* file_p,
    void* buf, uint32_t btr, uint32_t* br)
{
    SPIBusLock::Guard guard;

    if (!file_p || !buf || !br)
        return LV_FS_RES_INV_PARAM;

    *br = 0;
    File* f = static_cast<File*>(file_p);
    if (!(*f))
        return LV_FS_RES_NOT_EX;

    const size_t size = f->size();
    const size_t pos = f->position();
    const size_t remaining = pos < size ? size - pos : 0;
    const size_t expected = btr < remaining ? btr : remaining;
    const size_t actual = expected ? f->read((uint8_t*)buf, expected) : 0;
    if (actual != expected)
    {
        if (auto *storage = storageService()) storage->noteIOFailure();
        Log::error(Log::DISPLAY_TAG, "SD short read: offset=%u expected=%u actual=%u",
                   (unsigned)pos, (unsigned)expected, (unsigned)actual);
        return LV_FS_RES_FS_ERR;
    }
    *br = actual;
    if (actual && pos + actual == size)
        if (auto *storage = storageService()) storage->noteIOSuccess();
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(
    lv_fs_drv_t*, void* file_p, uint32_t pos, lv_fs_whence_t whence)
{
    SPIBusLock::Guard guard;

    if (!file_p)
        return LV_FS_RES_INV_PARAM;

    File* f = static_cast<File*>(file_p);
    if (!(*f))
        return LV_FS_RES_NOT_EX;

    bool ok = false;
    switch (whence)
    {
    case LV_FS_SEEK_SET:
        ok = f->seek(pos, SeekSet);
        break;
    case LV_FS_SEEK_CUR:
        ok = f->seek(pos, SeekCur);
        break;
    case LV_FS_SEEK_END:
        ok = f->seek(pos, SeekEnd);
        break;
    default:
        return LV_FS_RES_INV_PARAM;
    }

    if (!ok)
        return LV_FS_RES_UNKNOWN;

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(
    lv_fs_drv_t*, void* file_p, uint32_t* pos)
{
    SPIBusLock::Guard guard;

    if (!file_p || !pos)
        return LV_FS_RES_INV_PARAM;

    File* f = static_cast<File*>(file_p);
    if (!(*f))
        return LV_FS_RES_NOT_EX;

    *pos = f->position();
    return LV_FS_RES_OK;
}

void LVGLImageProvider::init()
{
    Log::info(Log::DISPLAY_TAG, "Initializing LVGL image provider (SD card access)");
    static bool registered = false;
    if (registered)
        return;

    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);

    drv.letter = 'S';
    drv.open_cb = fs_open;
    drv.close_cb = fs_close;
    drv.read_cb = fs_read;
    drv.seek_cb = fs_seek;
    drv.tell_cb = fs_tell;

    lv_fs_drv_register(&drv);
    registered = true;
}
