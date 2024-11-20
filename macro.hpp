#pragma once
#include <sc2api/sc2_api.h>

#include <iostream>
#include <unordered_set>
#include <vector>

#include "probe.hpp"
//#include "jps.hpp"
//#include "grid.hpp"
#include "tools.hpp"
//#include "constants.h"

using namespace sc2;

#define ACTION_CHECK_DT 10
#define PROBE_CHECK_DACTION 3

struct MacroAction {
    static int globalIndex;
    UnitTypeID unit_type;
    AbilityID ability;
    Point2D pos;
    int index;
    uint32_t lastChecked;

    MacroAction(UnitTypeID unit_type_, AbilityID ability_, Point2D pos_)
        : unit_type(unit_type_), ability(ability_), pos(pos_), index(globalIndex), lastChecked(0){
        globalIndex ++;
    }

    MacroAction(UnitTypeID unit_type_, AbilityID ability_, Point2D pos_, int index)
        : unit_type(unit_type_), ability(ability_), pos(pos_), index(index), lastChecked(0) {
        globalIndex++;
    }

    Cost cost(Agent *agent) {
        return Aux::abilityToCost(ability, agent);
    }

    operator Building() const {
        return Building{ability, pos};
    }
};

namespace Macro {
    map<UnitTypeID, vector<MacroAction>> actions;

    void addAction(UnitTypeID unit_type_, AbilityID ability_, Point2D pos_) {
        if (actions.find(unit_type_) == actions.end()) {
            actions[unit_type_] = vector<MacroAction>();
        }
        actions[unit_type_].push_back(MacroAction(unit_type_, ability_, pos_));
    }

    void addActionTop(UnitTypeID unit_type_, AbilityID ability_, Point2D pos_, int index_) {
        if (actions.find(unit_type_) == actions.end()) {
            actions[unit_type_] = vector<MacroAction>();
        }
        actions[unit_type_].insert(actions[unit_type_].begin(), MacroAction(unit_type_, ability_, pos_, index_));
    }

    void addProbe() {
        addAction(UNIT_TYPEID::PROTOSS_NEXUS, ABILITY_ID::TRAIN_PROBE, {0, 0});
    }

    void addBuilding(AbilityID ability_, Point2D pos_) {
        addAction(UNIT_TYPEID::PROTOSS_PROBE, ability_, pos_);
    }

    void addBuildingTop(AbilityID ability_, Point2D pos_, int index_) {
        addActionTop(UNIT_TYPEID::PROTOSS_PROBE, ability_, pos_, index_);
    }

    void execute(Agent *agent) {
        vector<MacroAction> topActions = vector<MacroAction>();
        for (auto it = actions.begin(); it != actions.end(); it++) {
            //vector<MacroAction> acts = it->second;
            uint32_t gt = agent->Observation()->GetGameLoop();
            if (it->second.size() == 0 || gt < it->second.front().lastChecked + ACTION_CHECK_DT) {
                continue;
            }
            it->second.front().lastChecked = gt;
            if (topActions.size() == 0) {
                topActions.push_back(it->second.front());
            }
            for (auto it2 = topActions.begin(); it2 != topActions.end(); it2++) {
                if (it2->index > it->second.front().index) {
                    topActions.insert(it2, it->second.front());
                    break;
                }
            }
        }
        for (int i = 0; i < topActions.size(); i ++) {
            MacroAction topAct = topActions[i];
            Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Self, 
                [topAct](const Unit &unit) -> bool { 
                //return (unit.unit_type == topAct.unit_type) &&
                //       ((unit.unit_type == UNIT_TYPEID::PROTOSS_PROBE) || unit.orders.size() == 0);
                return (unit.unit_type == topAct.unit_type) && 
                       ((unit.unit_type == UNIT_TYPEID::PROTOSS_PROBE &&
                         (unit.orders.size() == 0 || unit.orders.front().ability_id == ABILITY_ID::HARVEST_RETURN)) ||
                        unit.orders.size() == 0);
            });
            if (units.size() == 0)
                continue;

            Cost c = Aux::abilityToCost(topAct.ability, agent);
            int theoreticalMinerals = agent->Observation()->GetMinerals();
            int theoreticalVespene = agent->Observation()->GetVespene();

            auto probes = UnitManager::get(UNIT_TYPEID::PROTOSS_PROBE);
            for (int i = 0; i < probes.size(); i ++) {
                vector<Building> buildings = ((Probe *)probes[i])->buildings;
                for (int b = 0; b < buildings.size(); b++) {
                    Cost c = buildings[b].cost(agent);
                    theoreticalMinerals -= c.minerals;
                    theoreticalVespene -= c.vespene;
                }
            }

            //if (theoreticalMinerals < c.minerals - 30 && theoreticalVespene < c.vespene - 30) {
            //    break;
            //}

            const Unit *actionUnit;

            UnitTypeData unit_stats =
                agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(topAct.unit_type));

            UnitTypeID prerequisite = unit_stats.tech_requirement;

