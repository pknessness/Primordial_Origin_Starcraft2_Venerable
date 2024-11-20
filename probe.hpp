#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "unit.hpp"
#include "constants.h"

map<Tag, int8_t> probeTargetting;

class Probe : public UnitWrapper {
private:
    Tag target;

public:
    vector<Building> buildings;
    AvailableAbilities abilities;

    Probe(Tag self_) : UnitWrapper(self_, UNIT_TYPEID::PROTOSS_PROBE) {
        target = 0;
    }

    bool addBuilding(Building building) {
        buildings.push_back(building);
        return true;
    }

    Tag getTargetTag(Agent *agent) {
        if (target == NullTag) {
            Units minerals = agent->Observation()->GetUnits(Unit::Alliance::Neutral, Aux::isMineral);
            Units assimilators = agent->Observation()->GetUnits(Unit::Alliance::Self, Aux::isAssimilator);
            minerals.insert(minerals.end(), assimilators.begin(), assimilators.end());

            const Unit *nearest = nullptr;
            for (const Unit *targ : minerals) {
                if (targ->display_type != Unit::DisplayType::Visible) {
                    continue;
                }
                if (probeTargetting.find(targ->tag) == probeTargetting.end()) {
                    probeTargetting[targ->tag] = 0;
                }
                int limit = 0;
                if (Aux::isMineral(*targ)) {
                    limit = 2;
                } else if (Aux::isAssimilator(*targ)) {
                    limit = 3;
                } else {
                    printf("ERROR\n");
                    continue;
                }
                if (probeTargetting[targ->tag] >= limit)
                    continue;
                if (nearest == nullptr || Distance2D(nearest->pos, targ->pos) >
                                            Distance2D(agent->Observation()->GetUnit(self)->pos, targ->pos)) {
                    nearest = targ;
                }
            }
            
            if (nearest == nullptr) {
                return NullTag;
            }
            target = nearest->tag;
            probeTargetting[target] += 1;
        }
        return target;
    }

    bool execute(Agent *agent) {
        if (buildings.size() == 0) {
            if (agent->Observation()->GetUnit(self)->orders.size() == 0 ||
                (agent->Observation()->GetUnit(self)->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER &&
                 agent->Observation()->GetUnit(self)->orders[0].target_unit_tag != target)) {
                if (checkAbility(ABILITY_ID::HARVEST_RETURN)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_RETURN);
                } else {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, getTargetTag(agent));
                }
            }
            
        } else {
            Building top = buildings[0];
            if (Distance2D(agent->Observation()->GetUnit(self)->pos, top.pos) < 1) {
                Cost c = Aux::buildAbilityToCost(top.build, agent);
                if (c.minerals > agent->Observation()->GetMinerals() || c.vespene > agent->Observation()->GetVespene())
                    return false;
                if (top.build == ABILITY_ID::BUILD_ASSIMILATOR) {
                    //agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_RETURN);
                } else {
                    agent->Actions()->UnitCommand(self, top.build, top.pos);
                }
                buildings.erase(buildings.begin());
            } else {
                agent->Actions()->UnitCommand(self, ABILITY_ID::MOVE_MOVE, top.pos);
            }
        }
        return true;
    }

    static void loadAbilities(Agent *agent) {
        Units u;
        vector<UnitWrapper *> probes = UnitManager::get(UNIT_TYPEID::PROTOSS_PROBE);
        for (UnitWrapper *probe : probes) {
            u.push_back(agent->Observation()->GetUnit(probe->self));
        }
        vector<AvailableAbilities> allAb = agent->Query()->GetAbilitiesForUnits(u);
        for (int i = 0; i < allAb.size(); i++) {
            if (probes[i]->self == allAb[i].unit_tag) {
                ((Probe *)probes[i])->abilities = allAb[i];
            } else {
                printf("ABILITY ASSIGNMENT ERROR\n");
            }
        }
    }

    bool checkAbility(AbilityID ability) {
        for (int i = 0; i < abilities.abilities.size(); i++) {
            if (ability == abilities.abilities[i].ability_id) {
                return true;
            }
        }
        return false;
    }
};