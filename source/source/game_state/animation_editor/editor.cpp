/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General animation editor-related functions.
 */

#include <algorithm>
#include <queue>

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using std::queue;

namespace ANIM_EDITOR {

//Threshold for the flood-fill algorithm when picking sprite bitmap parts.
const float FLOOD_FILL_ALPHA_THRESHOLD = 0.008;

//Grid interval in the animation editor.
const float GRID_INTERVAL = 16.0f;

//Minimum radius that a hitbox can have.
const float HITBOX_MIN_RADIUS = 1.0f;

//Amount to pan the camera by when using the keyboard.
const float KEYBOARD_PAN_AMOUNT = 32.0f;

//How tall the animation timeline header is.
const size_t TIMELINE_HEADER_HEIGHT = 12;

//How tall the animation timeline is, in total.
const size_t TIMELINE_HEIGHT = 48;

//Size of each side of the triangle that marks the loop frame.
const size_t TIMELINE_LOOP_TRI_SIZE = 8;

//Pad the left, right, and bottom of the timeline by this much.
const size_t TIMELINE_PADDING = 6;

//Minimum width or height a Pikmin top can have.
const float TOP_MIN_SIZE = 1.0f;

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 32.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.05f;

}


/**
 * @brief Constructs a new animation editor object.
 */
AnimationEditor::AnimationEditor() :
    load_dialog_picker(this) {
    
    comparison_blink_timer =
        Timer(
            0.6,
    [this] () {
        this->comparison_blink_show = !this->comparison_blink_show;
        this->comparison_blink_timer.start();
    }
        );
    comparison_blink_timer.start();
    
    zoom_min_level = ANIM_EDITOR::ZOOM_MIN_LEVEL;
    zoom_max_level = ANIM_EDITOR::ZOOM_MAX_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&AnimationEditor::grid_toggle_cmd, "grid_toggle");
    register_cmd(&AnimationEditor::hitboxes_toggle_cmd, "hitboxes_toggle");
    register_cmd(
        &AnimationEditor::leader_silhouette_toggle_cmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&AnimationEditor::delete_anim_db_cmd, "delete_anim_db");
    register_cmd(&AnimationEditor::load_cmd, "load");
    register_cmd(&AnimationEditor::mob_radius_toggle_cmd, "mob_radius_toggle");
    register_cmd(&AnimationEditor::play_pause_anim_cmd, "play_pause_anim");
    register_cmd(&AnimationEditor::restart_anim_cmd, "restart_anim");
    register_cmd(&AnimationEditor::quit_cmd, "quit");
    register_cmd(&AnimationEditor::reload_cmd, "reload");
    register_cmd(&AnimationEditor::save_cmd, "save");
    register_cmd(
        &AnimationEditor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset"
    );
    register_cmd(&AnimationEditor::zoom_everything_cmd, "zoom_everything");
    register_cmd(&AnimationEditor::zoom_in_cmd, "zoom_in");
    register_cmd(&AnimationEditor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
    
}


/**
 * @brief Centers the camera on the sprite's parent bitmap, so the user
 * can choose what part of the bitmap they want to use for the sprite.
 *
 * @param instant If true, change the camera instantly.
 */
