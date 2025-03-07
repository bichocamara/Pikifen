/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob script action classes and
 * related functions.
 */


#pragma once

#include "mob_script.h"


//Types of script action.
enum MOB_ACTION {

    //Unknown.
    MOB_ACTION_UNKNOWN,
    
    //Add health.
    MOB_ACTION_ADD_HEALTH,
    
    //Plan something. Used for arachnorbs.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC,
    
    //Perform a numeric calculation.
    MOB_ACTION_CALCULATE,
    
    //Delete mob.
    MOB_ACTION_DELETE,
    
    //Drain liquid.
    MOB_ACTION_DRAIN_LIQUID,
    
    //Conditional "else" marker.
    MOB_ACTION_ELSE,
    
    //Conditional "end if" marker.
    MOB_ACTION_END_IF,
    
    //Finish the death procedure.
    MOB_ACTION_FINISH_DYING,
    
    //Focus on another mob.
    MOB_ACTION_FOCUS,
    
    //Follow a path randomly.
    MOB_ACTION_FOLLOW_PATH_RANDOMLY,
    
    //Follow a path towards an absolute position.
    MOB_ACTION_FOLLOW_PATH_TO_ABSOLUTE,
    
    //Get angle between two sets of coordinates.
    MOB_ACTION_GET_ANGLE,
    
    //Get information about the area.
    MOB_ACTION_GET_AREA_INFO,
    
    //Get chomped by another mob.
    MOB_ACTION_GET_CHOMPED,
    
    //Get coordinates from a given angle.
    MOB_ACTION_GET_COORDINATES_FROM_ANGLE,
    
    //Get distance between two sets of coordinates.
    MOB_ACTION_GET_DISTANCE,
    
    //Get information about an event.
    MOB_ACTION_GET_EVENT_INFO,
    
    //Get the Z of the floor at a set of coordinates.
    MOB_ACTION_GET_FLOOR_Z,
    
    //Get a script variable's value from the focused mob.
    MOB_ACTION_GET_FOCUS_VAR,
    
    //Get information about a mob.
    MOB_ACTION_GET_MOB_INFO,
    
    //Get a random float number.
    MOB_ACTION_GET_RANDOM_FLOAT,
    
    //Get a random integer number.
    MOB_ACTION_GET_RANDOM_INT,
    
    //Go to a different part of the script.
    MOB_ACTION_GOTO,
    
    //Hold focused mob.
    MOB_ACTION_HOLD_FOCUS,
    
    //Conditional "if" condition.
    MOB_ACTION_IF,
    
    //Label for use with "goto".
    MOB_ACTION_LABEL,
    
    //Create a link with the focused mob.
    MOB_ACTION_LINK_WITH_FOCUS,
    
    //Load focused mob from focused mobs memory.
    MOB_ACTION_LOAD_FOCUS_MEMORY,
    
    //Move to absolute coordinates.
    MOB_ACTION_MOVE_TO_ABSOLUTE,
    
    //Move to relative coordinates.
    MOB_ACTION_MOVE_TO_RELATIVE,
    
    //Move to a target.
    MOB_ACTION_MOVE_TO_TARGET,
    
    //Order to be released.
    MOB_ACTION_ORDER_RELEASE,
    
    //Play a sound.
    MOB_ACTION_PLAY_SOUND,
    
    //Print some debug text.
    MOB_ACTION_PRINT,
    
    //Receive a status effect.
    MOB_ACTION_RECEIVE_STATUS,
    
    //Release held mob.
    MOB_ACTION_RELEASE,
    
    //Release mobs that are stored inside.
    MOB_ACTION_RELEASE_STORED_MOBS,
    
    //Remove a status effect.
    MOB_ACTION_REMOVE_STATUS,
    
    //Save focused mob into focused mobs memory.
    MOB_ACTION_SAVE_FOCUS_MEMORY,
    
    //Send a message to the focused mob.
    MOB_ACTION_SEND_MESSAGE_TO_FOCUS,
    
    //Send a message to all linked mobs.
    MOB_ACTION_SEND_MESSAGE_TO_LINKS,
    
