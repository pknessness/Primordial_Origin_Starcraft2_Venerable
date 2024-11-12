#ifndef ARMY_CONTROLLER_H
#define ARMY_CONTROLLER_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

#include <list>
#include <map>

#include "constants.h"
#include "probes.hpp"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

// typedef std::list<UnitOrder> UnitOrders;
// typedef std::map<uint64_t, UnitOrders> AllUnitOrders;

namespace ArmyController {

Units allArmy;

//squad 1 is main, squad 2 is base defense, squad 3 is harrass
std::vector<Units> squads;

Units observers;

}  // namespace MacroQueue
#endif
