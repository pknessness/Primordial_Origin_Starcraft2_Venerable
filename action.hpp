#ifndef ACTION_H
#define ACTION_H

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"
#include <sc2api/sc2_interfaces.h>
#include "constants.h"

using namespace sc2;

constexpr auto MINERALS_PER_PROBE_PER_SEC = 55.0 / 60;
constexpr auto VESPENE_PER_PROBE_PER_SEC = 61.0 / 60;

class Action {
public:
    /* create an action */
    Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    /* create an action */
    Action(sc2::UnitTypeID unitType_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    /* create an action */
    Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_);

    /* create an action */
    /*static void setInterfaces(sc2::ActionInterface* actionInterface_,
                              const sc2::ObservationInterface* observationInterface_);

    static void setAgent(sc2::Agent* agent_);*/

    /* delete an action */
    //~Action();

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_);

    void buildStructure(sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    bool execute(sc2::Agent* a);

    UnitTypeID abilityIDToUnitID(sc2::AbilityID ability) {
        if (ability == sc2::ABILITY_ID::BUILD_PYLON) {
            return sc2::UNIT_TYPEID::PROTOSS_PYLON;
        }
        return 0;
    }

    // static sc2::ActionInterface* actionInterface;
    // static const sc2::ObservationInterface* observationInterface;
    sc2::AbilityID abilityId;
    const sc2::Unit* unit;
    sc2::UnitTypeID unitType;
    sc2::Point2D point;
    bool pointInitialized = false;
    int costMinerals = 0;
    int costVespene = 0;

private:
};

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
// void Action::setInterfaces(sc2::ActionInterface* actionInterface_,
//                            const sc2::ObservationInterface* observationInterface_) {
//     actionInterface = actionInterface_;
//     observationInterface = observationInterface_;
// }

/* delete an action */
// Action::~Action() {
// }

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

// bool Action::filterUnit(const Unit& unit) {
//     return unit.unit_type == unitType;
// }

bool Action::execute(sc2::Agent* a) {
    if (a->Actions() == nullptr) {
        printf("ERROR CODE 0x0101\n");
        return false;
    } else if (abilityId == 0) {
        printf("ERROR CODE 0x0102\n");
        return false;
    } else if (unitType == 0 && unit == nullptr) {
        printf("ERROR CODE 0x0103\n");
        return false;
    } else {
        if (unitType != 0 && unitType == UNIT_TYPEID::PROTOSS_PROBE) {
            sc2::Units units = a->Observation()->GetUnits(sc2::Unit::Alliance::Self);
            sc2::Units possibleUnits;
            const sc2::Unit *un;
            double dist = -1;
            for (auto u : units) {
                if (u->unit_type == unitType) {
                    double d = a->Query()->PathingDistance(u, point);
                    if (dist == -1 || dist > d) {
                        //possibleUnits.push_back(u);
                        un = u;
                        dist = d;
                    }
                }
            }
            const sc2::UnitTypes unit_data = a->Observation()->GetUnitTypeData();
            sc2::UnitTypeData unit_stats = unit_data.at(static_cast<uint32_t>(unitType));

            UnitTypeID building = abilityIDToUnitID(abilityId);
            sc2::UnitTypeData building_stats = unit_data.at(static_cast<uint32_t>(building));

            printf("%d->%d, %d->%d\n", building_stats.mineral_cost, int(a->Observation()->GetMinerals() + dist * unit_stats.movement_speed * MINERALS_PER_PROBE_PER_SEC), building_stats.vespene_cost, int(a->Observation()->GetVespene() + dist * unit_stats.movement_speed * VESPENE_PER_PROBE_PER_SEC));
            
            //int mineralCost = building_stats.mineral_cost;
            if (!(building_stats.mineral_cost < (a->Observation()->GetMinerals() + (dist / unit_stats.movement_speed) * MINERALS_PER_PROBE_PER_SEC)) ||
                !(building_stats.vespene_cost < (a->Observation()->GetVespene() + (dist / unit_stats.movement_speed) * VESPENE_PER_PROBE_PER_SEC))) {
                return false;
            }
            
            const Unit *prevTarget = a->Observation()->GetUnit(un->orders[0].target_unit_tag);
            AbilityID prevAction = un->orders[0].ability_id;

            printf("%s, %s\n", UnitTypeToName(prevTarget->unit_type), AbilityTypeToName(prevAction));

            a->Actions()->UnitCommand(un, abilityId, point, false);
            a->Actions()->UnitCommand(un, prevAction, prevTarget, true);
        } else {
            if (pointInitialized) {
                a->Actions()->UnitCommand(unit, abilityId, point, false);
            } else {
                a->Actions()->UnitCommand(unit, abilityId, false);
            }
        }
        
    }
    return true;
}

#endif
