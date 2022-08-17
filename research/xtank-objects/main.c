/*
  Filename: main.c
  Author: Paul Schulz <paul@mawsonlakes.org>
  - with parts import and modified from the original source code.

  All original code is licensed under the same original xtank license.

  Portions of the code are covered by the following Copyright notice.
*/

/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#include <math.h>  // cos()

#define APPLICATION_ID  "org.mawsonlakes.xtank"

struct timespec tms;
int64_t start_time, end_time, begin_time;
int64_t start_steps, end_steps;

//////////////////////////////////////////////////////////////////////////////
// From the  original program

#include "xtank.h"
#include "bullet.h"
#include "clfkr.h"
#include "proto.h"

#ifdef DEBUG
#define debug(str) puts(str)
#else
#define debug(str)
#endif

char team_char[] = "NROYGBV"; // Team Colors

Eset actual_eset, *eset;
Bset actual_bset, *bset;

Maze maze;
Map real_map;
Settings settings;
Boolean game_paused;
int frame;						/* the game clock */
char *executable_name;

/**************************************************************************** \
* A little terminology: a "dead" vehicle is one that is waiting a few frames *
* to be resurrected.  Robot threads that belonged to dead vehicles are	     *
* deallocated, but the Vehicle structure itself is kept.  The entry in	     *
* live_vehicles[] is moved to dead_vehicles[], though.			     *
* 									     *
* When a vehicle is out of the game for good (like if the restart setting is *
* off), it becomes "permanently dead" and its Vehicle structure is	     *
* deallocated.  Terminals that belonged to such vehicles are left in	     *
* observation mode, and robot threads are deallocacted.			     *
\****************************************************************************/

int num_veh;					/* number of vehicles in the game (some living,
				   some dead) */
int num_veh_alive;				/* number of currently living vehicles */
int num_veh_dead;				/* number of currently dead vehicles */
Vehicle actual_vehicles[MAX_VEHICLES];	/* up to "num_veh" entries are
					   valid (permanently dead vehicles
					   leave holes) */
Vehicle *live_vehicles[MAX_VEHICLES];	/* pointers into actual_vehicles[], the
					   first "num_veh_alive" entries are
					   valid */
Vehicle *dead_vehicles[MAX_VEHICLES];	/* pointers into actual_vehicles[], the
					   first "num_veh_dead" entries are
					   valid */
CLFkr command_options;	/* options for how xtank starts and exits */

extern char *version1;
extern char *version2;
extern char *version3;

void
debugger_break(void)
{
	;							/* this is a good place to but a breakpoint */
}

//////////////////////////////////////////////////////////////////////////////
// Vehicles
#include "display/objects/lightc.obj"
#include "display/objects/hexo.obj"
#include "display/objects/spider.obj"
#include "display/objects/psycho.obj"
#include "display/objects/tornado.obj"
#include "display/objects/marauder.obj"
#include "display/objects/tiger.obj"
#include "display/objects/rhino.obj"
#include "display/objects/medusa.obj"
#include "display/objects/malice.obj"
#include "display/objects/trike.obj"
#include "display/objects/panzy.obj"
#include "display/objects/disk.obj"
#include "display/objects/delta.obj"

// Turret
#include "display/objects/turret_sm.obj"

// Explosions
#include "display/objects/shock.obj"
#include "display/objects/gleam.obj"
#include "display/objects/tink.obj"
#include "display/objects/soft.obj"
#include "display/objects/circle.obj"
#include "display/objects/fiery.obj"
#include "display/objects/double.obj"
#include "display/objects/exhaust.obj"
#include "display/objects/electric.obj"
#include "display/objects/bullets.obj"
#include "display/objects/nuke.obj"

// Landmarks
#include "display/objects/anm_lmarks.obj"
#include "display/objects/map_lmarks.obj"
#include "display/objects/des_lmarks.obj"

// Other
#include "display/objects/xtank.obj"
#include "display/objects/team.obj"
#include "display/objects/terp.obj"

