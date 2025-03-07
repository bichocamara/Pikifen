/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob utility classes and functions.
 */

#pragma once

#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../core/pathing.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../animation/animation.h"
#include "../area/sector.h"
#include "../mob_type/bouncer_type.h"
#include "../mob_type/bridge_type.h"
#include "../mob_type/converter_type.h"
#include "../mob_type/decoration_type.h"
#include "../mob_type/drop_type.h"
#include "../mob_type/enemy_type.h"
#include "../mob_type/group_task_type.h"
#include "../mob_type/interactable_type.h"
#include "../mob_type/leader_type.h"
#include "../mob_type/pellet_type.h"
#include "../mob_type/pikmin_type.h"
#include "../mob_type/pile_type.h"
#include "../mob_type/resource_type.h"
#include "../mob_type/scale_type.h"
#include "../mob_type/tool_type.h"
#include "../mob_type/track_type.h"
#include "../mob_type/treasure_type.h"
#include "mob_enums.h"


using std::size_t;
using std::vector;

class Mob;

/**
 * @brief Info on a carrying spot around a mob's perimeter.
 */
struct CarrierSpot {

    //--- Members ---
    
    //State.
    CARRY_SPOT_STATE state = CARRY_SPOT_STATE_FREE;
    
    //Relative coordinates of each spot. Cache for performance.
    Point pos;
    
    //Pikmin that is in this spot.
    Mob* pik_ptr = nullptr;
    
    
    //--- Function declarations ---
    
    explicit CarrierSpot(const Point &pos);
    
};


/**
 * @brief Info on how the mob should be carried.
 */
struct CarryInfo {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Generic type of delivery destination.
    CARRY_DESTINATION destination = CARRY_DESTINATION_SHIP;
    
    //Information about each carrier spot.
    vector<CarrierSpot> spot_info;
    
    //Current carrying strength. Cache for performance.
    float cur_carrying_strength = 0.0f;
    
    //Number of carriers, including reserves. Cache for performance.
    size_t cur_n_carriers = 0;
    
    //Is the object moving at the moment?
    bool is_moving = false;
    
    //When the object begins moving, the idea is to carry it to this mob.
    Mob* intended_mob = nullptr;
    
    //When the object begins moving, the idea is to carry it to this point.
    Point intended_point;
    
    //When delivering to an Onion, this is the Pikmin type that will benefit.
    PikminType* intended_pik_type = nullptr;
    
    //True if a destination does exist, false otherwise.
    bool destination_exists = false;
    
    //Is the Pikmin meant to return somewhere after carrying?
    bool must_return = false;
    
    //Location to return to once they finish carrying.
    Point return_point;
    
    //Distance from the return point to stop at.
    float return_dist = 0.0f;
    
    
    //--- Function declarations ---
    
    CarryInfo(Mob* m, const CARRY_DESTINATION destination);
    bool is_empty() const;
    bool is_full() const;
    vector<Hazard*> get_carrier_invulnerabilities() const;
    bool can_fly() const;
    float get_speed() const;
    void rotate_points(float angle);
    
};


/**
 * @brief Info on what point the mob is chasing after.
 */
struct ChaseInfo {

    //--- Members ---
    
    //Current chasing state.
    CHASE_STATE state = CHASE_STATE_STOPPED;
    
    //Flags that control how to chase. Use CHASE_FLAG_*.
    bitmask_8_t flags = 0;
    
    //Chase after these coordinates, relative to the "origin" coordinates.
    Point offset;
    
    //Same as above, but for the Z coordinate.
    float offset_z = 0.0f;
    
    //Pointer to the origin of the coordinates, or nullptr for the world origin.
    Point* orig_coords = nullptr;
    
    //Same as above, but for the Z coordinate.
    float* orig_z = nullptr;
    
    //Distance from the target in which the mob is considered as being there.
    float target_dist = 0.0f;
    
    //Acceleration to apply, in units per second per second.
    float acceleration = 0.0f;
    
    //Current speed to move towards the target at.
    float cur_speed = 0.0f;
    
    //Maximum speed.
    float max_speed = -1.0f;
    
};


/**
 * @brief Info about what mob or point that this mob is circling around,
 * if any.
 */
struct CirclingInfo {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Mob that it is circling.
    Mob* circling_mob = nullptr;
    
    //Point that it is circling, if it's not circling a mob.
    Point circling_point;
    
    //Radius at which to circle around.
    float radius = 0.0f;
    
