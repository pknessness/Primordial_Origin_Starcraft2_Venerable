#include "action.hpp"

/* create an action */
Action::Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_) {
    unit = unit_;
    abilityId = abilityId_;
    point = point_;
    pointInitialized = true;
}

/* create an action */
Action::Action(sc2::UnitTypeID unitType_, sc2::AbilityID abilityId_, const sc2::Point2D& point_) {
    unitType = unitType_;
    abilityId = abilityId_;
    point = point_;
    pointInitialized = true;
}

/* create an action */
Action::Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_) {
    unit = unit_;
    abilityId = abilityId_;
}

/* create an action */
void Action::setInterfaces(sc2::ActionInterface* actionInterface_, const sc2::ObservationInterface* observationInterface_) {
    actionInterface = actionInterface_;
    observationInterface = observationInterface_;
}

/* delete an action */
//Action::~Action() {
//}

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
            sc2::Units units = observationInterface->GetUnits(sc2::Unit::Alliance::Self);
            sc2::Units possibleUnits;
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
    return true;
}