#include "cWorker.h"

cWorker::cWorker(struct coords pos) :
    cur_position(pos), cur_direction(DIR_RI),
    boost_manip(0), boost_drill(0), boost_drill_timer(0),
    boost_wheel(0), boost_wheel_timer(0), boost_x(0), boost_reset(0)
{
    manipulators.push_back(coords(1, -1));
    manipulators.push_back(coords(1, 0));
    manipulators.push_back(coords(1, 1));
    potential_manipulators.push_back(coords(-1, -1));
    potential_manipulators.push_back(coords(-1, 1));
    potential_manipulators.push_back(coords(0, -5));
    potential_manipulators.push_back(coords(0, 5));
    potential_manipulators.push_back(coords(0, -4));
    potential_manipulators.push_back(coords(0, 4));
    potential_manipulators.push_back(coords(-2, 0));
    potential_manipulators.push_back(coords(-1, 0));
    potential_manipulators.push_back(coords(0, -3));
    potential_manipulators.push_back(coords(0, 3));
    potential_manipulators.push_back(coords(0, -2));
    potential_manipulators.push_back(coords(0, -1));
    potential_manipulators.push_back(coords(0, 2));
    potential_manipulators.push_back(coords(0, 1));
    potential_manipulators.push_back(coords(2, 0));
}

cWorker::~cWorker()
{
    //dtor
}

struct sin_cos_t {
    int sin;
    int cos;
};
static const sin_cos_t sin_cos[] = {
    [ALPHA_0]   = {.sin =  0, .cos =  1},
    [ALPHA_90]  = {.sin =  1, .cos =  0},
    [ALPHA_180] = {.sin =  0, .cos = -1},
    [ALPHA_270] = {.sin = -1, .cos =  0}
};

struct coords cWorker::rotate_manip(angle_e alpha, struct coords manip)
{
    struct sin_cos_t sc = sin_cos[alpha];
    return coords (
        manip.x*sc.cos - manip.y*sc.sin,
        manip.y*sc.cos + manip.x*sc.sin
    );
}

angle_e cWorker::get_rotation_angle(directions_e target_direction)
{
    int steps = (int)target_direction - (int)cur_direction;
    if (steps < 0) steps += 4;
    return (angle_e) steps;
}

vector<struct coords> cWorker::get_manip_rel_pos(directions_e direction)
{
    vector<struct coords> manipulators_coords;
    if(direction == DIR_COUNT)
        direction = cur_direction;
    for(auto manip : manipulators)
    {
        manipulators_coords.push_back(rotate_manip((angle_e) direction, manip));
    }
    return manipulators_coords;
}

bool cWorker::try_attach_manip(struct coords new_manip)
{
    for(auto manip : manipulators)
    {
        if (new_manip == manip)
            return false;
    }
    manipulators.push_back(new_manip);
    return true;
}

void cWorker::take_booster(boosters_e booster)
{
    switch(booster)
    {
    case BOOST_EXT_MANIP:
        boost_manip++;
        break;
    case BOOST_FAST_WHEELS:
        boost_wheel++;
        break;
    case BOOST_DRILL:
        boost_drill++;
        break;
    case BOOST_X:
        boost_x++;
        break;
    case BOOST_RESET:
        boost_reset++;
        break;
    default:
        break;
    }
}

void cWorker::push_action(action_t act)
{
    if(boost_drill_timer > 0) boost_drill_timer--;
    if(boost_wheel_timer > 0) boost_wheel_timer--;
    mine_map->try_wrap(cur_position, get_manip_rel_pos());
    actions.push_back(act);
//    cout << "Do action: " << act.act << " at " << cur_position.tostr() <<" drill time: " << boost_drill_timer << endl;
//    cout << dump_log() << endl;
}


