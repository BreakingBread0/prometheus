// Hazno - 2026

#pragma once

#include "Base/Creator.h"

/**
 *  <b> STUHealthComponentCreator </b>
 *  \n  Size:           0x38 (56)
 *  \n  Factory:        0xCC3E50
 *  \n  VT:             0x15C5B10
 */
class ComponentCreator28_STU_HEALTH_COMPONENT : public ComponentCreator
{
    protected:
        ~ComponentCreator28_STU_HEALTH_COMPONENT() = default;
        int64 Construct() override;
};

/**
 *  <b> HealthComponent28 </b>
 *  \n  Size:           0x490 (1168)
 *  \n  Factory:        0x
 *  \n  VT:             0x
 *  \n  Dependencies:
 */