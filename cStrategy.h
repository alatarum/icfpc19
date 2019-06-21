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

private:
    cMap *mine_map;
    cWorker *worker;
};

#endif // CSTRATEGY_H