    //Is it circling clockwise?
    bool clockwise = true;
    
    //Speed at which to move.
    float speed = 0.0f;
    
    //Can the mob move freely, or only forward?
    bool can_free_move = false;
    
    //Angle of the circle to go to.
    float cur_angle = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit CirclingInfo(Mob* m);
    
};


/**
 * @brief Info on a mob that's being delivered to an Onion, ship, etc.
 */
struct DeliveryInfo {

    //--- Members ---
    
    //Animation type.
    DELIVERY_ANIM anim_type = DELIVERY_ANIM_SUCK;
    
    //Ratio of time left in the animation.
    float anim_time_ratio_left = 1.0f;
    
    //Color to make the mob glow with.
    ALLEGRO_COLOR color;
    
    //Intended delivery Pikmin type, in the case of Onions.
    PikminType* intended_pik_type = nullptr;
    
    
    //--- Function declarations ---
    
    DeliveryInfo();
    
};


/**
 * @brief Info on a mob's group.
 *
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct Group {

    //--- Misc. declarations ---
    
    //Ways for Pikmin to follow the leader.
    enum MODE {
    
        //Follow the leader's back.
        MODE_FOLLOW_BACK,
        
        //Casually shuffle with the leader, if needed.
        MODE_SHUFFLE,
        
        //Swarming.
        MODE_SWARM,
        
    };
    
    /**
     * @brief A spot in the group.
     */
    struct GroupSpot {
    
        //--- Members ---
        
        //Position relative to the anchor.
        Point pos;
        
        //Mob in this spot.
        Mob* mob_ptr = nullptr;
        
        
        //--- Function declarations ---
        
        explicit GroupSpot(const Point &p = Point(), Mob* m = nullptr) :
            pos(p), mob_ptr(m) {}
            
    };
    
    
    //--- Members ---
    
    //All group members.
    vector<Mob*> members;
    
    //Information about each spot.
    vector<GroupSpot> spots;
    
    //Radius of the group.
    float radius = 0.0f;
    
    //Absolute position of element 0 of the group (frontmost member).
    Point anchor;
    
    //Angle from the leader to the anchor.
    float anchor_angle = TAU / 2.0f;
    
    //Transformation to apply to the group, like from swarming.
    ALLEGRO_TRANSFORM transform;
    
    //Currently selected standby type.
    SubgroupType* cur_standby_type = nullptr;
    
    //Mode of operation.
    MODE mode = MODE_SHUFFLE;
    
    
    //--- Function declarations ---
    
    explicit Group(Mob* leader_ptr);
    void init_spots(Mob* affected_mob_ptr = nullptr);
    void sort(SubgroupType* leading_type);
    void change_standby_type_if_needed();
    size_t get_amount_by_type(const MobType* type) const;
    Point get_average_member_pos() const;
    vector<Hazard*> get_group_invulnerabilities(
        Mob* include_leader = nullptr
    ) const;
    bool get_next_standby_type(
        bool move_backwards, SubgroupType** new_type
    );
    Point get_spot_offset(size_t spot_idx) const;
    void reassign_spots();
    bool change_standby_type(bool move_backwards);
};


/**
 * @brief Info about how this mob is currently being held by
 * another, if it is.
 */
struct HoldInfo {

    //--- Members ---
    
    //Points to the mob holding the current one, if any.
    Mob* m = nullptr;
    
    //Index of the hitbox the mob is attached to.
    //If INVALID, it's attached to the mob center.
    size_t hitbox_idx = INVALID;
    
    //Ratio of distance from the hitbox/body center. 1 is the full radius.
    float offset_dist = 0.0f;
    
    //Angle the mob makes with the center of the hitbox/body.
    float offset_angle = 0.0f;
    
    //Ratio of distance from the hitbox/body's bottom. 1 is the very top.
    float vertical_dist = 0.0f;
    
    //If true, force the mob to be drawn above the holder?
    bool force_above_holder = false;
    
    //How should the held object rotate?
    HOLD_ROTATION_METHOD rotation_method = HOLD_ROTATION_METHOD_NEVER;
    
    
    //--- Function declarations ---
    
    void clear();
    Point get_final_pos(float* out_z) const;
    
};


class Bouncer;
class Bridge;
class Converter;
class Decoration;
class Drop;
class Enemy;
class GroupTask;
class Interactable;
class Leader;
class Onion;
class Pellet;
class Pikmin;
class Pile;
class Resource;
class Scale;
class Ship;
class Tool;
class Track;
class Treasure;

