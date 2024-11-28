#pragma once
#include <sc2api/sc2_api.h>

#include <map>
 
#include "constants.h"
#include "profiler.hpp"
#include "unit.hpp"

constexpr int BERTH = 1;

enum SquadMode {
    ATTACK,
    RETREAT,
    DEFEND,
    FULL_RETREAT
};

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
    return "HUH?";
}

struct DamageNet {
    Point2D pos;
    Weapon weapon;
};



class Squad {
public:    
    Point2D location;
    SquadMode mode;
    Composition comp;

    std::map<Tag, char> squadStates;

    float radius;
    int8_t ignoreFrames = 0;

    std::vector<UnitWrapper *> army;

    Squad() {
        army = std::vector<UnitWrapper *>();
        radius = 0;
        comp = Composition::Invalid;
        squadStates = std::map<Tag, char>();
    }

    bool has(UnitTypeID type) {
        for (UnitWrapper *u : army) {
            if (u->type == type) {
                return true;
            }
        }
        return false;
    }

    bool find(UnitTypeID tag) {
        for (UnitWrapper *u : army) {
            if (u->self == tag) {
                return true;
            }
        }
        return false;
    }

    Point2D center(Agent *agent) {
        Point2D center = {0, 0};
        if (army.size() == 0)
            return center;
        int cnt = 0;
        for (int i = 0; i < army.size(); i++) {
            if (army[i]->type == UNIT_TYPEID::PROTOSS_WARPPRISM) {
                continue;
            }
            center += army[i]->pos(agent);
            cnt++;
        }
        return (center / army.size());
    }

    bool withinRadius(Agent *agent) {
        float r = armyballRadius();
        Point2D cntr = center(agent);
        for (int i = 0; i < army.size(); i++) {
            if (army[i]->type == UNIT_TYPEID::PROTOSS_WARPPRISM) {
                continue;
            }
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
            if (army[i]->type == UNIT_TYPEID::PROTOSS_WARPPRISM) {
                continue;
            }
            if (Distance2D(army[i]->pos(agent), cntr) > r) {
                return false;
            }
        }
        return true;
    }

    Composition composition(Agent *agent) {
        if (comp != Composition::Invalid) {
            return comp;
        }
        bool air = false, gnd = false;
        for (int i = 0; i < army.size(); i++) {
            if (comp != Composition::Any) {
                singleUnitComp(army[i], agent);
            } else {
                break;
            }
        }
        return comp;
    }

    void singleUnitComp(UnitWrapper *unit, Agent *agent) {
        if (unit->get(agent)->is_flying) {
            if (comp == Composition::Invalid) {
                comp = Composition::Air;
                return;
            } else if (comp == Composition::Ground) {
                comp = Composition::Any;
                return;
            }
        } else {
            if (comp == Composition::Invalid) {
                comp = Composition::Ground;
                return;
            } else if (comp == Composition::Air) {
                comp = Composition::Any;
                return;
            }
        }
    }

    float priorityAttack(UnitTypeID self_type, Weapon weapon, const Unit *opponent, Agent *agent) {  // HIGHER IS MORE DESIRABLE TO ATTACK
        auto *padad = new Profiler("pA_o");
        //UnitTypes allData = Aux::allData(agent);
        //UnitTypeData myStats = allData.at(static_cast<uint32_t>(self_type));
        //UnitTypeData enemyStats = allData.at(static_cast<uint32_t>(opponent->unit_type));
        UnitTypeData myStats = Aux::getStats(self_type, agent);
        UnitTypeData enemyStats = Aux::getStats(opponent->unit_type, agent);
        Weapon strongestWeapon;
        float mostDmag = 0;
        delete padad;
        auto *pe = new Profiler("pA_eW");
        for (int i = 0; i < enemyStats.weapons.size(); i++) {
            if (Aux::hitsUnit(composition(agent), enemyStats.weapons[i].type)) {
                float damageE = enemyStats.weapons[i].damage_;
                for (int w = 0; w < enemyStats.weapons[i].damage_bonus.size(); w++) {
                    for (int a = 0; a < myStats.attributes.size(); a++) {
                        if (enemyStats.weapons[i].damage_bonus[w].attribute == myStats.attributes[a]) {
                            damageE += enemyStats.weapons[i].damage_bonus[w].bonus;
                        }
                    }
                }
                if ((mostDmag / strongestWeapon.speed) < (damageE / enemyStats.weapons[i].speed)) {
                    strongestWeapon = enemyStats.weapons[i];
                    mostDmag = damageE;
                }
            }
        }
        delete pe;
        auto pe2ads = Profiler("pA_mD");
        float damage = weapon.damage_;
        for (int w = 0; w < weapon.damage_bonus.size(); w++) {
            for (int a = 0; a < enemyStats.attributes.size(); a++) {
                if (weapon.damage_bonus[w].attribute == enemyStats.attributes[a]) {
                    damage += weapon.damage_bonus[w].bonus;
                }
            }
        }
        float prioriT = (damage / weapon.speed) * (strongestWeapon.damage_ / strongestWeapon.speed) /
                        (opponent->shield + opponent->health);
        return prioriT;
    }