void AnimationEditor::center_camera_on_sprite_bitmap(bool instant) {
    if(cur_sprite && cur_sprite->parent_bmp) {
        Point bmp_size = get_bitmap_dimensions(cur_sprite->parent_bmp);
        Point bmp_pos = 0.0f - bmp_size / 2.0f;
        
        center_camera(bmp_pos, bmp_pos + bmp_size);
    } else {
        game.cam.target_zoom = 1.0f;
        game.cam.target_pos = Point();
    }
    
    if(instant) {
        game.cam.pos = game.cam.target_pos;
        game.cam.zoom = game.cam.target_zoom;
    }
    update_transformations();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param new_state The new state.
 */
void AnimationEditor::change_state(const EDITOR_STATE new_state) {
    comparison = false;
    comparison_sprite = nullptr;
    state = new_state;
    set_status();
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void AnimationEditor::close_load_dialog() {
    if(manifest.internal_name.empty() && dialogs.size() == 1) {
        //If nothing got loaded, we can't return to the editor proper.
        //Quit out, since most of the time that's the user's intent. (e.g.
        //they entered the editor and want to leave without doing anything.)
        //Also make sure no other dialogs are trying to show up, like the load
        //failed dialog.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void AnimationEditor::close_options_dialog() {
    save_options();
}


/**
 * @brief Creates a new, empty animation database.
 *
 * @param path Path to the requested animation database's file.
 */
void AnimationEditor::create_anim_db(const string &path) {
    setup_for_new_anim_db_pre();
    changes_mgr.mark_as_non_existent();
    
    manifest.fill_from_path(path);
    db.manifest = &manifest;
    setup_for_new_anim_db_post();
    
    set_status(
        "Created animation database \"" +
        manifest.internal_name + "\" successfully."
    );
}


/**
 * @brief Code to run for the delete current animation database command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::delete_anim_db_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    open_dialog(
        "Delete animation database?",
        std::bind(&AnimationEditor::process_gui_delete_anim_db_dialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Deletes the current animation database.
 */
void AnimationEditor::delete_current_anim_db() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changes_mgr.exists_on_disk()) {
        //If the database doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the file.
        FS_DELETE_RESULT result = delete_file(manifest.path);
        
        switch(result) {
        case FS_DELETE_RESULT_OK:
        case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            message_box_text =
                "Animation database \"" + orig_internal_name +
                "\" deletion failed! The file was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Animation database \"" + orig_internal_name +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the file and "
                "try again.";
            go_to_load_dialog = false;
            break;
        }
        }
        
    }
    
    //This code will be run after everything is done, be it after the standard
    //procedure, or after the user hits OK on the message box.
    const auto finish_up = [this, go_to_load_dialog] () {
        if(go_to_load_dialog) {
            setup_for_new_anim_db_pre();
            open_load_dialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        set_status(
            "Deleted animation database \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        set_status(
            "Animation database \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        open_message_dialog(
            "Animation database deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Handles the logic part of the main loop of the animation editor.
 */
void AnimationEditor::do_logic() {
    Editor::do_logic_pre();
    
    process_gui();
    
    if(
        anim_playing && state == EDITOR_STATE_ANIMATION &&
        cur_anim_i.valid_frame()
    ) {
        Frame* f = &cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx];
        if(f->duration != 0) {
            vector<size_t> frame_sounds;
            cur_anim_i.tick(game.delta_t, nullptr, &frame_sounds);
            
            for(size_t s = 0; s < frame_sounds.size(); s++) {
                play_sound(frame_sounds[s]);
            }
        } else {
            anim_playing = false;
        }
    }
    
    cur_hitbox_alpha += TAU * 1.5 * game.delta_t;
    
    if(comparison_blink) {
        comparison_blink_timer.tick(game.delta_t);
    } else {
        comparison_blink_show = true;
    }
    
    Editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void AnimationEditor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.animation_ed->draw_canvas();
}


/**
 * @brief Returns the time in the animation in which the mouse cursor is
 * currently located, if the mouse cursor is within the timeline.
 *
 * @return The time.
 */
float AnimationEditor::get_cursor_timeline_time() {
    if(!cur_anim_i.valid_frame()) {
        return 0.0f;
    }
    float anim_x1 = canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING;
    float anim_w = (canvas_br.x - ANIM_EDITOR::TIMELINE_PADDING) - anim_x1;
    float mouse_x = game.mouse_cursor.s_pos.x - anim_x1;
    mouse_x = std::clamp(mouse_x, 0.0f, anim_w);
    return cur_anim_i.cur_anim->get_duration() * (mouse_x / anim_w);
}


/**
 * @brief Returns some tooltip text that represents an animation database
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string AnimationEditor::get_file_tooltip(const string &path) const {
    if(path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") != string::npos) {
        ContentManifest temp_manif;
        string cat;
        string type;
        game.content.mob_anim_dbs.path_to_manifest(
            path, &temp_manif, &cat, &type
        );
        return
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[temp_manif.pack].name + "\n"
            "Mob's internal name: " + type + " (category " + cat + ")";
    } else {
        ContentManifest temp_manif;
        game.content.global_anim_dbs.path_to_manifest(
            path, &temp_manif
        );
        return
            "Internal name: " + temp_manif.internal_name + "\n"
            "File path: " + path + "\n"
            "Pack: " + game.content.packs.list[temp_manif.pack].name;
    }
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string AnimationEditor::get_name() const {
    return "animation editor";
}


/**
 * @brief Returns the name to give the current database's entry for the history.
 *
 * @return The name.
 */
string AnimationEditor::get_name_for_history() const {
    if(loaded_mob_type) {
        return
            loaded_mob_type->name.empty() ?
            loaded_mob_type->manifest->internal_name :
            loaded_mob_type->name;
    } else {
        return
            db.name.empty() ?
            manifest.internal_name :
            db.name;
    }
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string AnimationEditor::get_opened_content_path() const {
    return manifest.path;
}


/**
 * @brief Imports the animation data from a different animation to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::import_animation_data(const string &name) {
    Animation* a = db.animations[db.find_animation(name)];
    
    cur_anim_i.cur_anim->frames = a->frames;
    cur_anim_i.cur_anim->hit_rate = a->hit_rate;
    cur_anim_i.cur_anim->loop_frame = a->loop_frame;
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite bitmap data from a different sprite to the current.
 *
 * @param name Name of the sprite to import from.
 */
void AnimationEditor::import_sprite_bmp_data(const string &name) {
    Sprite* s = db.sprites[db.find_sprite(name)];
    
    cur_sprite->set_bitmap(s->bmp_name, s->bmp_pos, s->bmp_size);
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite hitbox data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::import_sprite_hitbox_data(const string &name) {
    for(size_t s = 0; s < db.sprites.size(); s++) {
        if(db.sprites[s]->name == name) {
            cur_sprite->hitboxes = db.sprites[s]->hitboxes;
        }
    }
    
    update_cur_hitbox();
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite top data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::import_sprite_top_data(const string &name) {
    Sprite* s = db.sprites[db.find_sprite(name)];
    cur_sprite->top_visible = s->top_visible;
    cur_sprite->top_pos = s->top_pos;
    cur_sprite->top_size = s->top_size;
    cur_sprite->top_angle = s->top_angle;
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite transformation data from
 * a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void AnimationEditor::import_sprite_transformation_data(const string &name) {
    Sprite* s = db.sprites[db.find_sprite(name)];
    cur_sprite->offset = s->offset;
    cur_sprite->scale = s->scale;
    cur_sprite->angle = s->angle;
    cur_sprite->tint = s->tint;
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Returns whether the mouse cursor is inside the animation
 * timeline or not.
 *
 * @return Whether the cursor is inside.
 */
bool AnimationEditor::is_cursor_in_timeline() {
    return
        state == EDITOR_STATE_ANIMATION &&
        game.mouse_cursor.s_pos.x >= canvas_tl.x &&
        game.mouse_cursor.s_pos.x <= canvas_br.x &&
        game.mouse_cursor.s_pos.y >= canvas_br.y -
        ANIM_EDITOR::TIMELINE_HEIGHT &&
        game.mouse_cursor.s_pos.y <= canvas_br.y;
}


/**
 * @brief Loads the animation editor.
 */
void AnimationEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    load_custom_mob_cat_types(false);
    
    //Misc. setup.
    side_view = false;
    
    change_state(EDITOR_STATE_MAIN);
    game.audio.set_current_song(game.sys_content_names.sng_editors, false);
    
    //Set the background.
    if(!game.options.anim_editor.bg_path.empty()) {
        bg =
            load_bmp(
                game.options.anim_editor.bg_path,
                nullptr, false, false, false
            );
        use_bg = true;
    } else {
        use_bg = false;
    }
    
    //Automatically load a file if needed, or show the load dialog.
    if(!auto_load_file.empty()) {
        load_anim_db_file(auto_load_file, true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads an animation database.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void AnimationEditor::load_anim_db_file(
    const string &path, bool should_update_history
) {
    //Setup.
    setup_for_new_anim_db_pre();
    changes_mgr.mark_as_non_existent();
    
    //Load.
    manifest.fill_from_path(path);
    DataNode file = DataNode(manifest.path);
    
    if(!file.fileWasOpened) {
        open_message_dialog(
            "Load failed!",
            "Failed to load the animation database file \"" +
            manifest.path + "\"!",
        [this] () { open_load_dialog(); }
        );
        manifest.clear();
        return;
    }
    
    db.manifest = &manifest;
    db.load_from_data_node(&file);
    
    //Find the most popular file name to suggest for new sprites.
    last_spritesheet_used.clear();
    
    if(!db.sprites.empty()) {
        map<string, size_t> file_uses_map;
        vector<std::pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < db.sprites.size(); f++) {
            file_uses_map[db.sprites[f]->bmp_name]++;
        }
        for(auto &u : file_uses_map) {
            file_uses_vector.push_back(make_pair(u.second, u.first));
        }
        std::sort(
            file_uses_vector.begin(),
            file_uses_vector.end(),
            [] (
                std::pair<size_t, string> u1, std::pair<size_t, string> u2
        ) -> bool {
            return u1.first > u2.first;
        }
        );
        last_spritesheet_used = file_uses_vector[0].second;
    }
    
    //Finish up.
    changes_mgr.reset();
    setup_for_new_anim_db_post();
    if(should_update_history) {
        update_history(game.options.anim_editor.history, manifest, get_name_for_history());
    }
    
    set_status("Loaded file \"" + manifest.internal_name + "\" successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void AnimationEditor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        Point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an animation from the picker.
 *
 * @param name Name of the animation.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Is this a new animation or an existing one?
 */
void AnimationEditor::pick_animation(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    if(is_new) {
        db.animations.push_back(new Animation(name));
        db.sort_alphabetically();
        changes_mgr.mark_as_changed();
        set_status("Created animation \"" + name + "\".");
    }
    cur_anim_i.clear();
    cur_anim_i.anim_db = &db;
    cur_anim_i.cur_anim = db.animations[db.find_animation(name)];
}


/**
 * @brief Callback for when the user picks a sprite from the picker.
 *
 * @param name Name of the sprite.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Unused.
 * @param is_new Is this a new sprite or an existing one?
 */
void AnimationEditor::pick_sprite(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    if(is_new) {
        if(db.find_sprite(name) == INVALID) {
            db.sprites.push_back(new Sprite(name));
            db.sprites.back()->create_hitboxes(
                &db,
                loaded_mob_type ? loaded_mob_type->height : 128,
                loaded_mob_type ? loaded_mob_type->radius : 32
            );
            db.sort_alphabetically();
            changes_mgr.mark_as_changed();
            set_status("Created sprite \"" + name + "\".");
        }
    }
    cur_sprite = db.sprites[db.find_sprite(name)];
    update_cur_hitbox();
    
    if(is_new) {
        //New sprite. Suggest file name.
        cur_sprite->set_bitmap(last_spritesheet_used, Point(), Point());
    }
}


/**
 * @brief Plays one of the mob's sounds.
 *
 * @param sound_idx Index of the sound data in the mob type's sound list.
 */
void AnimationEditor::play_sound(size_t sound_idx) {
    if(!loaded_mob_type) return;
    MobType::Sound* sound_data = &loaded_mob_type->sounds[sound_idx];
    if(!sound_data->sample) return;
    game.audio.create_ui_sound_source(
        sound_data->sample,
        sound_data->config
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::grid_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    grid_visible = !grid_visible;
    string state_str = (grid_visible ? "Enabled" : "Disabled");
    set_status(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the hitboxes toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::hitboxes_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    hitboxes_visible = !hitboxes_visible;
    string state_str = (hitboxes_visible ? "Enabled" : "Disabled");
    set_status(state_str + " hitbox visibility.");
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::leader_silhouette_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    leader_silhouette_visible = !leader_silhouette_visible;
    string state_str = (leader_silhouette_visible ? "Enabled" : "Disabled");
    set_status(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the load file command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&AnimationEditor::open_load_dialog, this),
        std::bind(&AnimationEditor::save_anim_db, this)
    );
}


/**
 * @brief Code to run for the mob radius toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::mob_radius_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mob_radius_visible = !mob_radius_visible;
    string state_str = (mob_radius_visible ? "Enabled" : "Disabled");
    set_status(state_str + " object radius visibility.");
}


/**
 * @brief Callback for when the user picks a file from the picker.
 *
 * @param name Name of the file.
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Pointer to the file's content manifest.
 * @param is_new Unused.
 */
void AnimationEditor::pick_anim_db_file(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    string path = temp_manif->path;
    auto really_load = [this, path] () {
        close_top_dialog();
        load_anim_db_file(path, true);
    };
    
    if(
        temp_manif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.advanced.engine_dev
    ) {
        open_base_content_warning_dialog(really_load);
    } else {
        really_load();
    }
}


/**
 * @brief Code to run for the play/pause animation command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::play_pause_anim_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!cur_anim_i.valid_frame()) {
        anim_playing = false;
        return;
    }
    
    anim_playing = !anim_playing;
    if(anim_playing) {
        set_status("Animation playback started.");
    } else {
        set_status("Animation playback stopped.");
    }
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&AnimationEditor::leave, this),
        std::bind(&AnimationEditor::save_anim_db, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!changes_mgr.exists_on_disk()) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_anim_db_file(string(manifest.path), false); },
    std::bind(&AnimationEditor::save_anim_db, this)
    );
}


/**
 * @brief Code to run for the restart animation command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::restart_anim_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!cur_anim_i.valid_frame()) {
        anim_playing = false;
        return;
    }
    
    cur_anim_i.to_start();
    anim_playing = true;
    set_status("Animation playback started from the beginning.");
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    save_anim_db();
}


/**
 * @brief Code to run when the zoom and position reset button widget is pressed.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = Point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoom_everything_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    Sprite* s_ptr = cur_sprite;
    if(!s_ptr && cur_anim_i.valid_frame()) {
        const string &name =
            cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx].sprite_name;
        size_t s_pos = db.find_sprite(name);
        if(s_pos != INVALID) s_ptr = db.sprites[s_pos];
    }
    if(!s_ptr || !s_ptr->bitmap) return;
    
    Point cmin, cmax;
    get_transformed_rectangle_bounding_box(
        s_ptr->offset, s_ptr->bmp_size * s_ptr->scale,
        s_ptr->angle, &cmin, &cmax
    );
    
    if(s_ptr->top_visible) {
        Point top_min, top_max;
        get_transformed_rectangle_bounding_box(
            s_ptr->top_pos, s_ptr->top_size,
            s_ptr->top_angle,
            &top_min, &top_max
        );
        update_min_coords(cmin, top_min);
        update_max_coords(cmax, top_max);
    }
    
    for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
        Hitbox* h_ptr = &s_ptr->hitboxes[h];
        update_min_coords(cmin, h_ptr->pos - h_ptr->radius);
        update_max_coords(cmax, h_ptr->pos + h_ptr->radius);
    }
    
    center_camera(cmin, cmax);
}


/**
 * @brief Code to run for the zoom in command
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoom_in_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        std::clamp(
            game.cam.target_zoom +
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param input_value Value of the player input for the command.
 */
void AnimationEditor::zoom_out_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        std::clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Reloads all loaded animation databases.
 */
void AnimationEditor::reload_anim_dbs() {
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_MOB_ANIMATION,
    }
    );
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_GLOBAL_ANIMATION,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Renames an animation to the given name.
 *
 * @param anim Animation to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::rename_animation(
    Animation* anim, const string &new_name
) {
    //Check if it's valid.
    if(!anim) {
        return;
    }
    
    const string old_name = anim->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the animation's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < db.animations.size(); a++) {
        if(db.animations[a]->name == new_name) {
            set_status(
                "An animation by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    anim->name = new_name;
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed animation \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a body part to the given name.
 *
 * @param part Body part to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::rename_body_part(
    BodyPart* part, const string &new_name
) {
    //Check if it's valid.
    if(!part) {
        return;
    }
    
    const string old_name = part->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the body part's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t b = 0; b < db.body_parts.size(); b++) {
        if(db.body_parts[b]->name == new_name) {
            set_status(
                "A body part by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    for(size_t s = 0; s < db.sprites.size(); s++) {
        for(size_t h = 0; h < db.sprites[s]->hitboxes.size(); h++) {
            if(db.sprites[s]->hitboxes[h].body_part_name == old_name) {
                db.sprites[s]->hitboxes[h].body_part_name = new_name;
            }
        }
    }
    part->name = new_name;
    update_hitboxes();
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed body part \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a sprite to the given name.
 *
 * @param spr Sprite to rename.
 * @param new_name Its new name.
 */
void AnimationEditor::rename_sprite(
    Sprite* spr, const string &new_name
) {
    //Check if it's valid.
    if(!spr) {
        return;
    }
    
    const string old_name = spr->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the sprite's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < db.sprites.size(); s++) {
        if(db.sprites[s]->name == new_name) {
            set_status(
                "A sprite by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    spr->name = new_name;
    for(size_t a = 0; a < db.animations.size(); a++) {
        Animation* a_ptr = db.animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            if(a_ptr->frames[f].sprite_name == old_name) {
                a_ptr->frames[f].sprite_name = new_name;
            }
        }
    }
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed sprite \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void AnimationEditor::reset_cam_xy() {
    game.cam.target_pos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void AnimationEditor::reset_cam_zoom() {
    zoom_with_cursor(1.0f);
}


/**
 * @brief Resizes all sprites, hitboxes, etc. by a multiplier.
 *
 * @param mult Multiplier to resize by.
 */
void AnimationEditor::resize_everything(float mult) {
    if(mult == 0.0f) {
        set_status("Can't resize everything to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        set_status(
            "Resizing everything by 1 wouldn't make a difference!", true
        );
        return;
    }
    
    for(size_t s = 0; s < db.sprites.size(); s++) {
        resize_sprite(db.sprites[s], mult);
    }
    
    changes_mgr.mark_as_changed();
    set_status("Resized everything by " + f2s(mult) + ".");
}


/**
 * @brief Resizes a sprite by a multiplier.
 *
 * @param s Sprite to resize.
 * @param mult Multiplier to resize by.
 */
void AnimationEditor::resize_sprite(Sprite* s, float mult) {
    if(mult == 0.0f) {
        set_status("Can't resize a sprite to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        set_status("Resizing a sprite by 1 wouldn't make a difference!", true);
        return;
    }
    
    s->scale    *= mult;
    s->offset   *= mult;
    s->top_pos  *= mult;
    s->top_size *= mult;
    
    for(size_t h = 0; h < s->hitboxes.size(); h++) {
        Hitbox* h_ptr = &s->hitboxes[h];
        
        h_ptr->radius = fabs(h_ptr->radius * mult);
        h_ptr->pos    *= mult;
    }
    
    changes_mgr.mark_as_changed();
    set_status("Resized sprite by " + f2s(mult) + ".");
}


/**
 * @brief Saves the animation database onto the mob's file.
 *
 * @return Whether it succeded.
 */
bool AnimationEditor::save_anim_db() {
    db.engine_version = get_engine_version_string();
    db.sort_alphabetically();
    
    DataNode file_node = DataNode("", "");
    
    db.save_to_data_node(
        &file_node,
        loaded_mob_type && loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    );
    
    if(!file_node.saveFile(manifest.path)) {
        show_system_message_box(
            nullptr, "Save failed!",
            "Could not save the animation database!",
            (
                "An error occured while saving the animation database to "
                "the file \"" + manifest.path + "\". Make sure that the "
                "folder it is saving to exists and it is not read-only, "
                "and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the animation file!", true);
        return false;
        
    } else {
        set_status("Saved file successfully.");
        changes_mgr.mark_as_saved();
        update_history(game.options.anim_editor.history, manifest, get_name_for_history());
        return true;
        
    }
}


/**
 * @brief Sets up the editor for a new animation database,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void AnimationEditor::setup_for_new_anim_db_post() {
    vector<string> file_path_parts = split(manifest.path, "/");
    
    if(manifest.path.find(FOLDER_PATHS_FROM_PACK::MOB_TYPES + "/") != string::npos) {
        vector<string> path_parts = split(manifest.path, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == FILE_NAMES::MOB_TYPE_ANIMATION
        ) {
            MobCategory* cat =
                game.mob_categories.get_from_folder_name(
                    path_parts[path_parts.size() - 3]
                );
            if(cat) {
                loaded_mob_type =
                    cat->get_type(path_parts[path_parts.size() - 2]);
            }
        }
    }
    
    //Top bitmaps.
    for(unsigned char t = 0; t < N_MATURITIES; t++) {
        if(top_bmp[t]) top_bmp[t] = nullptr;
    }
    
    if(
        loaded_mob_type &&
        loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
        for(size_t m = 0; m < N_MATURITIES; m++) {
            top_bmp[m] = ((PikminType*) loaded_mob_type)->bmp_top[m];
        }
    }
    
    if(loaded_mob_type && db.name == "animations") {
        //Let's give it a proper default name, instead of the internal name
        //in the manifest, which is just "animations".
        db.name = loaded_mob_type->name + " animations";
    }
    
    if(loaded_mob_type) db.fill_sound_idx_caches(loaded_mob_type);
}


/**
 * @brief Sets up the editor for a new animation database,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void AnimationEditor::setup_for_new_anim_db_pre() {
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        //Ideally, states would be handled by a state machine, and this
        //logic would be placed in the sprite bitmap state's "on exit" code...
        game.cam.set_pos(pre_sprite_bmp_cam_pos);
        game.cam.set_zoom(pre_sprite_bmp_cam_zoom);
    }
    
    db.destroy();
    cur_anim_i.clear();
    manifest.clear();
    anim_playing = false;
    cur_sprite = nullptr;
    cur_hitbox = nullptr;
    cur_hitbox_idx = 0;
    loaded_mob_type = nullptr;
    
    game.cam.set_pos(Point());
    game.cam.set_zoom(1.0f);
    change_state(EDITOR_STATE_MAIN);
    
    //At this point we'll have nearly unloaded stuff like the current sprite.
    //Since Dear ImGui still hasn't rendered the current frame, which could
    //have had those assets on-screen, if it tries now it'll crash. So skip.
    game.skip_dear_imgui_frame = true;
}


/**
 * @brief Sets all sprite scales to the value specified in the textbox.
 *
 * @param scale Value to set the scales to.
 */
void AnimationEditor::set_all_sprite_scales(float scale) {
    if(scale == 0) {
        set_status("The scales can't be 0!", true);
        return;
    }
    
    for(size_t s = 0; s < db.sprites.size(); s++) {
        Sprite* s_ptr = db.sprites[s];
        s_ptr->scale.x = scale;
        s_ptr->scale.y = scale;
    }
    
    changes_mgr.mark_as_changed();
    set_status("Set all sprite scales to " + f2s(scale) + ".");
}


/**
 * @brief Sets the current frame to be the most apt sprite it can find,
 * given the current circumstances.
 *
 * Basically, it picks a sprite that's called something similar to
 * the current animation.
 */
void AnimationEditor::set_best_frame_sprite() {
    if(db.sprites.empty()) return;
    
    //Find the sprites that match the most characters with the animation name.
    //Let's set the starting best score to 3, as an arbitrary way to
    //sift out results that technically match, but likely aren't the same
    //term. Example: If the animation is called "running", and there is no
    //"runnning" sprite, we probably don't want a match with "rummaging".
    //Unless it's the exact same word.
    //Also, set the final sprite index to 0 so that if something goes wrong,
    //we default to the first sprite on the list.
    size_t final_sprite_idx = 0;
    vector<size_t> best_sprite_idxs;
    
    if(db.sprites.size() > 1) {
        size_t best_score = 3;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            size_t score = 0;
            if(
                str_to_lower(cur_anim_i.cur_anim->name) ==
                str_to_lower(db.sprites[s]->name)
            ) {
                score = 9999;
            } else {
                score =
                    get_matching_string_starts(
                        str_to_lower(cur_anim_i.cur_anim->name),
                        str_to_lower(db.sprites[s]->name)
                    ).size();
            }
            
            if(score < best_score) {
                continue;
            }
            if(score > best_score) {
                best_score = score;
                best_sprite_idxs.clear();
            }
            best_sprite_idxs.push_back(s);
        }
    }
    
    if(best_sprite_idxs.size() == 1) {
        //If there's only one best match, go for it.
        final_sprite_idx = best_sprite_idxs[0];
        
    } else if(best_sprite_idxs.size() > 1) {
        //Sort them alphabetically and pick the first.
        std::sort(
            best_sprite_idxs.begin(),
            best_sprite_idxs.end(),
        [this, &best_sprite_idxs] (size_t s1, size_t s2) {
            return
                str_to_lower(db.sprites[s1]->name) <
                str_to_lower(db.sprites[s2]->name);
        });
        final_sprite_idx = best_sprite_idxs[0];
    }
    
    //Finally, set the frame info then.
    Frame* cur_frame_ptr =
        &cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx];
    cur_frame_ptr->sprite_idx = final_sprite_idx;
    cur_frame_ptr->sprite_ptr = db.sprites[final_sprite_idx];
    cur_frame_ptr->sprite_name = db.sprites[final_sprite_idx]->name;
}


/**
 * @brief Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 *
 * @param bmp Locked bitmap to check.
 * @param selection_pixels Array that controls which pixels are selected or not.
 * @param x X coordinate to start on.
 * @param y Y coordinate to start on.
 */
void AnimationEditor::sprite_bmp_flood_fill(
    ALLEGRO_BITMAP* bmp, bool* selection_pixels, int x, int y
) {
    //https://en.wikipedia.org/wiki/Flood_fill#The_algorithm
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    
    if(x < 0 || x > bmp_w) return;
    if(y < 0 || y > bmp_h) return;
    if(selection_pixels[y * bmp_w + x]) return;
    if(al_get_pixel(bmp, x, y).a < ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD) {
        return;
    }
    
    /**
     * @brief A point, but with integer coordinates.
     */
    struct IntPoint {
    
        //--- Members ---
        
        //X coordinate.
        int x = 0;
        
        //Y coordinate.
        int y = 0;
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new int point object.
         *
         * @param p The float point coordinates.
         */
        explicit IntPoint(const Point &p) :
            x(p.x),
            y(p.y) { }
            
        /**
         * @brief Constructs a new int point object.
         *
         * @param x X coordinate.
         * @param y Y coordinate.
         */
        IntPoint(int x, int y) :
            x(x),
            y(y) { }
            
    };
    
    queue<IntPoint> pixels_left;
    pixels_left.push(IntPoint(x, y));
    
    while(!pixels_left.empty()) {
        IntPoint p = pixels_left.front();
        pixels_left.pop();
        
        if(
            selection_pixels[(p.y) * bmp_w + p.x] ||
            al_get_pixel(bmp, p.x, p.y).a <
            ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
        ) {
            continue;
        }
        
        IntPoint offset = p;
        vector<IntPoint> columns;
        columns.push_back(p);
        
        bool add = true;
        //Keep going left and adding columns to check.
        while(add) {
            if(offset.x  == 0) {
                add = false;
            } else {
                offset.x--;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        offset = p;
        add = true;
        //Keep going right and adding columns to check.
        while(add) {
            if(offset.x == bmp_w - 1) {
                add = false;
            } else {
                offset.x++;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        
        for(size_t c = 0; c < columns.size(); c++) {
            //For each column obtained, mark the pixel there,
            //and check the pixels above and below, to see if they should be
            //processed next.
            IntPoint col = columns[c];
            selection_pixels[col.y * bmp_w + col.x] = true;
            if(
                col.y > 0 &&
                !selection_pixels[(col.y - 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y - 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(IntPoint(col.x, col.y - 1));
            }
            if(
                col.y < bmp_h - 1 &&
                !selection_pixels[(col.y + 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(IntPoint(col.x, col.y + 1));
            }
        }
    }
}


/**
 * @brief Unloads the editor from memory.
 */
void AnimationEditor::unload() {
    Editor::unload();
    
    db.destroy();
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
    
    if(bg) {
        al_destroy_bitmap(bg);
        bg = nullptr;
    }
}


/**
 * @brief Updates the current hitbox pointer to match the same body part as
 * before, but on the hitbox of the current sprite. If not applicable,
 * it chooses a valid hitbox.
 *
 */
void AnimationEditor::update_cur_hitbox() {
    if(cur_sprite->hitboxes.empty()) {
        cur_hitbox = nullptr;
        cur_hitbox_idx = INVALID;
        return;
    }
    
    cur_hitbox_idx = std::min(cur_hitbox_idx, cur_sprite->hitboxes.size() - 1);
    cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
}


/**
 * @brief Update every frame's hitbox instances in light of new hitbox info.
 */
void AnimationEditor::update_hitboxes() {
    for(size_t s = 0; s < db.sprites.size(); s++) {
    
        Sprite* s_ptr = db.sprites[s];
        
        //Start by deleting non-existent hitboxes.
        for(size_t h = 0; h < s_ptr->hitboxes.size();) {
            string h_name = s_ptr->hitboxes[h].body_part_name;
            bool name_found = false;
            
            for(size_t b = 0; b < db.body_parts.size(); b++) {
                if(db.body_parts[b]->name == h_name) {
                    name_found = true;
                    break;
                }
            }
            
            if(!name_found) {
                s_ptr->hitboxes.erase(
                    s_ptr->hitboxes.begin() + h
                );
            } else {
                h++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t b = 0; b < db.body_parts.size(); b++) {
            bool hitbox_found = false;
            const string &name = db.body_parts[b]->name;
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
                if(s_ptr->hitboxes[h].body_part_name == name) {
                    hitbox_found = true;
                    break;
                }
            }
            
            if(!hitbox_found) {
                s_ptr->hitboxes.push_back(
                    Hitbox(
                        name, INVALID, nullptr, Point(), 0,
                        loaded_mob_type ? loaded_mob_type->height : 128,
                        loaded_mob_type ? loaded_mob_type->radius : 32
                    )
                );
            }
        }
        
        //Sort them with the new order.
        std::sort(
            s_ptr->hitboxes.begin(),
            s_ptr->hitboxes.end(),
        [this] (const Hitbox & h1, const Hitbox & h2) -> bool {
            size_t pos1 = 0, pos2 = 1;
            for(size_t b = 0; b < db.body_parts.size(); b++) {
                if(db.body_parts[b]->name == h1.body_part_name) pos1 = b;
                if(db.body_parts[b]->name == h2.body_part_name) pos2 = b;
            }
            return pos1 < pos2;
        }
        );
    }
}
