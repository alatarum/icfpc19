#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw

#include "cMap.h"
#include "lemon/dijkstra.h"

static const int WEIGHT_DEFAULT = 1;
static const int WEIGHT_NOT_LONE_TILE = 25;
static const int WEIGHT_NOT_STRAIGHT_DIR = 18;
static const int WEIGHT_NOT_UNWRAPPED_DST = 6;
static const int WEIGHT_NOT_UNWRAPPED_MANIP = 6;
static const int WEIGHT_WALL_MANIP = 10;
static const int MAX_TARGET_DISTANCE = 15;

using namespace std;

struct vertical_line
{
    int x;
    int y1, y2;
};

cMap::cMap(vector<struct coords> map_border_coords, vector<vector<struct coords>> obstacles_list) :
    map_size(0, 0), graph(), graph_tiles(graph),
    graph_filter_node(graph, false), graph_filter_edge(graph, true),
    dgraph(graph, graph_filter_node, graph_filter_edge), costMap(dgraph, 1),
    graph_full_node(graph, false), graph_full_edge(graph, true),
    fgraph(graph, graph_full_node, graph_full_edge), fcostMap(fgraph, 1),
    draw_use_colors(false)
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
    tiles = std::vector<std::vector<struct map_tile> > (
        map_size.x, std::vector<struct map_tile>(map_size.y));

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
            SmartGraph::Node nd = graph.addNode();
            tiles[x][y].node_id = graph.id(nd);
            tiles[x][y].position = coords(x, y);
            if(border_crosses % 2)
            {
                tiles[x][y].drilled = true;
            }
            if(tiles[x][y].drilled)
                tiles[x][y].wrapped = false;
            else
                tiles[x][y].wrapped = true;
            graph_tiles[nd] = &(tiles[x][y]);
            graph_filter_node[nd] = tiles[x][y].drilled;
            graph_full_node[nd] = true;
        }
    }

//4. Create graph and scan regions
    for(int x = 0; x < map_size.x; x++)
    {
        for(int y = 0; y < map_size.y; y++)
        {
            int neighbors = 0;
            if(create_edge(coords(x, y), coords(x+1, y)))
            {
                neighbors++;
            }
            if(create_edge(coords(x, y), coords(x, y+1)))
            {
                neighbors++;
            }
            if(create_edge(coords(x, y), coords(x-1, y)))
            {
                neighbors++;
            }
            if(create_edge(coords(x, y), coords(x, y-1)))
            {
                neighbors++;
            }

            if((neighbors >= 3) && (in_region(coords(x, y)) == nullptr))
            {
                cRegion *region = nullptr;
                if((region = in_region(coords(x+1, y))) ||
                   (region = in_region(coords(x, y+1))) ||
                   (region = in_region(coords(x-1, y))) ||
                   (region = in_region(coords(x, y-1))))
                {
                   region->add(coords(x, y));
                } else {
                    auto new_region = new cRegion(coords(x, y), graph);
                    new_region->set_id(regions.size());
                    regions.push_back(new_region);
                }
            }
        }
    }
    for(auto &region: regions)
    {
        region->commit(graph, graph_tiles);
    }
}

cRegion::cRegion(struct coords target, SmartGraph &graph):
        id(0), rect(coords(target.x-1,target.y-1), coords(target.x+2,target.y+2)),
        graph_filter_node(graph, true), graph_filter_edge(graph, true),
        dgraph(graph, graph_filter_node, graph_filter_edge), costMap(dgraph, 1)
{
}

void cRegion::commit(SmartGraph &graph, SmartGraph::NodeMap<struct map_tile *> &graph_tiles)
{
    for (SmartGraph::EdgeIt e(graph); e != INVALID; ++e)
    {
        graph_filter_edge[e]= true;
    }
    for (SmartGraph::NodeIt n(graph); n != INVALID; ++n)
    {
        auto tile = graph_tiles[n];
        if(in_region(tile->position) && tile->drilled)
            graph_filter_node[n]= true;
        else
            graph_filter_node[n]= false;
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
        if(region->in_region(target))
            return region;
    }

    return nullptr;
}

class cRegion * cMap::get_region(int reg_id)
{
    if(reg_id < 0)
        return nullptr;
    for(auto &region: regions)
    {
        if(region->get_id() == reg_id)
            return region;
    }

    return nullptr;
}

