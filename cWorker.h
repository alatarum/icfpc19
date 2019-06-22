#ifndef CWORKER_H
#define CWORKER_H

#include <vector>

#include "cMap.h"

using namespace std;

enum actions_e {
    ACT_MOVE_RI,        ///D
    ACT_MOVE_UP,        ///W
    ACT_MOVE_LE,        ///A
    ACT_MOVE_DN,        ///S
    ACT_DUP,            ///Z
    ACT_TURN_CW,        ///E
    ACT_TURN_CCW,       ///Q
    ACT_ATTACH_MANIP,   ///B(dx; dy)
    ACT_ATTACH_FAST,    ///F
    ACT_ATTACH_DRILL,   ///L
    ACT_INST_BEACON,    ///R
    ACT_TELEPORT,       ///T(x; y)
    ACT_COUNT
};

enum angle_e {
    ALPHA_0   = 0,
    ALPHA_90  = 1,
    ALPHA_180 = 2,
    ALPHA_270 = 3
};

class cWorker
{
public:
    cWorker(struct coords pos);
    virtual ~cWorker();
    const struct coords& get_pos() {return cur_position;}
    vector<struct coords> get_manip_rel_pos(directions_e direction = DIR_COUNT);
    void do_action(actions_e act);
    void do_rotate_manip(angle_e alpha);


    angle_e get_rotation_angle(directions_e target_direction);
    string dump_log();

private:
    struct coords cur_position;
    directions_e cur_direction;
    vector<actions_e> actions;
    vector<struct coords> manipulators;

    struct coords rotate_manip(angle_e alpha, struct coords manip);
};

#endif // CWORKER_H