    float priorityAvoid(UnitTypeID self_type, UnitTypeID opponent_type, Weapon weapon,
                        Agent *agent) {  // HIGHER IS MORE DESIRABLE TO AVOID
        //UnitTypes allData = agent->Observation()->GetUnitTypeData();
        //UnitTypeData myStats = allData.at(static_cast<uint32_t>(self_type));
        //UnitTypeData enemyStats = allData.at(static_cast<uint32_t>(opponent->unit_type));
        return 1;
    }

    float armyballRadius() {
        if (radius != 0)
            return radius;
        else {
            //printf("A:%d SQRT%.1f\n", army.size(), std::sqrt(army.size()));
            return std::sqrt(army.size())*2;
        }
    }

    bool execute(Agent *agent) {
        auto squadEx = Profiler("sE");
        if (army.size() == 6) {
            attack(agent->Observation()->GetGameInfo().enemy_start_locations[0]);
        }
        if (mode == ATTACK) {
            if (ignoreFrames > 0) {
                ignoreFrames--;
                return false;
            }
            UnitTypes allData = agent->Observation()->GetUnitTypeData();
            Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
            //std::vector<DamageNet> enemiesnet;
            //for (int e = 0; e < enemies.size(); e++) {
            //    UnitTypeData enemy_stats = allData.at(static_cast<uint32_t>(enemies[e]->unit_type));
            //    if (enemy_stats.weapons.size() == 0) {
            //        continue;
            //    }
            //    for (int w = 0; w < enemy_stats.weapons.size(); w++) {
            //        enemiesnet.push_back({enemies[e]->pos, enemy_stats.weapons[w]});
            //    }
            //}
            Tags move;
            //Tags all;
            //Tags atk;
            //Tag attackTarget;
            Targets attacking;
            Targets avoiding;
            map<Tag, Point2D> avoidingPos;
            int numArmyEngaged = 0;
            int numArmyEndangered = 0;
            for (auto wrap : army) {
                //auto preaf = Profiler(strprintf("sE - perUnit %s", UnitTypeToName(wrap->type)));
                auto preaf = Profiler("sE_pU");
                const Unit *wrapGet = wrap->get(agent);
                if (squadStates.find(wrap->self) == squadStates.end()) {
                    squadStates[wrap->self] = 'n';
                }

                if (squadStates[wrap->self] == 'n' && (wrapGet->shield / wrapGet->shield_max) < 0.05) {
                    squadStates[wrap->self] = 'r';
                } else if (squadStates[wrap->self] == 'r' && (wrapGet->shield/wrapGet->shield_max) > 0.95) {
                    squadStates[wrap->self] = 'n';
                }

                int berthRadius = BERTH;
                if (squadStates[wrap->self] == 'r') {
                    berthRadius += 4;
                }

                //all.push_back(wrap->self);
                if (enemies.size() == 0) {
                    move.push_back(wrap->self);
                } else {

                    UnitTypeData unit_stats = Aux::getStats(wrap->type, agent);  // allData.at(static_cast<uint32_t>(wrap->type));
                    UnitWrappers inRange = UnitWrappers();
                    std::vector<float> inRangePriority = std::vector<float>();
                    UnitWrappers dangerous = UnitWrappers();
                    std::vector<float> dangerousPriority = std::vector<float>();
                    for (auto it = UnitManager::enemies.begin(); it != UnitManager::enemies.end(); it++) {
                        auto all = it->second;
                        for (auto it2 = all.begin(); it2 != all.end(); it2++) {
                            if ((*it2)->pos(agent) == Point3D{0, 0, 0}) {
                                continue;
                            }
                            const Unit* enemy = (*it2)->get(agent);
                            // auto parg = Profiler(strprintf("sE - perEnemy %s",
                            // UnitTypeToName(enemies[e]->unit_type)));
                            auto parg = Profiler("sE_pE");
                            UnitTypeData enemy_stats =
                                Aux::getStats((*it2)->type, agent);  // allData.at(static_cast<uint32_t>(enemies[e]->unit_type));
                            for (int i = 0; i < enemy_stats.weapons.size(); i++) {
                                auto pre = Profiler("sE_pEW");
                                bool withinRange = ((Distance2D(wrapGet->pos, (*it2)->pos(agent)) - wrapGet->radius -
                                                     (*it2)->radius) < enemy_stats.weapons[i].range + berthRadius);
                                if (withinRange && Aux::hitsUnit(wrapGet, enemy_stats.weapons[i].type)) {
                                    // inRange.push_back(enemies[e]);
                                    bool inserted = false;
                                    float priority = priorityAvoid(wrap->type, (*it2)->type, enemy_stats.weapons[i], agent);
                                    if (dangerous.size() == 0) {
                                        inserted = true;
                                        dangerous.push_back(*it2);
                                        dangerousPriority.push_back(priority);
                                        // printf("%s %d\n", AbilityTypeToName(currentAction.ability),
                                        // currentAction.index);
                                    }
                                    for (int d = 0; d < dangerous.size(); d++) {
                                        if (dangerousPriority[d] < priority) {
                                            dangerous.insert(dangerous.begin() + d, *it2);
                                            dangerousPriority.insert(dangerousPriority.begin() + d, priority);
                                            inserted = true;
                                            break;
                                        }
                                    }
                                    if (inserted == false) {
                                        dangerous.push_back(*it2);
                                        dangerousPriority.push_back(priority);
                                    }
                                    numArmyEndangered++;
                                    break;
                                }
                            }
                            if (wrapGet->weapon_cooldown == 0 && enemy != nullptr) {
                                auto pdd = Profiler("sE_aMW");
                                for (int i = 0; i < unit_stats.weapons.size(); i++) {
                                    auto pdd = Profiler("sE_pMW");
                                    bool withinRange = ((Distance2D(wrapGet->pos, (*it2)->pos(agent)) - wrapGet->radius -
                                                         (*it2)->radius) < unit_stats.weapons[i].range);
                                    if (withinRange && Aux::hitsUnit(enemy, unit_stats.weapons[i].type)) {
                                        // inRange.push_back(enemies[e]);
                                        auto *pdd = new Profiler("sE_pMW_I");
                                        bool inserted = false;
                                        float priority =
                                            priorityAttack(wrap->type, unit_stats.weapons[i], enemy, agent);
                                        delete pdd;
                                        auto *pddads32 = new Profiler("sE_pMW_I1");
                                        if (inRange.size() == 0) {
                                            inserted = true;
                                            inRange.push_back(*it2);
                                            inRangePriority.push_back(priority);
                                            // printf("%s %d\n", AbilityTypeToName(currentAction.ability),
                                            // currentAction.index);
                                        }
                                        delete pddads32;
                                        auto *pddads33 = new Profiler("sE_pMW_I2");
                                        for (int d = 0; d < inRange.size(); d++) {
                                            if (inRangePriority[d] < priority) {
                                                inRange.insert(inRange.begin() + d, *it2);
                                                inRangePriority.insert(inRangePriority.begin() + d, priority);
                                                inserted = true;
                                                break;
                                            }
                                        }
                                        delete pddads33;
                                        auto *pddad4s = new Profiler("sE_pMW_I3");
                                        if (inserted == false) {
                                            inRange.push_back(*it2);
                                            inRangePriority.push_back(priority);
                                        }

                                        numArmyEngaged++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (squadStates[wrap->self] == 'r') {
                        if (dangerous.size() != 0) {
                            if (avoiding.find(dangerous.front()->self) == avoiding.end()) {
                                avoiding[dangerous.front()->self] = Tags();
                                avoidingPos[dangerous.front()->self] = dangerous.front()->pos(agent);
                            }
                            avoiding[dangerous.front()->self].push_back(wrap->self);
                        } else if (inRange.size() != 0) {
                            if (attacking.find(inRange.front()->self) == attacking.end()) {
                                attacking[inRange.front()->self] = Tags();
                            }
                            attacking[inRange.front()->self].push_back(wrap->self);
                        } else {
                            move.push_back(wrap->self);
                        }
                    } else {
                        if (inRange.size() != 0) {
                            if (attacking.find(inRange.front()->self) == attacking.end()) {
                                attacking[inRange.front()->self] = Tags();
                            }
                            attacking[inRange.front()->self].push_back(wrap->self);
                        } else if (dangerous.size() != 0) {
                            if (avoiding.find(dangerous.front()->self) == avoiding.end()) {
                                avoiding[dangerous.front()->self] = Tags();
                                avoidingPos[dangerous.front()->self] = dangerous.front()->pos(agent);
                            }
                            avoiding[dangerous.front()->self].push_back(wrap->self);
                        } else {
                            move.push_back(wrap->self);
                        }
                    }
                    
                    
                    
                    //for (int i = 0; i < enemiesnet.size(); i++) {
                    //    if (Aux::hitsUnit(wrap->type, enemiesnet[i].weapon.type)) {
                    //        
                    //    }
                    //}
                }
            }
            //agent->Actions()->UnitCommand(all, ABILITY_ID::ATTACK, location);
            auto pasd = Profiler("sE_uA");
            agent->Actions()->UnitCommand(move, ABILITY_ID::MOVE_MOVE, location);
            for (auto it = avoiding.begin(); it != avoiding.end(); it++) {
                Point3D avg = {0, 0, 0};
                for (int i = 0; i < it->second.size(); i++) {
                    avg += agent->Observation()->GetUnit(it->second[i])->pos;
                }
                avg /= (it->second.size());
                Point2D dir = normalize(avoidingPos[it->first] - avg) * 3;
                agent->Actions()->UnitCommand(it->second, ABILITY_ID::MOVE_MOVE, avg - dir);
                agent->Debug()->DebugLineOut(avg + Point3D{0, 0, 1}, Aux::addKeepZ(avg, -1*dir) + Point3D{0, 0, 1});
                //printf("AVOID %.1f,%.1f\n", -dir.x, -dir.y);
                ignoreFrames = 10;
            }
            for (auto it = attacking.begin(); it != attacking.end(); it++) {
                agent->Actions()->UnitCommand(it->second, ABILITY_ID::ATTACK, it->first);
                ignoreFrames = 10;
            }
            agent->Debug()->SendDebug();
            return false;
        } else if (mode == RETREAT) {
            return false;
        } else if (mode == DEFEND) {
            return false;
        } else if (mode == FULL_RETREAT) {
            return false;
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

    int8_t ignoreFrames = 0;

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
        if (ignoreFrames > 0) {
            ignoreFrames--;
            return false;
        }
        UnitTypes allData = agent->Observation()->GetUnitTypeData();
        Units enemies = agent->Observation()->GetUnits(Unit::Alliance::Enemy);
        std::vector<DamageNet> enemiesnet;
        Point2D displace;
        Point2D stutterDisplace;
        int numStutterAvg = 0;
        for (int e = 0; e < enemies.size(); e++) {
            UnitTypeData enemy_stats = allData.at(static_cast<uint32_t>(enemies[e]->unit_type));
            if (enemy_stats.weapons.size() == 0) {
                continue;
            }
            float distance = Distance2D(enemies[e]->pos, pos(agent));
            bool withinEnemyRadius = false;
            float r1r2 = enemies[e]->radius + get(agent)->radius;
            for (int i = 0; i < enemy_stats.weapons.size(); i++) {
                float r = enemy_stats.weapons[i].range +r1r2;
                if ((distance < r) &&
                    (enemy_stats.weapons[i].type ==
                         Weapon::TargetType::Any || (enemy_stats.weapons[i].type == Weapon::TargetType::Ground && !get(agent)->is_flying) ||
                     (enemy_stats.weapons[i].type == Weapon::TargetType::Air && get(agent)->is_flying))) {
                    //enemy_radius = enemy_stats.weapons[i].range;
                    withinEnemyRadius = true;
                    displace += normalize(pos(agent) - enemies[e]->pos) * (r - distance);
                }
            }
            if (displace == Point2D{0, 0}) {
                continue;
            }
        }
        if (std::sqrt(displace.x * displace.x + displace.y * displace.y) < 2) {
            displace = normalize(displace) * 2;
        }
        Point3D upos = pos3D(agent);
        Point3D blinkPos{upos.x + displace.x, upos.y + displace.y, upos.z};
        agent->Debug()->DebugLineOut(upos, blinkPos, {24, 123, 250});
        if (get(agent)->shield < 0.05) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::EFFECT_BLINK_STALKER, blinkPos);
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
        squad->comp = Composition::Invalid;
        for (auto it = squad->army.begin(); it != squad->army.end(); it++) {
            // printf("%lx %lx", )
            if ((*it)->self == self) {
                squad->army.erase(it);
                break;
            }
        }
    }
};