void cMap::delete_region(int reg_id)
{
    if(reg_id < 0)
        return;

    for (auto region = regions.begin(); region != regions.end(); ++region)
    {
        if((*region)->get_id() == reg_id)
        {
            delete *region;
            regions.erase(region);
            return;
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
        tile(booster_coords).booster = booster_coords.booster;
    }
}

void cMap::draw(struct coords worker, vector<struct coords> manips_rel, struct coords target)
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
            if (tiles[x][y].drilled)
            {
                struct map_tile &tile = tiles[x][y];
                string symbol;
                string color;
                string pre  = (tile.wrapped)?"<":" ";
                string post = (tile.wrapped)?">":" ";

                auto region = in_region(coords(x, y));
                if(region != nullptr)
                {
                    pre = std::to_string(region->get_id());
                }

                if(coords(x, y) == worker)
                    color = "\033[32m";
                else if(coords(x, y) == target)
                    color = "\033[31m";
                else
                {
                    for(struct coords manip: manips_rel)
                    {
                        if(worker + manip == coords(x, y))
                        {
                            auto reachable = test_wrappable(worker, manip, false);
                            if((reachable == WRP_CAN_WRAP) || (reachable == WRP_WRAPPED))
                            color = "\033[35m";
                        }
                    }
                }

                switch (tile.booster)
                {
                    case BOOST_NONE: symbol         = pre+"#"+post; break;
                    case BOOST_EXT_MANIP: symbol    = pre+"B"+post; break;
                    case BOOST_FAST_WHEELS: symbol  = pre+"F"+post; break;
                    case BOOST_DRILL: symbol        = pre+"L"+post; break;
                    case BOOST_X: symbol            = pre+"X"+post; break;
                    case BOOST_RESET: symbol        = pre+"R"+post; break;
                    case BOOST_CLONE: symbol        = pre+"C"+post; break;
                default:
                    cout << "Unknown tile type: " << tile.booster << endl;
                    exit(-1);
                }
                if(draw_use_colors)
                    symbol = color + symbol + "\033[0m";
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
    if((!rect_t(coords(0,0), coords(map_size)).in_rect(worker)) || (!tile(worker).drilled))
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }

    struct coords manip = worker + manip_rel;
    if (!rect_t(coords(0,0), coords(map_size)).in_rect(manip))
    {
        return WRP_WALL;
    }
    auto &manip_tile = tile(manip);
    if (!manip_tile.drilled)
    {
        return WRP_WALL;
    }
    if (manip_tile.wrapped)
    {
        return WRP_WRAPPED;
    }
//FIXME: Cheating: test for reachable
    if((manip_rel.x > 1) || (manip_rel.x < -1) || (manip_rel.y > 1) || (manip_rel.y < -1))
    {
        //long arms
        if (!tiles[worker.x + manip_rel.x/2][worker.y + manip_rel.y/2].drilled)
        {
            return WRP_CAN_NOT_WRAP;
        }
    }
    if (wrap)
    {
        manip_tile.wrapped = true;
        return WRP_WRAPRED_OK;
    }
    return WRP_CAN_WRAP;
}

bool cMap::is_accessible(coords target)
{
    if (!rect_t(coords(0,0), coords(map_size)).in_rect(target))
    {
        return false;
    }
    auto &target_tile = tile(target);
    if (!target_tile.drilled)
    {
        return false;
    }
    return true;
}


bool cMap::is_unwrapped(struct coords target)
{
    if (!rect_t(coords(0,0), coords(map_size)).in_rect(target))
    {
        return false;
    }
    auto &target_tile = tile(target);
    if (!target_tile.drilled)
    {
        return false;
    }
    if (target_tile.wrapped)
    {
        return false;
    }
    return true;
}

boosters_e cMap::pick_booster(struct coords target)
{
    auto &target_tile = tile(target);
    if (!target_tile.drilled)
    {
        return BOOST_NONE;
    }
    boosters_e booster = target_tile.booster;
    target_tile.booster = BOOST_NONE;
    return booster;
}

void cMap::try_wrap(struct coords worker, vector<struct coords> manips_rel)
{
    auto &worker_tile = tile(worker);
    if (!worker_tile.drilled)
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    worker_tile.wrapped = true;
    for(auto manip_rel : manips_rel)
    {
        test_wrappable(worker, manip_rel, true);
    }
}

bool cMap::drill_tile(struct coords target)
{
    if (!rect_t(coords(0,0), coords(map_size)).in_rect(target))
        return false;
    auto &target_tile = tile(target);
    target_tile.wrapped = true;
    target_tile.drilled = true;
    target_tile.booster = BOOST_NONE;
    for(auto &region: regions)
    {
        region->commit(graph, graph_tiles);
    }
    graph_filter_node[graph.nodeFromId(target_tile.node_id)] = true;
    return true;
}

