#pragma once
#include <sc2api/sc2_api.h>

#include <map>
#include "map2d.hpp"

#include "constants.h"
#include "unit.hpp"


namespace SpacialHash {
map2d<UnitWrappers>* grid;

void resetGrid(Agent *agent) {
    for (int i = 0; i < grid->width(); i++) {
        for (int j = 0; j < grid->height(); j++) {
            imRef(grid, i, j).clear(); 
        }
    }
}

void updateGrid(Agent *agent) {
    resetGrid(agent);
    for (auto it = UnitManager::units.begin(); it != UnitManager::units.end(); it++) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            Point2D p = (*it2)->pos(agent);
            float size = (*it2)->radius;
            float x = p.x - size;
            float y = p.y - size;
            for (int i = 0; i < size * 2 + 1; i++) {
                for (int j = 0; j < size * 2 + 1; j++) {
                    bool activate = false;
                    for (float a = 0; a <= 1; a += 0.25) {
                        for (float b = 0; b <= 1; b += 0.25) {
                            if (a == 0 || a == 1 || b == 0 || b == 1 || (a == 0.5 && b == 0.5)) {
                                Point2D temp{float((int)(i + x) + a), float((int)(j + y) + b)};
                                if (Distance2D(p, temp) < size ||
                                    ((int)(i + x) == (int)(p.x) && std::abs(j + y - p.y) < size) ||
                                    ((int)(j + y) == (int)(p.y) && std::abs(i + x - p.x) < size)) {
                                    activate = true;
                                    break;
                                }
                            }
                            
                        }
                    }
                    if (activate) {
                        imRef(grid, (int)(i + x), (int)(j + y)) = UnitWrappers();
                        imRef(grid, (int)(i + x), (int)(j + y)).push_back(*it2);
                    }
                }
            }
            agent->Debug()->DebugSphereOut((*it2)->pos3D(agent), size);
        }
    }
}

UnitWrappers findInRadius(Point2D pos, float radius, Agent *agent) {
    UnitWrappers found;
    float x = pos.x - radius;
    float y = pos.y - radius;
    for (int i = 0; i < radius * 2 + 1; i++) {
        for (int j = 0; j < radius * 2 + 1; j++) {
            bool activate = false;
            for (float a = 0; a <= 1; a += 0.25) {
                for (float b = 0; b <= 1; b += 0.25) {
                    if (a == 0 || a == 1 || b == 0 || b == 1 || (a == 0.5 && b == 0.5)) {
                        Point2D temp{float((int)(i + x) + a), float((int)(j + y) + b)};
                        if (Distance2D(pos, temp) < radius ||
                            ((int)(i + x) == (int)(pos.x) && std::abs(j + y - pos.y) < radius) ||
                            ((int)(j + y) == (int)(pos.y) && std::abs(i + x - pos.x) < radius)) {
                            activate = true;
                            break;
                        }
                    }
                }
                if (activate) {
                    break;
                }
            }
            if (activate) {
                UnitWrappers cell = imRef(grid, (int)(i + x), (int)(j + y));
                found.insert(found.end(), cell.begin(), cell.end());
            }
        }
    }
    return found;
}

}