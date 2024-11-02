#include <sc2api/sc2_api.h>
//#include "sc2api/sc2_unit_filters.h"
#include "sc2lib/sc2_lib.h"
//#include "dt.h"
#include "pathfinding.h"
#include "dist_transform.h"
#include "AStar.hpp"
#include "action.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <queue>
#include <list>
#include <cmath>

constexpr auto PI = 3.14159263;
constexpr auto PYLON_RADIUS = 6.5;

using namespace sc2;

typedef std::list<UnitOrder> UnitOrders;
typedef std::map<uint64_t, UnitOrders> AllUnitOrders;

class Bot : public Agent {
public:

    Bot() {
        
    }

    std::vector<Point3D> expansions;
    std::vector<Point3D> rankedExpansions;
    std::vector<double> expansionOrder;

    const ObservationInterface* observer;
    GameInfo game_info;
    Point3D startLocation;
    Point2DI staging_location;

    AStar::Generator generator;

    std::list<Action> actionList;
    std::vector<Point2DI> pylons;

    map<std::string>* display;

    //std::vector<Point2DI> numberDisplayLoc;
    //std::vector<double> numberDisplay;

    template <typename... Args>
    std::string strprintf(const std::string& format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
        if (size_s <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
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

        for (int x = 0; x < game_info.width; x++) {
            for (int y = 0; y < game_info.height; y++) {
                imRef(display, x, y) = "";
            }
        }

        // Temporary, we can replace this with observation->GetStartLocation() once implemented
        startLocation = observer->GetStartLocation();
        if (startLocation.x > game_info.width / 2) {
            staging_location.x = startLocation.x - 3;
        } else {
            staging_location.x = startLocation.x + 3;
        }

        if (startLocation.y > game_info.height / 2) {
            staging_location.y = startLocation.y - 3;
        } else {
            staging_location.y = startLocation.y + 3;
        }

        //staging_location = Point2DI(startLocation.x + ;
        expansions = sc2::search::CalculateExpansionLocations(observer, Query());
        for (auto point : expansions) {
            //printf("[%d,%d]->[%d,%d], ", staging_location.x, staging_location.y, ((Point2DI)point).x,
            //       ((Point2DI)point).y);
            auto path = generator.findPath(staging_location, (Point2DI)point);
            //for (auto& coordinate : path) {
                //std::cout << coordinate.x << " " << coordinate.y << "\n";
            //}
            expansionOrder.push_back(path.size());
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
                    inserted = true;
                    break;
                }
                
            }
            if (!inserted) {
                rankedExpansions.push_back(expansions[j]);
                rankedExpansionOrder.push_back(expansionOrder[j]);
            }
        }
        for (int i = 0; i < rankedExpansions.size() - 1; i++) {
            printf("{%d,%d, %f} ", ((Point2DI)rankedExpansions[i]).x, ((Point2DI)rankedExpansions[i]).y,
                   rankedExpansionOrder[i]);
        }

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
            imRef(display, coordinate.x, coordinate.y) = strprintf("%d", diam);
        }
        Point2D center(end1.x / 2.0 + end2.x / 2.0, end1.y / 2.0 + end2.y / 2.0);
        double half_leg = sqrt((end1.x - end2.x) * (end1.x - end2.x) + (end1.y - end2.y) * (end1.y - end2.y)) / 2;
        double root = sqrt(pow(PYLON_RADIUS, 2.l) - pow(half_leg, 2.l)) / (2*half_leg);
        Point2D ans1(center.x + root * (end1.y - end2.y), center.y + root * (end2.x - end1.x));
        Point2D ans2(center.x - root * (end1.y - end2.y), center.y - root * (end2.x - end1.x));
        if (Distance2D(ans1, rankedExpansions[0]) > Distance2D(ans2, rankedExpansions[0])) {
            pylons.push_back(ans2);
            //pylons.push_back(ans1);
        } else {
            pylons.push_back(ans1);
            //pylons.push_back(ans2);
        }
        //Action act(UNIT_TYPEID::PROTOSS_PROBE, ABILITY_ID::BUILD_PYLON, Point2D(pylons[0].x, pylons[0].y));
        //act.buildStructure();
        //actionList.push_back(act);
        actionList.emplace_back(UNIT_TYPEID::PROTOSS_PROBE, ABILITY_ID::BUILD_PYLON, Point2D(pylons[0].x, pylons[0].y));
        //pylons.push_back(wallOffPylonPath[5]);
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
        int wS = int(center.x) - 7;
        if (wS < 0)
            wS = 0;
        int hS = int(center.y) - 3;
        if (hS < 0)
            hS = 0;
        int wE = int(center.x) + 8;
        if (wE > mapWidth)
            wE = mapWidth;
        int hE = int(center.y) + 4;
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
                } else if (std::find(pylons.begin(), pylons.end(), point) != pylons.end()) {
                    c.r = 215;
                    c.g = 125;
                    c.b = 220;
                } else if (std::find(path.begin(), path.end(), point) != path.end()) {
                    c.r = 115;
                    c.g = 125;
                    c.b = 220;
                } else if (0 && placementgrid.IsPlacable(point)) {
                    c.r = 15;
                    c.g = 220;
                    c.b = 25;
                } else if (0 && pathinggrid.IsPathable(point)) {
                    c.r = 220;
                    c.g = 65;
                    c.b = 25;
                }
                #define BOX_BORDER 0.02
                if (!(c.r == 255 && c.g == 255 && c.b == 255)){
                    float height = heightmap.TerrainHeight(point);
                    Debug()->DebugBoxOut(Point3D(w + BOX_BORDER, h + BOX_BORDER, height + 0.01),
                                         Point3D(w + 1 - BOX_BORDER, h + 1 - BOX_BORDER, height + boxHeight), c);
                    Debug()->DebugTextOut(strprintf("%d, %d", w, h),
                                          Point3D(w + BOX_BORDER, h + 0.2 + BOX_BORDER, height + 0.1),
                                          Color(200, 90, 15),
                                          fontSize);
                    std::string cs = imRef(display, w, h);
                    float disp = cs.length() * 0.0667 * fontSize / 15;
                    Debug()->DebugTextOut(cs, Point3D(w + 0.5 - disp, h + 0.5, height + 0.1), Color(200, 190, 115),
                                          fontSize);
                }
            }
        }
        
    }

    void actions() {
        int i = 0;
        for (auto it = actionList.begin(); it != actionList.end(); ++it){
            std::string cs =
                strprintf("%d %s %s [%d, %d]\n", i, 
                it->unitType != 0 ? UnitTypeToName(it->unitType) : UnitTypeToName(it->unit->unit_type),
                          AbilityTypeToName(it->abilityId), it->point.x, it->point.y);
            Debug()->DebugTextOut(cs, Point2D(4, 4 + (i * 11)), Color(255, 255, 255), 10);
            //printf("%s", cs.c_str());
            i++;
        }
        /*for (int i = 0; i < actionList.size(); i++) {
            std::string cs = strprintf("%s %s [%d, %d]\n",
                                       actionList[i].unitType != 0 ? UnitTypeToName(actionList[i].unitType)
                                                                   : UnitTypeToName(actionList[i].unit->unit_type),
                          AbilityTypeToName(actionList[i].abilityId), actionList[i].point.x, actionList[i].point.y);
            Debug()->DebugTextOut(cs, Point2D(4, 4 + i * 11), Color(255,255,255), 10);
        }*/
    }

    void orders() {
        /*Units units = observer->GetUnits(sc2::Unit::Alliance::Self);
        for (const Unit *unit : units) {
            std::vector<UnitOrder> orders = unit->orders;
            std::string s = "";
            for (UnitOrder order : orders) {
                s.append(AbilityTypeToName(order.ability_id));
                s.append(", ");
                if (order.target_unit_tag != NullTag) {
                    s.append(strprintf("[%lu]", order.target_unit_tag));
                }
                s.append(" ");
                if (order.target_pos != Point2D(0,0)) {
                    s.append(strprintf("[%d,%d]", order.target_pos.x, order.target_pos.y));
                }
                s.append("%.2f\n", order.progress);
            }
            Debug()->DebugTextOut(s, unit->pos, Color(200, 190, 115), 8);
        }*/
        
        Units units = observer->GetUnits(sc2::Unit::Alliance::Self);
        for (const Unit* unit : units) {
            int i = 0;
            UnitOrders all = OrderQueue::getAllOrders()[unit->tag];
            std::string cs = "";
            for (auto it = all.begin(); it != all.end();
                 ++it) {
                cs.append(
                    strprintf("%s %ul [%.1f,%.1f] %.2f\n", AbilityTypeToName(it->ability_id),
                                           it->target_unit_tag, it->target_pos.x, it->target_pos.y, it->progress));
                //printf("%s", cs.c_str());
                Debug()->DebugTextOut(cs, unit->pos, Color(200, 190, 115), 8);
            }
        }
        
    }

    void actionQueue(){
        if (actionList.size() > 0) {
            Action a = actionList.front();
            if (a.execute(this)) {
                actionList.pop_front();
            }
        }
    }

    /*void orderQueue() {
        OrderQueue::allOrders
    }*/

    virtual void OnStep() final {
        const UnitTypes unit_data = observer->GetUnitTypeData();
        UnitTypeData probe_data = unit_data.at(static_cast<uint32_t>(UNIT_TYPEID::PROTOSS_PROBE));
        grid();
        actions();
        orders();
        Debug()->SendDebug();
        actionQueue();
    }

    virtual void OnUnitIdle(const Unit* unit) final {
        switch (unit->unit_type.ToType()) {
            case UNIT_TYPEID::PROTOSS_NEXUS: {
                //Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, Observation()->GetCameraPos(), false);
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
                //Debug()->DebugTextOut("TRAIN_PROBE");
                break;
            }
            //case UNIT_TYPEID::PROTOSS_PROBE: {
            //    // Actions()->UnitCommand(unit, ABILITY_ID::RALLY_NEXUS, Observation()->GetCameraPos(), false);
            //    Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER);
            //    //Debug()->DebugTextOut("HARVEST_GATHER");
            //    break;
            //}
            default: {
                OrderQueue::execute(unit, this);
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
    coordinator.SetStepSize(1);

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
    coordinator.StartGame(maps[std::rand() % 6]);
    while (coordinator.Update()) {
    }

    return 0;
}

//add generator to global variables
//if near supply cap, save for pylon
//build list
//army list
//per-unit micro
//squadrons
//combine rankedExpansions into normal expansionOrder generation


//ERROR CODES:
//ABILITY CODES ARE 0x01__