    //Send a message to nearby mobs.
    MOB_ACTION_SEND_MESSAGE_TO_NEARBY,
    
    //Set animation.
    MOB_ACTION_SET_ANIMATION,
    
    //Set whether it can block paths.
    MOB_ACTION_SET_CAN_BLOCK_PATHS,
    
    //Set its far reach.
    MOB_ACTION_SET_FAR_REACH,
    
    //Set whether it is flying.
    MOB_ACTION_SET_FLYING,
    
    //Set its gravity.
    MOB_ACTION_SET_GRAVITY,
    
    //Set its health.
    MOB_ACTION_SET_HEALTH,
    
    //Set its height.
    MOB_ACTION_SET_HEIGHT,
    
    //Set whether it is hiding.
    MOB_ACTION_SET_HIDING,
    
    //Set whether it is holdable.
    MOB_ACTION_SET_HOLDABLE,
    
    //Set whether it is huntable.
    MOB_ACTION_SET_HUNTABLE,
    
    //Set limb animation.
    MOB_ACTION_SET_LIMB_ANIMATION,
    
    //Set its near reach.
    MOB_ACTION_SET_NEAR_REACH,
    
    //Set its radius.
    MOB_ACTION_SET_RADIUS,
    
    //Set scrolling of its sector.
    MOB_ACTION_SET_SECTOR_SCROLL,
    
    //Set whether its shadow is visible.
    MOB_ACTION_SET_SHADOW_VISIBILITY,
    
    //Set state.
    MOB_ACTION_SET_STATE,
    
    //Set whether it is tangible.
    MOB_ACTION_SET_TANGIBLE,
    
    //Set team.
    MOB_ACTION_SET_TEAM,
    
    //Set a timer.
    MOB_ACTION_SET_TIMER,
    
    //Set a script variable.
    MOB_ACTION_SET_VAR,
    
    //Show a message that is inside a script variable.
    MOB_ACTION_SHOW_MESSAGE_FROM_VAR,
    
    //Spawn something.
    MOB_ACTION_SPAWN,
    
    //Stabilize its Z coordinate.
    MOB_ACTION_STABILIZE_Z,
    
    //Start chomping.
    MOB_ACTION_START_CHOMPING,
    
    //Start the death procedure.
    MOB_ACTION_START_DYING,
    
    //Start the height effect.
    MOB_ACTION_START_HEIGHT_EFFECT,
    
    //Start some particle generator.
    MOB_ACTION_START_PARTICLES,
    
    //Stop moving.
    MOB_ACTION_STOP,
    
    //Stop chomping.
    MOB_ACTION_STOP_CHOMPING,
    
    //Stop the height effect.
    MOB_ACTION_STOP_HEIGHT_EFFECT,
    
    //Stop some particle generator.
    MOB_ACTION_STOP_PARTICLES,
    
    //Stop a playing sound.
    MOB_ACTION_STOP_SOUND,
    
    //Stop vertical movement.
    MOB_ACTION_STOP_VERTICALLY,
    
    //Store the focused mob inside.
    MOB_ACTION_STORE_FOCUS_INSIDE,
    
    //Swallow some chomped Pikmin.
    MOB_ACTION_SWALLOW,
    
    //Swallow all chomped Pikmin.
    MOB_ACTION_SWALLOW_ALL,
    
    //Teleport to absolute coordinates.
    MOB_ACTION_TELEPORT_TO_ABSOLUTE,
    
    //Teleport to relative coordinates.
    MOB_ACTION_TELEPORT_TO_RELATIVE,
    
    //Throw focused mob.
    MOB_ACTION_THROW_FOCUS,
    
    //Turn towards an absolute angle.
    MOB_ACTION_TURN_TO_ABSOLUTE,
    
    //Turn towards a relative angle.
    MOB_ACTION_TURN_TO_RELATIVE,
    
    //Turn towards a target.
    MOB_ACTION_TURN_TO_TARGET,
    
    //Total amount of mob actions.
    N_MOB_ACTIONS,
    
};


