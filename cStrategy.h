#ifndef CSTRATEGY_H
#define CSTRATEGY_H

#include "cMap.h"
#include "cWorker.h"

class cStrategy
{
public:
    cStrategy(cMap *mine_map, cWorker *worker);
    virtual ~cStrategy();
    bool step();

    void set_use_boosters(bool manip, bool drill, bool wheel) {
        use_manip = manip; use_drill = drill; use_wheel = wheel;
    }
    void set_verbose(bool verb) {verbose = verb;}
private:
    cMap *mine_map;
    cWorker *worker;
    struct coords cur_target;
    int cur_region;
    int target_live;

    bool use_manip;
    bool use_drill;
    bool use_wheel;
    bool verbose;
};

#endif // CSTRATEGY_H
