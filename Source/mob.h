/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob class and mob-related functions.
 */

#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <map>
#include <vector>

#include <allegro5/allegro.h>

#include "animation.h"
#include "misc_structs.h"
#include "mob_script.h"
#include "pikmin_type.h"
#include "sector.h"

using namespace std;

struct party_spot_info;

class mob_type;
class mob;

/* ----------------------------------------------------------------------------
 * Information on a mob's party.
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct party_info {
    vector<mob*> members;
    party_spot_info* party_spots;
    float party_center_x, party_center_y;
    
    party_info(party_spot_info* ps, const float center_x, const float center_y) {
        party_spots = ps;
        party_center_x = center_x;
        party_center_y = center_y;
    }
};



/* ----------------------------------------------------------------------------
 * Structure with information on how
 * the mob should be carried.
 */
struct carrier_info_struct {
    unsigned int max_carriers;
    bool carry_to_ship;            //If true, this is carried to the ship. Otherwise, it's carried to an Onion.
    vector<float> carrier_spots_x; //These are the relative coordinates of each spot. They avoid calculating several sines and cosines over and over.
    vector<float> carrier_spots_y;
    
    float current_carrying_strength; //This is to avoid going through the vector only to find out the total strength.
    size_t current_n_carriers;       //Likewise, this is to avoid going through the vector only to find out the number. Note that this is the number of spaces reserved. A Pikmin could be on its way to its spot, not necessarily there already.
    vector<mob*> carrier_spots;      //Pikmin carrying, and their spots.
    pikmin_type* decided_type;       //Current Onion type it's being taken to.
    
    carrier_info_struct(mob* m, const unsigned int max_carriers, const bool carry_to_ship);
    ~carrier_info_struct();
};



/* ----------------------------------------------------------------------------
 * A mob, short for "mobile object" or "map object",
 * or whatever tickles your fancy, is any instance of
 * an object in the game world. It can move, follow a point,
 * has health, and can be a variety of different sub-types,
 * like leader, Pikmin, enemy, Onion, etc.
 */
class mob {
protected:
    void tick_animation();
    void tick_brain();
    void tick_misc_logic();
    void tick_physics();
    void tick_script();
    
public:
    mob(const float x, const float y, mob_type* type, const float angle, const string &vars);
    virtual ~mob(); //Needed so that typeid works.
    
    mob_type* type;
    
    animation_instance anim;
    
    //Flags.
    bool to_delete; //If true, this mob should be deleted.
    bool reached_destination;
    
    //Actual moving and other physics.
    float x, y, z;                    //Coordinates. Z is height, the higher the value, the higher in the sky.
    float speed_x, speed_y, speed_z;  //Physics only. Don't touch.
    float home_x, home_y;             //Starting coordinates; what the mob calls "home".
    float move_speed_mult;            //Multiply the normal moving speed by this. //TODO will this be used?
    float acceleration;               //Speed multiplies by this much each second. //TODO use this.
    float speed;                      //Speed moving forward. //TODO is this used?
    float angle;                      //0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
    float intended_angle;             //Angle the mob wants to be facing.
    float ground_z;                   //Z of the highest ground it's on.
    float lighting;                   //How light the mob is. Depends on the sector(s) it's on.
    bool affected_by_gravity;         //Is the mob currently affected by gravity? Wollywogs stop in mid-air when jumping, for instance.
    float push_amount;                //Amount it's being pushed by another mob.
    float push_angle;                 //Angle that another mob is pushing it to.
    
    void face(const float new_angle); //Makes the mob face an angle, but it'll turn at its own pace.
    void get_final_target(float* x, float* y); //Returns the final coordinates of a go_to_target target.
    virtual float get_base_speed();   //Returns the normal speed of this mob. Subclasses are meant to override this.
    
    //Target things.
    float target_x, target_y;           //When movement is automatic, this is the spot the mob is trying to go to.
    float* target_z;                    //When following a target in teleport mode, also change the z accordingly.
    float* target_rel_x, *target_rel_y; //Follow these coordinates.
    bool go_to_target;                  //If true, it'll try to go to the target spot on its own.
    bool gtt_instant;                   //If true, teleport instantly.
    bool gtt_free_move;                 //If true, the mob can move in a direction it's not facing.
    float target_distance;              //Distance from the target in which the mob is considered as being there.
    