//Arachnorb plan logic action sub-types.
enum MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE {

    //Plan to go home.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_HOME,
    
    //Plan to move forward.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_FORWARD,
    
    //Plan a clockwise turn.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CW_TURN,
    
    //Plan a counterclockwise turn.
    MOB_ACTION_ARACHNORB_PLAN_LOGIC_TYPE_CCW_TURN,
    
};


//Turn action sub-types.
enum MOB_ACTION_TURN_TYPE {

    //Logic for an arachnorb's head to turn.
    MOB_ACTION_TURN_TYPE_ARACHNORB_HEAD_LOGIC,
    
    //Turn towards the focused mob.
    MOB_ACTION_TURN_TYPE_FOCUSED_MOB,
    
    //Turn towards home.
    MOB_ACTION_TURN_TYPE_HOME,
    
};


//If action operator types.
enum MOB_ACTION_IF_OP {

    //Check if two values are equal.
    MOB_ACTION_IF_OP_EQUAL,
    
    //Check if two values are different.
    MOB_ACTION_IF_OP_NOT,
    
    //Check if a value is less than another value.
    MOB_ACTION_IF_OP_LESS,
    
    //Check if a value is more than another value.
    MOB_ACTION_IF_OP_MORE,
    
    //Check if a value is less than another value, or equal.
    MOB_ACTION_IF_OP_LESS_E,
    
    //Check if a value is more than another value, or equal.
    MOB_ACTION_IF_OP_MORE_E,
    
};


//Target types for actions that target mobs.
enum MOB_ACTION_MOB_TARGET_TYPE {

    //Targets the mob the script belongs to.
    MOB_ACTION_MOB_TARGET_TYPE_SELF,
    
    //Targets the currently focused mob, if any.
    MOB_ACTION_MOB_TARGET_TYPE_FOCUS,
    
    //Targets the mob that triggered the event, if any.
    MOB_ACTION_MOB_TARGET_TYPE_TRIGGER,
    
    //Targets the first linked object, if any.
    MOB_ACTION_MOB_TARGET_TYPE_LINK,
    
    //Targets the parent mob, if any.
    MOB_ACTION_MOB_TARGET_TYPE_PARENT,
    
};


//Get area info action info types.
enum MOB_ACTION_GET_AREA_INFO_TYPE {

    //Get time of day, in minutes.
    MOB_ACTION_GET_AREA_INFO_TYPE_DAY_MINUTES,
    
    //Get number of Pikmin on the field.
    MOB_ACTION_GET_AREA_INFO_TYPE_FIELD_PIKMIN,
    
};


//Get event info action info types.
enum MOB_ACTION_GET_EV_INFO_TYPE {

    //Get body part that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_BODY_PART,
    
    //Get frame signal that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_FRAME_SIGNAL,
    
    //Get name of hazard that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_HAZARD,
    
    //Get the name of the input that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_INPUT_NAME,
    
    //Get the value of the input that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_INPUT_VALUE,
    
    //Get message that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_MESSAGE,
    
    //Get the other body part that triggered the event.
    MOB_ACTION_GET_EV_INFO_TYPE_OTHER_BODY_PART,
    
};


//Get mob info action info types.
enum MOB_ACTION_GET_MOB_INFO_TYPE {

    //Get angle.
    MOB_ACTION_GET_MOB_INFO_TYPE_ANGLE,
    
    //Get amount of chomped Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_CHOMPED_PIKMIN,
    
    //Get distance towards focused mob.
    MOB_ACTION_GET_MOB_INFO_TYPE_FOCUS_DISTANCE,
    
    //Get total power on the group task.
    MOB_ACTION_GET_MOB_INFO_TYPE_GROUP_TASK_POWER,
    
    //Get health.
    MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH,
    
    //Get health ratio.
    MOB_ACTION_GET_MOB_INFO_TYPE_HEALTH_RATIO,
    
    //Get the numerical ID.
    MOB_ACTION_GET_MOB_INFO_TYPE_ID,
    
