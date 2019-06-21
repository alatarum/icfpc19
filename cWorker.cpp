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

vector<struct coords> cWorker::get_manip_pos()
{
    vector<struct coords> manipulators_coords;
    //TODO: rotation
    for(auto manip : manipulators)
    {
        manipulators_coords.push_back(coords(
                cur_position.x + manip.x,
                cur_position.y + manip.y
        ));
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
        default:
            cout << "Unknown action: " << act << endl;
            exit(-1);
        }
    }
    return log;
}
