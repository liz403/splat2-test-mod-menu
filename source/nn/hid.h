/**
 * @file hid.h
 * @brief Functions that help process gamepad inputs.
 */

#pragma once

#include "types.h"

namespace nn
{
    namespace hid
    {
        constexpr int AnalogStickMax = 0x7fff;
        enum NpadButton : u64{
            A = 1LL << 0,
            B = 1LL << 1,
            X = 1LL << 2,
            Y = 1LL << 3,
            LStick = 1LL << 4,
            RStick = 1LL << 5,
            L = 1LL << 6,
            R = 1LL << 7,
            ZL = 1LL << 8,
            ZR = 1LL << 9,
            Plus = 1LL << 10,
            Minus = 1LL << 11,
            Left = 1LL << 12,
            Up = 1LL << 13,
            Right = 1LL << 14,
            Down = 1LL << 15,
            StickLLeft = 1LL << 16,
            StickLUp = 1LL << 17,
            StickLRight = 1LL << 18,
            StickLDown = 1LL << 19,
            StickRLeft = 1LL << 20,
            StickRUp = 1LL << 21,
            StickRRight = 1LL << 22,
            StickRDown = 1LL << 23
        };
        typedef u64 NpadButtonSet;
        typedef u32 NpadAttributesSet;
        struct AnalogStickState {
            int32_t x;
            int32_t y; 
        };
        struct NpadState{
            int64_t readNum;
            NpadButtonSet buttons;
            AnalogStickState analogStickL;
            AnalogStickState analogStickR;
            NpadAttributesSet attributes;
        };
        struct NpadFullKeyState  {
            NpadState state;
        };
        struct NpadJoyDualState  {
            NpadState state;
        };
        struct NpadHandheldState {
            NpadState state;
        };
        struct NpadStyleTag;
        struct ControllerSupportArg {
            u8 mMinPlayerCount;
            u8 mMaxPlayerCount;
            u8 mTakeOverConnection;
            bool mLeftJustify;
            bool mPermitJoyconDual;
            bool mSingleMode;
            bool mUseColors;
            u8 mUsingControllerNames;
            char mControllerNames[4][0x81];
        };

        struct ControllerSupportResultInfo {
            int mPlayerCount;
            int mSelectedId;
        };

        void InitializeNpad();
        void SetSupportedNpadIdType(u32 const* , u64);
        u32 GetNpadStyleSet(u32 const &);
        void GetNpadStates(nn::hid::NpadHandheldState *, s32, u32 const &);
        void GetNpadStates(nn::hid::NpadFullKeyState *, s32, u32 const &);
        void GetNpadStates(nn::hid::NpadJoyDualState *, s32, u32 const &);
        void GetNpadState(nn::hid::NpadHandheldState *, u32 const &);
        void GetNpadState(nn::hid::NpadFullKeyState *, u32 const &);
        void GetNpadState(nn::hid::NpadJoyDualState *, u32 const &);
        int ShowControllerSupport(nn::hid::ControllerSupportResultInfo*, ControllerSupportArg const&);
    };
};