    void set_target(const float target_x, const float target_y, float* target_rel_x, float* target_rel_y, const bool instant, float* target_z = NULL, bool free_move = false, float target_distance = 3);
    void remove_target();
    
    //Party things.
    mob* following_party;      //The current mob is following this mob's party.
    bool was_thrown;           //Is the mob airborne because it was thrown?
    float unwhistlable_period; //During this period, the mob cannot be whistled into a party.
    float untouchable_period;  //During this period, the mob cannot be touched into a party.
    party_info* party;         //Info on the party this mob is a leader of.
    float party_spot_x;
    float party_spot_y;
    
    
    //Other properties.
    size_t id;              //Incremental ID. Used for minor things.
    float health;           //Current health.
    timer invuln_period;    //During this period, the mob cannot be attacked.
    unsigned char team;     //Mob's team (who it can damage), use MOB_TEAM_*.
    
    //Script.
    mob_fsm fsm;                      //Finitate-state machine.
    bool first_state_set;             //Have we set the mob's starting state yet?
    mob* focused_mob;                 //The mob it has focus on.
    timer script_timer;               //The timer.
    map<string, string> vars;         //Variables.
    bool big_damage_ev_queued;        //Are we waiting to report the big damage event?
    
    void set_animation(const size_t nr, const bool pre_named = true);
    void set_health(const bool rel, const float amount);
    void set_timer(const float time);
    void set_var(const string &name, const string &value);
    
    void eat(size_t nr);
    void start_dying();
    void finish_dying();
    
    bool dead;                     //Is the mob dead?
    float delivery_time;           //Time left until the mob is fully delivered onto an Onion.
    vector<int> chomp_hitboxes;    //List of hitboxes that will chomp Pikmin.
    vector<mob*> chomping_pikmin;  //Mobs it is chomping.
    size_t chomp_max;              //Max mobs it can chomp in the current attack.
    
    //Carrying.
    carrier_info_struct* carrier_info; //Structure holding information on how this mob should be carried. If NULL, it cannot be carried.
    
    void tick();
    virtual void draw();
    
    static void attack(mob* m1, mob* m2, const bool m1_is_pikmin, const float damage, const float angle, const float knockback, const float new_invuln_period, const float new_knockdown_period);
    
    static void lose_health(mob* m, void* info1, void* info2);
    
    //Drawing tools.
    void get_sprite_center(mob* m, frame* f, float* x, float* y);
    void get_sprite_dimensions(mob* m, frame* f, float* w, float* h, float* scale = NULL);
    float get_sprite_lighting(mob* m);
    
};



void add_to_party(mob* party_leader, mob* new_member);
void apply_knockback(mob* m, const float knockback, const float knockback_angle);
float calculate_damage(mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h);
void calculate_knockback(mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h, float* knockback, float* angle);
void cause_hitbox_damage(mob* attacker, mob* victim, hitbox_instance* attacker_h, hitbox_instance* victim_h, float* total_damage);
void create_mob(mob* m);
void delete_mob(mob* m);
void focus_mob(mob* m1, mob* m2);
hitbox_instance* get_closest_hitbox(const float x, const float y, mob* m);
hitbox_instance* get_hitbox_instance(mob* m, const size_t nr);
void make_uncarriable(mob* m);
void remove_from_party(mob* member);
bool should_attack(mob* m1, mob* m2);
void unfocus_mob(mob* m1);


extern size_t next_mob_id;

const float GRAVITY_ADDER = -1300.0f; //Accelerate the Z speed of mobs affected by gravity by this amount per second.

enum MOB_TEAMS {
    MOB_TEAM_NONE,       //Can hurt/target anyone and be hurt/targeted by anyone, on any team.
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    MOB_TEAM_ENEMY_1,
    MOB_TEAM_ENEMY_2,
    MOB_TEAM_OBSTACLE,  //Can only be hurt by Pikmin.
    MOB_TEAM_DECORATION, //Cannot be hurt or targeted by anything.
};

enum MOB_STATE_IDS {
    MOB_STATE_IDLE,
    MOB_STATE_BEING_CARRIED,
    MOB_STATE_BEING_DELIVERED, //Into an Onion.
    
};

#endif //ifndef MOB_INCLUDED
