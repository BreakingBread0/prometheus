// Hazno - 2026

#pragma once

#include "Base/Creator.h"

/**
 *  <b> STU6FComponentCreator </b>
 *  \n  Size:           0x38 (56)
 *  \n  Factory:        0x1017910
 *  \n  VT:             0x1614BB0
 */
class ComponentCreator6F : public ComponentCreator
{
    protected:
        ~ComponentCreator6F() = default;
        int64 Construct() override;
};


/**
 *  <b> UglyStupidComponent6F </b>
 *  \n  Size:           0x1890 (6288)
 *  \n  Factory:        0x1018780
 *  \n  VT:             0x1614BD0
 *  \n  Dependencies:   None because it has no friends bc its ugly and stupid :)
 */