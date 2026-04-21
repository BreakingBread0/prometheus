#ifndef PROMETHEUS_TIMESCALE_H
#define PROMETHEUS_TIMESCALE_H
#include "globals.h"

namespace TimeScale {
    //Normally at 1 = 100%. Game timescale changes based on value (0.5 = 50%)
    inline float* PhysicsTimescale() {
        return (float*)(globals::gameBase + 0x179f128);
    }

    //= 1 / PhysicsTimescale EXCEPT if timescale is 0, then its also 0
    inline float* NormalPhysicsTimescaleConvert() {
        return (float*)(globals::gameBase + 0x179f12c);
    }

    //Sets PhysicsTimescale.
    inline float* ServerPhysicsTimescale() {
        return (float*)(globals::gameBase + 0x17b7fe4);
    }
};

#endif //PROMETHEUS_TIMESCALE_H