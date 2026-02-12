#ifndef PROMETHEUS_COMPONENT_4_MODEL_H
#define PROMETHEUS_COMPONENT_4_MODEL_H
#include "Component.h"
#include "globals.h"
#include "Atlas/Common.h"

struct Component_4_Model {
    enum class ModelRenderingFlags : unsigned short {
        RENDER_NOFLAGS = 0,
        OUTLINE_VISIBLE = 0x40,
        OUTLINE_INVISIBLE = 0x80, //render invisible also renders it when visible
        OUTLINE_FUCKED = 0x100, //strange outline rendering mode, doesnt completely work?
        RENDER_NOTEXTURE = 0x200,

        OUTLINE_MASK = OUTLINE_INVISIBLE | OUTLINE_FUCKED | OUTLINE_VISIBLE,
    };

    union {
        ComponentBase base;
        STRUCT_PLACE(char, outline_field8, 0x178);
        STRUCT_PLACE(ModelRenderingFlags, model_rendering_flags, 0x17A);
        STRUCT_PLACE(uint32, outline_color, 0x17C);
    };

    //Works only on non-player entities, cause 0xd5a050 is called every call and overrides our definition.
    void EnableOutline(ModelRenderingFlags flags, uint32 colorRGBA) {
        *(ushort*)&model_rendering_flags |= (ushort)flags;
        outline_color = colorRGBA;
        outline_field8 = 3;
    }

    void DisableOutline() {
        model_rendering_flags = ModelRenderingFlags::RENDER_NOFLAGS;
        outline_color = 0;
    }
};


#endif //PROMETHEUS_COMPONENT_4_MODEL_H