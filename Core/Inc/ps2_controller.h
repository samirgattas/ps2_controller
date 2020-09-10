/*
 * ps2_controller.h
 *
 *  Created on: 9 sept. 2020
 *      Author: xx
 */

#ifndef INC_PS2_CONTROLLER_H_
#define INC_PS2_CONTROLLER_H_

#include "main.h"
#include "PS2X_lib.h"


int8_t conectar_ps2();
_Bool acusar_botones();

void ps2_controller();


#endif /* INC_PS2_CONTROLLER_H_ */