void cWorker::do_move(actions_e act)
{
    coords target(cur_position);
    coords delta(0, 0);
    boosters_e booster;
    switch(act)
    {
    case ACT_MOVE_RI:
        delta.x++;
        break;
    case ACT_MOVE_UP:
        delta.y++;
        break;
    case ACT_MOVE_LE:
        delta.x--;
        break;
    case ACT_MOVE_DN:
        delta.y--;
        break;
    default:
        cout << "Unknown move direction: " << act << endl;
        exit(-1);
    }
    target = target + delta;
    if(!mine_map->is_accessible(target))
    {
        if(!(drill_active() && mine_map->drill_tile(target)))
            return;
    }
    cur_position = target;
// check for booster
    if((booster = mine_map->pick_booster(cur_position)) != BOOST_NONE)
        take_booster(booster);


    if(wheel_active())
    {
        do {
            mine_map->try_wrap(cur_position, get_manip_rel_pos()); //additional wrap
            target = target + delta;
            if(!mine_map->is_accessible(target))
            {
                if(!(drill_active() && mine_map->drill_tile(target)))
                    break; // can't move forward
            }
            cur_position = target;
// check for booster
            if((booster = mine_map->pick_booster(cur_position)) != BOOST_NONE)
                take_booster(booster);
        } while(0);
    }
    push_action(action_t(act));
}

void cWorker::do_rotate_manip(angle_e alpha)
{
    switch(alpha)
    {
    case ALPHA_0:
        return;
    case ALPHA_90: {
        int steps = (int)cur_direction;
        steps++;
        if (steps > 3) steps -= 4;
        cur_direction = (directions_e)steps;
        push_action(action_t(ACT_TURN_CCW));
    }; break;
    case ALPHA_180: {
        int steps = (int)cur_direction;
        steps++;
        steps++;
        if (steps > 3) steps -= 4;
        cur_direction = (directions_e)steps;
        push_action(action_t(ACT_TURN_CCW));
        push_action(action_t(ACT_TURN_CCW));
    }; break;
    case ALPHA_270: {
        int steps = (int)cur_direction;
        steps--;
        if (steps < 0) steps += 4;
        cur_direction = (directions_e)steps;
        push_action(action_t(ACT_TURN_CW));
    }; break;
    default:
        cout << "Unknown angle: " << alpha << endl;
        exit(-1);
    }
}

bool cWorker::do_activate_manip()
{
    if(boost_manip <= 0)
        return false;
    if(potential_manipulators.size() == 0)
        return false;
    auto coords = potential_manipulators.back();
    potential_manipulators.pop_back();
    if(!try_attach_manip(coords))
    {
        cout << "Can't attach manip: " << coords.tostr() << endl;
        return false;
    }
    push_action(action_t(ACT_ATTACH_MANIP, rotate_manip((angle_e) cur_direction, coords)));
    boost_manip --;
    return true;
}
bool cWorker::do_activate_drill()
{
    if(boost_drill <= 0)
        return false;
    push_action(action_t(ACT_ATTACH_DRILL));
    boost_drill --;
    boost_drill_timer = DRILL_DURACTION;
    return true;
}
bool cWorker::do_activate_wheel()
{
    if(boost_wheel <= 0)
        return false;
    push_action(action_t(ACT_ATTACH_FAST));
    boost_wheel --;
    boost_wheel_timer = WHEEL_DURACTION;
    return true;
}

string cWorker::dump_log()
{
    string log;
    for(auto act: actions)
    {
        switch(act.act)
        {
        case ACT_MOVE_RI:
            log += "D";
            break;
        case ACT_MOVE_DN:
            log += "S";
            break;
        case ACT_MOVE_LE:
            log += "A";
            break;
        case ACT_MOVE_UP:
            log += "W";
            break;
        case ACT_TURN_CW:
            log += "E";
            break;
        case ACT_TURN_CCW:
            log += "Q";
            break;
        case ACT_ATTACH_MANIP:
            log += "B" + act.pos.tostr();
            break;
        case ACT_ATTACH_FAST:
            log += "F";
            break;
        case ACT_ATTACH_DRILL:
            log += "L";
            break;
        default:
            cout << "Unknown action: " << act.act << endl;
            exit(-1);
        }
    }
    return log;
}