int num_vehicle_objs;
Object *vehicle_obj[MAX_VEHICLE_OBJS];

int num_turret_objs;
Object *turret_obj[MAX_TURRET_OBJS];

int num_exp_objs;
Object *exp_obj[MAX_EXP_OBJS];

int num_bullet_objs;   /* NUM_BULLET_OBJS defined in bullets.obj */
Object *bullet_obj[NUM_BULLET_OBJS];

int num_landmark_objs;
Object *landmark_obj[MAX_LANDMARK_OBJS];

int object_error;

//////////////////////////////////////////////////////////////////////////////
Object *
process_object(Object *object, unsigned char **pixdata) {
    int pic = 0;
    //  fprintf(stderr,"-----------------------\n");
    //  fprintf(stderr, "type:        %s\n", (char *)object->type);
    //  fprintf(stderr, "num_pics:    %d\n", object->num_pics);
    //  fprintf(stderr, "num_turrets: %d\n", object->num_turrets);
    //  fprintf(stderr, "num_segs:    %d\n", object->num_segs);
    //  fprintf(stderr,"-----------------------\n");

    GdkPixbuf *pix_buffer;

    for (int pic=0; pic<object->num_pics; pic++) {
        Picture *picture = &object->pic[pic];
        unsigned char *data = pixdata[pic];

        int w = picture->width;
        int h = picture->height;
        int byte_width = w/8 + 1;

        // Create new pixbuf
        pix_buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, w, h);
        // fprintf(stderr, "rowstride: %d\n", gdk_pixbuf_get_rowstride(pix_buffer));
        // fprintf(stderr, "n_channels: %d\n", gdk_pixbuf_get_n_channels(pix_buffer));
        // fprintf(stderr, "w: %d (%d)  h: %d\n", w, byte_width, h);

        // fprintf(stderr, "rows: ");
        for (int j=0; j<h; j++){
            // fprintf(stderr, ".");
            for(int i=0; i<w; i++){
                int byte = byte_width*j + i/8;
                int bit  = i%8;

//                if (data[byte] & 1<<bit) {
//                    fprintf(stderr, "*");
//                } else {
//                fprintf(stderr, " ");
//                }

                int offset = j*gdk_pixbuf_get_rowstride(pix_buffer)
                    + i*gdk_pixbuf_get_n_channels(pix_buffer);
                guchar * pixel = &gdk_pixbuf_get_pixels(pix_buffer)[ offset ]; // get pixel pointer

                // Black and White
                if (data[byte] & 1<<bit) {
                    pixel[0] = 0xFF;
                    pixel[1] = 0xFF;
                    pixel[2] = 0xFF;
                    pixel[3] = 0xFF;
                } else {
                    pixel[0] = 0x00;
                    pixel[1] = 0x00;
                    pixel[2] = 0x00;
                    pixel[3] = 0x00;
                }
            }
            //          fprintf(stderr, "\n");
        }
        // fprintf(stderr,"\n");
        picture->pixbuf = pix_buffer;
    } // loop: pic

    return object;
}

