#pragma once
#include <sc2api/sc2_api.h>
#include <map>

using namespace sc2;

class UnitWrapper {
public:
    Tag self;
    UnitTypeID type;
    UnitWrapper(Tag self_);

    UnitWrapper(Tag self_, UnitTypeID id);

    bool equals(UnitWrapper *wrapper);

    virtual bool execute(Agent *agent);

    ~UnitWrapper();
};

namespace UnitManager {
    map<UnitTypeID, vector<UnitWrapper*>> units;

    bool checkExist(UnitTypeID id) {
        return units.find(id) != units.end();
    }

    vector<UnitWrapper *> get(UnitTypeID type) {
        if (checkExist(type)) {
            return units[type];
        }
        return vector<UnitWrapper *>();
    }

    UnitWrapper* find(UnitTypeID type, Tag tag) {
        vector<UnitWrapper *> v = get(type);
        for (int i = 0; i < v.size(); i++) {
            if (v[i]->self == tag) {
                return v[i];
            }
        }
        return nullptr;
    }
}  // namespace UnitManager


UnitWrapper::UnitWrapper(Tag self_) {
    self = self_;
    type = UNIT_TYPEID::INVALID;
    printf("WHY ARE YOU USING THE DEFAULT UNITWRAPPER CONSTRUCTOR\n");
}

UnitWrapper::UnitWrapper(Tag self_, UnitTypeID type) {
    self = self_;
    type = type;
    if (!UnitManager::checkExist(type)) {
            UnitManager::units[type] = vector<UnitWrapper *>();
    }
    UnitManager::units[type].push_back(this);
}

bool UnitWrapper::equals(UnitWrapper *wrapper) {
    return (wrapper->self == self) && (wrapper->type == type);
}

UnitWrapper::~UnitWrapper() {
    for (auto it = UnitManager::units[type].begin(); it != UnitManager::units[type].end(); it++) {
        UnitManager::units[type].erase(it);
    }
}

bool UnitWrapper::execute(Agent *agent) {
    return false;
}