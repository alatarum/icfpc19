#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw

#include "cMap.h"
#include "lemon/dijkstra.h"

static const int WEIGHT_DEFAULT = 1;
static const int WEIGHT_NOT_LONE_TILE = 25;
static const int WEIGHT_NOT_STRAIGHT_DIR = 15;
static const int WEIGHT_NOT_UNWRAPPED_DST = 4;
static const int WEIGHT_NOT_UNWRAPPED_MANIP = 6;
static const int WEIGHT_WALL_MANIP = 8;

using namespace std;

struct vertical_line
{
    int x;
    int y1, y2;
};

cMap::cMap(vector<struct coords> map_border_coords, vector<vector<struct coords>> obstacles_list) :
    map_size(0, 0), graph(), graph_tiles(graph), costMap(graph, 1)
{
//1. determine map size and extract vertical lines for borders
    vector<struct vertical_line> lines;

    auto it = map_border_coords.begin();
    map_border_coords.push_back(coords(it->x, it->y));

    it = map_border_coords.begin();
    int prev_point_y = it->y;
    for ( ; it != map_border_coords.end(); ++it)
    {
        if(it->x > map_size.x)
            map_size.x = it->x;
        if(it->y > map_size.y)
            map_size.y = it->y;

        if(it->y != prev_point_y)
        {
            struct vertical_line line;
            line.x = it->x;
            line.y1 = min(it->y, prev_point_y);
            line.y2 = max(it->y, prev_point_y);
            lines.push_back(line);
        }
        prev_point_y = it->y;
    }

//3. extract vertical lines for obstacles
    for(auto obstacle : obstacles_list)
    {
        auto it = obstacle.begin();
        obstacle.push_back(coords(it->x, it->y));

        it = obstacle.begin();
        int prev_point_y = it->y;
        for ( ; it != obstacle.end(); ++it)
        {
            if(it->y != prev_point_y)
            {
                struct vertical_line line;
                line.x = it->x;
                line.y1 = min(it->y, prev_point_y);
                line.y2 = max(it->y, prev_point_y);
                lines.push_back(line);
            }
            prev_point_y = it->y;
        }
    }

//3. Create map
    for(int y = 0; y < map_size.y; y++)
    {
        int border_crosses = 0;
        for(int x = 0; x < map_size.x; x++)
        {
            for(auto line : lines)
            {
                if((line.x == x) && (y >= line.y1) && (y < line.y2))
                {
                    border_crosses ++;
                    break;
                }
            }
            if(border_crosses % 2)
            {
                SmartGraph::Node nd = graph.addNode();
                string index = coords(x, y).tostr();
                tiles[index].position = coords(x, y);
                tiles[index].node_id = graph.id(nd);
                graph_tiles[nd] = &tiles[index];
            }
        }
    }

//4. Create graph and scan regions
    for(int x = 0; x < map_size.x; x++)
    {
        for(int y = 0; y < map_size.y; y++)
        {
            map<string, struct map_tile>::iterator it;
            it = tiles.find(coords(x, y).tostr());
            if (it == tiles.end())
                continue;
            struct map_tile &tile = it->second;
            int neighbors = 0;

            it = tiles.find(coords(x+1, y).tostr());
            if (it != tiles.end())
            {
                graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                neighbors++;
            }
            it = tiles.find(coords(x-1, y).tostr());
            if (it != tiles.end())
            {
                graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                neighbors++;
            }
            it = tiles.find(coords(x, y+1).tostr());
            if (it != tiles.end())
            {
                graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                neighbors++;
            }
            it = tiles.find(coords(x, y-1).tostr());
            if (it != tiles.end())
            {
                graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                neighbors++;
            }
            if(neighbors >= 3 && in_region(coords(x, y)) == nullptr)
            {
                cRegion *region = nullptr;
                if((region = in_region(coords(x+1, y))) ||
                   (region = in_region(coords(x, y+1))) ||
                   (region = in_region(coords(x-1, y))) ||
                   (region = in_region(coords(x, y-1))))
                {
                   region->add(coords(x, y));
                } else {
                    class cRegion new_region(coords(x, y));
                    new_region.set_id(regions.size());
                    regions.push_back(new_region);
                }
            }
        }
    }
}

