//
// Created by cereal on 18.04.26.
//

#ifndef PROMETHEUS_DATAFLOWMANAGER_H
#define PROMETHEUS_DATAFLOWMANAGER_H

#include "Components/Component_3_DataFlow.h"

class DataFlowManager
{
    static inline teList<DataFlow_ComponentRefBase*>* GetStaticDataflowRefs()
    {
        return (teList<DataFlow_ComponentRefBase*>*)(globals::gameBase + 0x17a68d0);
    }

};


#endif //PROMETHEUS_DATAFLOWMANAGER_H