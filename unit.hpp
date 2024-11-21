#pragma once
#include <sc2api/sc2_api.h>
#include <map>

using namespace sc2;

class UnitWrapper {
public:
    Tag self;
    UnitTypeID type;
    Point2D lastPos;

    UnitWrapper(Tag self_);

    UnitWrapper(Tag self_, UnitTypeID id);

    UnitWrapper(const Unit* unit);

    bool equals(UnitWrapper *wrapper);

    Point2D pos(Agent *agent);

    virtual bool execute(Agent *agent);

    ~UnitWrapper();
};

namespace UnitManager {
    map<UnitTypeID, vector<UnitWrapper*>> units;
    map<UnitTypeID, vector<UnitWrapper *>> neutrals;
    map<UnitTypeID, vector<UnitWrapper *>> enemies;

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

    bool checkExistNeutral(UnitTypeID id) {
        return neutrals.find(id) != neutrals.end();
    }

    vector<UnitWrapper *> getNeutral(UnitTypeID type) {
        if (checkExistNeutral(type)) {
            return neutrals[type];
        }
        return vector<UnitWrapper *>();
    }

    vector<UnitWrapper *> getVespene() {
        vector<UnitWrapper *> v1 = getNeutral(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
        vector<UnitWrapper *> v2 = getNeutral(UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER);
        vector<UnitWrapper *> v3 = getNeutral(UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER);
        vector<UnitWrapper *> v4 = getNeutral(UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
        vector<UnitWrapper *> v5 = getNeutral(UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER);
        vector<UnitWrapper *> v6 = getNeutral(UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
        v1.insert(v1.end(), v2.begin(), v2.end());
        v1.insert(v1.end(), v3.begin(), v3.end());
        v1.insert(v1.end(), v4.begin(), v4.end());
        v1.insert(v1.end(), v5.begin(), v5.end());
        v1.insert(v1.end(), v6.begin(), v6.end());
        return v1;
    }

    vector<UnitWrapper *> getMinerals() {
        vector<UnitWrapper *> v1 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD);
        vector<UnitWrapper *> v2 = getNeutral(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD);
        vector<UnitWrapper *> v3 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD750);
        vector<UnitWrapper *> v4 = getNeutral(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750);
        vector<UnitWrapper *> v5 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD450);
        v1.insert(v1.end(), v2.begin(), v2.end());
        v1.insert(v1.end(), v3.begin(), v3.end());
        v1.insert(v1.end(), v4.begin(), v4.end());
        v1.insert(v1.end(), v5.begin(), v5.end());
        return v1;
    }

    UnitWrapper *findNeutral(UnitTypeID type, Tag tag) {
        vector<UnitWrapper *> v = getNeutral(type);
        for (int i = 0; i < v.size(); i++) {
            if (v[i]->self == tag) {
                return v[i];
            }
        }
        return nullptr;
    }

    bool checkExistEnemy(UnitTypeID id) {
        return enemies.find(id) != enemies.end();
    }

    vector<UnitWrapper *> getEnemy(UnitTypeID type) {
        if (checkExistEnemy(type)) {
            return enemies[type];
        }
        return vector<UnitWrapper *>();
    }

    UnitWrapper *findEnemy(UnitTypeID type, Tag tag) {
        vector<UnitWrapper *> v = getEnemy(type);
        for (int i = 0; i < v.size(); i++) {
            if (v[i]->self == tag) {
                return v[i];
            }
        }
        return nullptr;
    }
}  // namespace UnitManager


UnitWrapper::UnitWrapper(Tag self_) : self(self_), type(UNIT_TYPEID::INVALID), lastPos{0, 0} {
    printf("WHY ARE YOU USING THE DEFAULT UNITWRAPPER CONSTRUCTOR\n");
}

UnitWrapper::UnitWrapper(Tag self_, UnitTypeID type_)
    : self(self_), type(type_) {
    if (!UnitManager::checkExist(type)) {
            UnitManager::units[type] = vector<UnitWrapper *>();
    }
    UnitManager::units[type].push_back(this);
}

UnitWrapper::UnitWrapper(const Unit *unit) : self(unit->tag), type(unit->unit_type), lastPos{0, 0} {
    if (unit->alliance == Unit::Alliance::Self) {
        if (!UnitManager::checkExist(type)) {
            UnitManager::units[type] = vector<UnitWrapper *>();
        }
        UnitManager::units[type].push_back(this);
    }else if (unit->alliance == Unit::Alliance::Neutral) {
        if (!UnitManager::checkExistNeutral(type)) {
            UnitManager::neutrals[type] = vector<UnitWrapper *>();
        }
        UnitManager::neutrals[type].push_back(this);
    }else if (unit->alliance == Unit::Alliance::Enemy) {
        if (!UnitManager::checkExistEnemy(type)) {
            UnitManager::enemies[type] = vector<UnitWrapper *>();
        }
        UnitManager::enemies[type].push_back(this);
    }
}

Point2D UnitWrapper::pos(Agent *agent) {
    const Unit *unit = agent->Observation()->GetUnit(self);
    if (unit != nullptr) {
        lastPos = unit->pos;
    }
    return lastPos;
}

bool UnitWrapper::equals(UnitWrapper *wrapper) {
    return (wrapper->self == self) && (wrapper->type == type);
}

UnitWrapper::~UnitWrapper() {
    for (auto it = UnitManager::units[type].begin(); it != UnitManager::units[type].end(); it++) {
        //printf("%lx %lx", )
        if ((*it)->self == self) {
            UnitManager::units[type].erase(it);
            break;
        }
    }
}

bool UnitWrapper::execute(Agent *agent) {
    return false;
}