/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship finite state machine logic.
 */

#pragma once

#include "../mob_type/mob_type.h"


/**
 * @brief Functions about the ship's finite state machine and behavior.
 */
namespace ship_fsm {
void create_fsm(MobType* typ);

void receive_mob(Mob* m, void* info1, void* info2);
void set_anim(Mob* m, void* info1, void* info2);
void start_delivery(Mob* m, void* info1, void* info2);
}
