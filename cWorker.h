#ifndef CWORKER_H
#define CWORKER_H

#include <vector>

#include "cMap.h"

using namespace std;

enum actions_e {
    ACT_MOVE_RI,        ///D
    ACT_MOVE_DN,        ///S
    ACT_MOVE_LE,        ///A
    ACT_MOVE_UP,        ///W
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

class cWorker
{
public:
    cWorker(struct coords pos);
    virtual ~cWorker();
    const struct coords& get_pos() {return cur_position;}
    vector<struct coords> get_manip_pos();
    void do_action(actions_e act);
    string dump_log();

private:
    struct coords cur_position;
    directions_e cur_direction;
    vector<actions_e> actions;
    vector<struct coords> manipulators;
};

#endif // CWORKER_H
