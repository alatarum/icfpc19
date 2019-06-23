#include "cWorker.h"

cWorker::cWorker(struct coords pos) :
    cur_position(pos), cur_direction(DIR_RI),
    boost_wheels(0), boost_drill(0), boost_x(0), boost_reset(0),
    boost_drill_timer(0)
{
    manipulators.push_back(coords(1, -1));
    manipulators.push_back(coords(1, 0));
    manipulators.push_back(coords(1, 1));
    potential_manipulators.push_back(coords(0, -4));
    potential_manipulators.push_back(coords(0, 4));
    potential_manipulators.push_back(coords(0, -3));
    potential_manipulators.push_back(coords(0, 3));
    potential_manipulators.push_back(coords(0, -3));
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
    case BOOST_EXT_MANIP: {
        auto coords = potential_manipulators.back();
        potential_manipulators.pop_back();
        if(!try_attach_manip(coords))
        {
            cout << "Can't attach manip: " << coords.tostr() << endl;
            return;
        }
        push_action(action_t(ACT_ATTACH_MANIP, rotate_manip((angle_e) cur_direction, coords)));
    }; break;
    case BOOST_FAST_WHEELS:
        boost_wheels++;
        break;
    case BOOST_DRILL:
cout << "drill found" << endl;
        boost_drill++;
        break;
    case BOOST_X:
        boost_x++;
        break;
    case BOOST_RESET:
        boost_reset++;
        break;
    }
}

void cWorker::push_action(action_t act)
{
    if(boost_drill_timer > 0) boost_drill_timer--;
    actions.push_back(act);
//    cout << "Do action: " << act.act << " at " << cur_position.tostr() <<" drill time: " << boost_drill_timer << endl;
//    cout << dump_log() << endl;
}


void cWorker::do_action(actions_e act)
{
    switch(act)
    {
    case ACT_MOVE_RI:
        cur_position.x++;
        break;
    case ACT_MOVE_DN:
        cur_position.y--;
        break;
    case ACT_MOVE_LE:
        cur_position.x--;
        break;
    case ACT_MOVE_UP:
        cur_position.y++;
        break;
    default:
        cout << "Unknown action: " << act << endl;
        exit(-1);
    }
    if(drill_active())
    {
        mine_map->drill_tile(cur_position);
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

bool cWorker::do_activate_drill()
{
    if(boost_drill <= 0)
        return false;
    push_action(action_t(ACT_ATTACH_DRILL));
    boost_drill --;
    boost_drill_timer = DRILL_DURACTION;
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