bool cMap::create_edge(coords node, coords to)
{
    if(to.x < 0 || to.y < 0 || to.x >= map_size.x || to.y >= map_size.y)
        return false;
    SmartGraph::Node from = graph.nodeFromId(tiles[node.x][node.y].node_id);
    SmartGraph::Node next = graph.nodeFromId(tiles[to.x][to.y].node_id);

    bool result = (tiles[to.x][to.y].drilled && tiles[node.x][node.y].drilled);

    for(SmartGraph::IncEdgeIt edge(graph, from); edge != INVALID; ++ edge)
    {
        SmartGraph::Node opp_node = graph.oppositeNode(from, edge);
        if(graph.id(opp_node) == graph.id(next)) return result;
    }
    graph_filter_edge[graph.addEdge(from, next)] = true;
    graph_full_edge[graph.addEdge(from, next)] = true;
    return result;
}

void cMap::reset_edges_cost(int region_id)
{
    SubGraph<SmartGraph>::ArcMap<int> *cm = &costMap;
    if(region_id >= 0)
    {
        cm = get_region(region_id)->get_graphcostmap();
    } else if (region_id == DRILL_ACTIVATED) {
        cm = &fcostMap;
    }
    for (SmartGraph::ArcIt arc(graph); arc != INVALID; ++arc)
    {
        (*cm)[arc] = WEIGHT_DEFAULT;
        fcostMap[arc] = WEIGHT_DEFAULT;
    }
}

