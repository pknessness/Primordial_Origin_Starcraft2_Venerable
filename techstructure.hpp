#pragma once
#include <sc2api/sc2_api.h>

#include <map>

#include "constants.h"
#include "unit.hpp"

class TechStructure : public UnitWrapper {
private:
public:
    TechStructure(Tag self_) : UnitWrapper(self_, UNIT_TYPEID::PROTOSS_NEXUS) {
    }
};