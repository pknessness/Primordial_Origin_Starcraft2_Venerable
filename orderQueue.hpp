#ifndef ORDER_QUEUE_H
#define ORDER_QUEUE_H

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

#include "sc2lib/sc2_lib.h"
#include <map>
#include <list>

using namespace sc2;

typedef std::list<UnitOrder> UnitOrders;
typedef std::map<uint64_t, UnitOrders> AllUnitOrders;

bool operator==(UnitOrder const &o1, UnitOrder const &c2) {
    return (o1.ability_id == c2.ability_id) && (o1.target_pos == c2.target_pos) &&
           (o1.target_unit_tag == c2.target_unit_tag);
}

namespace OrderQueue {
    static AllUnitOrders allOrders;

    static void add(const Unit *unit, UnitOrder order) {
        if (auto search = allOrders.find(unit->tag); search == allOrders.end())
            allOrders[unit->tag] = UnitOrders();
        allOrders[unit->tag].push_back(order);
    }

    static void remove(const Unit *unit, UnitOrder order) {

    }

    static void printOrder(UnitOrder order) {
        printf("ORDER: %s %ul [%.1f,%.1f] %.2f\n", AbilityTypeToName(order.ability_id), order.target_unit_tag, order.target_pos.x,
               order.target_pos.y, order.progress);
    }
    
    static bool execute(const Unit *unit, Agent *a) {
        UnitOrder order = allOrders[unit->tag].front();
        if (unit->orders.size() > 0)
            printOrder(unit->orders[0]);
        else
            printf("no prev order\n");
        if (order.target_unit_tag != NullTag) {
            a->Actions()->UnitCommand(unit, order.ability_id, a->Observation()->GetUnit(order.target_unit_tag), false);
        } else if (order.target_pos != Point2D(0, 0)) {
            a->Actions()->UnitCommand(unit, order.ability_id, order.target_pos, false);
        } else {
            a->Actions()->UnitCommand(unit, order.ability_id, false);
        }
        if (unit->orders.size() > 0) {
            printOrder(unit->orders[0]);
            printf("?%d\n", unit->orders[0] == order);
        } else {
            printf("no cur order\n");
        }
            
        if (unit->orders.size() != 0 && unit->orders[0] == order) {
            allOrders[unit->tag].pop_front();
            return true;
        }
        return false;
    }

    static AllUnitOrders getAllOrders() {
        return allOrders;
    }
}  // namespace OrderQueue
#endif