    //Get amount of latched Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN,
    
    //Get total weight of latched Pikmin.
    MOB_ACTION_GET_MOB_INFO_TYPE_LATCHED_PIKMIN_WEIGHT,
    
    //Get category of mob that triggered the event.
    MOB_ACTION_GET_MOB_INFO_TYPE_MOB_CATEGORY,
    
    //Get type of mob that triggered the event.
    MOB_ACTION_GET_MOB_INFO_TYPE_MOB_TYPE,
    
    //Get name of current state.
    MOB_ACTION_GET_MOB_INFO_TYPE_STATE,
    
    //Get current weight on top of it.
    MOB_ACTION_GET_MOB_INFO_TYPE_WEIGHT,
    
    //Get X.
    MOB_ACTION_GET_MOB_INFO_TYPE_X,
    
    //Get Y.
    MOB_ACTION_GET_MOB_INFO_TYPE_Y,
    
    //Get Z.
    MOB_ACTION_GET_MOB_INFO_TYPE_Z,
    
};


//Moving action sub-types.
enum MOB_ACTION_MOVE_TYPE {

    //Move away from focused mob.
    MOB_ACTION_MOVE_TYPE_AWAY_FROM_FOCUS,
    
    //Move towards focused mob.
    MOB_ACTION_MOVE_TYPE_FOCUS,
    
    //Move towards the position the focus mob is on right now.
    MOB_ACTION_MOVE_TYPE_FOCUS_POS,
    
    //Move towards home.
    MOB_ACTION_MOVE_TYPE_HOME,
    
    //Follow arachnorb foot movement logic.
    MOB_ACTION_MOVE_TYPE_ARACHNORB_FOOT_LOGIC,
    
    //Move towards the average spot of the linked mobs.
    MOB_ACTION_MOVE_TYPE_LINKED_MOB_AVERAGE,
    
};


//Calculate action sub-types.
enum MOB_ACTION_CALCULATE_TYPE {

    //Sum two numbers.
    MOB_ACTION_CALCULATE_TYPE_SUM,
    
    //Subtract one number from another.
    MOB_ACTION_CALCULATE_TYPE_SUBTRACT,
    
    //Multiply two numbers.
    MOB_ACTION_CALCULATE_TYPE_MULTIPLY,
    
    //Divide one number by another.
    MOB_ACTION_CALCULATE_TYPE_DIVIDE,
    
    //Get the modulo of a number with another.
    MOB_ACTION_CALCULATE_TYPE_MODULO,
    
};


//Stabilize Z action sub-types.
enum MOB_ACTION_STABILIZE_Z_TYPE {

    //Stabilize towards highest Z.
    MOB_ACTION_STABILIZE_Z_TYPE_HIGHEST,
    
    //Stabilize towards lowest z.
    MOB_ACTION_STABILIZE_Z_TYPE_LOWEST,
    
};


//Types of variables that a parameter can use.
enum MOB_ACTION_PARAM {

    //Signed integer.
    MOB_ACTION_PARAM_INT,
    
    //Float.
    MOB_ACTION_PARAM_FLOAT,
    
    //Boolean.
    MOB_ACTION_PARAM_BOOL,
    
    //STL string.
    MOB_ACTION_PARAM_STRING,
    
    //STL string that gets turned into an int.
    MOB_ACTION_PARAM_ENUM,
    
};


/**
 * @brief Info about a parameter that a mob action can receive.
 */
struct MobActionParam {

    //--- Members ---
    
    //Name of the parameter.
    string name;
    
    //Type of variable it's meant to hold.
    MOB_ACTION_PARAM type = MOB_ACTION_PARAM_STRING;
    
    //If true, it must be a constant value. Else, it can also be a var.
    bool force_const = false;
    
    //If true, this is an array of them (minimum amount 0).
    bool is_extras = false;
    
    
    //--- Function declarations ---
    
    MobActionParam(
        const string &name,
        const MOB_ACTION_PARAM type,
        bool force_const,
        bool is_extras
    );
    
};


/**
 * @brief Info about how to run a specific instance of a mob action.
 */