// Similar to 'make_objects' in original code.
static void
process_objects (void) {
    fprintf(stderr,"*** process_objects()\n");

    int num;

    // Clear the error flag
    object_error = 0;

    // Make all of the vehicle objects
    num = 0;
    vehicle_obj[num++] = process_object(&lightc_obj, lightc_bitmap);
    vehicle_obj[num++] = process_object(&trike_obj, trike_bitmap);
    vehicle_obj[num++] = process_object(&hexo_obj, hexo_bitmap);
    vehicle_obj[num++] = process_object(&spider_obj, spider_bitmap);
    vehicle_obj[num++] = process_object(&psycho_obj, psycho_bitmap);
    vehicle_obj[num++] = process_object(&tornado_obj, tornado_bitmap);
    vehicle_obj[num++] = process_object(&marauder_obj, marauder_bitmap);
    vehicle_obj[num++] = process_object(&tiger_obj, tiger_bitmap);
    vehicle_obj[num++] = process_object(&rhino_obj, rhino_bitmap);
    vehicle_obj[num++] = process_object(&medusa_obj, medusa_bitmap);
    vehicle_obj[num++] = process_object(&delta_obj, delta_bitmap);
    vehicle_obj[num++] = process_object(&disk_obj, disk_bitmap);
    vehicle_obj[num++] = process_object(&malice_obj, malice_bitmap);
    vehicle_obj[num++] = process_object(&panzy_obj, panzy_bitmap);
    num_vehicle_objs = num;

     // Make all of the explosion objects
    num = 0;
    exp_obj[num++] = process_object(&shock_obj, shock_bitmap);
    exp_obj[num++] = process_object(&gleam_obj, gleam_bitmap);
    exp_obj[num++] = process_object(&tink_obj, tink_bitmap);
    exp_obj[num++] = process_object(&soft_obj, soft_bitmap);
    exp_obj[num++] = process_object(&circle_obj, circle_bitmap);
    exp_obj[num++] = process_object(&fiery_obj, fiery_bitmap);
    exp_obj[num++] = process_object(&double_obj, double_bitmap);
    exp_obj[num++] = process_object(&exhaust_obj, exhaust_bitmap);
    num_exp_objs = num;

    /* Make the bullet object */
    num = 0;
    bullet_obj[num++] = process_object(&lmg_obj, lmg_bitmap);
    bullet_obj[num++] = process_object(&mg_obj, mg_bitmap);
    bullet_obj[num++] = process_object(&hmg_obj, hmg_bitmap);
    bullet_obj[num++] = process_object(&lac_obj, lac_bitmap);
    bullet_obj[num++] = process_object(&ac_obj, ac_bitmap);
    bullet_obj[num++] = process_object(&hac_obj, hac_bitmap);
    bullet_obj[num++] = process_object(&lrl_obj, lrl_bitmap);
    bullet_obj[num++] = process_object(&rl_obj, rl_bitmap);
    bullet_obj[num++] = process_object(&hrl_obj, hrl_bitmap);
    bullet_obj[num++] = process_object(&as_obj, as_bitmap);
    bullet_obj[num++] = process_object(&ft_obj, ft_bitmap);
    bullet_obj[num++] = process_object(&seek_obj, seek_bitmap);
    bullet_obj[num++] = process_object(&pr_obj, pr_bitmap);
    bullet_obj[num++] = process_object(&um_obj, um_bitmap);
    num_bullet_objs = num;


}

//////////////////////////////////////////////////////////////////////////////j
static void
draw_picture_gtk4 (cairo_t *cr, int x, int y, Picture *picture) {
    gdk_cairo_set_source_pixbuf (cr,
                                 picture->pixbuf,
                                 x - picture->offset_x,
                                 y - picture->offset_y);
    cairo_paint (cr);
}

static void
draw_object (cairo_t *cr, int x, int y, Object *object, int pic) {

    // TODO Add bounds checking for pic.
    Picture *picture = &object->pic[pic];
    draw_picture_gtk4(cr, x, y, picture);
}

//////////////////////////////////////////////////////////////////////////////
// gtk4.c - to be created
#define NO_TEXT_CLIP

// Defined in graphics.h
// #define L_FONT 0
// #define WHITE 0

void
draw_text_gtk4 (cairo_t *cr, int x, int y, char *str, int font, int color) {

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); // white
    cairo_move_to (cr, x, y);
    cairo_show_text (cr, str);

}

// Draws the specified view of the object with the string written beneath
// at the specified location in the animation window.  The adj parameter
// is added to the picture coordinates but not the text coordinates.
// TODO Fix me!
void
draw_picture_string_gtk4 (cairo_t *cr, Object *obj, int view, char *str, int x, int y, int adj) {
    draw_picture_gtk4(cr, x + adj, y + adj, &obj->pic[view]);
    //if (str[0] != '\0')
    //    draw_text_left(ANIM_WIN, x + TEXT_OFFSET, y - font_height(M_FONT) / 2,
    //str, M_FONT, DRAW_COPY, WHITE);
}

