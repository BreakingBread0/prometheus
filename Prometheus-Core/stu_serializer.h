#ifndef PROMETHEUS_STU_SERIALIZER_H
#define PROMETHEUS_STU_SERIALIZER_H
#include <nlohmann/json.hpp>

#include "Atlas/Common.h"
#include "STU_Editable.h"
#include "bsDataStore.h"

class stu_serializer {
public:
    struct save_struct {
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(save_struct, type, storage);

        uint32 type = 0;
        //Yes, in theory it would be possible to store everything in one bsDataStore.
        //But this works just as well.
        std::vector<std::vector<uint8_t>> storage;
    };

    static void deserialize(std::vector<uint8_t> input, STU_Object& into) {
        save_struct stru = nlohmann::json::from_bson(input);
        owassert(into.struct_info->AssignableToHash(stru.type));

        bsDataStore* datastore = bsDataStore::create();
        int index = 0;
        for (auto argument : into.struct_info->RangeArgs()) {
            owassert(index < stru.storage.size());
            auto& item = stru.storage[index];
            datastore->clear();
            datastore->expandBuffer(item.size());
            memcpy((void*)datastore->dataStore_ptr, item.data(), item.size());

            auto field_ptr = into.get_argument_raw(argument.second);
            argument.second->Constraint->DeserializeFromBitStream(datastore, 0, field_ptr);
            index++;
        }
        datastore->deallocate();
    }

    static std::vector<uint8_t> serialize(STU_Object& from) {
        save_struct stru{};
        stru.type = from.struct_info->Hash;

        bsDataStore* datastore = bsDataStore::create();
        for (auto argument : from.struct_info->RangeArgs()) {
            datastore->clear();

            auto field_ptr = from.get_argument_raw(argument.second);
            argument.second->Constraint->SerializeToBitstream(datastore, 0, field_ptr);

            std::vector<uint8_t> part_result{};
            part_result.resize(datastore->curr_pos + 1); //TODO is +1 needed?
            memcpy(part_result.data(), (void*)datastore->dataStore_ptr, datastore->curr_pos + 1);
            stru.storage.push_back(std::move(part_result));
        }
        datastore->deallocate();
        return nlohmann::json::to_bson(std::move(stru));
    }
};

#endif //PROMETHEUS_STU_SERIALIZER_H