struct MobActionRunData {

    //--- Members ---
    
    //Mob that will run the action.
    Mob* m = nullptr;
    
    //Action call information.
    MobActionCall* call = nullptr;
    
    //Arguments used.
    vector<string> args;
    
    //Event custom data 1.
    void* custom_data_1 = nullptr;
    
    //Event custom data 2.
    void* custom_data_2 = nullptr;
    
    //Return value, if applicable.
    bool return_value = false;
    
    
    //--- Function declarations ---
    
    MobActionRunData(Mob* m, MobActionCall* call);
    
};


/**
 * @brief Function that runs a mob action's logic.
 *
 * The first parameter is the data to run with.
 */
typedef void (mob_action_code_t)(MobActionRunData &data);

/**
 * @brief Function to run when a mob action is loaded from a script.
 *
 * The first parameter is the action call data.
 * Returns whether it loaded successfully.
 */
typedef bool (mob_action_load_code_t)(MobActionCall &call);


/**
 * @brief Info about a mob action.
 */
struct MobAction {

    //--- Members ---
    
    //Type of mob action.
    MOB_ACTION type = MOB_ACTION_UNKNOWN;
    
    //Name.
    string name;
    
    //Code to run.
    mob_action_code_t* code = nullptr;
    
    //Extra logic to run when this action is loaded from a script file.
    mob_action_load_code_t* extra_load_logic = nullptr;
    
    //Parameters that it can take.
    vector<MobActionParam> parameters;
    
};


/**
 * @brief Info about how a specific call to a mob action got declared
 * in the FSM.
 */
struct MobActionCall {

    //--- Members ---
    
    //Action to run, if any.
    MobAction* action = nullptr;
    
    //Custom code to run, if any.
    custom_action_code_t code = nullptr;
    
    //Arguments to use.
    vector<string> args;
    
    //List of which arguments are variable names.
    vector<bool> arg_is_var;
    
    //If something went wrong in parsing it, this describes the error.
    string custom_error;
    
    //Event the action belongs to.
    MOB_EV parent_event = MOB_EV_UNKNOWN;
    
    //Mob type that owns these calls.
    MobType* mt = nullptr;
    
    
    //--- Function declarations ---
    
    explicit MobActionCall(MOB_ACTION type = MOB_ACTION_UNKNOWN);
    explicit MobActionCall(custom_action_code_t code);
    bool load_from_data_node(DataNode* dn, MobType* mt);
    bool run(Mob* m, void* custom_data_1, void* custom_data_2);
    
};


