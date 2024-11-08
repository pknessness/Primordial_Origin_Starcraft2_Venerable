#ifndef PROBES_H
#define PROBES_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>
#include "constants.h"

using namespace sc2;

class Probe {
public:
    Tag minerals;
    std::list<UnitOrder> buildings;
    char state = 'm';

    static bool isMineral(const Unit &unit) {
        UnitTypeID type = unit.unit_type;
        return (type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD ||
                type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750 || type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
                type == UNIT_TYPEID::NEUTRAL_MINERALFIELD450);
    }

    static bool isVespene(const Unit &unit) {
        UnitTypeID type = unit.unit_type;
        return (type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
                type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER ||
                type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
        //return (type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER);
    }

    static bool isAssimilator(const Unit &unit) {
        UnitTypeID type = unit.unit_type;
        return (type == UNIT_TYPEID::PROTOSS_ASSIMILATOR);
    }

    bool init(Tag minerals_) {
        minerals = minerals_;
        return true;
    }

    bool addBuilding(AbilityID building, Point2D point, Tag geyser = NullTag) {
        buildings.push_back({ABILITY_ID::MOVE_MOVE, NullTag, point});
        buildings.push_back({building, geyser, point});
        return true;
    }

    bool execute(const Unit *unit, Agent *agent) {
        if (buildings.size() != 0) {
            Cost c = Aux::buildAbilityToCost(
                buildings.front().ability_id, agent);
            //printf("BuildAction: %d/%d %d/%d\n", agent->Observation()->GetMinerals(), c.minerals,
            //       agent->Observation()->GetVespene(), c.vespene);
            if (agent->Observation()->GetMinerals() < c.minerals || agent->Observation()->GetVespene() < c.vespene) {
                agent->Actions()->UnitCommand(unit, ABILITY_ID::STOP, false);
                return false;
            } 
            if (buildings.front().ability_id != ABILITY_ID::MOVE_MOVE && !agent->Query()->Placement(
                       buildings.front().ability_id, buildings.front().target_pos)) {
                if (agent->Query()->Placement(buildings.front().ability_id, buildings.front().target_pos + Point2D(0, 1))) {
                    buildings.front().target_pos = buildings.front().target_pos + Point2D(0, 1);
                } else if (agent->Query()->Placement(buildings.front().ability_id,buildings.front().target_pos + Point2D(0, -1))) {
                    buildings.front().target_pos = buildings.front().target_pos + Point2D(0, -1);
                } else if (agent->Query()->Placement(buildings.front().ability_id,buildings.front().target_pos + Point2D(1, 0))) {
                    buildings.front().target_pos = buildings.front().target_pos + Point2D(1, 0);
                } else if (agent->Query()->Placement(buildings.front().ability_id,buildings.front().target_pos + Point2D(-1, 0))) {
                    buildings.front().target_pos = buildings.front().target_pos + Point2D(-1, 0);
                } else {
                    return false;
                }
            }
            if (buildings.front().target_unit_tag == NullTag) {
                agent->Actions()->UnitCommand(unit, buildings.front().ability_id, buildings.front().target_pos, false);
            } else {
                const Unit *target = agent->Observation()->GetUnit(buildings.front().target_unit_tag);
                agent->Actions()->UnitCommand(unit, buildings.front().ability_id, target, false);
            }
            buildings.pop_front();
            return true;
        } else if (unit->orders.size() == 0 || unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER ||
                   buildings.size() == 0) {
            const Unit *target = agent->Observation()->GetUnit(minerals);
            agent->Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER, target, false);
            return true;
        }
        return false;
    }
};

std::map<uint64_t, Probe> probes;

#endif
