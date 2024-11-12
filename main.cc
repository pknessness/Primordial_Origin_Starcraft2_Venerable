#include <sc2api/sc2_api.h>

#include <iostream>
#include "jps.hpp"
#include "grid.hpp"
#include "tools.hpp"
#include "constants.h"

using namespace sc2;

class Bot : public Agent {
public:
    virtual void OnGameStart() final {
        const ObservationInterface* observe = Observation();
        //PathFinder pf(observe->GetGameInfo().width, observe->GetGameInfo().height);
        //cout << pf.findPath(observe->GetGameInfo().start_locations[0], observe->GetGameInfo().enemy_start_locations[0], this) << endl;
        unordered_set<Location> walls{{5, 0}, {5, 1}, {2, 2}, {5, 2}, {2, 3}, {5, 3}, {2, 4}, {5, 4},
                                      {2, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5}, {2, 6}, {2, 7}};
        Grid map{10, 10, walls};

        Location start{1, 1};
        Location goal{6, 2};

        auto came_from = jps(map, start, goal, Tool::euclidean);
        auto path = Tool::reconstruct_path(start, goal, came_from);
        Tool::draw_grid(map, {}, {}, path, came_from, start, goal);
    }

    virtual void OnUnitCreated(const Unit* unit) {

    }

    virtual void OnUnitDestroyed(const Unit* unit) {

    }

    virtual void OnStep() final {
        
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
//combine rankedExpansions into normal expansionOrder generation


//ERROR CODES:
//ABILITY CODES ARE 0x01__
