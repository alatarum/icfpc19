#include "cStrategy.h"

static const int ROTATION_COST = 5;
static const int DRILL_THRESHOLD = 15;

cStrategy::cStrategy(cMap *mine_map, cWorker *worker):
    mine_map(mine_map), worker(worker), cur_target(worker->get_pos()), cur_region(REGION_NOT_SELECTED),
    target_live(0), use_manip(true), use_drill(true), use_wheel(true), verbose(false)

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
    int boosters = 0;
    static bool was_drill = false;
    if(worker->drill_active()) boosters |= BOOSTF_DRILL;
    if(worker->wheel_active()) boosters |= BOOSTF_FAST_WHEELS;

    if((was_drill) && (!(boosters & BOOSTF_DRILL)))
    {
        cur_region = REGION_NOT_SELECTED;
        cur_target = cur_pos;
    }
    was_drill = boosters & BOOSTF_DRILL;

    if(boosters & BOOSTF_FAST_WHEELS)
    {
        int dx = cur_pos.x-cur_target.x;
        int dy = cur_pos.y-cur_target.y;
        if(((dx*dx + dy*dy) < 4) && !(cur_pos == cur_target))
        {
            cur_target = cur_pos;
        }
    }

    target_live++;
    if(target_live > 50)
    {
        target_live = 0;
        cur_region = REGION_NOT_SELECTED;
        cur_target = cur_pos;
    }

//1. find target
    class cRegion *region = nullptr;
    region = mine_map->get_region(cur_region);
    if(region == nullptr)
        cur_region = REGION_NOT_SELECTED;
    if((cur_region == REGION_NOT_SELECTED))
    {
        region = mine_map->in_region(cur_pos);
        if(region != nullptr)
            cur_region = region->get_id();
    }
    struct coords region_size = (region != nullptr)?region->get_rect().size():mine_map->get_size();

    if(!mine_map->is_unwrapped(cur_target))
    {
        target_live = 0;
        mine_map->reset_edges_cost(cur_region, boosters);
        cur_target = mine_map->find_target(cur_pos, cur_region, boosters);
        if(cur_target == cur_pos)
        {
            if(cur_region != REGION_NOT_SELECTED)
            {
                mine_map->delete_region(cur_region);
                cur_region = REGION_NOT_SELECTED;
                return true;
            } else {
                cout << "Done!" << endl;
                return false;
            }
        }
    }

//2. plan route
    int min_cost = -1;
    directions_e manip_dir = worker->get_dir();
    for(int tmp_dir = DIR_RI; tmp_dir <= DIR_DN; tmp_dir++)
    {
        mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos((directions_e)tmp_dir), cur_region, boosters);
        int cost = mine_map->estimate_route(cur_pos, cur_target, cur_region, boosters);
        auto angle = worker->get_rotation_angle((directions_e)tmp_dir);
        int rotation_cost = 0;
        switch(angle)
        {
        case ALPHA_0: break;
        case ALPHA_90:
        case ALPHA_270:
            rotation_cost = ROTATION_COST;
            break;
        case ALPHA_180:
            rotation_cost = ROTATION_COST*2;
            break;
        }
        cost += rotation_cost;
        if(((min_cost < 0) || (cost < min_cost)) && (cost >= 0))
        {
            min_cost = cost;
            manip_dir = (directions_e)tmp_dir;
        }
    }

    if(use_drill)
    {
        if((worker->have_drill()) && (!(boosters & BOOSTF_DRILL)))
        {
            mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos(DIR_RI), cur_region, boosters | BOOSTF_DRILL);
            int cost = mine_map->estimate_route(cur_pos, cur_target, cur_region, boosters | BOOSTF_DRILL);
            if(min_cost - cost > DRILL_THRESHOLD)
            {
                manip_dir = worker->get_dir();
                worker->do_activate_drill();
            }
        }
    }
    if(use_wheel)
    {
        if((worker->have_wheel()) && (!worker->wheel_active()))
            worker->do_activate_wheel();
    }
    if(use_manip)
    {
        if(worker->have_manip())
            worker->do_activate_manip();
    }
    boosters = 0;
    if(worker->drill_active()) boosters |= BOOSTF_DRILL;
    if(worker->wheel_active()) boosters |= BOOSTF_FAST_WHEELS;

    mine_map->update_edges_cost(region_size.y > region_size.x, worker->get_manip_rel_pos(manip_dir), cur_region, boosters);
    directions_e move_dir = mine_map->get_direction(cur_pos, cur_target, cur_region, boosters);

    if(verbose)
        mine_map->draw(cur_pos, worker->get_manip_rel_pos(), cur_target);

//3. move worker
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
    cur_pos = worker->get_pos();
    return true;
}
