#pragma once
#include <sc2api/sc2_api.h>

#include <map>
 
#include "constants.h"
#include "unit.hpp"

enum SquadMode {
    ATTACK,
    RETREAT,
    DEFEND,
    FULL_RETREAT
};

enum Composition {NONE, AIR, GND, BOTH};

char *SquadModeToString(SquadMode mode) {
    if (mode == ATTACK) {
        return "ATTACK";
    } else if (mode == RETREAT) {
        return "RETREAT";
    } else if (mode == DEFEND) {
        return "DEFEND";
    } else if (mode == FULL_RETREAT) {
        return "FULL_RETREAT";
    }
}

class Squad {
public:    
    Point2D location;
    SquadMode mode;
    Composition comp;

    float radius;


    std::vector<UnitWrapper *> army;

    Squad() {
        army = std::vector<UnitWrapper *>();
        radius = 0;
        comp = NONE;
    }

    Point2D center(Agent *agent) {
        Point2D center = {0, 0};
        if (army.size() == 0)
            return center;
        for (int i = 0; i < army.size(); i++) {
            center += army[i]->pos(agent);
        }
        return (center / army.size());
    }

    bool withinRadius(Agent *agent) {
        float r = armyballRadius();
        Point2D cntr = center(agent);
        for (int i = 0; i < army.size(); i++) {
            if (Distance2D(army[i]->pos(agent), cntr) > r) {
                return false;
            }
        }
        return true;
    }

    Point2D centerGND(Agent *agent) {
        Point2D center = {0, 0};
        if (army.size() == 0)
            return center;
        int cnt = 0;
        for (int i = 0; i < army.size(); i++) {
            if (army[i]->get(agent)->is_flying) {
                continue;
            }
            center += army[i]->pos(agent);
            cnt++;
        }
        return (center / cnt);
    }

    bool withinRadiusGND(Agent *agent) {
        float r = armyballRadius();
        Point2D cntr = centerGND(agent);
        for (int i = 0; i < army.size(); i++) {
            if (Distance2D(army[i]->pos(agent), cntr) > r) {
                return false;
            }
        }
        return true;
    }

    Composition composition(Agent *agent) {
        if (comp != NONE) {
            return comp;
        }
        bool air = false, gnd = false;
        for (int i = 0; i < army.size(); i++) {
            if (comp != BOTH) {
                singleUnitComp(army[i], agent);
            } else {
                break;
            }
        }
        return comp;
    }

    void singleUnitComp(UnitWrapper *unit, Agent *agent) {
        if (unit->get(agent)->is_flying) {
            if (comp == NONE) {
                comp = AIR;
                return;
            } else if (comp == GND) {
                comp = BOTH;
                return;
            }
        } else {
            if (comp == NONE) {
                comp = GND;
                return;
            } else if (comp == AIR) {
                comp = BOTH;
                return;
            }
        }
    }

    float armyballRadius() {
        if (radius != 0)
            return radius;
        else {
            //printf("A:%d SQRT%.1f\n", army.size(), std::sqrt(army.size()));
            return std::sqrt(army.size());
        }
    }

    bool execute(Agent *agent) {
        if (army.size() == 6) {
            attack(agent->Observation()->GetGameInfo().enemy_start_locations[0]);
        }
        return false;
    }

    bool attack(Point2D location_) {
        mode = ATTACK;
        location = location_;
        return false;
    }

    bool retreat(Point2D location_) {
        mode = RETREAT;
        location = location_;
        return false;
    }

    bool defend(Point2D location_) {
        mode = DEFEND;
        location = location_;
        return false;
    }

    bool fullRetreat(Point2D location_) {
        mode = FULL_RETREAT;
        location = location_;
        return false;
    }
};

std::vector<Squad> squads = std::vector<Squad>();

class ArmyUnit : public UnitWrapper {
private:
    Tag target;
    UnitTypeData stats;
    bool is_flying;

public:
    Squad* squad;

    ArmyUnit(const Unit* unit) : UnitWrapper(unit) {
        target = NullTag;
        if (squads.size() == 0) {
            squads.emplace_back();
        }
        squads[0].army.push_back(this);
        squad = &squads[0];
        is_flying = unit->is_flying; 
    }

