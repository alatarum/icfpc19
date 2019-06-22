#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw

#include "cMap.h"
#include "lemon/dijkstra.h"

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
                ListGraph::Node nd = graph.addNode();
                string index = coords(x, y).tostr();
                tiles[index].position = coords(x, y);
                tiles[index].node_id = graph.id(nd);
                graph_tiles[nd] = &tiles[index];
            }
        }
    }

//4. Create graph
    for(int x = 0; x < map_size.x; x++)
    {
        for(int y = 0; y < map_size.y; y++)
        {
            map<string, struct map_tile>::iterator it;
            it = tiles.find(coords(x, y).tostr());
            if (it == tiles.end())
                continue;
            struct map_tile &tile = it->second;

            it = tiles.find(coords(x+1, y).tostr());
            if (it != tiles.end())
            {
                tile.links[DIR_RI] = &it->second;
                auto edge = graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                costMap[graph.direct(edge, true)] = 1;
                costMap[graph.direct(edge, false)] = 1;
            }
            it = tiles.find(coords(x-1, y).tostr());
            if (it != tiles.end())
            {
                tile.links[DIR_LE] = &it->second;
                auto edge = graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                costMap[graph.direct(edge, true)] = 1;
                costMap[graph.direct(edge, false)] = 1;
            }
            it = tiles.find(coords(x, y+1).tostr());
            if (it != tiles.end())
            {
                tile.links[DIR_UP] = &it->second;
                auto edge = graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                costMap[graph.direct(edge, true)] = 1;
                costMap[graph.direct(edge, false)] = 1;
            }
            it = tiles.find(coords(x, y-1).tostr());
            if (it != tiles.end())
            {
                tile.links[DIR_DN] = &it->second;
                auto edge = graph.addEdge(graph.nodeFromId(tile.node_id), graph.nodeFromId(it->second.node_id));
                costMap[graph.direct(edge, true)] = 1;
                costMap[graph.direct(edge, false)] = 1;
            }

//            cout << "Tile " << tile.position.tostr_full() << " points to: ";
//            if(tile.links[DIR_RI]) cout << tile.links[DIR_RI]->position.tostr_full() << " ";
//            if(tile.links[DIR_LE]) cout << tile.links[DIR_LE]->position.tostr_full() << " ";
//            if(tile.links[DIR_UP]) cout << tile.links[DIR_UP]->position.tostr_full() << " ";
//            if(tile.links[DIR_DN]) cout << tile.links[DIR_DN]->position.tostr_full() << " ";
//            cout << endl;
        }
    }
}

cMap::~cMap()
{
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
                string pre = (tile.wrapped)?"<":" ";
                string post = (tile.wrapped)?">":" ";
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

void cMap::try_wrap(struct coords worker, vector<struct coords> manips)
{
    auto worker_pos_it = tiles.find(worker.tostr());
    if (worker_pos_it == tiles.end())
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    struct map_tile &worker_pos = worker_pos_it->second;
    worker_pos.wrapped = true;
//TODO: collect boosters
    for(auto manip : manips)
    {
        auto manip_pos_it = tiles.find(manip.tostr());
        if (manip_pos_it == tiles.end())
        {
            continue;
        }
        manip_pos_it->second.wrapped = true;
//TODO: test for reachable
    }
}

static int calc_dist(struct coords p1, struct coords p2)
{
    int x = p1.x - p2.x;
    int y = p1.y - p2.y;
    return x*x+y*y;
}

struct coords cMap::find_target(struct coords worker, vector<struct coords> manips)
{
    int min_dist = calc_dist(coords(0,0), map_size);
    struct coords target(worker);
    for(int y = 0; y < map_size.y; y++)
    {
        for(int x = 0; x < map_size.x; x++)
        {
            auto cur_tile = coords(x, y);
            auto tile_it = tiles.find(cur_tile.tostr());
            if (tile_it == tiles.end())
                continue;
            if(tile_it->second.wrapped == true)
                continue;
            int dist = calc_dist(worker, cur_tile);
            if(min_dist > dist)
            {
                min_dist = dist;
                target = cur_tile;
            }
        }
    }

    return target;
}

directions_e cMap::get_direction(struct coords worker, struct coords target)
{
    Dijkstra<lemon::ListGraph> dijkstra(graph, costMap);

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

    ListGraph::Node from = graph.nodeFromId(worker_it->second.node_id);
    ListGraph::Node to   = graph.nodeFromId(target_it->second.node_id);

    dijkstra.run(from, to);

    vector<struct map_tile*> path;
    for (lemon::ListGraph::Node v = to; v != from; v = dijkstra.predNode(v))
    {
        if (v != lemon::INVALID && dijkstra.reached(v)) //special LEMON node constant
        {
            path.push_back(graph_tiles[v]);
        }
    }
    struct coords next((*path.rbegin())->position);

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