#define PIC_X 200
#define PIC_Y 20

void
draw_objs_gtk4(cairo_t *cr,
               Object *obj[],
               Boolean text,
               int first,
               int last,
               int view,
               int x,
               int y,
               int height) {


    int i;

    for (i = first; i < last; i++)
        if (view < obj[i]->num_pics)
            draw_picture_string_gtk4,(obj[i], view, (text ? obj[i]->type : ""),
                                      x + PIC_X,
                                y + PIC_Y + height * (i - first), 0);

    // cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); // white
    // cairo_move_to (cr, x, y);
    // cairo_show_text (cr, str);

}
//////////////////////////////////////////////////////////////////////////////
// display.c

#define BULLET_X 180
#define VEH_Y 0

void
display_pics_gtk4(cairo_t *cr) {

    // Put in separator rectangles between the pictures */
    // draw_filled_rect(ANIM_WIN, BULLET_X - 1, 0, 3, EXP_Y, DRAW_COPY, WHITE);
    // draw_filled_rect(ANIM_WIN, LAND_X - 1, 0, 3, EXP_Y, DRAW_COPY, WHITE);
    // draw_filled_rect(ANIM_WIN, 0, EXP_Y - 1, ANIM_WIN_WIDTH, 3, DRAW_COPY,
    //                 WHITE);

    // Draw the vehicles in one column
    draw_text_gtk4(cr, BULLET_X / 2, VEH_Y + 5, "Vehicles", L_FONT, WHITE);
//    draw_objs_gtk4(cr, vehicle_objs, TRUE, 0, num_vehicle_objs, 0, VEH_X, VEH_Y, VEHICLE_H);

}

//////////////////////////////////////////////////////////////////////////////
        int cticks = 0;

    static void
        draw_function (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
        int tik;
        int x;
        int y;
        int vskip;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); // black
    cairo_paint (cr);

    display_pics_gtk4(cr);

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); // white

    // cairo_select_font_face(cr, "Purisa",
    //                        CAIRO_FONT_SLANT_NORMAL,
    // CAIRO_FONT_WEIGHT_BOLD);
    cairo_select_font_face(cr, "cairo:sans-serif",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size(cr, 13);
    cairo_move_to(cr, 20, 30);
    cairo_show_text(cr, "Most relationships seem so transitory");
    cairo_stroke(cr);


    tik = (cticks / 5)%4;
    x=100; y=100;
    draw_object(cr, x, y, &medusa_obj, tik);

    tik = (cticks / 5)%9;
    x = 50; y = 50;
    draw_object(cr, x, y, &circle_obj, tik);

    tik=0;

    x=200; y=40;
    vskip=60;
    for (int i=0; i<num_vehicle_objs; i++){
        draw_object(cr, x, y, vehicle_obj[i], 0);
        y+=vskip;
    }

    x=400; y=40;
    vskip=20;
    for (int i=0; i<num_bullet_objs; i++){
        draw_object(cr, x, y, bullet_obj[i], 0);
        y+=vskip;
    }

    x=600; y=40;
    vskip=20;
    for (int i=0; i<num_exp_objs; i++){
        draw_object(cr, x, y, exp_obj[i], 0);
        y+=vskip;
    }



    cticks++;
}

//////////////////////////////////////////////////////////////////////////////
gboolean time_handler(GtkWidget* widget) {
    //if (widget->window == NULL) return FALSE;

    //step = data_step_random(data, step);
     // step = data_step_full(data, step);
     return TRUE;
}

gboolean time_handler2(GtkWidget* widget) {
    gtk_widget_queue_draw(widget);

    return TRUE;
}


