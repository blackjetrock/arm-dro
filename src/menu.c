/*
 * menu.c
 *
 *  Created on: 13 Aug 2016
 *      Author: menadue
 */

#include <stdlib.h>
#include <string.h>

#include "i2c_functions.h"
#include "oled096.h"
#include "tft_il9341.h"
#include "menu.h"
#include "arm_dro.h"

// Settings that the menu sets up

int demo_mode = 0;
int draw_button_box = 0;
int px = 0;
int py = 0;
int pz = 0;
int is_mill = 0;
int is_lathe = 1;


struct MENU_ELEMENT *current_menu;
struct MENU_ELEMENT *last_menu=NULL;
struct MENU_ELEMENT *the_home_menu;

void to_back_menu(struct MENU_ELEMENT *e)
{
  current_menu = last_menu;
  draw_menu(current_menu, last_menu, 0);
}


void to_home_menu(struct MENU_ELEMENT *e)
{
  current_menu = the_home_menu;
  draw_menu(current_menu, last_menu, 0);
}

void send_t_request1(struct MENU_ELEMENT *e)
{

}

void send_t_request2(struct MENU_ELEMENT *e)
{

}

void send_hb_status_request(struct MENU_ELEMENT *e)
{

}


void send_hb_up_request(struct MENU_ELEMENT *e)
{

}

void send_hb_down_request(struct MENU_ELEMENT *e)
{

}

void send_hb_hol_on(struct MENU_ELEMENT *e)
{

}

void send_hb_hol_off(struct MENU_ELEMENT *e)
{

}

void set_boxes_on(struct MENU_ELEMENT *e)
{
  draw_button_box = 1;
}

void set_boxes_off(struct MENU_ELEMENT *e)
{
  draw_button_box = 0;
}

void demo_on(struct MENU_ELEMENT *e)
{
  demo_mode = 1;
}

void demo_off(struct MENU_ELEMENT *e)
{
  demo_mode = 0;
}

void set_px(struct MENU_ELEMENT *e)
{
  px = gauge_x-offset_x;
  oled_set_xy(&oled0, 0,0);
  oled_display_string(&oled0, "PX : ");
  oled_display_int(&oled0, px, 5);
}

void set_py(struct MENU_ELEMENT *e)
{
  py = gauge_y-offset_y;
  oled_set_xy(&oled1, 0,0);
  oled_display_string(&oled1, "PY : ");
  oled_display_int(&oled1, py, 5);
}

void set_pz(struct MENU_ELEMENT *e)
{
  pz = gauge_z-offset_z;
  oled_set_xy(&oled2, 0, 0);
  oled_display_string(&oled2, "PZ : ");
  oled_display_int(&oled2, pz, 5);
}

// Centres X.
// X is zeroed at A
// X is then moved to B
// Centre X then sets the offset to the original offset plus half of the current value displayed

void centre_x(struct MENU_ELEMENT *e)
{
  offset_x += (gauge_x-offset_x)/2;
}

void centre_y(struct MENU_ELEMENT *e)
{
  offset_y += (gauge_y-offset_y)/2;
}

void centre_z(struct MENU_ELEMENT *e)
{
  offset_z += (gauge_z-offset_z)/2;
}

void sum_x_xy(struct MENU_ELEMENT *e)
{
  sum_mode_x = SUM_MODE_X_XY;
  oled_display_scaled_string_xy(&oled0, "X+Y", 10, 20, AXIS_SCALE);
}

void sum_x_x(struct MENU_ELEMENT *e)
{
  sum_mode_x = SUM_MODE_X_X;
  oled_display_scaled_string_xy(&oled0, " X ", 10, 20, AXIS_SCALE);
}

void sum_x_xz(struct MENU_ELEMENT *e)
{
  sum_mode_x = SUM_MODE_X_XZ;
  oled_display_scaled_string_xy(&oled0, "X+Z", 10, 20, AXIS_SCALE);
}

