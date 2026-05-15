#include "entity_admin.h"
#include "Component_54_LobbyMap.h"

Component_54_Lobbymap* Component_54_Lobbymap::hookSwitchCam()
{
    if (s_camera_switch_hook) {
        return s_camera_switch_hook;
    }
    return LobbyEntityAdmin()->getSingletonComponent<Component_54_Lobbymap>(0x54);
}