static void
app_activate (GApplication *app, gpointer user_data) {
    // Main Window
    GtkWidget  *win;
    GtkWidget  *area = gtk_drawing_area_new ();
    GtkBuilder *build;

    build = gtk_builder_new_from_resource("/org/mawsonlakes/xtank/xtank.ui");
    win = GTK_WIDGET(gtk_builder_get_object(build, "win"));

    // Main: add drawable Area
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw_function, NULL, NULL);
    gtk_window_set_child(GTK_WINDOW(win), area);

    gtk_window_set_application(GTK_WINDOW(win), GTK_APPLICATION(app));

    // Initialisation
    // data = data_init();

    // Statistics - required prior to calculating rate stats
    // start_time initially setup elseware
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        printf("*** ERROR Unable to get CLOCK_REALTIME\n");
    }
    /* seconds, multiplied with 1 million */
    end_time = tms.tv_sec * 1000000;
    /* Add full microseconds */
    end_time += tms.tv_nsec/1000;
    /* round up if necessary */
    if (tms.tv_nsec % 1000 >= 500) {
        ++end_time;
    }
    start_time = end_time;
    begin_time = end_time;

    end_steps = 0;
    start_steps = end_steps;

    // Timer Event
    // g_timeout_add(1,  (GSourceFunc) time_handler, (gpointer) area);
    g_idle_add((GSourceFunc)time_handler, (gpointer)area);
    g_timeout_add(20, (GSourceFunc)time_handler2, (gpointer)area);

    gtk_widget_show(win);
}

//////////////////////////////////////////////////////////////////////////////
int
InitConfigStruct(CLFkr *ConfigRunning)
{
	ConfigRunning->AutoExit = FALSE;
	ConfigRunning->AutoStart = FALSE;
	ConfigRunning->UseSetting = FALSE;
	ConfigRunning->PrintScores = FALSE;
	ConfigRunning->Settings = NULL;

	ConfigRunning->NoDelay = FALSE;
	ConfigRunning->NoIO = FALSE;

	return (0);
}
//////////////////////////////////////////////////////////////////////////////