void cRegion::add(struct coords target)
{
    if(in_region(target))
        return;
    rect_t new_rect(rect.base_point(), rect.base_point()+rect.size());
    if(target.x < new_rect.p1.x)
        new_rect.p1.x = target.x;
    if(target.x+1 > new_rect.p2.x)
        new_rect.p2.x = target.x+1;
    if(target.y < new_rect.p1.y)
        new_rect.p1.y = target.y;
    if(target.y+1 > new_rect.p2.y)
        new_rect.p2.y = target.y+1;
    rect = new_rect;
}

class cRegion * cMap::in_region(struct coords target)
{
    for(auto &region: regions)
    {
        if(region.in_region(target))
            return &region;
    }

    return nullptr;
}

class cRegion * cMap::get_region(int reg_id)
{
    if(reg_id < 0)
        return nullptr;
    for(auto &region: regions)
    {
        if(region.get_id() == reg_id)
            return &region;
    }

    return nullptr;
}

void cMap::delete_region(int reg_id)
{
    if(reg_id < 0)
        return;

    for (auto region = regions.begin(); region != regions.end(); ++region)
    {
        if(region->get_id() == reg_id)
        {
            regions.erase(region);
            return;
        }
    }
}

cMap::~cMap()
{
}

void cMap::reset_edges_cost()
{
    for (SmartGraph::ArcIt arc(graph); arc != INVALID; ++arc)
    {
        costMap[arc] = WEIGHT_DEFAULT;
    }
}

void cMap::update_edges_cost(bool is_vertical, vector<struct coords> manips)
{
    for (SmartGraph::ArcIt arc(graph); arc != INVALID; ++arc)
    {
        auto src = graph_tiles[graph.source(arc)];
        auto dst = graph_tiles[graph.target(arc)];
        bool vertical_edge = src->position.x == dst->position.x;
        costMap[arc] = WEIGHT_DEFAULT;
        if(vertical_edge != is_vertical)
        {
            costMap[arc] += WEIGHT_NOT_STRAIGHT_DIR;
        }
        if(dst->wrapped == true)
        {
            costMap[arc] += WEIGHT_NOT_UNWRAPPED_DST;
        }
        for(auto manip_coord: manips)
        {
            wrappable_e wrp = test_wrappable(dst->position, manip_coord, false);
            switch(wrp)
            {
            case WRP_WALL:
                costMap[arc] += WEIGHT_WALL_MANIP;
                break;
            case WRP_WRAPPED:
            case WRP_CAN_NOT_WRAP:
                costMap[arc] += WEIGHT_NOT_UNWRAPPED_MANIP;
                break;
            }
        }

        int neighbors = 0;
        map<string, struct map_tile>::iterator it;
        it = tiles.find(coords(dst->position.x+1, dst->position.y).tostr());
        if (it != tiles.end() && !it->second.wrapped)
            neighbors++;
        it = tiles.find(coords(dst->position.x-1, dst->position.y).tostr());
        if (it != tiles.end() && !it->second.wrapped)
            neighbors++;
        it = tiles.find(coords(dst->position.x, dst->position.y+1).tostr());
        if (it != tiles.end() && !it->second.wrapped)
            neighbors++;
        it = tiles.find(coords(dst->position.x, dst->position.y-1).tostr());
        if (it != tiles.end() && !it->second.wrapped)
            neighbors++;
        if(neighbors > 1 || dst->wrapped)
        {
            costMap[arc] += WEIGHT_NOT_LONE_TILE;
        }
    }
}


void cMap::place_boosters(vector<struct coords> boosters_coords)
{
    for(auto booster_coords : boosters_coords)
    {
        auto it = tiles.find(booster_coords.tostr());
        if (it == tiles.end())
        {
            cout << "Trying to place booster on not not-existence tile: " << booster_coords.tostr() << endl;
            exit(-1);
        }
        struct map_tile &tile = it->second;
        tile.position.booster = booster_coords.booster;
    }
}

