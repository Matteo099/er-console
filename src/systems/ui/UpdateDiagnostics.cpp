#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL>0
#include "UpdateDiagnostics.h"
#include "systems/update/UpdateEngine.h"
#include "systems/update/UpdateMetadata.h"
#include "systems/update/UpdateSystem.h"
#include <esp_ota_ops.h>
#include "config/Updates.h"
#include "scenes/update/UpdateView.h"
#include "systems/ui/Scene.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/UIFeedback.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <esp_app_format.h>
#include <mbedtls/sha256.h>
#include <algorithm>
#include <cstring>

namespace
{
    std::string digest(const std::vector<uint8_t> &data)
    {
        uint8_t hash[32]; mbedtls_sha256_ret(data.data(),data.size(),hash,0);
        char value[65]{}; for (unsigned i=0;i<32;++i) snprintf(value+i*2,3,"%02x",hash[i]); return value;
    }
    std::vector<uint8_t> image()
    {
        std::vector<uint8_t> data(8192,0x5a);
        esp_image_header_t header{}; header.magic=ESP_IMAGE_HEADER_MAGIC; header.segment_count=2; header.chip_id=ESP_CHIP_ID_ESP32S3;
        esp_app_desc_t description{}; description.magic_word=ESP_APP_DESC_MAGIC_WORD;
        std::memcpy(data.data(),&header,sizeof(header));
        std::memcpy(data.data()+sizeof(header)+sizeof(esp_image_segment_header_t),&description,sizeof(description));
        return data;
    }
    struct Input : UpdateInput
    {
        const std::vector<uint8_t> &data;
        size_t position=0,limit=SIZE_MAX,chunk=211;
        int64_t advertised=-1;
        bool framed=true;
        explicit Input(const std::vector<uint8_t> &value) : data(value) {}
        int read(uint8_t *out,size_t size) override
        {
            if (position>=limit) return -1;
            size=std::min(size,std::min(chunk,data.size()-position));
            if (size) std::memcpy(out,data.data()+position,size);
            position+=size; return size;
        }
        int64_t contentLength() const override { return advertised; }
        bool complete() const override { return framed && position==data.size(); }
    };
    struct Flash : UpdateFlash
    {
        unsigned begins=0,finishes=0,aborts=0;
        size_t written=0,failWrite=SIZE_MAX;
        bool failBegin=false,failFinish=false,failActivate=false,activated=false;
        bool begin(size_t) override { ++begins; return !failBegin; }
        bool write(const uint8_t *,size_t size) override { if (written>=failWrite) return false; written+=size; return true; }
        bool finish() override { ++finishes; return !failFinish; }
        bool activate(const std::string &) override { activated=!failActivate; return activated; }
        void abort() override { ++aborts; }
    };
    std::string releaseJson()
    {
        return R"({"tag_name":"v1.2.3","name":"Firmware 1.2.3","body":"Release notes","published_at":"2026-11-10","draft":false,"prerelease":false,"assets":[{"id":11,"name":"manifest.json","size":1000,"state":"uploaded"},{"id":12,"name":"firmware.bin","size":8192,"state":"uploaded"}]})";
    }
    UpdateError release(std::string value,UpdateRelease &out) { return UpdateMetadata::parseRelease(&value[0],value.size(),out); }
    std::string manifestJson(const std::string &hash)
    {
        JsonDocument value;
        value["schema_version"]=1; value["version"]="1.2.3"; value["channel"]="stable"; value["dirty"]=false;
        value["hardware"]=OTA_HARDWARE_ID; value["platformio_environment"]=OTA_HARDWARE_ID;
        value["firmware_type"]="application"; value["partition_table_sha256"]=OTA_PARTITION_TABLE_SHA256;
        value["ota_slot_size"]=6553600;
        value["files"]["firmware"]["file"]="firmware.bin"; value["files"]["firmware"]["size"]=8192;
        value["files"]["firmware"]["sha256"]=hash;
        std::string result; serializeJson(value,result); return result;
    }
    UpdateError manifest(std::string value,UpdateRelease &out,const char *hardware=OTA_HARDWARE_ID,size_t slot=6553600)
    { return UpdateMetadata::parseManifest(&value[0],value.size(),out,hardware,OTA_PARTITION_TABLE_SHA256,slot); }
    lv_obj_t *button(lv_obj_t *root,const char *caption)
    {
        if (!lv_obj_is_visible(root)) return nullptr;
        for (int i=int(lv_obj_get_child_count(root))-1;i>=0;--i)
            if (auto *found=button(lv_obj_get_child(root,i),caption)) return found;
        if (lv_obj_check_type(root,&lv_label_class) && !std::strcmp(lv_label_get_text(root),caption) &&
            lv_obj_check_type(lv_obj_get_parent(root),&lv_button_class)) return lv_obj_get_parent(root);
        return nullptr;
    }
    lv_obj_t *textarea(lv_obj_t *root)
    {
        if (!lv_obj_is_visible(root)) return nullptr;
        if (lv_obj_check_type(root,&lv_textarea_class)) return root;
        for (unsigned i=0;i<lv_obj_get_child_count(root);++i)
            if (auto *found=textarea(lv_obj_get_child(root,i))) return found;
        return nullptr;
    }
    struct Capture : UIEventHandler
    {
        unsigned installs=0;
        bool handleUIEvent(const UIEvent &event) override
        {
            if (event.getType()==ButtonClickedEvent::TYPE && static_cast<const ButtonClickedEvent &>(event).buttonId=="confirm_install") ++installs;
            return true;
        }
    };
    UpdateSnapshot preview(unsigned mode)
    {
        UpdateSnapshot value;
        value.release.version="1.2.3"; value.release.title=tr(Text::OtaConfirmTitle);
        value.release.notes="Firmware, UI, audio and storage improvements.\nSHA-256 verified before activation.";
        value.release.firmwareId=12; value.release.size=value.total=8192;
        value.state=mode==1 ? UpdateState::Downloading : mode==2 ? UpdateState::Error : mode==3 ? UpdateState::Installing:UpdateState::Available;
        value.error=mode==2 ? UpdateError::Integrity:UpdateError::None;
        value.downloaded=mode ? 4096:0; value.written=mode ? 3800:0;
        return value;
    }
    class Preview : public Scene
    {
    public:
        explicit Preview(unsigned mode) : state(preview(mode)) {}
        void onEnter() override { view.setEventHandler(this); view.create(); view.render(state); }
        void onUpdate() override {}
        void onExit() override { view.destroy(); }
        void onPreferencesChanged() override { view.render(state); }
        View &getView() override { return view; }
        SceneRequest getRequest() const override { return request; }
        bool handleUIEvent(const UIEvent &event) override
        {
            if (event.getType()!=ButtonClickedEvent::TYPE) return true;
            const auto &id=static_cast<const ButtonClickedEvent &>(event).buttonId;
            if (id=="back") request=SceneRequest(SceneChange::GoToSettingsSystem,SceneTransition::SlideRight);
            else if (id=="install") view.askInstall(state.release.version);
            else if (id=="confirm_install") { state=preview(1); view.render(state); }
            else if (id=="check" && state.busy()) { state=preview(0); state.state=UpdateState::Cancelled; view.render(state); }
            return true;
        }
    private:
        UpdateView view; UpdateSnapshot state; SceneRequest request;
    };
}
Scene *createUpdatePreview(unsigned mode) { return new Preview(mode); }
void runNativeUpdateFlashDiagnostics(size_t size,const char *sha256)
{
    if (size<288 || size>6553600 || !sha256 || strlen(sha256)!=64) { Serial.println("@OTANATIVE rejected"); return; }
    for (unsigned i=0;i<64;++i) if (!isxdigit(sha256[i])) { Serial.println("@OTANATIVE rejected"); return; }
    class SerialInput : public UpdateInput
    {
    public:
        explicit SerialInput(size_t count) : total(count) {}
        int read(uint8_t *data,size_t capacity) override
        {
            if (position==total) return 0;
            const auto requested=std::min<size_t>(capacity,total-position);
            Serial.printf("@OTARX %u\n",unsigned(requested)); Serial.flush();
            const auto received=Serial.readBytes(data,requested); position+=received;
            if (received!=requested) broken=true;
            return received ? received:-1;
        }
        bool complete() const override { return !broken && position==total; }
        int64_t contentLength() const override { return total; }
    private:
        size_t position=0,total;
        bool broken=false;
    } input(size);
    const auto *before=esp_ota_get_boot_partition(); const auto address=before ? before->address:UINT32_MAX;
    UpdateRelease release; release.version="diagnostic-only"; release.sha256=sha256; release.size=size;
    const auto timeout=Serial.getTimeout(); Serial.setTimeout(15000);
    Serial.printf("@OTANATIVE READY baud=921600 size=%u\n",unsigned(size)); Serial.flush();
    Serial.updateBaudRate(921600); vTaskDelay(pdMS_TO_TICKS(150));
    const auto result=UpdateSystem::instance().validateInactiveForDiagnostics(input,release);
    const auto *after=esp_ota_get_boot_partition();
    Serial.printf("@OTANATIVE result=%u unchanged=%u bytes=%u\n",unsigned(result),unsigned(after && after->address==address),unsigned(size));
    Serial.flush(); Serial.updateBaudRate(115200); Serial.setTimeout(timeout); vTaskDelay(pdMS_TO_TICKS(150));
}
void runUpdateDiagnostics()
{
    unsigned passed=0,failed=0;
    const auto check=[&](bool okay,const char *name) { okay ? ++passed:++failed; Serial.printf("@OTACHECK %s %s\n",okay ? "PASS":"FAIL",name); };
    using namespace UpdateMetadata;
    check(versionValid("1.2.3") && versionValid("v1.2.3-dev.2+sha.abc") && !versionValid("1.02.3") && !versionValid("1.2.3-dev.01"),"strict SemVer validates installed and release versions");
    check(compareVersions("1.10.0","1.9.9")>0 && compareVersions("2.0.0","1.99.99")>0,"version comparison is numeric, not lexical");
    check(compareVersions("1.2.3","1.2.3-dev.1")>0 && compareVersions("1.2.3-dev.10","1.2.3-dev.2")>0 && compareVersions("1.2.3+a","1.2.3+b")==0,"prerelease precedence and ignored build metadata match SemVer");
    check(!trustedDownloadUrl("http://api.github.com/data") && !trustedDownloadUrl("https://api.github.com.evil.invalid/data") && !trustedDownloadUrl("https://api.github.com@evil.invalid/data"),"redirects reject plaintext, lookalike hosts and userinfo tricks");
    check(trustedDownloadUrl("https://release-assets.githubusercontent.com/data?signature=example") && trustedDownloadUrl("https://api.github.com/repos/example/repo/releases"),"public GitHub API and release CDN downloads are allowed");
    bool cut=false; const auto notes=readable("Release \xf0\x9f\x9a\x80: \xe2\x80\x99 notes",20,cut);
    check(notes.find("\xf0")==std::string::npos && notes.find("'")!=std::string::npos,"release notes strip unsupported emoji and normalize readable punctuation");
    readable("long release notes",4,cut); check(cut,"long release notes are explicitly truncated");
    auto data=image(); UpdateRelease candidate;
    check(release(releaseJson(),candidate)==UpdateError::None,"published stable release metadata is accepted");
    check(manifest(manifestJson(digest(data)),candidate)==UpdateError::None && candidate.firmwareId==12 && candidate.size==8192,"pipeline manifest binds version, board, layout, file, size and digest");
    UpdateRelease temporary;
    auto bad=releaseJson(); bad.replace(bad.find("v1.2.3"),6,"v1.2.3-beta.1");
    check(release(bad,temporary)==UpdateError::Metadata,"prerelease versions cannot enter the stable OTA channel");
    bad=releaseJson(); bad.replace(bad.find("\"draft\":false"),13,"\"draft\":true");
    check(release(bad,temporary)==UpdateError::Metadata,"draft releases are never offered for installation");
    check(manifest(manifestJson(digest(data)),candidate,"other-board")==UpdateError::Incompatible,"wrong hardware is rejected before any flash operation");
    check(manifest(manifestJson(digest(data)),candidate,OTA_HARDWARE_ID,4096)==UpdateError::Incompatible,"partition-layout/slot changes are not applied over OTA");
    bad=manifestJson(digest(data)); bad.replace(bad.find("1.2.3"),5,"1.2.4");
    check(manifest(bad,candidate)==UpdateError::Metadata,"release and manifest versions must match");
    bad=manifestJson(digest(data)); bad.replace(bad.find("\"stable\""),8,"\"dev\"");
    check(manifest(bad,candidate)==UpdateError::Metadata,"development packages cannot masquerade as stable updates");
    bad=manifestJson(digest(data)); bad.replace(bad.find("8192"),4,"9999");
    check(manifest(bad,candidate)==UpdateError::Size,"GitHub asset and manifest byte counts must match");
    check(release("{broken",temporary)==UpdateError::Metadata,"truncated JSON cannot produce an update candidate");
    release(releaseJson(),candidate); manifest(manifestJson(digest(data)),candidate);
    const auto never=[](){return false;}; const auto quiet=[](UpdateState,size_t,size_t){};
    { Input input(data); Flash flash;
      auto result=UpdateEngine::install(input,flash,candidate,quiet,never);
      check(result==UpdateError::None && flash.written==data.size() && flash.finishes==1 && flash.activated && !flash.aborts,"chunked download activates exactly once after complete SHA/image validation"); }
    { Input input(data); input.advertised=1; Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Size && !flash.begins,"wrong HTTP Content-Length cannot erase a slot"); }
    { auto wrong=data; wrong[0]=0; Input input(wrong); Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Image && !flash.begins,"non-ESP payload is rejected before erasing flash"); }
    { auto wrong=data; wrong[12]=ESP_CHIP_ID_ESP32C3; Input input(wrong); Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Image && !flash.begins,"a real ESP image for another chip is rejected before erasing flash"); }
    { Input input(data); input.limit=500; Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Network && flash.aborts==1 && !flash.finishes && !flash.activated,"interrupted transfer aborts the inactive slot without boot selection changes"); }
    { Input input(data); input.framed=false; Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Size && flash.aborts==1 && !flash.activated,"incomplete HTTP framing is rejected even after the expected byte count"); }
    { auto extra=data; extra.push_back(42); Input input(extra); Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Size && !flash.activated,"appended bytes cannot bypass the declared image length"); }
    { auto corrupt=data; corrupt.back()^=1; Input input(corrupt); Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Integrity && flash.aborts==1 && !flash.finishes && !flash.activated,"SHA-256 mismatch never reaches image finalization or activation"); }
    { Input input(data); Flash flash;
      check(UpdateEngine::install(input,flash,candidate,quiet,[](){return true;})==UpdateError::Cancelled && !flash.begins,"cancelling before download cannot touch flash"); }
    { Input input(data); Flash flash; bool stop=false;
      auto progress=[&](UpdateState,size_t,size_t written){if (written) stop=true;};
      check(UpdateEngine::install(input,flash,candidate,progress,[&](){return stop;})==UpdateError::Cancelled && flash.aborts==1 && !flash.activated,"cancelling during transfer preserves the current boot image"); }
    { Input input(data); Flash flash; bool stop=false;
      auto progress=[&](UpdateState state,size_t,size_t){if (state==UpdateState::Installing) stop=true;};
      check(UpdateEngine::install(input,flash,candidate,progress,[&](){return stop;})==UpdateError::Cancelled && !flash.finishes && !flash.activated,"a cancellation already latched at the commit boundary is honored"); }
    { Input input(data); Flash flash; flash.failWrite=288;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Flash && flash.aborts==1 && !flash.activated,"flash write failure aborts without selecting the new image"); }
    { Input input(data); Flash flash; flash.failFinish=true;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Image && !flash.activated,"native image validation failure cannot activate firmware"); }
    { Input input(data); Flash flash; flash.failActivate=true;
      check(UpdateEngine::install(input,flash,candidate,quiet,never)==UpdateError::Persistence && !flash.activated,"activation/persistence failure leaves the boot selection unchanged"); }
    const bool sound=UIFeedback::soundEnabled(); UIFeedback::setSoundEnabled(false);
    auto *original=lv_display_get_default(); auto *display=lv_display_create(320,240); lv_display_set_default(display);
    {
        UpdateView view; Capture events; view.setEventHandler(&events); view.create();
        auto *screen=view.getScreen(); lv_screen_load(screen); view.render(preview(0)); lv_obj_update_layout(screen);
        auto *install=button(screen,tr(Text::OtaInstall)),*back=button(screen,tr(Text::Back));
        lv_area_t area{}; if (install) lv_obj_get_coords(install,&area);
        check(install && back && area.y1>=192 && area.x1>=0 && area.x2<320 && lv_obj_get_height(install)>=44,"update actions fit the fixed footer with a 44-pixel touch target");
        view.askInstall("1.2.3"); lv_obj_update_layout(screen);
        auto *confirm=button(screen,tr(Text::OtaInstall)); if (confirm) lv_obj_send_event(confirm,LV_EVENT_CLICKED,nullptr);
        check(events.installs==1,"installation requires the explicit update confirmation dialog");
        view.render(preview(1));
        check(lv_obj_has_state(back,LV_STATE_DISABLED) && lv_obj_has_state(install,LV_STATE_DISABLED) && button(screen,tr(Text::Cancel)),"downloading disables navigation/install while offering cancellation");
        view.render(preview(3));
        auto *cancel=button(screen,tr(Text::Cancel));
        check(cancel && lv_obj_has_state(cancel,LV_STATE_DISABLED),"cancellation is disabled during final activation");
        auto current=preview(0); current.state=UpdateState::UpToDate; view.render(current);
        check(lv_obj_has_state(install,LV_STATE_DISABLED),"up-to-date firmware cannot be installed again");
        check(!textarea(screen),"public update UI contains no credential input or keyboard");
        view.destroy();
    }
    lv_display_delete(display); lv_display_set_default(original); UIFeedback::setSoundEnabled(sound);
    Serial.printf("@OTATEST passed=%u failed=%u stack=%u\n",passed,failed,unsigned(uxTaskGetStackHighWaterMark(nullptr)));
}
#endif
