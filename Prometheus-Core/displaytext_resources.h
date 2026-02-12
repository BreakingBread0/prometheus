#ifndef PROMETHEUS_DISPLAYTEXT_RESOURCES_H
#define PROMETHEUS_DISPLAYTEXT_RESOURCES_H

#include "ResourceManager.h"

class displaytext_resources {
public:
    static void initialize() {
        resource_handler_helper::hookInitialize("DISPLAYTEXT", hook_initialize);
    }
private:
    static void hook_initialize(resource_handler* handler) {
        s_origConstruct = handler->resource_constructor;
        handler->resource_constructor = constructHook;
    }

    static inline resource_construct_fn s_origConstruct;

    static void constructHook(ResourceLoadEntry* entry) {
        //printf("construct %p\n", entry->resource_id);
        s_origConstruct(entry);

        static std::set<__int64> cass_replacements = {
            0x0DE000000000005C,
            0x0DE0000000000A77,
            0x0DE0000000000A78,
            0x0DE0000000000A79,
            0x0DE0000000000A7A,
            0x0DE0000000000A8A,
            0x0DE0000000000BE1,
            0x0DE0000000000BED,
            0x0DE000000000105E,
            0x0DE0000000001BEE
        };

        if (cass_replacements.find(entry->resource_id) != cass_replacements.end()) {
            auto text = std::string(*(char**)entry->DATA);
            replace_first(text, "McCree", "Cass");
            strcpy(*(char**)entry->DATA, text.c_str());
        }
    }
};

#endif //PROMETHEUS_DISPLAYTEXT_RESOURCES_H