void cMap::update_edges_cost(bool is_vertical, vector<struct coords> manips, int region_id)
{
    SubGraph<SmartGraph>::ArcMap<int> *cm = &costMap;
    if(region_id >= 0)
    {
        cm = get_region(region_id)->get_graphcostmap();
    } else if (region_id == DRILL_ACTIVATED) {
        cm = &fcostMap;
    }
    for (SubGraph<SmartGraph>::ArcIt arc(dgraph); arc != INVALID; ++arc)
    {
        auto src = graph_tiles[graph.source(arc)];
        auto dst = graph_tiles[graph.target(arc)];
        bool vertical_edge = src->position.x == dst->position.x;
        (*cm)[arc] = WEIGHT_DEFAULT;
        if(vertical_edge != is_vertical)
        {
            (*cm)[arc] += WEIGHT_NOT_STRAIGHT_DIR;
        }
        if(dst->wrapped == true)
        {
            (*cm)[arc] += WEIGHT_NOT_UNWRAPPED_DST;
        }
        for(auto manip_coord: manips)
        {
            wrappable_e wrp = test_wrappable(dst->position, manip_coord, false);
            switch(wrp)
            {
            case WRP_WALL:
                (*cm)[arc] += WEIGHT_WALL_MANIP;
                break;
            case WRP_WRAPPED:
            case WRP_CAN_NOT_WRAP:
                (*cm)[arc] += WEIGHT_NOT_UNWRAPPED_MANIP;
                break;
            case WRP_CAN_WRAP:
                (*cm)[arc] += 0;
                break;
            default:
                cout << "Unexpected test_wrappable result: " << wrp << endl;
                exit(-1);
            }
        }

        int neighbors = 0;
        coords neighbor_pos;

        neighbor_pos = coords(dst->position.x+1, dst->position.y);
        if (rect_t(coords(0,0), coords(map_size)).in_rect(neighbor_pos) && tile(neighbor_pos).drilled)
            neighbors++;
        neighbor_pos = coords(dst->position.x, dst->position.y+1);
        if (rect_t(coords(0,0), coords(map_size)).in_rect(neighbor_pos) && tile(neighbor_pos).drilled)
            neighbors++;
        neighbor_pos = coords(dst->position.x-1, dst->position.y);
        if (rect_t(coords(0,0), coords(map_size)).in_rect(neighbor_pos) && tile(neighbor_pos).drilled)
            neighbors++;
        neighbor_pos = coords(dst->position.x, dst->position.y-1);
        if (rect_t(coords(0,0), coords(map_size)).in_rect(neighbor_pos) && tile(neighbor_pos).drilled)
            neighbors++;
        if(neighbors > 1 || dst->wrapped)
        {
            (*cm)[arc] += WEIGHT_NOT_LONE_TILE;
        }
        fcostMap[arc] = (*cm)[arc];
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
    rect_t rect(coords(0,0), map_size);

    SubGraph<SmartGraph> *sg = &dgraph;
    SubGraph<SmartGraph>::ArcMap<int> *cm = &costMap;
    if(region_id >= 0)
    {
        auto region = get_region(region_id);
        if(region->in_region(worker))
        {
            sg = region->get_subgraph();
            cm = region->get_graphcostmap();
        }
        rect = region->get_rect();
    } else if (region_id == DRILL_ACTIVATED) {
        sg = &fgraph;
        cm = &fcostMap;
        auto region = in_region(worker);
        if(region != nullptr)
            rect = region->get_rect();
    }
    Dijkstra<SubGraph<SmartGraph>> dijkstra(*sg, *cm);

    auto &worker_tile = tile(worker);
    if (!worker_tile.drilled)
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    SubGraph<SmartGraph>::Node from = sg->nodeFromId(worker_tile.node_id);
    dijkstra.run(from);

    for (SubGraph<SmartGraph>::NodeIt n(*sg); n != INVALID; ++n)
    {
        if(!dijkstra.reached(n))
            continue;
        bool is_booster = (graph_tiles[n]->booster == BOOST_EXT_MANIP) ||
                          (graph_tiles[n]->booster == BOOST_FAST_WHEELS) ||
                          (graph_tiles[n]->booster == BOOST_DRILL);
        if((graph_tiles[n]->wrapped == true) && (is_booster == false))
            continue;
        if(!rect.in_rect(graph_tiles[n]->position))
            continue;
        int dist = dijkstra.dist(n);
        if(min_dist < 0 || dist < min_dist)
        {
            min_dist = dist;
            min_target = graph_tiles[n]->position;
        }

        if(region_id != REGION_NOT_SELECTED)
        {
            struct coords tmp_sz(rect_t(worker, graph_tiles[n]->position).size());
            if(max_dist < dist && dist < MAX_TARGET_DISTANCE && ((tmp_sz.x < 3) || (tmp_sz.y < 3)))
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
    int tgt_dist = (max_dist>0)?max_dist:min_dist;

    if(min_dist_boost > 0 && min_dist_boost < 5)
    {
        target = min_booster;
        tgt_dist = min_dist_boost;
    }
    std::cout << "Path from " << worker.tostr()  << " to " << target.tostr() << " is: " << tgt_dist << std::endl;
    return target;
}

int cMap::estimate_route(struct coords worker, struct coords target, int region_id)
{
    SubGraph<SmartGraph> *sg = &dgraph;
    SubGraph<SmartGraph>::ArcMap<int> *cm = &costMap;
    if(region_id >= 0)
    {
        auto region = get_region(region_id);
        if(region->in_region(worker) && region->in_region(target))
        {
            sg = region->get_subgraph();
            cm = region->get_graphcostmap();
        }
    } else if (region_id == DRILL_ACTIVATED) {
        sg = &fgraph;
        cm = &fcostMap;
    }
    Dijkstra<SubGraph<SmartGraph>> dijkstra(*sg, *cm);

    auto &worker_tile = tile(worker);
    if (!worker_tile.drilled)
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    auto &target_tile = tile(target);
    if (!target_tile.drilled && (region_id != DRILL_ACTIVATED))
    {
        cout << "Worker can't get here: " << target.tostr() << endl;
        return -1;
    }

    SubGraph<SmartGraph>::Node from = sg->nodeFromId(worker_tile.node_id);
    SubGraph<SmartGraph>::Node to   = sg->nodeFromId(target_tile.node_id);

    dijkstra.run(from, to);
    if(!dijkstra.reached(to))
        return -1;
    return dijkstra.dist(to);
}

directions_e cMap::get_direction(struct coords worker, struct coords target, int region_id)
{
    SubGraph<SmartGraph> *sg = &dgraph;
    SubGraph<SmartGraph>::ArcMap<int> *cm = &costMap;
    if(region_id >= 0)
    {
        auto region = get_region(region_id);
        if(region->in_region(worker) && region->in_region(target))
        {
            sg = region->get_subgraph();
            cm = region->get_graphcostmap();
        }
    } else if (region_id == DRILL_ACTIVATED) {
        sg = &fgraph;
        cm = &fcostMap;
    }
    Dijkstra<SubGraph<SmartGraph>> dijkstra(*sg, *cm);
    struct coords next(worker);

    auto &worker_tile = tile(worker);
    if (!worker_tile.drilled)
    {
        cout << "Worker can't be here: " << worker.tostr() << endl;
        exit(-1);
    }
    auto &target_tile = tile(target);
    if (!target_tile.drilled && (region_id != DRILL_ACTIVATED))
    {
        cout << "Worker can't get here: " << target.tostr() << endl;
        return DIR_COUNT;
    }

    SubGraph<SmartGraph>::Node from = sg->nodeFromId(worker_tile.node_id);
    SubGraph<SmartGraph>::Node to   = sg->nodeFromId(target_tile.node_id);

    dijkstra.run(from, to);
    if(!dijkstra.reached(to))
        return DIR_COUNT;

    vector<struct map_tile*> path;
    for (SubGraph<SmartGraph>::Node v = to; v != from; v = dijkstra.predNode(v))
    {
        if((sg->id(v) == 1) &&
           (sg->id(dijkstra.predNode(v)) == 1))
        {
            return DIR_COUNT;
        }
        if (v != INVALID && dijkstra.reached(v)) //special LEMON node constant
        {
            next = graph_tiles[v]->position;
        }
    }

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


