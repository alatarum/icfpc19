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

struct action_t {
    enum actions_e act;
    struct coords pos;

    action_t(): act(ACT_COUNT), pos(0, 0) {}
    action_t(actions_e _act): act(_act), pos(0, 0) {}
    action_t(actions_e _act, struct coords _pos): act(_act), pos(_pos) {}
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
    directions_e get_dir() {return cur_direction;}
    vector<struct coords> get_manip_rel_pos(directions_e direction = DIR_COUNT);
    void take_booster(boosters_e booster);
    bool have_manip()   {return (boost_manip > 0)? true: false;}
    bool drill_active() {return (boost_drill_timer > 0)? true: false;}
    bool have_drill()   {return (boost_drill > 0)? true: false;}
    bool wheel_active() {return (boost_wheel_timer > 0)? true: false;}
    bool have_wheel()   {return (boost_wheel > 0)? true: false;}

    void do_move(actions_e act);
    void do_rotate_manip(angle_e alpha);
    bool do_activate_manip();
    bool do_activate_drill();
    bool do_activate_wheel();

    angle_e get_rotation_angle(directions_e target_direction);
    string dump_log();

    cMap *mine_map;
private:
    struct coords cur_position;
    directions_e cur_direction;
    vector<action_t> actions;
    vector<struct coords> manipulators;
    vector<struct coords> potential_manipulators;
    int boost_manip;
    int boost_drill;
    int boost_drill_timer;
    int boost_wheel;
    int boost_wheel_timer;
    int boost_x;
    int boost_reset;

    struct coords rotate_manip(angle_e alpha, struct coords manip);
    bool try_attach_manip(struct coords new_manip);
    void push_action(action_t act);
};

#endif // CWORKER_H
