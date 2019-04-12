/*
 * arm_dro.h
 *
 *  Created on: 25 Aug 2016
 *      Author: menadue
 */

#ifndef ARM_DRO_H_
#define ARM_DRO_H_

extern I2C_SLAVE_DESC oled0;
extern I2C_SLAVE_DESC oled1;
extern I2C_SLAVE_DESC oled2;

extern long int gauge_x;
extern long int gauge_y;
extern long int gauge_z;
extern long int offset_x;
extern long int offset_y;
extern long int offset_z;
extern int demo_mode;
extern int draw_button_box;
extern int is_lathe, is_mill;
extern int px, py, pz;

#define AXIS_SCALE 4

typedef enum _SUM_MODE 
  {
    SUM_MODE_X_X = 10,
    SUM_MODE_X_XY,
    SUM_MODE_X_XZ,
    SUM_MODE_Y_Y,
    SUM_MODE_Y_YZ,
  } SUM_MODE;

extern SUM_MODE sum_mode_x, sum_mode_y, sum_mode_z;

#endif /* ARM_DRO_H_ */
