/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#pragma once

#include <vector>

#include "../../util/general_utils.h"
#include "../mob_type/leader_type.h"
#include "mob.h"


class Pikmin;

using std::size_t;


namespace LEADER {
extern const float AUTO_THROW_FASTEST_INTERVAL;
extern const float AUTO_THROW_RAMP_TIME;
extern const float AUTO_THROW_SLOWEST_INTERVAL;
extern const float BORED_ANIM_MAX_DELAY;
extern const float BORED_ANIM_MIN_DELAY;
extern const float DISMISS_ANGLE_RANGE;
extern const float DISMISS_MEMBER_SIZE_MULTIPLIER;
extern const size_t DISMISS_PARTICLE_AMOUNT;
extern const float DISMISS_PARTICLE_ALPHA;
extern const float DISMISS_PARTICLE_FRICTION;
extern const float DISMISS_PARTICLE_MIN_DURATION;
extern const float DISMISS_PARTICLE_MIN_SPEED;
extern const float DISMISS_PARTICLE_MAX_DURATION;
extern const float DISMISS_PARTICLE_MAX_SPEED;
extern const float DISMISS_PARTICLE_SIZE;
extern const float DISMISS_SUBGROUP_DISTANCE;
extern const float HEALTH_CAUTION_RATIO;
extern const float HEALTH_CAUTION_RING_DURATION;
extern const float HELD_GROUP_MEMBER_ANGLE;
extern const float HELD_GROUP_MEMBER_H_DIST;
extern const float HELD_GROUP_MEMBER_V_DIST;
extern const float INVULN_PERIOD_KB;
extern const float INVULN_PERIOD_NORMAL;
extern const float SWARM_ARROW_INTERVAL;
extern const float SWARM_PARTICLE_ALPHA;
extern const float SWARM_PARTICLE_ANGLE_DEVIATION;
extern const float SWARM_PARTICLE_FRICTION;
extern const float SWARM_PARTICLE_MAX_DURATION;
extern const float SWARM_PARTICLE_MIN_DURATION;
extern const float SWARM_PARTICLE_SIZE;
extern const float SWARM_PARTICLE_SPEED_DEVIATION;
extern const float SWARM_PARTICLE_SPEED_MULT;
extern const float THROW_COOLDOWN_DURATION;
extern const float THROW_PREVIEW_FADE_IN_RATIO;
extern const float THROW_PREVIEW_FADE_OUT_RATIO;
extern const float THROW_PREVIEW_MIN_THICKNESS;
extern const float THROW_PREVIEW_DEF_MAX_THICKNESS;
}


/**
 * @brief A leader controls Pikmin, and
 * is controlled by the player.
 */
class Leader : public Mob {

public:

    //--- Members ---
    
    //What type of leader it is.
    LeaderType* lea_type = nullptr;
    
    //Is it active? i.e. being controlled by a player.
    bool active = false;
    
    //Is it currently auto-plucking?
    bool auto_plucking = false;
    
    //Pikmin it wants to pluck.
    Pikmin* pluck_target = nullptr;
    
    //Has the player asked for the auto-plucking to stop?
    bool queued_pluck_cancel = false;
    
    //Mid Go Here.
    bool mid_go_here = false;
    
    //Is the leader currently in the walking animation?
    bool is_in_walking_anim = false;
    
    //Is the leader currently turning in place, in the active state?
    bool is_active_turning = false;
    
    //Is the leader currently walking, in the active state?
    bool is_active_walking = false;
    
    //Time until the next arrow in the list of swarm arrows appears.
    Timer swarm_next_arrow_timer = Timer(LEADER::SWARM_ARROW_INTERVAL);
    
    //List of swarm mode arrows.
    vector<float> swarm_arrows;
    
    //Time left before the leader can throw again.
    float throw_cooldown = 0.0f;
    
    //Whether or not a throw has been queued to be pulled off.
    bool throw_queued = false;
    
    //Auto-repeater for auto-throwing.
    AutoRepeater auto_throw_repeater;
    
    //Provided there's a throw, this is the mob to throw.
    Mob* throwee = nullptr;
    
    //Provided there's a throw, this is the angle.
    float throwee_angle = 0.0f;
    
    //Provided there's a throw, this is the max Z.
    float throwee_max_z = 0.0f;
    
    //Provided there's a throw, this is the horizontal speed.
    Point throwee_speed;
    
    //Provided there's a throw, this is the vertical speed.
    float throwee_speed_z = 0.0f;
    
    //Provided there's a throw, this indicates whether it's low enough to reach.
    bool throwee_can_reach = false;
    
    //How much the health wheel is filled. Gradually moves to the target amount.
    float health_wheel_visible_ratio = 1.0f;
    
    //Timer for the animation of the health wheel's caution ring.
    float health_wheel_caution_timer = 0.0f;
    
    //Is it currently doing some boredom-related animation?
    bool in_bored_animation = false;
    
    //Temporary variable. Hacky, but effective. Only use within the same state!
    size_t temp_i = 0;
    
    
    //--- Function declarations ---
    
    Leader(const Point &pos, LeaderType* type, float angle);
    bool check_throw_ok() const;
    bool can_grab_group_member(Mob* m) const;
    void dismiss();
    bool order_pikmin_to_onion(
        const PikminType* type, PikminNest* n_ptr, size_t amount
    );
    void queue_throw();
    void signal_swarm_start() const;
    void signal_swarm_end() const;
    void start_auto_throwing();
    void start_throw_trail();
    void start_whistling();
    void stop_auto_throwing();
    void stop_whistling();
    void swap_held_pikmin(Mob* new_pik);
    void update_throw_variables();
    bool can_receive_status(StatusType* s) const override;
    void get_group_spot_info(
        Point* out_spot, float* out_dist
    ) const override;
    void draw_mob() override;
    
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(float delta_t) override;
    
private:

    //--- Members ---
    
    //Sound effect source ID of the whistle, or 0 for none.
    size_t whistle_sound_source_id = 0;
    
    //Returns how many rows are needed for all members' dismissal.
    size_t get_dismiss_rows(size_t n_members) const;
    
};


void change_to_next_leader(
    bool forward, bool force_success, bool keep_idx
);
bool grab_closest_group_member();
