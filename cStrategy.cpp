#include "cStrategy.h"

static const int ROTATION_COST = 2;

cStrategy::cStrategy(cMap *mine_map, cWorker *worker):
    mine_map(mine_map), worker(worker), cur_target(worker->get_pos()), cur_region(-1)
{
}

cStrategy::~cStrategy()
{
    //dtor
}

bool cStrategy::step()
{
    struct coords cur_pos(worker->get_pos());
//0. Wrap worker position (required by algorithm).
    mine_map->try_wrap(cur_pos, vector<struct coords> ());
//1. find target
    class cRegion *region;
    region = mine_map->get_region(cur_region);
    if(region == nullptr)
        region = mine_map->in_region(cur_pos);
    if(region != nullptr)
        cur_region = region->get_id();
    struct coords region_size = (region != nullptr)?region->get_rect().size():mine_map->get_size();
    if(!mine_map->is_unwrapped(cur_target))
    {
        mine_map->reset_edges_cost();
cout << "In region: " << cur_region << endl;
        cur_target = mine_map->find_target(cur_pos, cur_region);
        if(cur_target == cur_pos)
        {
            if(cur_region < 0)
            {
                cout << "Done!" << endl;
                return false;
            }
            mine_map->delete_region(cur_region);
            cur_region = -1;
            return true;
        }
    }

//2. plan route
    int min_cost = -1;
    directions_e manip_dir;
    for(int tmp_dir = DIR_RI; tmp_dir <= DIR_DN; tmp_dir++)
    {
        mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos((directions_e)tmp_dir));
        int cost = mine_map->estimate_route(cur_pos, cur_target);
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
    directions_e move_dir = mine_map->get_direction(cur_pos, cur_target);
//3. wrap tiles
    mine_map->try_wrap(cur_pos, worker->get_manip_rel_pos());

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
