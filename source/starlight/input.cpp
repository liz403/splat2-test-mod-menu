#include "starlight/input.hpp"

namespace starlight {

    Controller::Controller() {
        ctrlId = 0;
        data = NULL;
        leftStick = sead::Vector2<float>::zero;
        rightStick = sead::Vector2<float>::zero;
        mCntr = 0;
        memset(mLastPoll, 0, sizeof(mLastPoll));
    }

    Controller::Controller(int id){
        ctrlId = id;
        data = NULL;
        leftStick = sead::Vector2<float>::zero;
        rightStick = sead::Vector2<float>::zero;
        mCntr = 0;
        memset(mLastPoll, 0, sizeof(mLastPoll));
    }
    bool Controller::fetchState(nn::hid::NpadState *state, int cid){
        u32 styleSet = nn::hid::GetNpadStyleSet(cid);
        switch(styleSet){
        case 1:
            nn::hid::GetNpadState((nn::hid::NpadFullKeyState*)state, cid);
            break;
        case 2:
            nn::hid::GetNpadState((nn::hid::NpadHandheldState*)state, cid);
            break;
        case 4:
            nn::hid::GetNpadState((nn::hid::NpadJoyDualState*)state, cid);
            break;
        default:
            memset(state, 0, sizeof(*state));
            return 0;
        };
        return 1;
    }
    void Controller::update(){
        for(int i = 19; i > 0; i--){
            mLastPoll[i] = mLastPoll[i - 1];
        }
        mLastPoll[0] = data;
        mCntr-=1;
        if(mCntr < 0){
            mCntr = 5;
        }
        data = NULL;
        nn::hid::NpadState state;
        if(!this->fetchState(&state, ctrlId) and ctrlId == 0) this->fetchState(&state, 32);
        #define MAP_BUTTON(hidBtn, resBtn) if(state.buttons & hidBtn) data |= resBtn
        MAP_BUTTON(nn::hid::NpadButton::A, Controller::Buttons::A);
        MAP_BUTTON(nn::hid::NpadButton::B, Controller::Buttons::B);
        MAP_BUTTON(nn::hid::NpadButton::X, Controller::Buttons::X);
        MAP_BUTTON(nn::hid::NpadButton::Y, Controller::Buttons::Y);
        MAP_BUTTON(nn::hid::NpadButton::LStick, Controller::Buttons::LStick);
        MAP_BUTTON(nn::hid::NpadButton::RStick, Controller::Buttons::RStick);
        MAP_BUTTON(nn::hid::NpadButton::L, Controller::Buttons::L);
        MAP_BUTTON(nn::hid::NpadButton::R, Controller::Buttons::R);
        MAP_BUTTON(nn::hid::NpadButton::ZL, Controller::Buttons::ZL);
        MAP_BUTTON(nn::hid::NpadButton::ZR, Controller::Buttons::ZR);
        MAP_BUTTON(nn::hid::NpadButton::Plus, Controller::Buttons::Plus1);
        MAP_BUTTON(nn::hid::NpadButton::Plus, Controller::Buttons::Plus2);
        MAP_BUTTON(nn::hid::NpadButton::Minus, Controller::Buttons::Minus1);
        MAP_BUTTON(nn::hid::NpadButton::Minus, Controller::Buttons::Minus2);
        MAP_BUTTON(nn::hid::NpadButton::Left, Controller::Buttons::LeftDpad);
        MAP_BUTTON(nn::hid::NpadButton::Right, Controller::Buttons::RightDpad);
        MAP_BUTTON(nn::hid::NpadButton::Up, Controller::Buttons::UpDpad);
        MAP_BUTTON(nn::hid::NpadButton::Down, Controller::Buttons::DownDpad);
        MAP_BUTTON(nn::hid::NpadButton::StickLLeft, Controller::Buttons::LeftLStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickLUp, Controller::Buttons::UpLStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickLRight, Controller::Buttons::RightLStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickLDown, Controller::Buttons::DownLStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickRLeft, Controller::Buttons::LeftRStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickRUp, Controller::Buttons::UpRStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickRRight, Controller::Buttons::RightRStickOrdinal);
        MAP_BUTTON(nn::hid::NpadButton::StickRDown, Controller::Buttons::DownRStickOrdinal);
        #undef MAP_BUTTON
        leftStick = {float(state.analogStickL.x) / float(nn::hid::AnalogStickMax), float(state.analogStickL.y) / float(nn::hid::AnalogStickMax)};
        rightStick = {float(state.analogStickR.x) / float(nn::hid::AnalogStickMax), float(state.analogStickR.y) / float(nn::hid::AnalogStickMax)};
    }

    u32 Controller::isHeld(Buttons inputs) {
        return (data & inputs) != 0;
    }

    u32 Controller::wasPressed(Buttons inputs) {
        bool buttonHeld = data & inputs;
        return (!buttonHeld & (data & mLastPoll[0] & inputs)) != 0;
    }

    u32 Controller::isPressed(Buttons inputs) {
        bool buttonHeld = data & inputs;
        return (buttonHeld & !(data & mLastPoll[0] & inputs)) != 0;
    }

    u32 Controller::isPressedWithRepeat(Buttons inputs){
        bool val = (data & inputs);
        for(int i = 0; i < 20; i++){
            val = val and (mLastPoll[i] & inputs);
        }
        return ((this->isPressed(inputs)) or (val and (mCntr == 0)));
    }
};