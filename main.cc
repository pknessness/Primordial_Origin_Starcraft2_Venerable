#include <sc2api/sc2_api.h>
#include <sc2lib/sc2_lib.h>

#include <iostream>
#include "jps.hpp"
//#include "jps_old.hpp"
#include "grid.hpp"
#include "tools.hpp"
#include "constants.h"

using namespace sc2;

class Bot : public Agent {
public:
    //std::vector<Location> path;

    std::vector<Point3D> expansions;
    std::vector<Point3D> rankedExpansions;
    std::vector<double> expansionDistance;
    std::vector<double> rankedExpansionDistance;
    Point2DI staging_location;

    Grid gridmap;

    Point3D P3D(const Point2D& p) {
        return Point3D(p.x, p.y, Observation()->TerrainHeight(p));
    }

    void initializeStartings() {
        GameInfo game_info = Observation()->GetGameInfo();
        if (Observation()->GetStartLocation().x > game_info.width / 2) {
            staging_location.x = Observation()->GetStartLocation().x - 3;
        } else {
            staging_location.x = Observation()->GetStartLocation().x + 3;
        }

        if (Observation()->GetStartLocation().y > game_info.height / 2) {
            staging_location.y = Observation()->GetStartLocation().y - 3;
        } else {
            staging_location.y = Observation()->GetStartLocation().y + 3;
        }
    }

