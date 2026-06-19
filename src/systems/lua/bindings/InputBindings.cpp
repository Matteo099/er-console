#include "InputBindings.h"

#include "systems/input/InputSystem.h"

#include "ChannelBindings.h"
#include "RFIDBindings.h"
#include "OLEDBindings.h"
#include "RelayBindings.h"
#include "RGBBindings.h"
#include "SevenSegmentBindings.h"

InputBindings::InputBindings(InputSystem *inputSystem) : input(inputSystem) {}

void InputBindings::registerAPI(lua_State *L)
{
    if (!input)
        return;
    if (auto *channels = input->channels())
    {
        ChannelBindings(channels).registerAPI(L);
    }
    if (auto *rfid = input->rfid())
    {
        RFIDBindings(rfid).registerAPI(L);
    }
    if (auto *oled = input->oled())
    {
        OLEDBindings(oled).registerAPI(L);
    }
    if (auto *relays = input->relays())
    {
        RelayBindings(relays).registerAPI(L);
    }
    if (auto *rgb = input->rgb())
    {
        RGBBindings(rgb).registerAPI(L);
    }
    if (auto *sevenSegment = input->sevenSegment())
    {
        SevenSegmentBindings(sevenSegment).registerAPI(L);
    }
}