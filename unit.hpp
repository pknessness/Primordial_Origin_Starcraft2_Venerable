#pragma once
#include <sc2api/sc2_api.h>
#include <map>

using namespace sc2;
using namespace std;

class UnitWrapper {
public:
    Tag self;
    UnitTypeID type;
    Point3D lastPos;
    float radius;
    Unit::Alliance team;

    //UnitWrapper(Tag self_);

    //UnitWrapper(Tag self_, UnitTypeID id);

    UnitWrapper(const Unit* unit);

    bool equals(UnitWrapper *wrapper);

    inline bool exists(Agent *agent);

    inline const Unit *get(Agent *agent);

    Point2D pos(Agent *agent);
    Point3D pos3D(Agent *agent);

    virtual bool execute(Agent *agent);

    virtual ~UnitWrapper();
};

using UnitWrappers = vector<UnitWrapper*>;

namespace UnitManager {
    map<UnitTypeID, UnitWrappers> units;
    map<UnitTypeID, UnitWrappers> neutrals;
    map<UnitTypeID, UnitWrappers> enemies;

    bool checkExist(UnitTypeID id) {
        return units.find(id) != units.end();
    }

    UnitWrappers get(UnitTypeID type) {
        if (checkExist(type)) {
            return units[type];
        }
        return UnitWrappers();
    }

    UnitWrapper* find(UnitTypeID type, Tag tag) {
        UnitWrappers v = get(type);
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

    UnitWrappers getNeutral(UnitTypeID type) {
        if (checkExistNeutral(type)) {
            return neutrals[type];
        }
        return UnitWrappers();
    }

    UnitWrappers getVespene() {
        UnitWrappers v1 = getNeutral(UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
        UnitWrappers v2 = getNeutral(UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER);
        UnitWrappers v3 = getNeutral(UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER);
        UnitWrappers v4 = getNeutral(UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
        UnitWrappers v5 = getNeutral(UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER);
        UnitWrappers v6 = getNeutral(UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
        v1.insert(v1.end(), v2.begin(), v2.end());
        v1.insert(v1.end(), v3.begin(), v3.end());
        v1.insert(v1.end(), v4.begin(), v4.end());
        v1.insert(v1.end(), v5.begin(), v5.end());
        v1.insert(v1.end(), v6.begin(), v6.end());
        return v1;
    }

    UnitWrappers getMinerals() {
        UnitWrappers v1 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD);
        UnitWrappers v2 = getNeutral(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD);
        UnitWrappers v3 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD750);
        UnitWrappers v4 = getNeutral(UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750);
        UnitWrappers v5 = getNeutral(UNIT_TYPEID::NEUTRAL_MINERALFIELD450);
        v1.insert(v1.end(), v2.begin(), v2.end());
        v1.insert(v1.end(), v3.begin(), v3.end());
        v1.insert(v1.end(), v4.begin(), v4.end());
        v1.insert(v1.end(), v5.begin(), v5.end());
        return v1;
    }

    UnitWrapper *findNeutral(UnitTypeID type, Tag tag) {
        UnitWrappers v = getNeutral(type);
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

    UnitWrappers getEnemy(UnitTypeID type) {
        if (checkExistEnemy(type)) {
            return enemies[type];
        }
        return UnitWrappers();
    }

    UnitWrapper *findEnemy(UnitTypeID type, Tag tag) {
        UnitWrappers v = getEnemy(type);
        for (int i = 0; i < v.size(); i++) {
            if (v[i]->self == tag) {
                return v[i];
            }
        }
        return nullptr;
    }
}  // namespace UnitManager


//UnitWrapper::UnitWrapper(Tag self_) : self(self_), type(UNIT_TYPEID::INVALID), lastPos{0, 0} {
//    printf("WHY ARE YOU USING THE DEFAULT UNITWRAPPER CONSTRUCTOR\n");
//}

//UnitWrapper::UnitWrapper(Tag self_, UnitTypeID type_)
//    : self(self_), type(type_), radius(0){
//    if (!UnitManager::checkExist(type)) {
//            UnitManager::units[type] = UnitWrappers();
//    }
//    UnitManager::units[type].push_back(this);
//}

UnitWrapper::UnitWrapper(const Unit *unit) : self(unit->tag), type(unit->unit_type), lastPos{0, 0, 0}, radius(unit->radius), team(unit->alliance) {
    if (unit->alliance == Unit::Alliance::Self) {
        if (!UnitManager::checkExist(type)) {
            UnitManager::units[type] = UnitWrappers();
        }
        UnitManager::units[type].push_back(this);
    }else if (unit->alliance == Unit::Alliance::Neutral) {
        if (!UnitManager::checkExistNeutral(type)) {
            UnitManager::neutrals[type] = UnitWrappers();
        }
        UnitManager::neutrals[type].push_back(this);
    }else if (unit->alliance == Unit::Alliance::Enemy) {
        if (!UnitManager::checkExistEnemy(type)) {
            UnitManager::enemies[type] = UnitWrappers();
        }
        UnitManager::enemies[type].push_back(this);
    }
}

Point2D UnitWrapper::pos(Agent *agent) {
    const Unit *unit = agent->Observation()->GetUnit(self);
    if (unit != nullptr) {
        lastPos = unit->pos;
    } else if (agent->Observation()->GetVisibility(lastPos) == Visibility::Visible) {
        lastPos = {0, 0, 0};
    }
    return lastPos;
}

Point3D UnitWrapper::pos3D(Agent *agent) {
    const Unit *unit = agent->Observation()->GetUnit(self);
    if (unit != nullptr) {
        lastPos = unit->pos;
    } else if (agent->Observation()->GetVisibility(lastPos) == Visibility::Visible) {
        lastPos = {0, 0, 0};
    }
    return lastPos;
}

inline bool UnitWrapper::exists(Agent *agent) {
    return agent->Observation()->GetUnit(self) == nullptr;
}

inline const Unit* UnitWrapper::get(Agent *agent) {
    return agent->Observation()->GetUnit(self);
}

bool UnitWrapper::equals(UnitWrapper *wrapper) {
    return (wrapper->self == self) && (wrapper->type == type);
}

UnitWrapper::~UnitWrapper() {
    if (team == Unit::Alliance::Self) {
        for (auto it = UnitManager::units[type].begin(); it != UnitManager::units[type].end(); it++) {
            if ((*it)->self == self) {
                UnitManager::units[type].erase(it);
                break;
            }
        }
    } else if (team == Unit::Alliance::Neutral) {
        for (auto it = UnitManager::neutrals[type].begin(); it != UnitManager::neutrals[type].end(); it++) {
            if ((*it)->self == self) {
                UnitManager::neutrals[type].erase(it);
                break;
            }
        }
    } else if (team == Unit::Alliance::Enemy) {
        for (auto it = UnitManager::enemies[type].begin(); it != UnitManager::enemies[type].end(); it++) {
            if ((*it)->self == self) {
                UnitManager::enemies[type].erase(it);
                break;
            }
        }
    } else {
        printf("WHAT THE FUCK TEAM IS THIS UNIT ON\n");
    }

}

bool UnitWrapper::execute(Agent *agent) {
    return false;
}