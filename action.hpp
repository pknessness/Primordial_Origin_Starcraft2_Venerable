#ifndef ACTION_H
#define ACTION_H

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"

class Action {
public:
    /* create an action */
    Action();

    /* create an action */
    static void setInterfaces(sc2::ActionInterface* actionInterface_,
                              const sc2::ObservationInterface* observationInterface_);

    static void setAgent(sc2::Agent* agent_);

    /* delete an action */
    ~Action();

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_);

    void buildStructure(sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    bool filterUnit(const Unit& unit) {
        return unit.unit_type == unitType;
    }

    bool execute() const;

private:
    static sc2::ActionInterface* actionInterface;
    static const sc2::ObservationInterface* observationInterface;
    sc2::AbilityID abilityId;
    const sc2::Unit* unit;
    sc2::UnitTypeID unitType;
    sc2::Point2D point;
    bool pointInitialized = false;
    int costMinerals = 0;
    int costVespene = 0;
};


/* create an action */
Action::Action() {
}

/* create an action */
void Action::setInterfaces(sc2::ActionInterface* actionInterface_, const sc2::ObservationInterface* observationInterface_) {
    actionInterface = actionInterface_;
    observationInterface = observationInterface_;
}

/* delete an action */
Action::~Action() {
}

void Action::init(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_) {
    unit = unit_;
    abilityId = abilityId_;
    point = point_;
    pointInitialized = true;
}

void Action::init(const sc2::Unit* unit_, sc2::AbilityID abilityId_) {
    unit = unit_;
    abilityId = abilityId_;
}

void Action::buildStructure(sc2::AbilityID abilityId_, const sc2::Point2D& point_) {
    unitType = sc2::UNIT_TYPEID::PROTOSS_PROBE;
    abilityId = abilityId_;
    point = point_;
    pointInitialized = true;
}

//bool Action::filterUnit(const Unit& unit) {
//    return unit.unit_type == unitType;
//}

bool Action::execute() const {
    if (actionInterface == nullptr) {
        printf("ERROR CODE 0x0101\n");
    } else if (abilityId == 0) {
        printf("ERROR CODE 0x0102\n");
    } else if (unitType == 0 && unit == nullptr) {
        printf("ERROR CODE 0x0103\n");
    } else {
        if (unitType != 0) {
            Units units = observationInterface->GetUnits(Unit::Alliance::Self);
            Units possibleUnits;
            for (auto u : units) {
                if (u->unit_type == unitType) {
                    possibleUnits.push_back(u);
                }
            }
            // unit =
        }
        if (pointInitialized) {
            actionInterface->UnitCommand(unit, abilityId, point, false);
        } else {
            actionInterface->UnitCommand(unit, abilityId, false);
        }
    }
}

//struct Action {  // Structure declaration
//    sc2::AbilityID abilityId;
//    const sc2::Unit* unit;
//    sc2::UnitTypeID unitType;
//    sc2::Point2D point;
//    bool pointInitialized;
//};

#endif