void sum_y_y(struct MENU_ELEMENT *e)
{
  sum_mode_y = SUM_MODE_Y_Y;
  oled_display_scaled_string_xy(&oled1, " Y ", 10, 20, AXIS_SCALE);
}

void sum_y_yz(struct MENU_ELEMENT *e)
{
  sum_mode_y = SUM_MODE_Y_YZ;
  oled_display_scaled_string_xy(&oled1, "Y+Z", 10, 20, AXIS_SCALE);
}

extern unsigned char gauge_data[4];

char hex_to_str(int c)
{
  char a;
  if ( c > 9 )
    {
      a = 'A'+ (c - 9);
    }
  else
    {
      a = '0' + c;
    }

  return a;
}

void insert_hex_byte(int x, char *dest)
{
  *dest = hex_to_str(x >> 8);
  *(dest+1) = hex_to_str(x &0xf);
}

void set_as_mill(struct MENU_ELEMENT *e)
{
  is_mill = 1;
  is_lathe = 0;
}

void set_as_lathe(struct MENU_ELEMENT *e)
{
  is_mill = 0;
  is_lathe = 1;
}

void show_gauge_data(struct MENU_ELEMENT *e)
{
  char gs[20] = "AAAAAAAAAAAAAAAAAAA";
  tft_drawString(100, 180, gs, ILI9341_WHITE);
  tft_drawBox (10, 10, 20, 20, ILI9341_WHITE);

  insert_hex_byte(gauge_data[0], &(gs[0]));
  insert_hex_byte(gauge_data[1], &(gs[2]));
  insert_hex_byte(gauge_data[2], &(gs[4]));
  insert_hex_byte(gauge_data[3], &(gs[6]));
  gs[8] =':';
  gs[9] ='\0';
  tft_drawString(100, 80, gs, ILI9341_WHITE);
}

#define COLOR_W_BLUE  ILI9341_WHITE
#define COLOR_W_BLACK ILI9341_BLACK

struct MENU_ELEMENT elem_mill[] =
  {
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Set Mill",  NULL, set_as_mill,              0,    0,  150, 179},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "BCK",       NULL, to_home_menu,             0,  180,  150, 230},
    {COLOR_W_BLUE,  MENU_END,       "button",    NULL, NULL,   0,   0,   0,   0},
  };

struct MENU_ELEMENT elem_lathe[] =
  {
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "X=SUM X,Y", NULL, sum_x_xy,                 0,    0,  150,  59},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "X=X",       NULL, sum_x_x,                  0,   60,  150, 119},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Set Lathe", NULL, set_as_lathe,             0,  120,  150, 179},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "BCK",       NULL, to_home_menu,             0,  180,  150, 230},
    {COLOR_W_BLUE,  MENU_END,       "button",    NULL, NULL,   0,   0,   0,   0},
  };

// Garage controller
struct MENU_ELEMENT elem_demo[] =
  {
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "DEMO ON",   NULL, demo_on,           0,   0,  200,   59},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "DEMO OFF",  NULL, demo_off,          0,  60,  200,  119},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "BACK",      NULL, to_home_menu,      0, 120,  200,  159},
    {COLOR_W_BLUE,   MENU_END,       "",          NULL, NULL,              0,   0,    0,    0},
  };

struct MENU_ELEMENT elem_setup[] =
  {
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "MILL",     (void *)&(elem_mill[0]),          0,    0,  149,   39},
    {COLOR_W_BLUE,   SUB_MENU,       "LATHE",    (void *)&(elem_lathe[0]), NULL,   0,   40,  149,   79},
    {COLOR_W_BLUE,   SUB_MENU,       "Demo",     (void *)&(elem_demo[0]),  NULL,   0,   80,  149,  119},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "Boxes Off", NULL,     set_boxes_off,       151,    0,  300,   39},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "Boxes On",  NULL,      set_boxes_on,       151,   40,  300,   79},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "BACK",     NULL, to_home_menu,               0,  120,  149,  159},

    {COLOR_W_BLUE,   MENU_END,       "",         NULL, NULL,              0,   0,   0,   0},
  };

