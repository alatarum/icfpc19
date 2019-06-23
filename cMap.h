#ifndef CMAP_H
#define CMAP_H

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <lemon/list_graph.h>

using namespace std;
using namespace lemon;

enum boosters_e {
    BOOST_NONE,
    BOOST_EXT_MANIP,
    BOOST_FAST_WHEELS,
    BOOST_DRILL,
    BOOST_X,
    BOOST_RESET,
    BOOST_COUNT
};

const string booster_name[] = {
    [BOOST_NONE] = "NONE",
    [BOOST_EXT_MANIP] = "B",
    [BOOST_FAST_WHEELS] = "F",
    [BOOST_DRILL] = "L",
    [BOOST_X] = "X",
    [BOOST_RESET] = "R"
};

struct coords
{
    coords(int _x, int _y): x(_x), y(_y), booster(BOOST_NONE) {}
    coords(int _x, int _y, boosters_e _b): x(_x), y(_y), booster(_b) {}
    string tostr() {
        std::ostringstream sstr;
        sstr << "(" << x << ";" << y << ")";
        return sstr.str();
        }
    string tostr_full() {
        std::ostringstream sstr;
        sstr << tostr();
        if(booster != BOOST_NONE)
            sstr << booster_name[booster];
        return sstr.str();
        }
    void print() {cout << tostr_full() << endl;}
    bool operator == (const struct coords &other) const {
        return (x == other.x) && (y == other.y);
    }
    struct coords operator+(struct coords const & c1) {
        return coords(x+c1.x, y+c1.y);
    }

    int x;
    int y;
    boosters_e booster;
};

struct rect_t
{
    rect_t(struct coords _p1, struct coords _p2): p1(_p1), p2(_p2) {}
    struct coords p1;
    struct coords p2;
    struct coords base_point()
    {
        return coords((p1.x<p2.x)?p1.x:p2.x, (p1.y<p2.y)?p1.y:p2.y);
    }
    struct coords size()
    {
        int w = p1.x - p2.x;
        int h = p1.y - p2.y;
        return coords(w>0?w:-w, h>0?h:-h);
    }
    bool in_rect(struct coords target)
    {
        auto bp = base_point();
        auto sz = size();
        return  (target.x >= bp.x && target.x < bp.x+sz.x) &&
                (target.y >= bp.y && target.y < bp.y+sz.y);
    }
};

enum directions_e {
    DIR_RI = 0,
    DIR_UP = 1,
    DIR_LE = 2,
    DIR_DN = 3,
    DIR_COUNT
};

struct map_tile {
    map_tile(): position(0, 0), wrapped(false), node_id(-1) {
    }

    struct coords position;
    bool wrapped;
    int node_id;
};

class cRegion
{
public:
    cRegion(struct coords target): rect(coords(target.x-1,target.y-1), coords(target.x+2,target.y+2)) {}
    bool in_region(struct coords target) { return rect.in_rect(target); }
    void add(struct coords target);
    rect_t get_rect() {return rect;}
    void set_id(int _id) {id=_id;}
    int get_id() {return id;}
private:
    int id;
    rect_t rect;
//    ListGraph graph;
//    ListGraph::NodeMap<struct map_tile *> graph_tiles;
//    ListGraph::ArcMap<int> costMap;
};

class cMap
{
public:
    cMap(vector<struct coords> map_border_coords, vector<vector<struct coords>> obstacles_list);
    virtual ~cMap();
    void place_boosters(vector<struct coords> boosters_coords);
    void draw(void);
    void try_wrap(struct coords worker, vector<struct coords> manips_rel);
    struct coords find_target(struct coords worker, int region_id);
    int estimate_route(struct coords worker, struct coords target);
    directions_e get_direction(struct coords worker, struct coords target);
    void reset_edges_cost();
    void update_edges_cost(bool is_vertical, vector<struct coords> manips_rel);
    bool is_unwrapped(struct coords target);

    struct coords get_size() {return map_size;}
    class cRegion * in_region(struct coords target);
    class cRegion * get_region(int reg_id);
    void delete_region(int reg_id);
private:
    struct coords map_size;
    map<string, struct map_tile> tiles;
    vector<class cRegion> regions;


    ListGraph graph;
    ListGraph::NodeMap<struct map_tile *> graph_tiles;
    ListGraph::ArcMap<int> costMap;

    bool test_wrappable(struct coords worker, struct coords manip_rel, bool wrap);
};

#endif // CMAP_H
