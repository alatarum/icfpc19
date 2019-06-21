#ifndef CMAP_H
#define CMAP_H

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

enum boosters_e {
    BOOST_NONE,
    BOOST_EXT_MANIP,
    BOOST_FAST_WHEELS,
    BOOST_DRILL,
    BOOST_X,
    BOOST_COUNT
};

const string booster_name[] = {
    [BOOST_NONE] = "NONE",
    [BOOST_EXT_MANIP] = "B",
    [BOOST_FAST_WHEELS] = "F",
    [BOOST_DRILL] = "L",
    [BOOST_X] = "X"
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

    int x;
    int y;
    boosters_e booster;
};

enum directions_e {
    DIR_RI = 0,
    DIR_UP,
    DIR_LE,
    DIR_DN,
    DIR_COUNT
};

struct map_tile {
    map_tile(): position(0, 0), visited(false) {
        links[DIR_RI] = nullptr;
        links[DIR_UP] = nullptr;
        links[DIR_LE] = nullptr;
        links[DIR_DN] = nullptr;
    }

    struct coords position;
    bool visited;
    map<directions_e, struct map_tile *> links;
//    struct map_tile *teleport;
};

class cMap
{
public:
        cMap(vector<struct coords> map_border_coords, vector<vector<struct coords>> obstacles_list);
        virtual ~cMap();
        void place_boosters(vector<struct coords> boosters_coords);
        void draw(void);

private:
    map<string, struct map_tile> tiles;
    struct coords map_size;
};

#endif // CMAP_H
