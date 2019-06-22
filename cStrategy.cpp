#include "cStrategy.h"

static const int ROTATION_COST = 3;

cStrategy::cStrategy(cMap *mine_map, cWorker *worker):
    mine_map(mine_map), worker(worker), cur_target(worker->get_pos())
{
}

cStrategy::~cStrategy()
{
    //dtor
}

bool cStrategy::step()
{
    mine_map->try_wrap(worker->get_pos(), vector<struct coords> ());
//1. find target
    struct coords region_size = mine_map->get_size();
    if(!mine_map->is_unwrapped(cur_target))
    {
        mine_map->reset_edges_cost();
        cur_target = mine_map->find_target(worker->get_pos(), rect_t(coords(0,0), region_size));
        if(cur_target == worker->get_pos())
        {
            cout << "Done!" << endl;
            return false;
        }
    }

//2. plan route
    int min_cost = -1;
    directions_e manip_dir;
    for(int tmp_dir = DIR_RI; tmp_dir <= DIR_DN; tmp_dir++)
    {
        mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos((directions_e)tmp_dir));
        int cost = mine_map->estimate_route(worker->get_pos(), cur_target);
        auto angle = worker->get_rotation_angle((directions_e)tmp_dir);
        switch(angle)
        {
        case ALPHA_0: break;
        case ALPHA_90:
        case ALPHA_270:
            cost += ROTATION_COST;
            break;
        case ALPHA_180:
            cost += ROTATION_COST*2;
            break;
        }
        if((min_cost < 0) || (cost < min_cost))
        {
            min_cost = cost;
            manip_dir = (directions_e)tmp_dir;
        }
    }

    mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos(manip_dir));
    directions_e move_dir = mine_map->get_direction(worker->get_pos(), cur_target);
//3. wrap tiles
    mine_map->try_wrap(worker->get_pos(), worker->get_manip_rel_pos());

//4. move worker
    worker->do_rotate_manip(worker->get_rotation_angle(manip_dir));

    actions_e action;
    switch (move_dir)
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
        cout << "Unknown direction: " << move_dir << endl;
        exit(-1);
    }
    worker->do_action(action);
    return true;
}