namespace mob_action_runners {
void add_health(MobActionRunData &data);
void add_health(MobActionRunData &data);
void arachnorb_plan_logic(MobActionRunData &data);
void calculate(MobActionRunData &data);
void delete_function(MobActionRunData &data);
void drain_liquid(MobActionRunData &data);
void finish_dying(MobActionRunData &data);
void focus(MobActionRunData &data);
void follow_path_randomly(MobActionRunData &data);
void follow_path_to_absolute(MobActionRunData &data);
void get_angle(MobActionRunData &data);
void get_chomped(MobActionRunData &data);
void get_coordinates_from_angle(MobActionRunData &data);
void get_distance(MobActionRunData &data);
void get_event_info(MobActionRunData &data);
void get_area_info(MobActionRunData &data);
void get_floor_z(MobActionRunData &data);
void get_mob_info(MobActionRunData &data);
void get_focus_var(MobActionRunData &data);
void get_random_float(MobActionRunData &data);
void get_random_int(MobActionRunData &data);
void goto_function(MobActionRunData &data);
void hold_focus(MobActionRunData &data);
void if_function(MobActionRunData &data);
void link_with_focus(MobActionRunData &data);
void load_focus_memory(MobActionRunData &data);
void move_to_absolute(MobActionRunData &data);
void move_to_relative(MobActionRunData &data);
void move_to_target(MobActionRunData &data);
void order_release(MobActionRunData &data);
void play_sound(MobActionRunData &data);
void print(MobActionRunData &data);
void receive_status(MobActionRunData &data);
void release(MobActionRunData &data);
void release_stored_mobs(MobActionRunData &data);
void remove_status(MobActionRunData &data);
void save_focus_memory(MobActionRunData &data);
void send_message_to_focus(MobActionRunData &data);
void send_message_to_links(MobActionRunData &data);
void send_message_to_nearby(MobActionRunData &data);
void set_animation(MobActionRunData &data);
void set_can_block_paths(MobActionRunData &data);
void set_far_reach(MobActionRunData &data);
void set_flying(MobActionRunData &data);
void set_gravity(MobActionRunData &data);
void set_health(MobActionRunData &data);
void set_height(MobActionRunData &data);
void set_hiding(MobActionRunData &data);
void set_holdable(MobActionRunData &data);
void set_huntable(MobActionRunData &data);
void set_limb_animation(MobActionRunData &data);
void set_near_reach(MobActionRunData &data);
void set_radius(MobActionRunData &data);
void set_state(MobActionRunData &data);
void set_sector_scroll(MobActionRunData &data);
void set_shadow_visibility(MobActionRunData &data);
void set_tangible(MobActionRunData &data);
void set_team(MobActionRunData &data);
void set_timer(MobActionRunData &data);
void set_var(MobActionRunData &data);
void show_message_from_var(MobActionRunData &data);
void spawn(MobActionRunData &data);
void stabilize_z(MobActionRunData &data);
void start_chomping(MobActionRunData &data);
void start_dying(MobActionRunData &data);
void start_height_effect(MobActionRunData &data);
void start_particles(MobActionRunData &data);
void stop(MobActionRunData &data);
void stop_chomping(MobActionRunData &data);
void stop_height_effect(MobActionRunData &data);
void stop_particles(MobActionRunData &data);
void stop_sound(MobActionRunData &data);
void stop_vertically(MobActionRunData &data);
void store_focus_inside(MobActionRunData &data);
void swallow(MobActionRunData &data);
void swallow_all(MobActionRunData &data);
void teleport_to_absolute(MobActionRunData &data);
void teleport_to_relative(MobActionRunData &data);
void throw_focus(MobActionRunData &data);
void turn_to_absolute(MobActionRunData &data);
void turn_to_relative(MobActionRunData &data);
void turn_to_target(MobActionRunData &data);
};


namespace mob_action_loaders {
bool arachnorb_plan_logic(MobActionCall &call);
bool calculate(MobActionCall &call);
bool focus(MobActionCall &call);
bool get_area_info(MobActionCall &call);
bool get_event_info(MobActionCall &call);
bool get_mob_info(MobActionCall &call);
bool hold_focus(MobActionCall &call);
bool if_function(MobActionCall &call);
bool move_to_target(MobActionCall &call);
bool play_sound(MobActionCall &call);
bool receive_status(MobActionCall &call);
bool remove_status(MobActionCall &call);
bool set_animation(MobActionCall &call);
bool set_far_reach(MobActionCall &call);
bool set_holdable(MobActionCall &call);
bool set_near_reach(MobActionCall &call);
bool set_team(MobActionCall &call);
bool spawn(MobActionCall &call);
bool stabilize_z(MobActionCall &call);
bool start_chomping(MobActionCall &call);
bool start_particles(MobActionCall &call);
bool turn_to_target(MobActionCall &call);

void report_enum_error(MobActionCall &call, size_t arg_idx);
bool load_mob_target_type(MobActionCall &call, size_t arg_idx);
};


bool assert_actions(
    const vector<MobActionCall*> &actions, const DataNode* dn
);
Mob* get_trigger_mob(MobActionRunData &data);
Mob* get_target_mob(
    MobActionRunData &data, MOB_ACTION_MOB_TARGET_TYPE type
);
void insert_event_actions(
    MobEvent* ev, const vector<MobActionCall*> &actions, bool at_end
);
void load_actions(
    MobType* mt, DataNode* node,
    vector<MobActionCall*>* out_actions, bitmask_8_t* out_settings = 0
);
