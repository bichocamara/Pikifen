/*
 * Copyright (c) Andr� 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

//TODO check for ".c_str()" in the code, as apparently I have some atois and atof instead of toi and tof.

#include <fstream>
#include <math.h>
#include <string>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "animation_editor.h"
#include "area_editor.h"
#include "const.h"
#include "controls.h"
#include "drawing.h"
#include "functions.h"
#include "gate.h"
#include "init.h"
#include "LAFI/button.h"
#include "LAFI/checkbox.h"
#include "LAFI/frame.h"
#include "LAFI/label.h"
#include "LAFI/radio_button.h"
#include "LAFI/scrollbar.h"
#include "LAFI/textbox.h"
#include "logic.h"
#include "menus.h"
#include "sector.h"
#include "vars.h"

using namespace std;

/*
 * Main function.
 * It begins by loading Allegro stuff,
 * the options, setting some settings,
 * and loading all of the game content.
 * Once that's done, it enters the main loop.
 */
int main(int argc, char** argv) {
    //Allegro initializations.
    init_allegro();
    
    //Controls and options.
    init_controls();
    load_options();
    save_options();
    
    //Event stuff.
    ALLEGRO_TIMER* logic_timer;
    ALLEGRO_EVENT_QUEUE* logic_queue;
    ALLEGRO_EVENT ev;
    init_event_things(logic_timer, logic_queue);
    
    //Other fundamental initializations.
    init_misc();
    init_error_bitmap();
    init_fonts();
    
    //The icon is used a lot, so load it here.
    bmp_icon = load_bmp("Icon.png");
    
    //Draw the basic loading screen.
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Init some other things.
    init_mob_categories();
    init_special_mob_types();
    init_sector_types();
    
    unsigned int first_game_state = GAME_STATE_MAIN_MENU;
    if(argc >= 2){
        string arg(argv[1]);
        if(arg == "play")
            first_game_state = GAME_STATE_GAME;
        else if(arg == "anim")
            first_game_state = GAME_STATE_ANIMATION_EDITOR;
        else if(arg == "area")
            first_game_state = GAME_STATE_AREA_EDITOR;
    }
    
    change_game_state(first_game_state);
    
    //Main loop.
    al_start_timer(logic_timer);
    while(running) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(logic_queue, &ev);
        
        if(cur_game_state == GAME_STATE_MAIN_MENU) {
            main_menu::handle_controls(ev);
        }else if(cur_game_state == GAME_STATE_GAME) {
            handle_game_controls(ev);
        } else if(cur_game_state == GAME_STATE_AREA_EDITOR) {
            area_editor::handle_controls(ev);
        } else if(cur_game_state == GAME_STATE_ANIMATION_EDITOR) {
            animation_editor::handle_controls(ev);
        }
        
        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
            
        } else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            //scr_w = ev.display.width;
            //scr_h = ev.display.height;
            
        } else if(ev.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(logic_queue)) {
            double cur_time = al_get_time();
            if(reset_delta_t){
                prev_frame_time = cur_time - 1.0f / game_fps; //Failsafe.
                reset_delta_t = false;
            }
            delta_t = cur_time - prev_frame_time;
            
            if(cur_game_state == GAME_STATE_MAIN_MENU) {
                main_menu::do_logic();
            } else if(cur_game_state == GAME_STATE_GAME) {
                do_logic();
                do_drawing();
            } else if(cur_game_state == GAME_STATE_AREA_EDITOR) {
                area_editor::do_logic();
            } else if(cur_game_state == GAME_STATE_ANIMATION_EDITOR) {
                animation_editor::do_logic();
            }
            
            prev_frame_time = cur_time;
        }
    }
    
}