class OnionType;
class ShipType;

/**
 * @brief Lists of all mobs in the area.
 */
struct MobLists {

    //--- Members ---
    
    //All mobs in the area.
    vector<Mob*> all;
    
    //Bouncers.
    vector<Bouncer*> bouncers;
    
    //Bridges.
    vector<Bridge*> bridges;
    
    //Converters.
    vector<Converter*> converters;
    
    //Decorations.
    vector<Decoration*> decorations;
    
    //Drops.
    vector<Drop*> drops;
    
    //Enemies.
    vector<Enemy*> enemies;
    
    //Group tasks.
    vector<GroupTask*> group_tasks;
    
    //Interactables.
    vector<Interactable*> interactables;
    
    //Leaders.
    vector<Leader*> leaders;
    
    //Onions.
    vector<Onion*> onions;
    
    //Pellets.
    vector<Pellet*> pellets;
    
    //Pikmin.
    vector<Pikmin*> pikmin_list;
    
    //Piles.
    vector<Pile*> piles;
    
    //Resources.
    vector<Resource*> resources;
    
    //Mobs that can be walked on top of. Cache for performance.
    vector<Mob*> walkables;
    
    //Scales.
    vector<Scale*> scales;
    
    //Ships.
    vector<Ship*> ships;
    
    //Tools.
    vector<Tool*> tools;
    
    //Tracks.
    vector<Track*> tracks;
    
    //Treasures.
    vector<Treasure*> treasures;
    
};


/**
 * @brief Lists of all mob types.
 */
struct MobTypeLists {

    //--- Members ---
    
    //Bouncer types.
    map<string, BouncerType*> bouncer;
    
    //Bridge types.
    map<string, BridgeType*> bridge;
    
    //Converter types.
    map<string, ConverterType*> converter;
    
    //Custom mob types.
    map<string, MobType*> custom;
    
    //Decoration types.
    map<string, DecorationType*> decoration;
    
    //Drop types.
    map<string, DropType*> drop;
    
    //Enemy types.
    map<string, EnemyType*> enemy;
    
    //Group task types.
    map<string, GroupTaskType*> group_task;
    
    //Interactable types.
    map<string, InteractableType*> interactable;
    
    //Leader types.
    map<string, LeaderType*> leader;
    
    //Onion types.
    map<string, OnionType*> onion;
    
    //Pellet types.
    map<string, PelletType*> pellet;
    
    //Pikmin types.
    map<string, PikminType*> pikmin;
    
    //Pile types.
    map<string, PileType*> pile;
    
    //Resource types.
    map<string, ResourceType*> resource;
    
    //Scale types.
    map<string, ScaleType*> scale;
    
    //Ship types.
    map<string, ShipType*> ship;
    
    //Tool types.
    map<string, ToolType*> tool;
    
    //Track types.
    map<string, TrackType*> track;
    
    //Treasure types.
    map<string, TreasureType*> treasure;
    
};


/**
 * @brief Info about this mob's parent, if any.
 */
struct Parent {

    //--- Members ---
    
    //Mob serving as the parent.
    Mob* m = nullptr;
    
    //Should the child handle damage?
    bool handle_damage = false;
    
    //Should the child relay damage to the parent?
    bool relay_damage = false;
    
    //Should the child handle status effects?
    bool handle_statuses = false;
    
    //Should the child relay status effects to the parent?
    bool relay_statuses = false;
    
    //Should the child handle script events?
    bool handle_events = false;
    
    //Should the child relay script events to the parent?
    bool relay_events = false;
    
    //Animation used for the limb connecting child and parent.
    AnimationInstance limb_anim;
    
    //Thickness of the limb.
    float limb_thickness = 32.0f;
    
    //Body part of the parent to link the limb to.
    size_t limb_parent_body_part = INVALID;
    
    //Offset from the parent body part to link the limb at.
    float limb_parent_offset = 0.0f;
    
    //Body part of the child to link the limb to.
    size_t limb_child_body_part = INVALID;
    
    //Offset from the child body part to link the limb at.
    float limb_child_offset = 0.0f;
    
    //Method by which the limb should be drawn.
    LIMB_DRAW_METHOD limb_draw_method = LIMB_DRAW_METHOD_ABOVE_CHILD;
    
    
    //--- Function declarations ---
    
    explicit Parent(Mob* m);
    
};


/**
 * @brief Info on how to travel through the path graph that
 * the mob currently intends to travel.
 */
struct Path {

