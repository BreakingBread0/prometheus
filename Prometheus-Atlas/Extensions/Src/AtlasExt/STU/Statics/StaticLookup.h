// Hazno - 2026

#pragma once

#include <set>

#include "Atlas/Common.h"

//Temp hacky lookup until migration to something like frozen + athena

namespace Atlas::STU::Statics
{
    namespace Primitive {
        constexpr uint32 teMtx43A = 0x6fb9880b;
        constexpr uint32 teVec3A = 0x2357e676;
        constexpr uint32 teVec2 = 0xf6ec8926;
        constexpr uint32 teVec3 = 0x81ebb9b0;
        constexpr uint32 teVec4 = 0x1f8f2c13;
        constexpr uint32 teQuat = 0xda9494c8;
        constexpr uint32 teColorRGB = 0x525114ab;
        constexpr uint32 teColorRGBA = 0xa9e19537;
        constexpr uint32 teStructuredDataDateAndTime = 0xe1b9e763;
        constexpr uint32 teEntityID = 0xc525b48;

        constexpr uint32 DBID = 0x20b3d5a9;
        constexpr uint32 teUUID = 0x809899f3;
        const uint32 teString = 0xbbca304b;

        //s8 gibts nicht?
        constexpr uint32 s16 = 0xa0119d30;
        constexpr uint32 s32 = 0x954a3bab;
        constexpr uint32 s64 = 0x15e6adb;

        constexpr uint32 u8 = 0x3b9327d2;
        constexpr uint32 u16 = 0xa49ce182;
        constexpr uint32 u32 = 0x91c74719;
        constexpr uint32 u64 = 0x5d31669;

        constexpr uint32 f32 = 0x8fa75a30;
        constexpr uint32 f64 = 0x1bb30b40;

        inline std::set all = {
            teMtx43A,
            teVec3A,
            teVec2,
            teVec3,
            teVec4,
            teQuat,
            teColorRGB,
            teColorRGBA,
            teStructuredDataDateAndTime,

            DBID,
            teUUID,
            teString,

            s16,
            s32,
            s64,

            u8,
            u16,
            u32,
            u64,

            f32,
            f64,
        };

        inline int _primitive_size(uint32 primitive_hash) {
            switch (primitive_hash) {
            case u8:
                return 1;
            case u16:
            case s16:
                return 2;
            case u32:
            case f32:
            case s32:
            case teEntityID:
                return 4;
            case s64:
            case u64:
            case f64:
            case teVec2:
            case teStructuredDataDateAndTime:
                return 8;
            case teColorRGB:
                return 0xC;
            case teVec3:
            case teVec4:
            case teVec3A:
            case teQuat:
            case DBID:
            case teUUID:
            case teString:
            case teColorRGBA:
                return 0x10;
            case teMtx43A:
                return 0x40;
            default:
                return 0;
            }
        }
    }
}