    void initializeExpansions() {
        // staging_location = Point2DI(startLocation.x + ;
        expansions = sc2::search::CalculateExpansionLocations(Observation(), Query());
        for (auto point : expansions) {
            if (point.x == 0 && point.y == 0)
                continue;
            // auto path = generator.findPath(staging_location, (Point2DI)point);
            // expansionDistance.push_back(path.size());
            auto came_from = jps(gridmap, staging_location, {int(point.x),int(point.y)}, Tool::euclidean, this);
            auto pathToExpansion = Tool::reconstruct_path(staging_location, {int(point.x), int(point.y)}, came_from);

            double length = 0;
            for (int i = 0; i < pathToExpansion.size() - 1; i++) {
                length += Distance2D(P2D(pathToExpansion[i]), P2D(pathToExpansion[i + 1]));
            }

            expansionDistance.push_back(length);
        }

        for (int j = 0; j < expansions.size() - 1; j++) {
            if (expansions[j].x == 0 && expansions[j].y == 0)
                continue;
            if (rankedExpansions.size() == 0) {
                rankedExpansions.push_back(expansions[j]);  // might be useless now
                rankedExpansionDistance.push_back(expansionDistance[j]);
                continue;
            }
            bool inserted = false;
            for (int i = 0; i < rankedExpansions.size(); i++) {
                if (rankedExpansionDistance[i] > expansionDistance[j]) {
                    rankedExpansions.insert(rankedExpansions.begin() + i, expansions[j]);  // might be useless now
                    rankedExpansionDistance.insert(rankedExpansionDistance.begin() + i, expansionDistance[j]);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                rankedExpansions.push_back(expansions[j]);  // might be useless now
                rankedExpansionDistance.push_back(expansionDistance[j]);
            }
        }

        //for (int i = 0; i < rankedExpansions.size(); i++) {
        //    printf("[%.1f,%.1f %.1f]\n", expansions[i].x, expansions[i].y, expansionDistance[i]);
        //}
    }

    void grid() {
        GameInfo game_info = Observation()->GetGameInfo();

        int mapWidth = game_info.width;
        int mapHeight = game_info.height;

        Point2D center = Observation()->GetCameraPos();
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

        #define BOX_BORDER 0.02

        for (int w = wS; w < wE; w++) {
            for (int h = hS; h < hE; h++) {
                Point2DI point = Point2DI(w, h);
                int boxHeight = 0.1;
                Color c(255,255,255);
                //for (auto loc : path) {
                //    if (loc.x == w && loc.y == h) {
                //        c = Color(120, 23, 90);
                //        break;
                //    }
                //}

                if (!(c.r == 255 && c.g == 255 && c.b == 255)) {
                    float height = Observation()->TerrainHeight(P2D(point));

                    Debug()->DebugBoxOut(Point3D(w + BOX_BORDER, h + BOX_BORDER, height + 0.01),
                                         Point3D(w + 1 - BOX_BORDER, h + 1 - BOX_BORDER, height + boxHeight), c);
                    Debug()->DebugTextOut(strprintf("%d, %d", w, h),
                                          Point3D(w + BOX_BORDER, h + 0.2 + BOX_BORDER, height + 0.1),
                                          Color(200, 90, 15), fontSize);
                    /*std::string cs = imRef(display, w, h);
                    float disp = cs.length() * 0.0667 * fontSize / 15;
                    Debug()->DebugTextOut(cs, Point3D(w + 0.5 - disp, h + 0.5, height + 0.1 + displace),
                                          Color(200, 190, 115), fontSize);*/
                }
            }
        }
    }

    virtual void OnGameStart() final {

        gridmap = Grid{Observation()->GetGameInfo().width, Observation()->GetGameInfo().height};

        const ObservationInterface* observe = Observation();
        //PathFinder pf(observe->GetGameInfo().width, observe->GetGameInfo().height);
        //cout << pf.findPath(observe->GetGameInfo().start_locations[0], observe->GetGameInfo().enemy_start_locations[0], this) << endl;
        //unordered_set<Location> walls{{5, 0}, {5, 1}, {2, 2}, {5, 2}, {2, 3}, {5, 3}, {2, 4}, {5, 4},
        //                              {2, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5}, {2, 6}, {2, 7}};

        //Point2DI start{142, 45};
        //Point2DI goal{130, 60};

        //auto came_from = jps(gridmap, start, goal, Tool::euclidean, this);
        //path = Tool::reconstruct_path(start, goal, came_from);
        /*for (auto loc : path) {
            printf("%d,%d\n", loc.x, loc.y);
        }*/
        //Tool::draw_grid(this, map, {}, {}, path, came_from, start, goal);
        initializeStartings();
        initializeExpansions();
    }

    virtual void OnUnitCreated(const Unit* unit) {

    }

    virtual void OnUnitDestroyed(const Unit* unit) {

    }

    virtual void OnStep() final {
        grid();
        Debug()->SendDebug();
    }
};

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    Bot bot;
    if (std::rand() % 2 == 1) {
        coordinator.SetParticipants({CreateParticipant(Race::Protoss, &bot), CreateComputer(Race::Random)});
    } else {
        coordinator.SetParticipants({CreateComputer(Race::Random), CreateParticipant(Race::Protoss, &bot)});
    }

    coordinator.LaunchStarcraft();
    std::string maps[6] = {"5_13/Oceanborn513AIE.SC2Map",  "5_13/Equilibrium513AIE.SC2Map",
                           "5_13/GoldenAura513AIE.SC2Map", "5_13/Gresvan513AIE.SC2Map",
                           "5_13/HardLead513AIE.SC2Map",   "5_13/SiteDelta513AIE.SC2Map"};
    int r = 5;  //(std::rand() % 12000)/2000;
    printf("rand %d [%d %d %d %d %d %d]\n", r, std::rand(), std::rand(), std::rand(), std::rand(), std::rand(),
           std::rand());

    coordinator.StartGame(maps[r]);
    while (coordinator.Update()) {
    }

    return 0;
}

//have nexus stop checking when it has two vespenes
//fix probe targetting
//fix probe build-check
//better probe queue
//fix assimilator condition
//have both 
//add generator to global variables
//if near supply cap, save for pylon
//add place-able check to actionQueue
//build list
//army list
//per-unit micro
//squadrons
//combine rankedExpansions into normal expansionDistance generation


//ERROR CODES:
//ABILITY CODES ARE 0x01__
