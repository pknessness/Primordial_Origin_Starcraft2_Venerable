#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

#include "sc2lib/sc2_lib.h"
#include <map>
#include <list>
#include "probes.hpp"

using namespace sc2;

constexpr auto MINERALS_PER_PROBE_PER_SEC = 55.0 / 60;
constexpr auto VESPENE_PER_PROBE_PER_SEC = 61.0 / 60;

//typedef std::list<UnitOrder> UnitOrders;
//typedef std::map<uint64_t, UnitOrders> AllUnitOrders;
typedef struct {
    int minerals = 0;
    int vespene = 0;
    int energy = 0;
} Cost;

namespace MacroQueue {

    std::list<UnitTypeID> unitTypes;
	std::list<UnitOrder> actions;
	std::list<Cost> costs;

	static bool add(UnitTypeID unit_type, UnitOrder order, Cost c) {
        unitTypes.push_back(unit_type);
        actions.push_back(order);
        costs.push_back(c);
        return true;
	}

    static bool execute(Agent *agent) {
        int theoreticalMinerals = agent->Observation()->GetMinerals();
        int theoreticalVespene = agent->Observation()->GetVespene();

        if (costs.size() == 0 || actions.size() == 0 || unitTypes.size() == 0) {
            return false;
        }
        Cost cost = costs.front();
        UnitOrder action = actions.front();
        UnitTypeID unitType = unitTypes.front();

        int numMineralMiners = 0;
        int numVespeneMiners = 0;
        for (auto it = probes.begin(); it != probes.end(); it++) {
            if (Probe::isMineral(*agent->Observation()->GetUnit(it->second.minerals))) {
                numMineralMiners ++;
            }else if (Probe::isMineral(*agent->Observation()->GetUnit(it->second.minerals))) {
                numVespeneMiners++;
            }
        }

        sc2::Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Self);
        const sc2::Unit *un;
        if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
            double dist = -1;
            for (auto u : units) {
                if (u->unit_type == unitType) {
                    double d = agent->Query()->PathingDistance(u, action.target_pos);
                    if (dist == -1 || dist > d) {
                        // possibleUnits.push_back(u);
                        un = u;
                        dist = d;
                    }
                }
            }
        } else {
            for (auto u : units) {
                if (u->unit_type == unitType) {
                    un = u;
                }
            }
        }
        if (un == nullptr)
            return false;

        if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
            double dist = agent->Query()->PathingDistance(un, action.target_pos);
            const sc2::UnitTypes unit_data = agent->Observation()->GetUnitTypeData();
            sc2::UnitTypeData unit_stats = unit_data.at(static_cast<uint32_t>(UNIT_TYPEID::PROTOSS_PROBE));
            theoreticalMinerals += (dist / unit_stats.movement_speed) * MINERALS_PER_PROBE_PER_SEC * numMineralMiners;
            theoreticalVespene += (dist / unit_stats.movement_speed) * VESPENE_PER_PROBE_PER_SEC * numVespeneMiners;
        }

        if (cost.minerals <= theoreticalMinerals && cost.vespene <= theoreticalVespene) {
            if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
                agent->Actions()->UnitCommand(un, ABILITY_ID::STOP, false);
                probes[un->tag].addBuilding(action.ability_id,action.target_pos);
            } else {
                if (action.target_unit_tag != NullTag) {
                    agent->Actions()->UnitCommand(un, action.ability_id, action.target_pos,
                                                  false);
                } else {
                    const Unit *target = agent->Observation()->GetUnit(action.target_unit_tag);
                    agent->Actions()->UnitCommand(un, action.ability_id, target, false);
                }
            }

            costs.pop_front();
            unitTypes.pop_front();
            actions.pop_front();
            return true;
        }
        return false;
    }

}  // namespace OrderQueue
#endif
