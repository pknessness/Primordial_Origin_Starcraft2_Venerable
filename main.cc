#include <sc2api/sc2_api.h>
//#include "sc2api/sc2_unit_filters.h"
#include "sc2lib/sc2_lib.h"
//#include "dt.h"

#include "pathfinding.h"
#include "dist_transform.h"
#include "AStar.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <queue>
#include <list>
#include <cmath>
#include "actionQueue.hpp"
#include "probes.hpp"


constexpr auto PI = 3.14159263;
constexpr auto PYLON_RADIUS = 6.5;

using namespace sc2;

class Bot : public Agent {
public:
    std::vector<Point3D> expansions;
    std::vector<Point3D> rankedExpansions;
    std::vector<double> expansionOrder;

    const ObservationInterface* observer;
    GameInfo game_info;
    Point3D startLocation;
    Point2DI staging_location;

    AStar::Generator generator;

    std::vector<Point2D> pylons;
    int pylonIndex = 0;
    std::vector<Point2D> buildingSpots;
    int buildSpotIndex = 0;

    map<std::string>* display;

    map<uint8_t>* placements;

    //std::vector<Point2DI> numberDisplayLoc;
    //std::vector<double> numberDisplay;

    static bool isPylon(const Unit& unit) {
        UnitTypeID type = unit.unit_type;
        return (type == UNIT_TYPEID::PROTOSS_PYLON);
    }

    static bool isNexus(const Unit& unit) {
        UnitTypeID type = unit.unit_type;
        return (type == UNIT_TYPEID::PROTOSS_NEXUS);
    }

    bool addNewPylonSlot() {
        for (int i = 1; i < pylons.size(); i++) {
            for (int j = 0; j < 4; j ++) {
                float angle = 2 * PI * (std::rand() % 8192) / 8192.0;
                Point2D potential(pylons[i].x + cos(angle) * 6.2,pylons[i].y + sin(angle) * 6.2);
                if (Query()->Placement(ABILITY_ID::BUILD_PYLON, potential)) {
                    pylons.push_back(potential);
                    generatePlacement(potential, 2);
                    return true;
                }
            }
        }
        return false;
    }

    bool generatePlacement(Point2D p, int size) {
        int x = p.x - (size / 2);
        int y = p.y - (size / 2);
        for (int i = x; i < x + size; i++) {
            for (int j = y; j < y + size; j++) {
                printf("{%d,%d}", i, j);
                imRef(placements, i, j) = 1;
            }
        }
        return true;
    }

    bool ungeneratePlacement(Point2D p, int size) {
        int x = p.x - (size / 2);
        int y = p.y - (size / 2);
        for (int i = x; i < x + size; i++) {
            for (int j = y; j < y + size; j++) {
                imRef(placements, i, j) = 0;
            }
        }
        return true;
    }

    bool checkPlacement(Point2D p, int size) {
        int x = p.x - (size / 2);
        int y = p.y - (size / 2);
        for (int i = x; i < x + size; i++) {
            for (int j = y; j < y + size; j++) {
                Point2D check(i, j);
                if (!observer->IsPlacable(check) || imRef(placements, int(check.x), int(check.y))) {
                    return false;
                }
            }
        }
        return true;
    }

    bool check3x3Placement(Point2D p) {
        /*for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (!observer->IsPlacable(p + Point2D(i, j)) || imRef(placements, int(p.x + i), int(p.y + j))) {
                    return false;
                }
            }
        }
        return true;*/
        return checkPlacement(p, 3);
    }

    bool check2x2Placement(Point2D p) {
        /*for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (!observer->IsPlacable(p + Point2D(i, j)) || imRef(placements, int(p.x + i), int(p.y + j))) {
                    return false;
                }
            }
        }
        return true;*/
        return checkPlacement(p, 2);
    }

