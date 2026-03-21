#include "LVGLImageProvider.h"
#include "log/Logger.h"
#include <SD.h>

static void* fs_open(lv_fs_drv_t*, const char* path, lv_fs_mode_t)
{
    Log::debug(Log::DISPLAY_TAG, "LVGL loading image: %s", path);
    return new File(SD.open(path));
}

static lv_fs_res_t fs_close(lv_fs_drv_t*, void* file_p)
{
    delete static_cast<File*>(file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(
    lv_fs_drv_t*, void* file_p,
    void* buf, uint32_t btr, uint32_t* br)
{
    File* f = static_cast<File*>(file_p);
    *br = f->read((uint8_t*)buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(
    lv_fs_drv_t*, void* file_p, uint32_t pos, lv_fs_whence_t)
{
    static_cast<File*>(file_p)->seek(pos);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(
    lv_fs_drv_t*, void* file_p, uint32_t* pos)
{
    *pos = static_cast<File*>(file_p)->position();
    return LV_FS_RES_OK;
}

void LVGLImageProvider::init()
{
    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);

    drv.letter = 'S';
    drv.open_cb = fs_open;
    drv.close_cb = fs_close;
    drv.read_cb = fs_read;
    drv.seek_cb = fs_seek;
    drv.tell_cb = fs_tell;

    lv_fs_drv_register(&drv);
}