    UnitTypeData getStats(Agent *agent) {
        if (stats.unit_type_id == UNIT_TYPEID::INVALID) {
            UnitTypes allData = agent->Observation()->GetUnitTypeData();
            stats = allData.at(static_cast<uint32_t>(type));
        }
        return stats;
    }

    virtual bool execute(Agent *agent) {
        if (squad->mode == ATTACK) {
            return executeAttack(agent);
        } else if (squad->mode == RETREAT) {
            return executeRetreat(agent);
        } else if (squad->mode == DEFEND) {
            return executeDefend(agent);
        } else if (squad->mode == FULL_RETREAT) {
            return executeFullRetreat(agent);
        }
        return false;
    }

    virtual bool executeAttack(Agent *agent) {
        //UnitTypes allData = agent->Observation()->GetUnitTypeData();
        //Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
        //for (int e = 0; e < enemies.size(); e++) {
        //    UnitTypeData enemy_stats = allData.at(static_cast<uint32_t>(enemies[e]->unit_type));
        //    int enemy_radius = 0;
        //    bool withinEnemyRadius = false;
        //    for (int i = 0; i < enemy_stats.weapons.size(); i++) {
        //        if ((enemy_radius < enemy_stats.weapons[i].range) &&
        //            (enemy_stats.weapons[i].type ==
        //                 Weapon::TargetType::Any || (enemy_stats.weapons[i].type == Weapon::TargetType::Ground && !get(agent)->is_flying) ||
        //             (enemy_stats.weapons[i].type == Weapon::TargetType::Air && get(agent)->is_flying))) {
        //            enemy_radius = enemy_stats.weapons[i].range;
        //            withinEnemyRadius = true;
        //        }
        //    }

        //    int radius = 0;
        //    for (int i = 0; i < stats.weapons.size(); i++) {
        //        if (radius < stats.weapons[i].range) {
        //            radius = stats.weapons[i].range;
        //        }
        //    }
        //}
        if (is_flying) { //AIR UNIT
            if (squad->composition(agent) == AIR) {  // ALL AIR UNITS
                if (!squad->withinRadius(agent)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->center(agent));
                } else if (get(agent)->orders.size() == 0 || get(agent)->orders[0].target_pos != squad->location) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->location);
                }
            } else if (squad->composition(agent) == GND) {  // ALL GND UNITS
                printf("IMPOSSIBLE, HOW??");
            } else if (squad->composition(agent) == BOTH) {
                if (!squad->withinRadiusGND(agent)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->centerGND(agent));
                } else if (get(agent)->orders.size() == 0 || get(agent)->orders[0].target_pos != squad->location) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->location);
                }
            } else {
                printf("NONE, WHY??");
            }
        } else { //GROUND UNIT
            if (squad->composition(agent) == AIR) {  // ALL AIR UNITS
                printf("IMPOSSIBLE, HOW??");
            } else if (squad->composition(agent) == GND) {  // ALL GND UNITS
                if (!squad->withinRadius(agent)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->center(agent));
                } else if (get(agent)->orders.size() == 0 || get(agent)->orders[0].target_pos != squad->location) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->location);
                }
            } else if (squad->composition(agent) == BOTH) {  // BOTH
                if (!squad->withinRadiusGND(agent)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->centerGND(agent));
                } else if (!squad->withinRadius(agent)) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::STOP);
                } else if (get(agent)->orders.size() == 0 || get(agent)->orders[0].target_pos != squad->location) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->location);
                }
            } else {
                printf("NONE, WHY??");
            }
        }
        return false;
    }

    virtual bool executeRetreat(Agent *agent) {
        Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
        return false;
    }

    virtual bool executeDefend(Agent *agent) {
        Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
        float dist = Distance2D(squad->location, pos(agent));
        //printf("%s %.1f\n", UnitTypeToName(type), dist);
        if ((get(agent)->orders.size() == 0 || get(agent)->orders[0].target_pos != squad->location) &&
            dist > squad->armyballRadius()) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, squad->location);
        }
        return false;
    }

    virtual bool executeFullRetreat(Agent *agent) {
        Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
        return false;
    }

    virtual ~ArmyUnit() {
        squad->comp = NONE;
        for (auto it = squad->army.begin(); it != squad->army.end(); it++) {
            // printf("%lx %lx", )
            if ((*it)->self == self) {
                squad->army.erase(it);
                break;
            }
        }
    }
};