    bool addNewBuildingSlot() {
        Units pyl = Observation()->GetUnits(Unit::Alliance::Self, isPylon);
        for (int i = 0; i < pyl.size(); i++) {
            for (int j = 0; j < 4; j++) {
                float angle = j * PI / 2;
                Point2D potential(pyl[i]->pos.x + cos(angle) * 3, pyl[i]->pos.y + sin(angle) * 3);
                printf("[%.1f, %.1f]", potential.x, potential.y);
                if (checkPlacement(potential, 3)) {
                    printf("YES");
                    imRef(display, int(potential.x), int(potential.y)) = strprintf("YES");
                    buildingSpots.push_back(potential);
                    generatePlacement(potential, 3);
                    return true;
                } else {
                    printf("NO");
                    imRef(display, int(potential.x), int(potential.y)) = strprintf("NO");
                }
            }
        }
        return false;
    }

    Point2D getBuildingSpot() {
        if (buildingSpots.size() <= buildSpotIndex)
            addNewBuildingSlot();
        return buildingSpots[buildSpotIndex++];
    }

    Point2D getPylonSpot() {
        if (pylons.size() <= pylonIndex)
            addNewPylonSlot();
        return pylons[pylonIndex++];
    }

    const Unit* FindNearestMineralPatch(const Point2D& start) {
        Units units = Observation()->GetUnits(Unit::Alliance::Neutral, Probe::isMineral);
        Units neutrals = Observation()->GetUnits(Unit::Alliance::Self, Probe::isAssimilator);
        units.insert(units.end(), neutrals.begin(), neutrals.end());

        float distance = std::numeric_limits<float>::max();
        const Unit* target = nullptr;
        for (const auto& u : units) {
            if (u->display_type != Unit::DisplayType::Visible)
                continue;
            if (mineralTargetting.find(u->tag) != mineralTargetting.end() &&
                ((Probe::isMineral(*u) && mineralTargetting[u->tag] >= 2) ||
                 (Probe::isAssimilator(*u) && mineralTargetting[u->tag] >= 3))) {
                continue;
            }
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
        // If we never found one return false;
        if (distance == std::numeric_limits<float>::max()) {
            return target;
        }
        return target;
    }

    const Unit* FindNearestVespene(const Point2D& start, float range = 100) {
        Units units = Observation()->GetUnits(Unit::Alliance::Neutral, Probe::isVespene);
        Units assimilators = Observation()->GetUnits(Unit::Alliance::Self, Probe::isAssimilator);
        float distance = range;
        if (distance == 0)
            distance = std::numeric_limits<float>::max();
        const Unit* target = nullptr;
        for (const auto& u : units) {
            float d = DistanceSquared2D(u->pos, start);
            
            bool assim = false;
            for (int i = 0; i < assimilators.size(); i++) {
                if (DistanceSquared2D(u->pos, assimilators[i]->pos) < 1) {
                    assim = true;
                    break;
                }
            }
            printf("<%.1f %d %d>\n", d, !assim, !imRef(placements, int(u->pos.x), int(u->pos.y)));
            if (!assim && d < distance && !imRef(placements, int(u->pos.x), int(u->pos.y))) {
                distance = d;
                target = u;
            }
        }
        // If we never found one return false;
        if (distance == std::numeric_limits<float>::max()) {
            return target;
        }
        if (target != nullptr)
            generatePlacement(target->pos, 3);
        return target;
    }

    Point3D P3D(const Point2D& p) {
        return Point3D(p.x, p.y, observer->TerrainHeight(p));
    }

    Point2D P2D(const Point2DI& p) {
        return Point2D(p.x, p.y);
    }

    Point2D P2D(const Point3D& p) {
        return Point2D(p.x, p.y);
    }

    virtual void OnGameStart() final {
        std::cout << "Hello, World!" << std::endl;

        observer = Observation();
        game_info = observer->GetGameInfo();

        //Action::setInterfaces(Actions(), observer);

        generator.setWorldSize(Point2DI(game_info.width, game_info.height));
        generator.setHeuristic(AStar::Heuristic::octagonal);
        generator.setDiagonalMovement(true);
        generator.setGameInfo(&game_info);

        display = new map<std::string>(game_info.width, game_info.height, true);
        placements = new map<uint8_t>(game_info.width, game_info.height, true);

        for (int x = 0; x < game_info.width; x++) {
            for (int y = 0; y < game_info.height; y++) {
                imRef(display, x, y) = "";
            }
        }

        #define stageing 3;

        // Temporary, we can replace this with observation->GetStartLocation() once implemented
        startLocation = observer->GetStartLocation();
        if (startLocation.x > game_info.width / 2) {
            staging_location.x = startLocation.x - stageing;
        } else {
            staging_location.x = startLocation.x + stageing;
        }

        if (startLocation.y > game_info.height / 2) {
            staging_location.y = startLocation.y - stageing;
        } else {
            staging_location.y = startLocation.y + stageing;
        }

        //staging_location = Point2DI(startLocation.x + ;
        expansions = sc2::search::CalculateExpansionLocations(observer, Query());
        for (auto point : expansions) {
            //auto path = generator.findPath(staging_location, (Point2DI)point);
            //expansionOrder.push_back(path.size());

            expansionOrder.push_back(Distance2D(P2D(staging_location), point));
        }

        std::vector<double> rankedExpansionOrder;

        for (int j = 0; j < expansions.size() - 1; j++) {
            if (expansions[j].x == 0 && expansions[j].y == 0)
                continue;
            //printf("sorting [%2.1f,%2.1f, %2.1f]\n", expansions[j].x, expansions[j].y, expansionOrder[j]);
            if (rankedExpansions.size() == 0){
                rankedExpansions.push_back(expansions[j]);
                rankedExpansionOrder.push_back(expansionOrder[j]);
                continue;
            }
            bool inserted = false;
            for (int i = 0; i < rankedExpansions.size(); i++) {
                //printf("insert into [%2.1f,%2.1f, %2.1f]?\n", rankedExpansions[i].x, rankedExpansions[i].y,
                //       rankedExpansionOrder[i]);
                if (rankedExpansionOrder[i] > expansionOrder[j]) {
                    rankedExpansions.insert(rankedExpansions.begin() + i, expansions[j]);
                    rankedExpansionOrder.insert(rankedExpansionOrder.begin() + i, expansionOrder[j]);
                    imRef(display, int(expansions[j].x), int(expansions[j].y)) = strprintf("%.1f", expansionOrder[j]);
                    inserted = true;
                    break;
                }
                
            }
            if (!inserted) {
                rankedExpansions.push_back(expansions[j]);
                rankedExpansionOrder.push_back(expansionOrder[j]);
                imRef(display, int(expansions[j].x), int(expansions[j].y)) = strprintf("%.1f", expansionOrder[j]);
            }
        }
        /*for (int i = 0; i < rankedExpansions.size() - 1; i++) {
            printf("{%d,%d, %f} ", ((Point2DI)rankedExpansions[i]).x, ((Point2DI)rankedExpansions[i]).y,
                   rankedExpansionOrder[i]);
        }*/

        //pylons.push_back()
        PathingGrid pathinggrid(game_info);

        auto wallOffPylonPath = generator.findPath(Point2DI(rankedExpansions[0]), {game_info.width / 2, game_info.height / 2});
        std::reverse(wallOffPylonPath.begin(), wallOffPylonPath.end());
        float angle = 0;
        Point2DI end1(0, 0);
        Point2DI end2(0, 0);
        for (auto& coordinate : wallOffPylonPath) {
            //std::cout << coordinate.x << " " << coordinate.y << "\n";
            Point2D pt(coordinate.x, coordinate.y);
            int diam = -1;
            for (int a = 0; a < 8; a++) {
                double angle = a * PI / 16;

                #define MAX_RANGE 16

                int d1 = MAX_RANGE;
                int d2 = MAX_RANGE;
                Point2D disp(cos(angle), sin(angle));

                //printf("[c%f,s%f]\n", disp.x, disp.y);
                for (int od = 0; od < MAX_RANGE; od++) {

                    //printf("[c%f,%fc]\n", (pt + od * disp).x, (pt + od * disp).y);
                    if (!pathinggrid.IsPathable(pt + od * disp)) {
                        //printf("[c%f,%fc]\n", (pt + od * disp).x, (pt + od * disp).y);
                        d1 = od;
                        break;
                    }
                }
                for (int od = 0; od < MAX_RANGE; od++) {
                    //printf("[c%f,%fc]\n", (pt - od * disp).x, (pt - od * disp).y);
                    if (!pathinggrid.IsPathable(pt - od * disp)) {
                        //printf("[c%f,%fc]\n", (pt - od * disp).x, (pt - od * disp).y);
                        d2 = od;
                        break;
                    }
                }
                if (diam == -1 || diam > (d1 + d2)) {
                    diam = d1 + d2;
                    end1 = pt + (d1 - 1) * disp;
                    end2 = pt - (d2 - 1) * disp;
                }
                //printf("[%d,%d; %f %d]", coordinate.x, coordinate.y, angle, d1 + d2);
            }
            if (diam < 15) {
                imRef(display, end1.x, end1.y) = strprintf("end1");
                imRef(display, end2.x, end2.y) = strprintf("end2");
                break;
            }
            //printf("[%d,%d; %d]", coordinate.x, coordinate.y, diam);
            //imRef(display, coordinate.x, coordinate.y) = strprintf("%d", diam);
        }
        Point2D center(end1.x / 2.0 + end2.x / 2.0, end1.y / 2.0 + end2.y / 2.0);
        double half_leg = sqrt((end1.x - end2.x) * (end1.x - end2.x) + (end1.y - end2.y) * (end1.y - end2.y)) / 2;
        double root = sqrt(pow(PYLON_RADIUS, 2.l) - pow(half_leg, 2.l)) / (2*half_leg);
        Point2D ans1(center.x + root * (end1.y - end2.y), center.y + root * (end2.x - end1.x));
        Point2D ans2(center.x - root * (end1.y - end2.y), center.y - root * (end2.x - end1.x));
        if (Distance2D(ans1, rankedExpansions[0]) > Distance2D(ans2, rankedExpansions[0])) {
            pylons.push_back(ans2);
        } else {
            pylons.push_back(ans1);
        }

        pylons.push_back(P2D(staging_location));
        Point2D gateway;
        Point2D cyber;

        if (pylons[0].x > game_info.width / 2) {
            gateway.x = pylons[0].x;
            cyber.x = pylons[0].x - 2.5;
        } else {
            gateway.x = pylons[0].x;
            cyber.x = pylons[0].x + 2.5;
        }

        if (pylons[0].y > game_info.height / 2) {
            gateway.y = pylons[0].y - 2.5;
            cyber.y = pylons[0].y;
        } else {
            gateway.y = pylons[0].y + 2.5;
            cyber.y = pylons[0].y;
        }

        buildingSpots.push_back(gateway);
        buildingSpots.push_back(cyber);
        addNewBuildingSlot();

        /*sc2::Units units = observer->GetUnits(sc2::Unit::Alliance::Self);
        const sc2::Unit* un;
        for (auto u : units) {
            if (u->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
                probes[u->tag] = Probe();
            }
            Actions()->UnitCommand(u, ABILITY_ID::STOP, false);
        }*/

        /*for (auto it = probes.begin(); it != probes.end(); it++) {
            const Unit* mineral_target = FindNearestMineralPatch(observer->GetUnit(it->first)->pos);
            if (!mineral_target) {
                break;
            }
            it->second.init(mineral_target->tag);
        }*/

        //MacroQueue::addBuilding({ABILITY_ID::BUILD_PYLON, NullTag, Point2D(pylons[0].x, pylons[0].y)}, this);
        //TODO: actionList.emplace_back(UNIT_TYPEID::PROTOSS_PROBE, ABILITY_ID::BUILD_PYLON, Point2D(pylons[0].x, pylons[0].y));
    }

    virtual void OnUnitCreated(const Unit* unit) {
        if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
            probes[unit->tag] = Probe();
            const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
            if (!mineral_target) {
                return;
            }
            probes[unit->tag].init(mineral_target->tag);
            //Actions()->UnitCommand(unit, ABILITY_ID::STOP);
            Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER, mineral_target->tag);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
            Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, P2D(unit->pos));
            generatePlacement(P2D(unit->pos), 5);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_PYLON) {
            generatePlacement(P2D(unit->pos), 2);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY) {
            generatePlacement(P2D(unit->pos), 3);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) {
            generatePlacement(P2D(unit->pos), 3);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
            generatePlacement(P2D(unit->pos), 3);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) {
            generatePlacement(P2D(unit->pos), 3);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) {
            generatePlacement(P2D(unit->pos), 3);
        } 

    }

    virtual void OnUnitDestroyed(const Unit* unit) {
        if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
            mineralTargetting[probes[unit->tag].minerals] -= 1;
            probes.erase(unit->tag);
        } else if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
            Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, P2D(unit->pos));
        }
    }

    void buildingUnits() {
        sc2::Units units = observer->GetUnits(sc2::Unit::Alliance::Self);
        for (auto u : units) {
            if (u->build_progress != 1.0) {
                UnitTypeData prereqData = observer->GetUnitTypeData().at(static_cast<uint32_t>(u->unit_type));
                Debug()->DebugTextOut(
                    strprintf("%.1fs remaining out of %.1fs", prereqData.build_time / fps * (1.0 - u->build_progress),
                              prereqData.build_time / fps),
                    u->pos, Color(200, 190, 115), 8);
            }
        }
    }

    void grid() {
        // std::cout << Observation()->GetGameLoop() << std::endl;
        // printf("%d\n", Observation()->GetMinerals());
        HeightMap heightmap(game_info);
        PlacementGrid placementgrid(game_info);
        PathingGrid pathinggrid(game_info);

        int mapWidth = game_info.width;
        int mapHeight = game_info.height;

        //std::cout << "Generate path ... \n";
        Point2DI start = rankedExpansions[0];
        auto path = generator.findPath(start, {mapWidth / 2, mapHeight / 2});
        // auto path = generator.findPath({45, 135}, {51, 136});

        //for (auto& coordinate : path) {
        //    std::cout << coordinate.x << " " << coordinate.y << "\n";
        //}

        map<float>* dist_t = dt(pathinggrid, mapWidth, mapHeight);

        Point2D center = observer->GetCameraPos();
        int wS = int(center.x) - 8;
        if (wS < 0)
            wS = 0;
        int hS = int(center.y) - 3;
        if (hS < 0)
            hS = 0;
        int wE = int(center.x) + 9;
        if (wE > mapWidth)
            wE = mapWidth;
        int hE = int(center.y) + 6;
        if (hE > mapHeight)
            hE = mapHeight;

        int fontSize = 8;

        for (int w = wS; w < wE; w++) {
            for (int h = hS; h < hE; h++) {
                Point2DI point = Point2DI(w, h);
                int boxHeight = 0.1;
                Color c;
                if (point == staging_location) {
                    c.r = 215;
                    c.g = 125;
                    c.b = 220;
                } else if (std::find(pylons.begin(), pylons.end(), P2D(point)) != pylons.end()) {
                    c.r = 215;
                    c.g = 125;
                    c.b = 220;
                } else if (std::find(buildingSpots.begin(), buildingSpots.end(), P2D(point)) != buildingSpots.end()) {
                    c.r = 115;
                    c.g = 225;
                    c.b = 20;
                } else if (std::find(path.begin(), path.end(), point) != path.end()) {
                    c.r = 115;
                    c.g = 125;
                    c.b = 220;
                } else if (1 && placementgrid.IsPlacable(point) && pathinggrid.IsPathable(point)) {
                    c.r = 40;
                    c.g = 40;
                    c.b = 40;
                } else if (0 && pathinggrid.IsPathable(point)) {
                    c.r = 220;
                    c.g = 65;
                    c.b = 25;
                }

                
                #define BOX_BORDER 0.02

                float displace = 0;
                if (imRef(placements, w, h) == 1) {
                    displace = 3;
                }

                if (!(c.r == 255 && c.g == 255 && c.b == 255) || imRef(display, w, h) != "" || displace != 0) {
                    float height = heightmap.TerrainHeight(point);
                    
                    Debug()->DebugBoxOut(Point3D(w + BOX_BORDER, h + BOX_BORDER, height + 0.01 + displace),
                                         Point3D(w + 1 - BOX_BORDER, h + 1 - BOX_BORDER, height + boxHeight), c);
                    Debug()->DebugTextOut(strprintf("%d, %d", w, h),
                                          Point3D(w + BOX_BORDER, h + 0.2 + BOX_BORDER, height + 0.1 + displace),
                                          Color(200, 90, 15),
                                          fontSize);
                    std::string cs = imRef(display, w, h);
                    float disp = cs.length() * 0.0667 * fontSize / 15;
                    Debug()->DebugTextOut(cs, Point3D(w + 0.5 - disp, h + 0.5, height + 0.1 + displace),
                                          Color(200, 190, 115),
                                          fontSize);
                }
            }
        }
        
    }

    void orders() {
        for (auto it = probes.begin(); it != probes.end(); it++) {
            auto all = it->second.buildings;
            
            std::string cs = "";
            const Unit* probe = observer->GetUnit(it->first);
            if (probe == nullptr)
                continue;
            if (probe->orders.size() > 0) {
                UnitOrder o = probe->orders[0];
                cs.append(strprintf("CUR: %s %lx [%.1f,%.1f]\n", AbilityTypeToName(o.ability_id), o.target_unit_tag,
                                    o.target_pos.x, o.target_pos.y));
            }
            
            for (auto it2 = all.begin(); it2 != all.end(); ++it2) {
                cs.append(strprintf("%s %lx [%.1f,%.1f]\n", AbilityTypeToName(it2->ability_id),
                it2->target_unit_tag,
                                    it2->target_pos.x, it2->target_pos.y));
                // printf("%s", cs.c_str());
                Debug()->DebugTextOut(cs, probe->pos, Color(200, 190, 115), 8);
            }
        }
    }

    void tags() {
        /*sc2::Units units = observer->GetUnits(sc2::Unit::Alliance::Self);
        for (auto u : units) {
            Debug()->DebugTextOut(strprintf("%s %lx", UnitTypeToName(u->unit_type), u->tag), u->pos,
                                  Color(200, 190, 115), 12);
        }*/
        sc2::Units neutrals = observer->GetUnits(sc2::Unit::Alliance::Neutral, Probe::isMineral);
        for (auto u : neutrals) {
            Debug()->DebugTextOut(strprintf("%lx:\n %d", u->tag, mineralTargetting[u->tag]),
                                  u->pos, Color(200, 190, 115), 8);
        }
    }

    void mineralLines() {
        for (auto it = probes.begin(); it != probes.end(); it++) {
            //printf("Probe %xu Mineral %xu\n", it->first, it->second.minerals);
            if (observer->GetUnit(it->second.minerals) != nullptr && observer->GetUnit(it->first) != nullptr)
                Debug()->DebugLineOut(observer->GetUnit(it->second.minerals)->pos + Point3D(0,0,1),
                                      observer->GetUnit(it->first)->pos + Point3D(0, 0, 1),
                                      Color(200, 190, 115));
            else if (observer->GetUnit(it->second.minerals) == nullptr && observer->GetUnit(it->first) != nullptr) {
                Debug()->DebugLineOut(observer->GetUnit(it->first)->pos,
                                      observer->GetUnit(it->first)->pos + Point3D(0, 0, 1), Color(200, 190, 115));
            }
        }
    }

    /*void orderQueue() {
        OrderQueue::allOrders
    }*/

    void actionQueue() {
        /*for (auto it = MacroQueue::actions.begin(); it != MacroQueue::actions.end(); ++it) {
            std::cout << it->name;
        }*/

        std::string cs = "";
        int i = 0;
        for (auto it = MacroQueue::actions.begin(); it != MacroQueue::actions.end(); ++it) {
            cs.append(
                strprintf("%s %lx [%.1f, %.1f]\n", AbilityTypeToName(it->ability_id), it->target_unit_tag, it->target_pos.x, it->target_pos.y));

            Point3D building(it->target_pos.x, it->target_pos.y, observer->TerrainHeight(it->target_pos) + 1);

            Debug()->DebugTextOut(strprintf("%s %lx [%.1f, %.1f]", AbilityTypeToName(it->ability_id),
                it->target_unit_tag, it->target_pos.x, it->target_pos.y),
                building, Color(100, 190, 215), 8);
            Debug()->DebugSphereOut(building, 2);
            Debug()->DebugTextOut(strprintf("%s %lx [%.1f, %.1f]", AbilityTypeToName(it->ability_id),
                                            it->target_unit_tag, it->target_pos.x, it->target_pos.y),
                                  Point2D(0.01, 0.01 + (i * 0.02)), Color(100, 190, 215), 8);
            // printf("%s", cs.c_str());
            i++;
        }
        //printf("DISPLAY: %s\n", cs.c_str());
        //Debug()->DebugTextOut(cs);
    }

    virtual void OnStep() final {
        const UnitTypes unit_data = observer->GetUnitTypeData();
        UnitTypeData probe_data = unit_data.at(static_cast<uint32_t>(UNIT_TYPEID::PROTOSS_PROBE));
        grid();
        orders();
        mineralLines();
        tags();
        buildingUnits();
        actionQueue();
        Debug()->DebugTextOut(strprintf("GL:%d %.1fs", observer->GetGameLoop(), observer->GetGameLoop()/22.4),
                              Point2D(0.3, 0.01), Color(100, 190, 215), 8);
        Debug()->SendDebug();

        /*for (auto it = probes.begin(); it != probes.end(); it++) {
            if (it->second.minerals == 0) {
                const Unit* mineral_target = FindNearestMineralPatch(observer->GetUnit(it->first)->pos);
                if (!mineral_target) {
                    break;
                }
                it->second.init(mineral_target->tag);
            }
        }*/

        MacroQueue::execute(this);

        //Debug()->DebugTextOut(strprintf("%d", observer->GetGameLoop()));

        if (observer->GetGameLoop() == 20) {
            const Unit* vesp = FindNearestVespene(startLocation);
            MacroQueue::addBuilding({ABILITY_ID::BUILD_PYLON, NullTag, getPylonSpot()}, this);
            MacroQueue::addBuilding({ABILITY_ID::BUILD_GATEWAY, NullTag, getBuildingSpot()}, this);
            MacroQueue::addBuilding({ABILITY_ID::BUILD_ASSIMILATOR, vesp->tag, vesp->pos}, this);
            MacroQueue::addBuilding(
                {ABILITY_ID::BUILD_NEXUS, NullTag, Point2D(rankedExpansions[0].x, rankedExpansions[0].y + 0.5)}, this);
            MacroQueue::addBuilding({ABILITY_ID::BUILD_CYBERNETICSCORE, NullTag, getBuildingSpot()}, this);
            MacroQueue::addBuilding({ABILITY_ID::BUILD_PYLON, NullTag, getPylonSpot()}, this);
            MacroQueue::addUpgrade(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE, this);
            MacroQueue::add(UNIT_TYPEID::PROTOSS_GATEWAY, {ABILITY_ID::TRAIN_STALKER}, {125, 50, 0});
        } else if (observer->GetGameLoop() == 2600) {
            MacroQueue::addBuilding({ABILITY_ID::BUILD_ROBOTICSFACILITY, NullTag, getBuildingSpot()}, this);
            MacroQueue::add(UNIT_TYPEID::PROTOSS_GATEWAY, {ABILITY_ID::TRAIN_STALKER}, {125, 50, 0});
            MacroQueue::addBuilding({ABILITY_ID::BUILD_TWILIGHTCOUNCIL, NullTag, getBuildingSpot()}, this);
            MacroQueue::add(UNIT_TYPEID::PROTOSS_GATEWAY, {ABILITY_ID::TRAIN_STALKER}, {125, 50, 0});
        } else {
            if (observer->GetGameLoop() > 2600) {
                if (observer->GetFoodCap() < 200 && observer->GetFoodCap() - 3 < observer->GetFoodUsed() &&
                    (MacroQueue::actions.size() == 0 ||
                     MacroQueue::actions.front().ability_id != ABILITY_ID::BUILD_PYLON)) {
                    while (pylonIndex >= pylons.size()) {
                        addNewPylonSlot();
                    }
                    MacroQueue::addPylon(pylons[pylonIndex++], this);
                }
            }
            Units nexi = Observation()->GetUnits(Unit::Alliance::Self, isNexus);
            for (const Unit *nexus : nexi) {
                if (nexus->assigned_harvesters >= 14) {
                    const Unit* vesp = FindNearestVespene(nexus->pos);
                    if (vesp != nullptr)
                        MacroQueue::addBuilding({ABILITY_ID::BUILD_ASSIMILATOR, vesp->tag, vesp->pos}, this);
                }
            }
        }
            
    }

    virtual void OnUnitIdle(const Unit* unit) final {
        switch (unit->unit_type.ToType()) {
            case UNIT_TYPEID::PROTOSS_NEXUS: {
                //Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, Observation()->GetCameraPos(), false);
                if (unit->assigned_harvesters < unit->ideal_harvesters) {
                    Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
                }
                //Debug()->DebugTextOut("TRAIN_PROBE");
                break;
            }
            case UNIT_TYPEID::PROTOSS_PROBE: {
                // Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, Observation()->GetCameraPos(), false);
                //Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER);
                //Debug()->DebugTextOut("HARVEST_GATHER");
                //printf("IDLE");
                probes[unit->tag].execute(unit, this);
                break;
            }
            default: {
                break;
            }
        }
    }