            if (topAct.unit_type == UNIT_TYPEID::PROTOSS_PROBE && topAct.ability != ABILITY_ID::MOVE_MOVE &&
                topAct.ability != ABILITY_ID::MOVE_MOVEPATROL) {

                if (prerequisite != UNIT_TYPEID::INVALID && UnitManager::get(prerequisite).size() == 0) {
                    //addBuildingTop(Aux::unitToBuildAbility(prerequisite), )
                    continue;
                }

                Units viablePylons = Units();

                if (Aux::requiresPylon(topAct.ability)) {
                    auto pylons = UnitManager::get(UNIT_TYPEID::PROTOSS_PYLON);
                    bool foundPylon = false;
                    for (int i = 0; i < pylons.size(); i++) {
                        const Unit *pylon = agent->Observation()->GetUnit(pylons[i]->self);
                        if (Distance2D(pylon->pos, topAct.pos) < Aux::PYLON_RADIUS) {
                            viablePylons.push_back(pylon);
                            foundPylon = true;
                            if (pylon->is_building == false) {
                                break;
                            }
                        }
                    }
                    if (!foundPylon) {
                        continue;
                    }
                }
                

                float mindist = -1;
                for (const Unit *uni : units) {
                    Point2DI start = uni->pos;
                    Point2DI goal = topAct.pos;

                    auto came_from = jps(gridmap, start, goal, Tool::euclidean, agent);
                    vector<Location> pat = Tool::reconstruct_path(start, goal, came_from);
                    float dist = fullDist(pat);
                    //printf("%.2f %.2f\n", agent->Query()->PathingDistance(uni, topAct.pos), dist);

                    if (mindist == -1 || dist < mindist) {
                        mindist = dist;
                        actionUnit = uni;
                    }
                }

                // float dt = agent->Query()->PathingDistance(actionUnit, topAct.pos) / (unit_stats.movement_speed *
                // timeSpeed);
                float dt = mindist / (unit_stats.movement_speed * timeSpeed);

                
                if (Aux::requiresPylon(topAct.ability) && viablePylons.back()->build_progress != 1.0) {
                    UnitTypeData pylon_stats =
                        agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(UNIT_TYPEID::PROTOSS_PYLON));
                    bool found = false;
                    for (int i = 0; i < viablePylons.size(); i++) {
                        if (((1.0 - viablePylons[i]->build_progress) * pylon_stats.build_time / fps) < dt) {
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        continue;
                    }
                }

                if (prerequisite != UNIT_TYPEID::INVALID) {
                    UnitTypeData prereq_stats =
                        agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(prerequisite));

                    auto prereqs = UnitManager::get(prerequisite);
                    bool found = false;
                    for (int i = 0; i < prereqs.size(); i++) {
                        const Unit *prereq = agent->Observation()->GetUnit(prereqs[i]->self);
                        if (((1.0 - prereq->build_progress) * prereq_stats.build_time / fps) < dt) {
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        continue;
                    }
                }

                //printf("%.2f %.2f\n", agent->Query()->PathingDistance(actionUnit, topAct.pos), mindist);

                int numMineralMiners = 0, numVespeneMiners = 0;
                for (int i = 0; i < probes.size(); i ++) {
                    const Unit* target = agent->Observation()->GetUnit(((Probe *)probes[i])->getTargetTag(agent));
                    if (target == nullptr)
                        continue;
                    if (Aux::isMineral(*target)) {
                        numMineralMiners++;
                    } else if (Aux::isAssimilator(*target)) {
                        numVespeneMiners++;
                    }
                }

                theoreticalMinerals += (dt * Aux::MINERALS_PER_PROBE_PER_SEC * numMineralMiners);
                theoreticalVespene += (dt * Aux::VESPENE_PER_PROBE_PER_SEC * numVespeneMiners);
            } else {
                actionUnit = units[0];
            }

            printf("M:%d V:%d | %s %llx %s M:%d/%d V:%d/%d S:%d/%d\n", agent->Observation()->GetMinerals(),
                   agent->Observation()->GetVespene(),
                   UnitTypeToName(topAct.unit_type), actionUnit->tag,
                   AbilityTypeToName(topAct.ability), theoreticalMinerals,
                   c.minerals, theoreticalVespene, c.vespene, agent->Observation()->GetFoodUsed(), c.psi);
            if (theoreticalMinerals >= int(c.minerals) && theoreticalVespene >= int(c.vespene)) {
                if (topAct.pos != Point2D{0, 0}) {
                    if (topAct.unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
                        for (UnitWrapper* probe : probes) {
                            if (probe->self == actionUnit->tag) {
                                ((Probe *)probe)->addBuilding(topAct);
                                break;
                            }
                        }
                    } else {
                        agent->Actions()->UnitCommand(actionUnit, topAct.ability, topAct.pos);
                    }
                } else {
                    agent->Actions()->UnitCommand(actionUnit, topAct.ability);
                }
                actions[topAct.unit_type].erase(actions[topAct.unit_type].begin());
            }
            break;
        }
    }
}

int MacroAction::globalIndex = 0;