/*
  Filename: xtank.c
  Version: NG

  Author: Paul Schulz <paul@mawsonlakes.org>
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
#define MAXCOLORS 18
int colors[MAXCOLORS][3] = {{255,  0,  0},   // red
                            {255, 85,  0},
                            {255,170,  0},
                            {255,255,  0},
                            {170,255,  0},
                            { 85,255,  0},
                            {  0,255,  0},   // green
                            {  0,255, 85},
                            {  0,255,170},
                            {  0,255,255},
                            {  0,170,255},
                            {  0, 85,255},
                            {  0,  0,255},   // blue
                            { 85,  0,255},
                            {170,  0,255},
                            {255,  0,255},
                            {255,  0,170},
                            {255,  0, 85}};
// Darker  colors
int colors2[MAXCOLORS][3] = {{127,   0,   0},   // red
                             {127,  42,   0},
                             {127,  85,   0},
                             {127, 127,   0},
                             { 85, 127,   0},
                             { 42, 127,   0},
                             {  0, 127,   0},   // green
                             {  0, 127,  42},
                             {  0, 127,  85},
                             {  0, 127, 127},
                             {  0,  85, 127},
                             {  0,  42, 127},
                             {  0,   0, 127},   // blue
                             { 42,   0, 127},
                             { 85,   0, 127},
                             {127,   0, 127},
                             {127,   0,  85},
                             {127,   0,  42}};
// Darkest  colors
int colors3[MAXCOLORS][3] = {{ 63,   0,   0},   // red
                             { 63,  21,   0},
                             { 63,  43,   0},
                             { 63,  63,   0},
                             { 43,  63,   0},
                             { 21,  63,   0},
                             {  0,  63,   0},   // green
                             {  0,  63,  21},
                             {  0,  63,  43},
                             {  0,  63,  63},
                             {  0,  43,  63},
                             {  0,  21,  63},
                             {  0,   0,  63},   // blue
                             { 21,   0,  63},
                             { 43,   0,  63},
                             { 63,   0,  63},
                             { 63,   0,  43},
                             { 63,   0,  21}};

//////////////////////////////////////////////////////////////////////////////

static void
draw_function (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {

    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_set_line_width (cr, 1.0);
    // cairo_move_to (cr, 100, 100);

    cairo_arc (cr, 100, 100, 10, 0, 2 * M_PI);
    cairo_stroke (cr);
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

int
main (int argc, char **argv) {
    GtkApplication *app;
    int stat;

    app = gtk_application_new (APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    stat = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return stat;
}
