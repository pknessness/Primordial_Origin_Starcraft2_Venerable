#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

#include "sc2lib/sc2_lib.h"
#include <map>
#include <list>
#include "probes.hpp"
#include "constants.h"

using namespace sc2;

constexpr auto MINERALS_PER_PROBE_PER_SEC = 55.0 / 60;
constexpr auto VESPENE_PER_PROBE_PER_SEC = 61.0 / 60;

//typedef std::list<UnitOrder> UnitOrders;
//typedef std::map<uint64_t, UnitOrders> AllUnitOrders;

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

    static bool addUpgrade(UnitTypeID unit_type, ABILITY_ID upgrade, Agent *agent) {
        unitTypes.push_back(unit_type);
        actions.push_back({upgrade});
        costs.push_back(Aux::UpgradeToCost(upgrade, agent));
        return true;
    }

    static bool addBuilding(UnitOrder order, Agent *agent) {
        unitTypes.push_back(UNIT_TYPEID::PROTOSS_PROBE);
        actions.push_back(order);
        costs.push_back(Aux::buildAbilityToCost(order.ability_id, agent));
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

        UnitTypeData unitToBuild = agent->Observation()->GetUnitTypeData().at(
            static_cast<uint32_t>(Aux::buildAbilityToUnit(action.ability_id)));

        UnitTypeID prereq = unitToBuild.tech_requirement;
        if (Aux::buildAbilityToUnit(action.ability_id) == UNIT_TYPEID::PROTOSS_GATEWAY) {
            prereq = UNIT_TYPEID::PROTOSS_PYLON;
        }else if (action.ability_id == ABILITY_ID::RESEARCH_WARPGATE ||
            action.ability_id == ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL1 ||
            action.ability_id == ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1) {
            prereq = UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
        } else if (action.ability_id == ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1 ||
                   action.ability_id == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1) {
            prereq = UNIT_TYPEID::PROTOSS_FORGE;
        }

        int numMineralMiners = -1;
        int numVespeneMiners = 0;
        for (auto it = probes.begin(); it != probes.end(); it++) {
            if (agent->Observation()->GetUnit(it->second.minerals) == nullptr)
                continue;
            if (Probe::isMineral(*agent->Observation()->GetUnit(it->second.minerals))) {
                numMineralMiners ++;
            }else if (Probe::isAssimilator(*agent->Observation()->GetUnit(it->second.minerals))) {
                numVespeneMiners++;
            }
        }

        bool prerequisite = (prereq == UNIT_TYPEID::INVALID);

        sc2::Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Self);
        const sc2::Unit *un;
        bool foundUnit = false;
        if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
            double dist = -1;
            for (auto u : units) {
                const Unit *mineral = agent->Observation()->GetUnit(probes[u->tag].minerals);
                if (u->unit_type == unitType && mineral != nullptr && Probe::isMineral(*mineral)) {
                    double d = agent->Query()->PathingDistance(u, action.target_pos);
                    if (dist == -1 || dist > d) {
                        // possibleUnits.push_back(u);
                        un = u;
                        dist = d;
                        foundUnit = true;
                    }
                }
            }
        } else {
            for (auto u : units) {
                //printf("-%lx, %s?%s-\n", u->tag, UnitTypeToName(u->unit_type), UnitTypeToName(unitType));
                if (u->unit_type == unitType) {
                    un = u;
                    foundUnit = true;
                }
            }
        }
        if (!foundUnit)
            return false;

        if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
            UnitTypeData prereqData = agent->Observation()->GetUnitTypeData().at(
                static_cast<uint32_t>(prereq));

            UnitTypeData unit_stats =
                agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(UNIT_TYPEID::PROTOSS_PROBE));
            
            double dt = agent->Query()->PathingDistance(un, action.target_pos) / (unit_stats.movement_speed * timeSpeed);

            theoreticalMinerals += dt * MINERALS_PER_PROBE_PER_SEC * numMineralMiners;
            theoreticalVespene += dt * VESPENE_PER_PROBE_PER_SEC * numVespeneMiners;

            for (auto u : units) {
                if (u->unit_type == prereq && ((1.0 - u->build_progress) * prereqData.build_time/fps) < dt) {
                    prerequisite = true;
                }
            }
        } else {
            for (auto u : units) {
                if (u->unit_type == prereq && u->build_progress == 1.0) {
                    prerequisite = true;
                }
            }
        }

        

        //printf("%s [%xu] [%.1f, %.1f] M%d/%d V%d/%d PRE:%s? %d\n", AbilityTypeToName(action.ability_id), action.target_unit_tag, action.target_pos.x, action.target_pos.y, theoreticalMinerals,
        //       cost.minerals, theoreticalVespene, cost.vespene, UnitTypeToName(prereq), prerequisite);

        agent->Debug()->DebugTextOut(
            strprintf("%s [%xu] [%.1f, %.1f] M%d/%d V%d/%d PRE:%s? %d\n", AbilityTypeToName(action.ability_id),
                      action.target_unit_tag, action.target_pos.x, action.target_pos.y, theoreticalMinerals,
                      cost.minerals, theoreticalVespene, cost.vespene, UnitTypeToName(prereq), prerequisite),
                              Point2D(0.01, 0.6), Color(100, 190, 215), 8);

        if (cost.minerals <= theoreticalMinerals && cost.vespene <= theoreticalVespene && prerequisite) {
            if (unitType == UNIT_TYPEID::PROTOSS_PROBE) {
                agent->Actions()->UnitCommand(un, ABILITY_ID::STOP, false);
                probes[un->tag].addBuilding(action.ability_id,action.target_pos, action.target_unit_tag);
            } else {
                if (action.target_unit_tag == NullTag) {
                    agent->Actions()->UnitCommand(un, action.ability_id, action.target_pos,
                                                  false);
                } else if(action.target_pos.x != 0 || action.target_pos.y != 0){
                    const Unit *target = agent->Observation()->GetUnit(action.target_unit_tag);
                    agent->Actions()->UnitCommand(un, action.ability_id, target, false);
                } else {

                    agent->Actions()->UnitCommand(un, action.ability_id, false);
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
