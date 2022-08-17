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
#include "display/objects/bullets.obj"
#include "display/objects/circle.obj"
#include "display/objects/medusa.obj"


//////////////////////////////////////////////////////////////////////////////
static void
process_object(Object *object, unsigned char **pixdata) {
    int pic = 0;
    fprintf(stderr,"-----------------------\n");
    fprintf(stderr, "type:        %s\n", (char *)object->type);
    fprintf(stderr, "num_pics:    %d\n", object->num_pics);
    fprintf(stderr, "num_turrets: %d\n", object->num_turrets);
    fprintf(stderr, "num_segs:    %d\n", object->num_segs);
    fprintf(stderr,"-----------------------\n");

    GdkPixbuf *pix_buffer;

    for (int pic=0; pic<object->num_pics; pic++) {
        Picture *picture = &object->pic[pic];
        unsigned char *data = pixdata[pic];

        int w = picture->width;
        int h = picture->height;
        int byte_width = w/8 + 1;

        // Create new pixbuf
        pix_buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, w, h);
        fprintf(stderr, "rowstride: %d\n", gdk_pixbuf_get_rowstride(pix_buffer));
        fprintf(stderr, "n_channels: %d\n", gdk_pixbuf_get_n_channels(pix_buffer));
        fprintf(stderr, "w: %d (%d)  h: %d\n", w, byte_width, h);

        fprintf(stderr, "rows: ");
        for (int j=0; j<h; j++){
            fprintf(stderr, ".");
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
        fprintf(stderr,"\n");
        picture->pixbuf = pix_buffer;
    } // loop: pic
}


//////////////////////////////////////////////////////////////////////////////j
static void
draw_object (cairo_t *cr, int x, int y, Object *object, int pic) {

    GdkPixbuf *pix_buffer;
    Picture *picture = &object->pic[pic];

    pix_buffer = picture->pixbuf;
    gdk_cairo_set_source_pixbuf (cr,
                                 pix_buffer,
                                 x - picture->offset_x,
                                 y - picture->offset_y);
    cairo_paint (cr);

}


//////////////////////////////////////////////////////////////////////////////j
int cticks = 0;

static void
draw_function (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    int tik;
    int x;
    int y;

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */
    cairo_paint (cr);

    // cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    // cairo_set_line_width (cr, 1.0);

    // cairo_arc (cr, 100, 100, 10, 0, 2 * M_PI);
    // cairo_stroke (cr);

    tik = (cticks / 5)%4;
    x=100; y=100;
    draw_object(cr, x, y, &medusa_obj, tik);

    tik = (cticks / 5)%9;
    x = 50; y = 50;
    draw_object(cr, x, y, &circle_obj, tik);

    tik=0;
    x=400; y=100;
    int vskip=20;

    draw_object(cr, x, y, &lmg_obj, 0);
    y+=vskip; draw_object(cr, x, y, &mg_obj, 0);
    y+=vskip; draw_object(cr, x, y, &hmg_obj, 0);

    y+=vskip; draw_object(cr, x, y, &lac_obj, 0);
    y+=vskip; draw_object(cr, x, y, &ac_obj, 0);
    y+=vskip; draw_object(cr, x, y, &hac_obj, 0);

    y+=vskip; draw_object(cr, x, y, &lrl_obj, 0);
    y+=vskip; draw_object(cr, x, y, &rl_obj, 0);
    y+=vskip; draw_object(cr, x, y, &hrl_obj, 0);

    y+=vskip; draw_object(cr, x, y, &as_obj, 0);
    y+=vskip; draw_object(cr, x, y, &ft_obj, 0);
    y+=vskip; draw_object(cr, x, y, &seek_obj, 0);
    y+=vskip; draw_object(cr, x, y, &pr_obj, 0);
    y+=vskip; draw_object(cr, x, y, &um_obj, 0);



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
      if (make_terminal(argv[i]))
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
    process_object(&lmg_obj, lmg_bitmap);
    process_object(&mg_obj,  mg_bitmap);
    process_object(&hmg_obj, hmg_bitmap);

    process_object(&lac_obj, lac_bitmap);
    process_object(&ac_obj,  ac_bitmap);
    process_object(&hac_obj, hac_bitmap);

    process_object(&lrl_obj, lrl_bitmap);
    process_object(&rl_obj,  rl_bitmap);
    process_object(&hrl_obj, hrl_bitmap);

    process_object(&as_obj,   as_bitmap);
    process_object(&ft_obj,   ft_bitmap);
    process_object(&seek_obj, seek_bitmap);
    process_object(&pr_obj,   pr_bitmap);
    process_object(&um_obj,   um_bitmap);

    process_object(&medusa_obj, medusa_bitmap);
    process_object(&circle_obj, circle_bitmap);

    //////////////////////////////////////////////////////////////////////////
    app = gtk_application_new (APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    stat = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return stat;
}
