/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale type class and scale type-related functions.
 */

#include "scale_type.h"

#include "../../util/string_utils.h"
#include "../mob/scale.h"


/**
 * @brief Constructs a new scale type object.
 */
ScaleType::ScaleType() :
    MobType(MOB_CATEGORY_SCALES) {
    
    target_type = MOB_TARGET_FLAG_NONE;
    walkable = true;
    
    AreaEditorProp aep_goal;
    aep_goal.name = "Goal weight";
    aep_goal.var = "goal_number";
    aep_goal.type = AEMP_TYPE_INT;
    aep_goal.def_value = i2s(goal_number);
    aep_goal.tooltip = "Pikmin weight required for the goal, if any.";
    area_editor_props.push_back(aep_goal);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ScaleType::load_cat_properties(DataNode* file) {
    ReaderSetter rs(file);
    
    rs.set("goal_number", goal_number);
    
    area_editor_props.back().def_value = i2s(goal_number);
}
