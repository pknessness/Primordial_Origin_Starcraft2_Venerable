/*
    Copyright (c) 2015, Damian Barczynski <daan.net@wp.eu>
    Following tool is licensed under the terms and conditions of the ISC license.
    For more information visit https://opensource.org/licenses/ISC.
*/
#ifndef __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__
#define __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__

#include <sc2api/sc2_api.h>
#include <vector>
#include <functional>
#include <set>
#include <math.h>
#include <algorithm>

namespace AStar
{
    /*struct Vec2i
    {
        int x, y;

        bool operator == (const Vec2i& coordinates_);
        friend Vec2i operator + (const AStar::Vec2i& left_, const AStar::Vec2i& right_) {
            return{ left_.x + right_.x, left_.y + right_.y };
        }
    };*/

    using uint = unsigned int;
    using HeuristicFunction = std::function<uint(sc2::Point2DI, sc2::Point2DI)>;
    using CoordinateList = std::vector<sc2::Point2DI>;

    struct Node
    {
        uint G, H;
        sc2::Point2DI coordinates;
        Node *parent;

        Node(sc2::Point2DI coord_, Node *parent_ = nullptr);
        uint getScore();
    };

    using NodeSet = std::vector<Node*>;

    class Generator
    {
        bool detectCollision(sc2::Point2DI coordinates_);
        Node* findNodeOnList(NodeSet& nodes_, sc2::Point2DI coordinates_);
        void releaseNodes(NodeSet& nodes_);

    public:
        Generator();
        void setWorldSize(sc2::Point2DI worldSize_);
        void setGameInfo(sc2::GameInfo* gameInfo_);
        void setDiagonalMovement(bool enable_);
        void setHeuristic(HeuristicFunction heuristic_);
        CoordinateList findPath(sc2::Point2DI source_, sc2::Point2DI target_);

    private:
        HeuristicFunction heuristic;
        CoordinateList direction;
        sc2::Point2DI worldSize;
        sc2::GameInfo *gameInfo;
        uint directions;
    };

    class Heuristic
    {
        static sc2::Point2DI getDelta(sc2::Point2DI source_, sc2::Point2DI target_);

    public:
        static uint manhattan(sc2::Point2DI source_, sc2::Point2DI target_);
        static uint euclidean(sc2::Point2DI source_, sc2::Point2DI target_);
        static uint octagonal(sc2::Point2DI source_, sc2::Point2DI target_);
    };
}

#endif // __ASTAR_HPP_8F637DB91972F6C878D41D63F7E7214F__

//#include "AStar.hpp"

using namespace std::placeholders;

AStar::Node::Node(sc2::Point2DI coordinates_, Node* parent_) {
        parent = parent_;
        coordinates = coordinates_;
        G = H = 0;
}

AStar::uint AStar::Node::getScore() {
        return G + H;
}

AStar::Generator::Generator(){
        //setPathingGrid(grid);
        setDiagonalMovement(false);
        setHeuristic(&Heuristic::manhattan);
        direction = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}};
}

void AStar::Generator::setWorldSize(sc2::Point2DI worldSize_) {
        worldSize = worldSize_;
}

void AStar::Generator::setGameInfo(sc2::GameInfo* gameInfo_) {
        gameInfo = gameInfo_;
}

void AStar::Generator::setDiagonalMovement(bool enable_) {
        directions = (enable_ ? 8 : 4);
}

void AStar::Generator::setHeuristic(HeuristicFunction heuristic_) {
        heuristic = std::bind(heuristic_, _1, _2);
}

AStar::CoordinateList AStar::Generator::findPath(sc2::Point2DI source_, sc2::Point2DI target_) {
        Node* current = nullptr;
        NodeSet openSet, closedSet;
        openSet.reserve(100);
        closedSet.reserve(100);
        openSet.push_back(new Node(source_));

        while (!openSet.empty()) {
            auto current_it = openSet.begin();
            current = *current_it;

            for (auto it = openSet.begin(); it != openSet.end(); it++) {
                auto node = *it;
                if (node->getScore() <= current->getScore()) {
                    current = node;
                    current_it = it;
                }
            }
            //printf("[%d,%d]\n", current->coordinates.x, current->coordinates.y);

            if (current->coordinates == target_) {
                break;
                //printf("Found");
            }

            closedSet.push_back(current);
            openSet.erase(current_it);

            for (uint i = 0; i < directions; ++i) {
                sc2::Point2DI newCoordinates(current->coordinates.x + direction[i].x,
                                             current->coordinates.y + direction[i].y);
                
                //printf("{%d,%d} %d| ", newCoordinates.x, newCoordinates.y, detectCollision(newCoordinates));

                if (detectCollision(newCoordinates) || findNodeOnList(closedSet, newCoordinates)) {
                    //printf("Continue");
                    continue;
                }

                uint totalCost = current->G + ((i < 4) ? 10 : 14);

                Node* successor = findNodeOnList(openSet, newCoordinates);

                //printf("{%p}", successor);

                if (successor == nullptr) {
                    successor = new Node(newCoordinates, current);
                    successor->G = totalCost;
                    successor->H = heuristic(successor->coordinates, target_);
                    openSet.push_back(successor);
                } else if (totalCost < successor->G) {
                    successor->parent = current;
                    successor->G = totalCost;
                }
            }
        }

        CoordinateList path;
        while (current != nullptr) {
            path.push_back(current->coordinates);
            current = current->parent;
        }

        releaseNodes(openSet);
        releaseNodes(closedSet);

        return path;
}

AStar::Node* AStar::Generator::findNodeOnList(NodeSet& nodes_, sc2::Point2DI coordinates_) {
        for (auto node : nodes_) {
            if (node->coordinates == coordinates_) {
                //printf("found node\n");
                return node;
            }
        }
        return nullptr;
}

void AStar::Generator::releaseNodes(NodeSet& nodes_) {
        for (auto it = nodes_.begin(); it != nodes_.end();) {
            delete *it;
            it = nodes_.erase(it);
        }
}

bool AStar::Generator::detectCollision(sc2::Point2DI coordinates_) {
        if (!gameInfo) {
            printf("NO GAMEINFO");
            return true;
        }
        //PlacementGrid placementgrid(*gameInfo);
        //printf("%d %d %d %d %d", coordinates_.x < 0, coordinates_.x >= worldSize.x, coordinates_.y < 0, coordinates_.y >= worldSize.y, !pathingGrid.IsPathable(coordinates_));
        if (coordinates_.x < 0 || coordinates_.x >= worldSize.x || coordinates_.y < 0 ||
            coordinates_.y >= worldSize.y || !sc2::PathingGrid(*gameInfo).IsPathable(coordinates_)) {
            return true;
        }
        return false;
}

sc2::Point2DI AStar::Heuristic::getDelta(sc2::Point2DI source_, sc2::Point2DI target_) {
        return {abs(source_.x - target_.x), abs(source_.y - target_.y)};
}

AStar::uint AStar::Heuristic::manhattan(sc2::Point2DI source_, sc2::Point2DI target_) {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * (delta.x + delta.y));
}

AStar::uint AStar::Heuristic::euclidean(sc2::Point2DI source_, sc2::Point2DI target_) {
        auto delta = std::move(getDelta(source_, target_));
        return static_cast<uint>(10 * sqrt(pow(delta.x, 2) + pow(delta.y, 2)));
}

AStar::uint AStar::Heuristic::octagonal(sc2::Point2DI source_, sc2::Point2DI target_) {
        auto delta = std::move(getDelta(source_, target_));
        return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
}