    //--- Members ---
    
    //Mob that this struct belongs to.
    Mob* m = nullptr;
    
    //Path to take the mob to while being carried.
    vector<PathStop*> path;
    
    //Index of the current stop in the projected carrying path.
    size_t cur_path_stop_idx = 0;
    
    //Result of the path calculation.
    PATH_RESULT result = PATH_RESULT_NOT_CALCULATED;
    
    //Is the way forward currently blocked? If so, why?
    PATH_BLOCK_REASON block_reason = PATH_BLOCK_REASON_NONE;
    
    //Settings about how the path should be followed.
    PathFollowSettings settings;
    
    
    //--- Function declarations ---
    
    Path(
        Mob* m,
        const PathFollowSettings &settings
    );
    bool check_blockage(PATH_BLOCK_REASON* out_reason = nullptr);
    
};


/**
 * @brief Info that a mob type may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct PikminNestType {

    //--- Members ---
    
    //Pikmin types it can manage.
    vector<PikminType*> pik_types;
    
    //Body parts that represent legs -- pairs of hole + foot.
    vector<string> leg_body_parts;
    
    //Speed at which Pikmin enter the nest.
    float pikmin_enter_speed = 0.7f;
    
    //Speed at which Pikmin exit the nest.
    float pikmin_exit_speed = 2.0f;
    
    
    //--- Function declarations ---
    
    void load_properties(DataNode* file);
    
};


/**
 * @brief Info that a mob may have about how to nest Pikmin inside,
 * like an Onion or a ship.
 */
struct PikminNest {

    public:
    
    //--- Members ---
    
    //Pointer to the nest mob responsible.
    Mob* m_ptr = nullptr;
    
    //Pointer to the type of nest.
    PikminNestType* nest_type = nullptr;
    
    //How many Pikmin are inside, per type, per maturity.
    vector<vector<size_t> > pikmin_inside;
    
    //How many Pikmin are queued up to be called out, of each type.
    vector<size_t> call_queue;
    
    //Which leader is calling the Pikmin over?
    Leader* calling_leader = nullptr;
    
    //Time left until it can eject the next Pikmin in the call queue.
    float next_call_time = 0.0f;
    
    
    //--- Function declarations ---
    
    PikminNest(Mob* m_ptr, PikminNestType* type);
    bool call_pikmin(Mob* m_ptr, size_t type_idx);
    size_t get_amount_by_type(const PikminType* type);
    void read_script_vars(const ScriptVarReader &svr);
    void request_pikmin(
        size_t type_idx, size_t amount, Leader* l_ptr
    );
    void store_pikmin(Pikmin* p_ptr);
    void tick(float delta_t);
    
};


/**
 * @brief Info about the track mob that a mob is currently
 * riding. Includes things like current progress.
 */
struct TrackRideInfo {

    //--- Members ---
    
    //Pointer to the track mob.
    Mob* m = nullptr;
    
    //List of checkpoints (body part indexes) to cross.
    vector<size_t> checkpoints;
    
    //Current checkpoint of the track. This is the last checkpoint crossed.
    size_t cur_cp_idx = 0;
    
    //Progress within the current checkpoint. 0 means at the checkpoint.
    //1 means it's at the next checkpoint.
    float cur_cp_progress = 0.0f;
    
    //Speed to ride at, in ratio per second.
    float ride_speed = 0.0f;
    
    
    //--- Function declarations ---
    
    TrackRideInfo(
        Mob* m, const vector<size_t> &checkpoints, float speed
    );
    
};


float calculate_mob_physical_span(
    float radius, float anim_hitbox_span,
    const Point &rectangular_dim
);
Mob* create_mob(
    MobCategory* category, const Point &pos, MobType* type,
    float angle, const string &vars,
    std::function<void(Mob*)> code_after_creation = nullptr,
    size_t first_state_override = INVALID
);
void delete_mob(Mob* m, bool complete_destruction = false);
string get_error_message_mob_info(Mob* m);
vector<Hazard*> get_mob_type_list_invulnerabilities(
    const unordered_set<MobType*> &types
);
MobType::SpawnInfo* get_spawn_info_from_child_info(
    MobType* type, const MobType::Child* child_info
);
bool is_mob_in_reach(
    MobType::Reach* reach_t_ptr, const Distance &dist_between, float angle_diff
);
MOB_TARGET_FLAG string_to_mob_target_type(const string &type_str);
MOB_TEAM string_to_team_nr(const string &team_str);