private:
    bool warpStructure(const Unit* unit, AbilityID build_tag, const Point2D& point) {
        if (unit->unit_type.ToType() != UNIT_TYPEID::PROTOSS_PROBE)
            return false;
        Actions()->UnitCommand(unit, build_tag, point, false);
        return true;    
    }
};

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);
    //coordinator.SetStepSize(1);

    Bot bot;
    if (std::rand()%2 == 1) {
        coordinator.SetParticipants({CreateParticipant(Race::Protoss, &bot), CreateComputer(Race::Zerg)});
    } else {
        coordinator.SetParticipants({CreateComputer(Race::Zerg), CreateParticipant(Race::Protoss, &bot)});
    }
    //coordinator.SetParticipants({CreateParticipant(Race::Protoss, &bot), CreateComputer(Race::Zerg)});

    coordinator.LaunchStarcraft();
    //coordinator.StartGame("5_13/Oceanborn513AIE.SC2Map");
    std::string maps[6] = {"5_13/Oceanborn513AIE.SC2Map", "5_13/Equilibrium513AIE.SC2Map", "5_13/GoldenAura513AIE.SC2Map",
                           "5_13/Gresvan513AIE.SC2Map", "5_13/HardLead513AIE.SC2Map", "5_13/SiteDelta513AIE.SC2Map"};
    int r = 5;//(std::rand() % 12000)/2000;
    printf("rand %d [%d %d %d %d %d %d]\n", r, std::rand(), std::rand(), std::rand(), std::rand(), std::rand(),
           std::rand());

    coordinator.StartGame(maps[r]);
    while (coordinator.Update()) {
    }

    return 0;
}

//fix probe targetting
//fix probe build-check
//fix assimilator condition
//have both 
//add generator to global variables
//if near supply cap, save for pylon
//add place-able check to actionQueue
//build list
//army list
//per-unit micro
//squadrons
//combine rankedExpansions into normal expansionOrder generation


//ERROR CODES:
//ABILITY CODES ARE 0x01__