struct MENU_ELEMENT elem_sum[] =
  {
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "X+Y",     NULL, sum_x_xy,                    0,    0,  149,   37},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "X+Z",     NULL, sum_x_xz,                    0,   40,  149,   77},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "X=X",     NULL, sum_x_x,                     0,   80,  149,   117},

    {COLOR_W_BLUE,   BUTTON_ELEMENT, "Y+Z",     NULL, sum_y_yz,                  155,   40,  300,   77},
    {COLOR_W_BLUE,   BUTTON_ELEMENT, "Y=Y",     NULL, sum_y_y,                   155,   80,  300,   117},

    {COLOR_W_BLUE,   BUTTON_ELEMENT, "BACK",     NULL, to_home_menu,               0,  120,  300,  157},

    {COLOR_W_BLUE,   MENU_END,       "",         NULL, NULL,              0,   0,   0,   0},
  };

struct MENU_ELEMENT elem_temp[] =
  {
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Mill",   NULL, set_as_mill,       0,   0, 100,  50},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Lathe",  NULL, set_as_lathe,      0,  60, 100, 110},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "BACK",   NULL, to_home_menu,      0,  120,  100,  170},
    {COLOR_W_BLUE,  MENU_END,       "",       NULL, NULL,              0,   0,   0,   0},
  };

struct MENU_ELEMENT elem_layout[] =
  {
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Centre X",  NULL, centre_x,       0,    0,  100,  59},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Centre Y",  NULL, centre_y,       0,   60,  100, 119},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Centre Z",  NULL, centre_z,       0,  120,  100, 179},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "BACK",   NULL, to_home_menu,      0,  180,  100, 230},
    {COLOR_W_BLUE,  MENU_END,       "button",    NULL, NULL,   0,   0,   0,   0},
  };

struct MENU_ELEMENT elem_debug[] =
  {
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "Show Gauge Data",  NULL, show_gauge_data, 0,    0,  300,  100},
    {COLOR_W_BLUE,  BUTTON_ELEMENT, "BACK",             NULL, to_home_menu,      0,  180,  100, 230},
    {COLOR_W_BLUE,  MENU_END,       "button",           NULL, NULL,   0,   0,   0,   0},
  };

struct MENU_ELEMENT home_menu[] =
   {
     {COLOR_W_BLUE,  SUB_MENU,       "Setup",  (void *)&(elem_setup[0]),  NULL,    0,   60,  150,  110},
     {COLOR_W_BLUE,  SUB_MENU,       "Layout", (void *)&(elem_layout[0]), NULL,    0,  120,  150,  170},
     {COLOR_W_BLUE,  SUB_MENU,       "Sums",   (void *)&(elem_sum[0]),    NULL,  151,  120,  300,  170},
     {COLOR_W_BLUE,  SUB_MENU,       "DRO",    (void *)&(elem_temp[0]),   NULL,    0,  180,  150,  230},
     {COLOR_W_BLUE,  BUTTON_ELEMENT, "Set PX", NULL,                    set_px,  151,    0,  300,   39},
     {COLOR_W_BLUE,  BUTTON_ELEMENT, "Set PY", NULL,                    set_py,  151,   40,  300,   79},
     {COLOR_W_BLUE,  BUTTON_ELEMENT, "Set PZ", NULL,                    set_pz,  151,   80,  300,  119},
     {COLOR_W_BLUE,  MENU_END,       "",       NULL,                      NULL,    0,    0,    0,    0},
  };

int x, y;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";

#define X_INNER_OFFSET 4
#define Y_INNER_OFFSET 4

void undraw_button(int x, int y, int x1, int y1, char *text, int col)
{
  if( draw_button_box )
    {
    tft_drawBox (x, y, x1, y1, ILI9341_BLACK);
    }
  tft_drawString((x+x1)/2-strlen(text)*TSCALE*5, (y+y1)/2-3, text, ILI9341_BLACK);

}

