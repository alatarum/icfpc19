#include "cWorker.h"

cWorker::cWorker(struct coords pos) :
    cur_position(pos), cur_direction(DIR_RI)
{
    manipulators.push_back(coords(1, -1));
    manipulators.push_back(coords(1, 0));
    manipulators.push_back(coords(1, 1));
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
    actions.push_back(act);
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
        actions.push_back(ACT_TURN_CCW);
    }; break;
    case ALPHA_180: {
        int steps = (int)cur_direction;
        steps++;
        steps++;
        if (steps > 3) steps -= 4;
        cur_direction = (directions_e)steps;
        actions.push_back(ACT_TURN_CCW);
        actions.push_back(ACT_TURN_CCW);
    }; break;
    case ALPHA_270: {
        int steps = (int)cur_direction;
        steps--;
        if (steps < 0) steps += 4;
        cur_direction = (directions_e)steps;
        actions.push_back(ACT_TURN_CW);
    }; break;
    default:
        cout << "Unknown angle: " << alpha << endl;
        exit(-1);
    }
}

string cWorker::dump_log()
{
    string log;
    for(auto act: actions)
    {
        switch(act)
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
        default:
            cout << "Unknown action: " << act << endl;
            exit(-1);
        }
    }
    return log;
}
