#ifndef ACTION_H
#define ACTION_H

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"
#include <sc2api/sc2_interfaces.h>
#include "constants.h"

class Action {
public:
    /* create an action */
    Action();

    /* create an action */
    Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    /* create an action */
    Action(sc2::UnitTypeID unitType_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    /* create an action */
    Action(const sc2::Unit* unit_, sc2::AbilityID abilityId_);

    /* create an action */
    static void setInterfaces(sc2::ActionInterface* actionInterface_,
                              const sc2::ObservationInterface* observationInterface_);

    static void setAgent(sc2::Agent* agent_);

    /* delete an action */
    //~Action();

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    void init(const sc2::Unit* unit_, sc2::AbilityID abilityId_);

    void buildStructure(sc2::AbilityID abilityId_, const sc2::Point2D& point_);

    bool execute() const;


    static sc2::ActionInterface* actionInterface;
    static const sc2::ObservationInterface* observationInterface;
    sc2::AbilityID abilityId;
    const sc2::Unit* unit;
    sc2::UnitTypeID unitType;
    sc2::Point2D point;
    bool pointInitialized = false;
    int costMinerals = 0;
    int costVespene = 0;

private:
};

//struct Action {  // Structure declaration
//    sc2::AbilityID abilityId;
//    const sc2::Unit* unit;
//    sc2::UnitTypeID unitType;
//    sc2::Point2D point;
//    bool pointInitialized;
//};

#endif
