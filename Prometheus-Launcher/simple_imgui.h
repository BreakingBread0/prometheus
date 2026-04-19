//
// Created by cereal on 19.04.26.
//

#ifndef PROMETHEUS_SIMPLE_IMGUI_H
#define PROMETHEUS_SIMPLE_IMGUI_H

namespace simple_imgui
{
    bool imgui_init(); //0 = success
    bool imgui_loop(); //false => exit main loop
    void imgui_render(); //call after rendering window
    void imgui_cleanup();
}


#endif //PROMETHEUS_SIMPLE_IMGUI_H