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

#ifndef INCLUDED_X11_H
#define INCLUDED_X11_H

#include <assert.h>
#include <X11/Xlib.h>


#ifdef lint
/* stupid X11/Xlib.h leaves these undefined, which makes lint complain: */
  struct _XrmHashBucketRec {
	  int x;
  };
  struct XKeytrans {
	  int x;
  };

#endif


#define WIN_exposed	(1<<0)
#define WIN_mapped      (1<<1)

#define DRAW_XOR	0
#define DRAW_COPY	1
#define MAX_DRAW_FUNCS	2

extern Boolean un_stingy;

/*********************
** Window functions **
*********************/

#define win_exposed(w) \
   (vid->win[w].flags & WIN_exposed)

#define win_mapped(w) \
   (vid->win[w].flags & WIN_mapped)

#define expose_win(w,status) \
  (status ? (vid->win[w].flags |= WIN_exposed) : \
            (vid->win[w].flags &= ~WIN_exposed))

#define color_display() \
  (vid->planes > 1)

/**********************
** Drawing functions **
**********************/

#ifdef BATCH_POINTS

#define BATCHPDEPTH 50

extern XPoint pointBatch[MAX_COLORS][BATCHPDEPTH];
extern int pointsBatched[];
extern int pointBatchFunc;
extern int pointBatchWin;

#endif

#ifdef USE_BATCHED_POINTS
#define flush_point_batch \
   { \
      int col; \
      for (col = 0; col < MAX_COLORS; ++col) { \
         if (pointsBatched[col]) { \
            XDrawPoints(vid->dpy, \
                 vid->win[pointBatchWin].id, \
                 vid->graph_gc[pointBatchFunc][col], \
                 &pointBatch[col][0], \
                 pointsBatched[col], \
                 CoordModeOrigin); \
            pointsBatched[col] = 0; \
         } \
      } \
   }
#else
#define flush_point_batch
#endif

#ifndef USE_BATCHED_POINTS

#define draw_point(w,x,y,func,color) \
  (XDrawPoint(vid->dpy,vid->win[w].id,vid->graph_gc[func][color],x,y))

#else

#define draw_point(w,xx,yy,func,color) \
	{ \
		if ( (w != pointBatchWin) || (func != pointBatchFunc) || (pointsBatched[color] == BATCHPDEPTH) ) { \
			flush_point_batch; \
			pointBatchWin = w; \
			pointBatchFunc = func; \
		}  \
		pointBatch[color][pointsBatched[color]].x = xx; \
		pointBatch[color][pointsBatched[color]].y = yy; \
		++pointsBatched[color]; \
	}
#endif

#define draw_points(w,points,num_points,x,y,func,color) \
  (XDrawPoints(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
	      points,num_points,CoordModeOrigin))

#ifdef BATCH_LINES

#define BATCHDEPTH 80

#ifdef BATCH_COLOR_LINES
extern XSegment lineBatch[MAX_COLORS][BATCHDEPTH];
extern int linesBatched[MAX_COLORS];

#else /* BATCH_COLOR_LINES */
extern XSegment lineBatch[];
extern int linesBatched;
extern int lineBatchColor;

#endif /* BATCH_COLOR_LINES */
extern int lineBatchFunc;
extern int lineBatchWin;

#endif

#ifdef USE_BATCHED_LINES
#ifndef BATCH_COLOR_LINES
#define flush_line_batch \
   if (linesBatched) { \
      XDrawSegments(vid->dpy, \
                 vid->win[lineBatchWin].id, \
                 vid->graph_gc[lineBatchFunc][lineBatchColor], \
                 lineBatch, \
                 linesBatched); \
      linesBatched = 0; \
  }
#else /* BATCH_COLOR_LINES */
#define flush_line_batch \
   { \
      int col; \
      for (col = 0; col < MAX_COLORS; ++col) { \
         if (linesBatched[col]) { \
            XDrawSegments(vid->dpy, \
                 vid->win[lineBatchWin].id, \
                 vid->graph_gc[lineBatchFunc][col], \
                 lineBatch[col], \
                 linesBatched[col]); \
      linesBatched[col] = 0; \
         } \
      } \
   }
#endif /* BATCH_COLOR_LINES */
#else
#define flush_line_batch
#endif

#ifndef USE_BATCHED_LINES

#define draw_line(w,x1,y1,x2,y2,func,color) \
  (XDrawLine(vid->dpy,vid->win[w].id,vid->graph_gc[func][color],x1,y1,x2,y2))

#else

#ifdef BATCH_COLOR_LINES

