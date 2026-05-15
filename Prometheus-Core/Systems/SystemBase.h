#ifndef PROMETHEUS_SYSTEMBASE_H
#define PROMETHEUS_SYSTEMBASE_H

#include <utility>
#include <vector>
#include "entity_admin.h"

//This is used for Preexisting VTables
struct SystemVTBase {
    __int64 (*get_inheritance());
    bool (*is_assignable_to(__int64));
    bool (*is_instance_of(__int64));

    const char* (*get_system_name());
    void (*deallocate());

    void (*OnInitialize());
    //Unknown what this is
    void (*field_1());
    void (*PreDelete());

    char* (*GetSubscriptions(int* count));
    void (*OnCreationAfterEmplaced(Entity*));
    void (*OnCreation(Entity*));
    void (*field_3());
    void (*OnDeletion(Entity*));

    void (*OnTick(float));
};

/**
 *
 * Represents an Entity Admin's system. Used as a base for custom systems.
 *.
 */
class SystemBase {
public:
    __int64 get_inheritance() { return 0; }
    virtual bool is_assignable_to(__int64) { return false; }
    virtual bool is_instance_of(__int64) { return false; }

    virtual const char* get_system_name() {
        return _name;
    }
    virtual void deallocate() {}

    virtual void OnInitialize() {}
    //Unknown what this is
    virtual void field_1() {}
    virtual void PreDelete() {}

    virtual char* GetSubscriptions(int* count) {
        *count = _subscriptions.size();
        return _subscriptions.data();
    }
    virtual void OnCreationAfterEmplaced(Entity*) {}
    virtual void OnCreation(Entity*) {}
    virtual void field_3() {}
    virtual void OnDeletion(Entity*) {}

    virtual void OnTick(float) = 0;

    SystemBase(const char* name, std::vector<char> subs) : _name(name), _subscriptions(std::move(subs)) { }
protected:
    std::vector<char> _subscriptions;
    const char* _name;
};

#endif //PROMETHEUS_SYSTEMBASE_H