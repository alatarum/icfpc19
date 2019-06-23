#include "cStrategy.h"

static const int ROTATION_COST = 5;
static const int DRILL_THRESHOLD = 15;

cStrategy::cStrategy(cMap *mine_map, cWorker *worker):
    mine_map(mine_map), worker(worker), cur_target(worker->get_pos()), cur_region(REGION_NOT_SELECTED)
{
    worker->mine_map = mine_map;
}

cStrategy::~cStrategy()
{
    //dtor
}

bool cStrategy::step()
{
    struct coords cur_pos(worker->get_pos());
    if(cur_region == DRILL_ACTIVATED)
    {
        if(!worker->drill_active())
        {
            cur_region = REGION_NOT_SELECTED;
            cur_target = cur_pos;
        }
    }

//0. Wrap worker position (required by algorithm).
    mine_map->try_wrap(cur_pos, vector<struct coords> ());
//1. find target
    class cRegion *region = nullptr;
    if(cur_region >= 0)
    {
        region = mine_map->get_region(cur_region);
        if(region == nullptr)
        cur_region = REGION_NOT_SELECTED;
    }
    if((cur_region == REGION_NOT_SELECTED))
    {
        region = mine_map->in_region(cur_pos);
        if(region != nullptr)
            cur_region = region->get_id();
    }

    struct coords region_size = (region != nullptr)?region->get_rect().size():mine_map->get_size();
    if(!mine_map->is_unwrapped(cur_target))
    {
        mine_map->reset_edges_cost(cur_region);
        cur_target = mine_map->find_target(cur_pos, cur_region);
        if(cur_target == cur_pos)
        {
            if(cur_region < 0)
            {
                cout << "Done!" << endl;
                return false;
            }
            mine_map->delete_region(cur_region);
            if(cur_region != DRILL_ACTIVATED)
                cur_region = REGION_NOT_SELECTED;
            return true;
        }
    }

//2. plan route
    int min_cost = -1;
    directions_e manip_dir;
    for(int tmp_dir = DIR_RI; tmp_dir <= DIR_DN; tmp_dir++)
    {
        mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos((directions_e)tmp_dir), cur_region);
        int cost = mine_map->estimate_route(cur_pos, cur_target, cur_region);
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
        if(((min_cost < 0) || (cost < min_cost)) && (cost >= 0))
        {
            min_cost = cost;
            manip_dir = (directions_e)tmp_dir;
        }
    }

    if((worker->have_drill()) && (!worker->drill_active()))
    {
        mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos(DIR_RI), DRILL_ACTIVATED);
        int cost = mine_map->estimate_route(cur_pos, cur_target, DRILL_ACTIVATED);
        if(min_cost - cost > DRILL_THRESHOLD)
        {
            manip_dir = worker->get_dir();
            if(worker->do_activate_drill())
                cur_region = DRILL_ACTIVATED;
        }
    }

    mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos(manip_dir), cur_region);
    directions_e move_dir = mine_map->get_direction(cur_pos, cur_target, cur_region);
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
        mine_map->delete_region(cur_region);
        cur_region = -1;
        return true;
    }
    worker->do_move(action);
//5. check for booster
    boosters_e booster = mine_map->pick_booster(worker->get_pos());
    {
        if(booster != BOOST_NONE)
        {
            coords tmp_pos = worker->get_pos();
            worker->take_booster(booster);
        }
    }
    return true;
}
