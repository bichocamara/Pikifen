/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion class and Onion-related functions.
 */

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "../../core/misc_structs.h"
#include "../../util/general_utils.h"
#include "../mob_type/onion_type.h"
#include "../mob_type/pikmin_type.h"
#include "mob.h"


namespace ONION {
extern const float FADE_SPEED;
extern const float GENERATION_DELAY;
extern const float NEW_SEED_Z_OFFSET;
extern const float NEXT_GENERATION_INTERVAL;
extern const unsigned char SEETHROUGH_ALPHA;
extern const float SPEW_ANGLE_SHIFT;
extern const float SPEW_H_SPEED;
extern const float SPEW_H_SPEED_DEVIATION;
extern const float SPEW_V_SPEED;
}


/**
 * @brief An Onion is where Pikmin are stored.
 */
class Onion : public Mob {

public:

    //--- Members ---
    
    //What type of Onion it is.
    OnionType* oni_type = nullptr;
    
    //Nest data.
    PikminNest* nest = nullptr;
    
    //Is this Onion currently activated?
    bool activated = true;
    
    //How many seeds are queued up to be spat, of each type.
    vector<size_t> generation_queue;
    
    //Time left until it starts the Pikmin generation process.
    Timer generation_delay_timer = Timer(ONION::GENERATION_DELAY);
    
    //Time left until it generates the next Pikmin in the queue.
    Timer next_generation_timer = Timer(ONION::NEXT_GENERATION_INTERVAL);
    
    //Angle at which the next seed will be spit.
    float next_spew_angle = 0.0f;
    
    //The Onion's alpha.
    unsigned char seethrough = 255;
    
    //Spit a new seed.
    void generate();
    
    
    //--- Function declarations ---
    
    Onion(const Point &pos, OnionType* type, float angle);
    ~Onion();
    void draw_mob() override;
    void read_script_vars(const ScriptVarReader &svr) override;
    void start_generating();
    void stop_generating();
    
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(float delta_t) override;
    
};
