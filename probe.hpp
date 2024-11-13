#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "unit.hpp"
#include "constants.h"

map<Tag, int8_t> probeTargetting;

class Probe : public UnitWrapper {
private:
    vector<Building> buildings;

public:
    Tag target;

    Probe(Tag self_) : UnitWrapper(self_, UNIT_TYPEID::PROTOSS_PROBE) {
        target = 0;
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
                if (Aux::isMineral(*targ))
                    limit = 2;
                else if (Aux::isAssimilator(*targ)) {
                    limit = 3;
                } else {
                    printf("ERROR\n");
                    continue;
                }
                if (nearest == nullptr || ((Distance2D(nearest->pos, targ->pos) >
                                            Distance2D(agent->Observation()->GetUnit(self)->pos, targ->pos)) &&
                                           probeTargetting[targ->tag] < limit)) {
                    nearest = targ;
                }
            }
            target = nearest->tag;
            probeTargetting[target] += 1;
        }
        return target;
    }

    bool execute(Agent *agent) {
        if (buildings.size() == 0) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, getTargetTag(agent));
            agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_RETURN);
        }    
        return true;
    }
};