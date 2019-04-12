/*
 * menu.h
 *
 *  Created on: 14 Aug 2016
 *      Author: menadue
 */

#ifndef MENU_H_
#define MENU_H_

enum ELEMENT_TYPE
  {
    BUTTON_ELEMENT = 10,
    SUB_MENU,
    MENU_END,
  };

struct MENU_ELEMENT
{
  int col;
  enum ELEMENT_TYPE type;
  char *text;
  void *submenu;
  void (*function)(struct MENU_ELEMENT *e);
  int x, y, x1, y1;
};


void draw_menu(struct MENU_ELEMENT *e, struct MENU_ELEMENT *last, int undraw);
void process_menu(void);
void menu_setup(void);

extern int demo_mode;
#endif /* MENU_H_ */
