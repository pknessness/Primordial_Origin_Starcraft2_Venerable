#ifndef NEXUS_H
#define NEXUS_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

#include "constants.h"
#include "probes.hpp"

using namespace sc2;

class Nexus {
public:
    Point3D loc;
    Tag vesp1;
    Tag vesp2;
    Tag self;
    bool vesp1Built;
    bool vesp2Built;

    Nexus(Point3D pos) : loc(pos), vesp1(NullTag), vesp2(NullTag), self(NullTag), vesp1Built(true), vesp2Built(true) {
    }

    bool exists(Agent *agent) {
        const Unit* nexus = agent->Observation()->GetUnit(self);
        return !(nexus == nullptr);
    }

    void built1() {
        vesp1Built = true;
    }

    void built2() {
        vesp2Built = true;
    }

    bool init(Agent *agent) {
        Units units = agent->Observation()->GetUnits(Unit::Alliance::Neutral, Probe::isVespene);
        for (const auto& u : units) {
            if (u->display_type != Unit::DisplayType::Visible)
                continue;
            float d = DistanceSquared2D(u->pos, loc);
            //printf("<%.1f [%.1f %.1f] %I64x %I64x>\n", d, u->pos.x, u->pos.y, vesp1, vesp2);
            if (d < 100) {
                if (vesp1 == NullTag) {
                    vesp1 = u->tag;
                    vesp1Built = false;
                    //printf("V1\n");
                } else if (vesp2 == NullTag) {
                    vesp2 = u->tag;
                    vesp2Built = false;
                    //printf("V2\n");
                } else {
                    printf("THIRD VESPENE????");
                }
            }
        }
        return true;
    }
    
};

std::vector<Nexus> nexuses;

#endif