int
main (int argc, char **argv) {
    GtkApplication *app;
    int stat;
    //////////////////////////////////////////////////////////////////////////
    // Setup - Copied from original xtank
    extern int num_terminals;
	extern char displayname[], video_error_str[];
	int iNumOpts = 0;
	int ret, i;

	bset = &actual_bset;
	eset = &actual_eset;

    // TODO: Fixup reading environment variables
    /*
    {
		extern char *network_error_str[];

		// Get environment variables
		debug("Getting environment variables");
		get_environment();
		executable_name = malloc((unsigned) strlen(argv[0]) + 1);
		(void) strcpy(executable_name, argv[0]);

		// If there are multiple display names, check to make sure all displays
        // are on same subnet, to avoid producing gateway traffic.
		if (argc > 1) {
			debug("Checking internet addresses");
			// ret = check_internet(argc - 1, argv + 1);
            ret = 0;
            if (ret) {
				fprintf(stderr, "%s", network_error_str[ret]);
				fprintf(stderr, "\n");
				if (ret == 5) {
					fprintf(stderr, "You can't play xtank with terminals that are far\n");
					fprintf(stderr, "away from each other, since the entire network\n");
					fprintf(stderr, "would slow down.\n");
				}
				rorre("Cannot continue.");
			}
		}
	}
    */

    InitConfigStruct(&command_options);

    // TODO: Convert command-line parsing to use GOption
    if (argc > 1) {
        int ctr;

        for (ctr = 1; ctr < argc; ctr++) {
            if (argv[ctr][0] == '-') {
                iNumOpts++;
                switch (argv[ctr][1]) {
                case 'h':
                case 'H':
#ifdef REALLY_RUDE
                    puts("Well, you really suck if it's taken");
                    puts("this long to get around to playing");
                    puts("xtank.   -ORACLE");
#else
                    puts("\nUSAGE:");
                    printf("   %s [-option]* [screens]*\n\n", argv[0]);
                    puts("where option can be one of the following:\n");
                    puts("    -s    == -S -F");
                    puts("    -i    == -I -X -P -D");
                    puts("    -x    == -X -P");
                    puts("    -h    == -H\n");
                    puts("    -F <filename>   => use settings file");
                    puts("    -X              => exit (auto)");
                    puts("    -P              => print scores");
                    puts("    -D              => no delay *");
                    puts("    -I              => no i/o(not specifiable) *");
                    puts("    -H              => this help screen");
                    puts("    -S              => auto-start");
                    puts("    -Z              => Zephyr Broadcast List *\n");
                    puts("    -V              => Version Information\n");
                    puts(" * Note implemented yet.\n");
#endif
                    return (0);
                    break;

                case 'I':
                    command_options.NoIO = TRUE;
                    break;

                case 'i':
                    command_options.NoIO = TRUE;
                    command_options.AutoExit = TRUE;
                    command_options.PrintScores = TRUE;
                    command_options.NoDelay = TRUE;
                    break;

                case 'D':
                    command_options.NoDelay = TRUE;
                    break;

                case 'V':
                    puts(version1);
                    puts(version2);
                    puts(version3);
                    puts("\nCompiled with the following options:");
                    puts(ALLDEFINES);
                    // printf("\nCompiled in XTANK_DIR: ");
                    // puts(XTANK_DIR);
                    return (0);
                    break;

                case 'S':
                    command_options.AutoStart = TRUE;
                    break;

                case 's':
                    command_options.AutoStart = TRUE;
                case 'F':
                    if (argc > ctr + 1) {
                        iNumOpts++;
                        command_options.UseSetting = TRUE;
                        command_options.Settings = argv[ctr + 1];
                    }
                    break;

                case 'P':
                    command_options.PrintScores = TRUE;
                    break;

                case 'X':
                    command_options.AutoExit = TRUE;
                    break;

                case 'x':
                    command_options.AutoExit = TRUE;
                    command_options.PrintScores = TRUE;
                    break;
				}
			}
		}
#ifdef DEBUG_ARGUMENTS
		puts("AutoExit is:");
		puts((command_options.AutoExit) ? "ON" : "OFF");
		puts("PrintScores is:");
		puts((command_options.PrintScores) ? "ON" : "OFF");
		puts("AutoStart is:");
		puts((command_options.AutoStart) ? "ON" : "OFF");
		puts("UseSetting is:");
		puts((command_options.UseSetting) ? "ON" : "OFF");
#endif
	}
	/* Initialize various and sundry things */
	debug("Initializing various things");
	// init_random();
	// init_prog_descs();
	// init_vdesign();
	// init_settings();
	// init_threader();
	// init_msg_sys();
	// init_box_names();	/* Init box names here; display_pics() may */
    /* use them before setup_game() is called */

	/* Rotate vehicle objects */
	debug("Rotating vehicle objects");
	// rotate_objects();

	/* Open the graphics toolkit */
	debug("Opening graphics toolkit");
	// open_graphics();

	/* Parse command line for display names and make a terminal for each one */
	debug("Making terminals");
    /*
      if (argc - iNumOpts > 1) {
      for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
      if (argv[i][1] == 's' || argv[i][1] == 'F') {
      i++;
      }
      continue;
      }
      if (process_terminal(argv[i]))
      rorre(video_error_str);
      }
      } else if (make_terminal(displayname))
		rorre(video_error_str);

	if (num_terminals == 0)
		rorre("No terminals opened.  Cannot continue program.");
    */

#ifdef SOUND
	init_sounds();
#endif /* SOUND */

	/* Load descriptions after determining num_vehicle_objs */
	debug("Loading vehicle and maze descriptions");
	// load_desc_lists();

	/* Ask each terminal for player name and vehicle name */
	debug("Getting player info");
    /*
	for (i = 0; i < num_terminals; i++) {
		set_terminal(i);
		get_player_info();
	}
    */

	/* Go to main interface */
	debug("Entering main interface");
	// main_interface();

	/* Close up shop in a friendly way */
	//free_everything();

	//return (0);

    // Test
    // Evaluate graphical objects
    process_objects();

    //////////////////////////////////////////////////////////////////////////
    app = gtk_application_new (APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    stat = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return stat;
}
