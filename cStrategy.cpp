#include "cStrategy.h"

cStrategy::cStrategy(cMap *mine_map, cWorker *worker):
    mine_map(mine_map), worker(worker)
{
    //ctor
}

cStrategy::~cStrategy()
{
    //dtor
}

bool cStrategy::step()
{
//1. wrap tiles
    mine_map->try_wrap(worker->get_pos(), worker->get_manip_pos());
//2. find target
    struct coords target = mine_map->find_target(worker->get_pos(), worker->get_manip_pos());
    if(target == worker->get_pos())
    {
        cout << "Done!" << endl;
        return false;
    }
//3. move worker
    directions_e dir = mine_map->get_direction(worker->get_pos(), target);
    actions_e action;
    switch (dir)
    {
    case DIR_RI:
        action = ACT_MOVE_RI;
        break;
    case DIR_DN:
        action = ACT_MOVE_DN;
        break;
    case DIR_LE:
        action = ACT_MOVE_LE;
        break;
    case DIR_UP:
        action = ACT_MOVE_UP;
        break;
    default:
        cout << "Unknown direction: " << dir << endl;
        exit(-1);
    }
    worker->do_action(action);
    return true;
}