#define draw_line(w,xx1,yy1,xx2,yy2,func,color) \
   { \
      if ( (w != lineBatchWin) || (func != lineBatchFunc) || (linesBatched[color] == BATCHDEPTH) ) { \
         flush_line_batch; \
         lineBatchWin = w; \
         lineBatchFunc = func; \
      }  \
      lineBatch[color][linesBatched[color]].x1 = xx1; \
      lineBatch[color][linesBatched[color]].y1 = yy1; \
      lineBatch[color][linesBatched[color]].x2 = xx2; \
      lineBatch[color][linesBatched[color]].y2 = yy2; \
      ++linesBatched[color]; \
   }

#else /* BATCH_COLOR_LINES */

#define draw_line(w,xx1,yy1,xx2,yy2,func,color) \
   { \
      if ( (w != lineBatchWin) || (func != lineBatchFunc) || (color != lineBatchColor) || (linesBatched == BATCHDEPTH) ) { \
         flush_line_batch; \
         lineBatchWin = w; \
         lineBatchFunc = func; \
         lineBatchColor = color; \
      }  \
      lineBatch[linesBatched].x1 = xx1; \
      lineBatch[linesBatched].y1 = yy1; \
      lineBatch[linesBatched].x2 = xx2; \
      lineBatch[linesBatched].y2 = yy2; \
      ++linesBatched; \
   }

#endif /* BATCH_COLOR_LINES */

#endif

#define draw_lines(w,lines,num_lines,func,color) \
  (XDrawSegments(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
	     lines,num_lines,CoordModeOrigin))

#ifndef USE_BATCHED_LINES

#define draw_vert(w,x,y,size,func,color) \
  (draw_line(w,x,y,x,y+size-1,func,color))

#define draw_hor(w,x,y,size,func,color) \
  (draw_line(w,x,y,x+size-1,y,func,color))

#else

#define draw_vert(w,x,y,size,func,color) draw_line(w,x,y,x,y+size-1,func,color)

#define draw_hor(w,x,y,size,func,color) draw_line(w,x,y,x+size-1,y,func,color)

#endif

#define draw_rect(w,x,y,width,height,func,color) \
    (XDrawR//ectangle(vid->dpy,vid->win[w].id,vid->graph_gc[func][color],  \
		  x,y,width,height))

#define draw_filled_rect(w,x,y,width,height,func,color) \
  (XFillRectangle(vid->dpy,vid->win[w].id,vid->graph_gc[func][color], \
		  x,y,width,height))

#define draw_square(w,x,y,size,func,color) \
  (draw_rect(w,x,y,size,size,func,color))

#define draw_filled_square(w,x,y,size,func,color) \
  (draw_filled_rect(w,x,y,size,size,func,color))

#define draw_picture(w,x,y,pic,func,color) \
  { \
    int tmp_x = x - (pic)->offset_x; \
    int tmp_y = y - (pic)->offset_y; \
      if(x > -(pic)->offset_x && tmp_x < vid->win[w].width && \
         y > -(pic)->offset_y && tmp_y < vid->win[w].height) { \
	XCopyPlane(vid->dpy, vid->pixid[(pic)->pixmap], vid->win[w].id, \
                   vid->graph_gc[func][color],0,0, \
		   vid->width[(pic)->pixmap], vid->height[(pic)->pixmap], \
                   tmp_x,tmp_y, 1);\
      } \
 }

  typedef struct {
	  Window id;
	  int width, height;
	  int flags;
  }
Win;

  struct Kludge_Defaults {
	  char player_name[MAX_STRING + 1];
	  char tank_name[MAX_STRING + 1];
	  int wants_keypad;
	  int mouse_speed;
	  int mouse_drive;
	  int mouse_heat;
	  char keymap[MAX_STRING +1];
  };

  typedef struct {
	  char display_name[50];
	  Display *dpy;
	  int planes;
	  long input_mask;
	  Window parent_id;
	  int num_windows;
	  Win win[MAX_WINDOWS];
	  GC graph_gc[MAX_DRAW_FUNCS][MAX_COLORS];
	  GC text_gc[MAX_FONTS][MAX_DRAW_FUNCS][MAX_COLORS];
	  XFontStruct *fs[MAX_FONTS];
	  int num_pixids;
	  Pixmap pixid[MAX_PIXMAPS];
	  int width[MAX_PIXMAPS], height[MAX_PIXMAPS];
	  unsigned long fg, bg, color[MAX_COLORS];
	  Cursor cursor[MAX_CURSORS];
	  int escher_width;
	  int last_expose_frame;
	  unsigned int beep_flag:1;
	  unsigned int display_names_flag:1;
	  struct Kludge_Defaults kludge;
	  int dest_wall;
	  int rdf_green;
  }
Video;

/*
 * Pseudo colors, depends on set_video being
 * called to set vid                 --ane
 */

#define DEST_WALL vid->dest_wall
#define RDF_SAFE vid->rdf_green

extern Video *vid;

#endif