void cMap::draw(void)
{
    struct map_tile cur_tile;
    cout << "      " << "   ";
    for(int x = 0; x < map_size.x; x++)
    {
        cout <<  setw(3) << x;
    }
    cout << endl;
    for(int y = map_size.y-1; y >= 0; y--)
    {
        cout << setw(6) << y << "   ";
        for(int x = 0; x < map_size.x; x++)
        {
            auto it = tiles.find(coords(x, y).tostr());
            if (it != tiles.end())
            {
                struct map_tile &tile = it->second;
                string symbol;
                string pre  = (tile.wrapped)?"<":" ";
                string post = (tile.wrapped)?">":" ";

                auto region = in_region(coords(x, y));
                if(region != nullptr)
                {
                    pre = std::to_string(region->get_id());
                }
                switch (tile.position.booster)
                {
                    case BOOST_NONE: symbol         = pre+"#"+post; break;
                    case BOOST_EXT_MANIP: symbol    = pre+"B"+post; break;
                    case BOOST_FAST_WHEELS: symbol  = pre+"F"+post; break;
                    case BOOST_DRILL: symbol        = pre+"L"+post; break;
                    case BOOST_X: symbol            = pre+"X"+post; break;
                    case BOOST_RESET: symbol        = pre+"R"+post; break;
                default:
                    cout << "Unknown tile type: " << tile.position.booster << endl;
                    exit(-1);
                }
                cout << symbol;
            } else
                cout << "   ";
        }
        cout << endl;
    }
    cout << endl;
}

wrappable_e cMap::test_wrappable(struct coords worker, struct coords manip_rel, bool wrap)
{
    auto worker_pos_it = tiles.find(worker.tostr());
    if (worker_pos_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }

    struct coords manip = worker + manip_rel;

    auto manip_pos_it = tiles.find(manip.tostr());
    if (manip_pos_it == tiles.end())
    {
        return WRP_WALL;
    }
    if (manip_pos_it->second.wrapped)
    {
        return WRP_WRAPPED;
    }
//FIXME: Cheating: test for reachable
    if((manip_rel.x > 1) || (manip_rel.x < -1) || (manip_rel.y > 1) || (manip_rel.y < -1))
    {
        //long arms
        struct coords tmp(worker.x + manip_rel.x/2, worker.y + manip_rel.y/2);
        auto pos_it = tiles.find(tmp.tostr());
        if (pos_it == tiles.end())
        {
            return WRP_CAN_NOT_WRAP;
        }
    }
    if (wrap)
    {
        manip_pos_it->second.wrapped = true;
        return WRP_WRAPRED_OK;
    }
    return WRP_CAN_WRAP;
}

bool cMap::is_unwrapped(struct coords target)
{
    auto pos_it = tiles.find(target.tostr());
    if (pos_it == tiles.end())
    {
        return false;
    }
    if (pos_it->second.wrapped)
    {
        return false;
    }
    return true;
}

boosters_e cMap::pick_booster(struct coords target)
{
    auto pos_it = tiles.find(target.tostr());
    if (pos_it == tiles.end())
    {
        return BOOST_NONE;
    }
    boosters_e booster = pos_it->second.position.booster;
    pos_it->second.position.booster = BOOST_NONE;
    return booster;
}