void draw_button(int x, int y, int x1, int y1, char *text, int col)
{
  if( draw_button_box )
    {
      tft_drawBox (x, y, x1, y1, col);
    }

  tft_drawString((x+x1)/2-strlen(text)*TSCALE*5, (y+y1)/2-3, text, col);

}

void draw_menu(struct MENU_ELEMENT *e, struct MENU_ELEMENT *last, int undraw)
{
  int i;
  struct MENU_ELEMENT *drawn_menu = e;

  // Clear screen by undrawing last menu

  //tft_fillScreen(ILI9341_BLACK);
  if( last != NULL )
    {
      draw_menu(last, NULL, 1);
    }

  while( e->type != MENU_END )
    {
      switch(e->type)
	{
	case BUTTON_ELEMENT:
	  if( undraw )
	    {
	      undraw_button(e->x, e->y, e->x1, e->y1, e->text, e->col);
	    }
	  else
	    {
	      draw_button(e->x, e->y, e->x1, e->y1, e->text, e->col);
	    }
	  break;

	case SUB_MENU:
	  if( undraw )
	    {
	      undraw_button(e->x, e->y, e->x1, e->y1, e->text, e->col);
	    }
	  else
	    {
	      draw_button(e->x, e->y, e->x1, e->y1, e->text, e->col);
	    }
	  break;
	}
      e++;
      i++;
    }

  if( !undraw )
    {
      last_menu = drawn_menu;
    }
}

int press_counter = 0;
struct MENU_ELEMENT *press_elem = NULL;
struct MENU_ELEMENT *pressed_button = NULL;
int new_press = 0;

#define PRESS_THRESHOLD 10

void process_menu(void)
{
  int x, y;
  struct MENU_ELEMENT *menu_start = current_menu;
  struct MENU_ELEMENT *p;

  // Process button presses
  if (TFT_Touch_Pressed())
    {
      x=TFT_Touch_X();
      y=TFT_Touch_Y();

      // See if this is within a button area
      for(p=menu_start; p->type != MENU_END; p++)
	{
	  if( (x>=p->x) && (x<=(p->x1)) && (y>=p->y) && (y<=(p->y1)))
	    {
	      // This button has a press
	      if( p == press_elem )
		{
		  // Still pressed
		  press_counter++;
		  if( press_counter >= PRESS_THRESHOLD )
		    {
		      press_counter = PRESS_THRESHOLD;
		    }
		  else
		    {

		    }
		}
	      else
		{
		  // New button
		  press_counter = 0;
		  press_elem = p;
		}
	    }
	}
    }
  else 
    {
      // Nothing pressed, decrement counter
      if( press_elem != NULL )
	{
	  if( press_counter > 0 )
	    {
	      press_counter--;
	      if( press_counter == 0 )
		{
		  // Button press registered
		  pressed_button = press_elem;
		  new_press = 1;
		  //		  myGLCD.printNumI(24, 200, 100);
		}
	    }
	}
    }

  //  myGLCD.printNumI(press_counter, 200, 200);
  //  myGLCD.printNumI((long int)pressed_button, 200, 120);

  if( new_press )
    {
      //      myGLCD.printNumI(31, 200, 170);   
      
      if( pressed_button->type == SUB_MENU )
	{
	  //last_menu = current_menu;
	  current_menu = (struct MENU_ELEMENT *)(pressed_button->submenu);
	  draw_menu(current_menu, last_menu, 0);
	  //	  myGLCD.printNumI(34, 200, 220);
	}
      
      if( pressed_button->function != NULL )
	{
	  (pressed_button->function)(pressed_button);
	  //	  myGLCD.printNumI(35, 200, 190);
	}

      new_press = 0;;
    }
  

}

void menu_setup(void)
{
  the_home_menu = &(home_menu[0]);
  current_menu = &(home_menu[0]);

  //  drawButtons();  
  draw_menu(current_menu, NULL, 0);
}