void cMap::try_wrap(struct coords worker, vector<struct coords> manips_rel)
{
    auto worker_pos_it = tiles.find(worker.tostr());
    if (worker_pos_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    struct map_tile &worker_pos = worker_pos_it->second;
    worker_pos.wrapped = true;
    for(auto manip_rel : manips_rel)
    {
        test_wrappable(worker, manip_rel, true);
    }
}

struct coords cMap::find_target(struct coords worker, int region_id)
{
//looking for far point in region
    struct coords max_target(worker);
    int max_dist = 0;
    struct coords min_target(worker);
    int min_dist = -1;
    struct coords min_booster(worker);
    int min_dist_boost = -1;

    Dijkstra<lemon::SmartGraph> dijkstra(graph, costMap);

    auto worker_it = tiles.find(worker.tostr());
    if (worker_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    SmartGraph::Node from = graph.nodeFromId(worker_it->second.node_id);
    dijkstra.run(from);

    for (SmartGraph::NodeIt n(graph); n != INVALID; ++n)
    {
        bool is_booster = (graph_tiles[n]->position.booster == BOOST_EXT_MANIP) ||
//                          (graph_tiles[n]->position.booster == BOOST_FAST_WHEELS) ||
                          (graph_tiles[n]->position.booster == BOOST_DRILL);
        if(region_id >= 0)
        {
            if(!get_region(region_id)->in_region(graph_tiles[n]->position))
                continue;
        }
        if((graph_tiles[n]->wrapped == true) && (is_booster == false))
            continue;
        int dist = dijkstra.dist(n);
        if(min_dist < 0 || dist < min_dist)
        {
            min_dist = dist;
            min_target = graph_tiles[n]->position;
        }

        if(region_id >= 0)
        {
            struct coords tmp_sz(rect_t(worker, graph_tiles[n]->position).size());
            if(max_dist < dist && dist < 8 && ((tmp_sz.x<2) || (tmp_sz.y<2)))
            {
                max_dist = dist;
                max_target = graph_tiles[n]->position;
            }
        }
        if((is_booster) && (min_dist_boost < 0 || dist < min_dist_boost))
        {
            min_dist_boost = dist;
            min_booster = graph_tiles[n]->position;
        }
    }
    struct coords target((max_dist>0)?max_target:min_target);
//    int tgt_dist = (max_dist>0)?max_dist:min_dist;
//    std::cout << "Path from " << worker.tostr()  << " to " << target.tostr() << " is: " << tgt_dist << std::endl;

    if(min_dist_boost > 0 && min_dist_boost < 5)
    {
        target = min_booster;
    }
    return target;
}

int cMap::estimate_route(struct coords worker, struct coords target)
{
    Dijkstra<lemon::SmartGraph> dijkstra(graph, costMap);

    auto worker_it = tiles.find(worker.tostr());
    if (worker_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    auto target_it = tiles.find(target.tostr());
    if (target_it == tiles.end())
    {
        cout << "Worker can't get here: " << target.tostr() << endl;
        exit(-1);
    }

    SmartGraph::Node from = graph.nodeFromId(worker_it->second.node_id);
    SmartGraph::Node to   = graph.nodeFromId(target_it->second.node_id);

    dijkstra.run(from, to);
    return dijkstra.dist(to);
}

directions_e cMap::get_direction(struct coords worker, struct coords target)
{
    Dijkstra<lemon::SmartGraph> dijkstra(graph, costMap);
    struct coords next(worker);

    auto worker_it = tiles.find(worker.tostr());
    if (worker_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    auto target_it = tiles.find(target.tostr());
    if (target_it == tiles.end())
    {
        cout << "Worker can't get here: " << target.tostr() << endl;
        exit(-1);
    }

    SmartGraph::Node from = graph.nodeFromId(worker_it->second.node_id);
    SmartGraph::Node to   = graph.nodeFromId(target_it->second.node_id);

    dijkstra.run(from, to);

    vector<struct map_tile*> path;
    for (lemon::SmartGraph::Node v = to; v != from; v = dijkstra.predNode(v))
    {
        if (v != lemon::INVALID && dijkstra.reached(v)) //special LEMON node constant
        {
            next = graph_tiles[v]->position;
        }
    }

//    int cost = dijkstra.dist(to);
//    //print out the path with reverse iterator
//    std::cout << "Path from " << worker.tostr()  << " to " << target.tostr() << " is: ";
//    for (auto p = path.rbegin(); p != path.rend(); ++p)
//        std::cout << (*p)->position.tostr() << " ";
//    std::cout << std::endl << "Total cost for the shortest path is: "<< cost << std::endl;
//
//    std::cout << "Arcs from node " << worker.tostr() << std::endl;
//    for (SmartGraph::OutArcIt a(graph, from); a != INVALID; ++a)
//    {
//        std::cout << "   Arc " << graph.id(a) << " to node " << graph_tiles[graph.target(a)]->position.tostr() << " cost " << costMap[a] << std::endl;
//
//    }


    int dx = next.x - worker.x;
    int dy = next.y - worker.y;

    directions_e res;
    if( dx > 0 && dy == 0)
    {
        res = DIR_RI;
    } else if( dx == 0 && dy < 0) {
        res = DIR_DN;
    } else if( dx < 0 && dy == 0) {
        res = DIR_LE;
    } else if( dx == 0 && dy > 0) {
        res = DIR_UP;
    } else {
        cout << "Can't find a way from " << worker.tostr() << " to " << target.tostr() << endl;
        exit(-1);
    }
    return res;
}


