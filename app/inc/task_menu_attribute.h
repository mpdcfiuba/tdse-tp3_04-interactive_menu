/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file   : task_menu_attribute.h
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

#ifndef TASK_INC_TASK_MENU_ATTRIBUTE_H_
#define TASK_INC_TASK_MENU_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/
/* Menu Statechart - State Transition Table */
/* 	------------------------+-----------------------+-----------------------+-----------------------+------------------------
 * 	| Current               | Event                 |                       | Next                  |                       |
 * 	| State                 | (Parameters)          | [Guard]               | State                 | Actions               |
 * 	|=======================+=======================+=======================+=======================+=======================|
 * 	| INICIAL               |                       |                       | ST_MEN_XX_IDLE        |                       |
 * 	|-----------------------+-----------------------+-----------------------+-----------------------+-----------------------|
 * 	| ST_MEN_XX_IDLE        | EV_MEN_MEN_ACTIVE     |                       | ST_MEN_XX_ACTIVE      |                       |
 * 	|                       |                       |                       |                       |                       |
 * 	|-----------------------+-----------------------+-----------------------+-----------------------+-----------------------|
 * 	| ST_MEN_XX_ACTIVE      | EV_MEN_MEN_IDLE       |                       | ST_MEN_XX_IDLE        |                       |
 * 	|                       |                       |                       |                       |                       |
 * 	------------------------+-----------------------+-----------------------+-----------------------+------------------------
 */

/* Events to excite Task Menu */
typedef enum task_menu_ev {EV_MEN_ENT_IDLE,
						   EV_MEN_ENT_ACTIVE,
						   EV_MEN_NEX_IDLE,
						   EV_MEN_NEX_ACTIVE,
						   EV_MEN_ESC_IDLE,
						   EV_MEN_ESC_ACTIVE} task_menu_ev_t;

/* State of Task Menu */
typedef enum task_menu_st {ST_MEN_XX_MAIN,
						   ST_MEN_XX_MOTOR_1,
						   ST_MEN_XX_MOTOR_2,
						   ST_MEN_XX_POWER_1,
						   ST_MEN_XX_POWER_2,
						   ST_MEN_XX_SPEED_1,
						   ST_MEN_XX_SPEED_2,
						   ST_MEN_XX_SPIN_1,
						   ST_MEN_XX_SPIN_2,
						   ST_MEN_XX_POWER_1_ON,
						   ST_MEN_XX_POWER_1_OFF,
						   ST_MEN_XX_POWER_2_ON,
						   ST_MEN_XX_POWER_2_OFF,
						   ST_MEN_XX_SPIN_1_L,
						   ST_MEN_XX_SPIN_1_R,
						   ST_MEN_XX_SPIN_2_L,
						   ST_MEN_XX_SPIN_2_R,
						   ST_MEN_XX_SPEED_1_0,
						   ST_MEN_XX_SPEED_1_1,
						   ST_MEN_XX_SPEED_1_2,
						   ST_MEN_XX_SPEED_1_3,
						   ST_MEN_XX_SPEED_1_4,
						   ST_MEN_XX_SPEED_1_5,
						   ST_MEN_XX_SPEED_1_6,
						   ST_MEN_XX_SPEED_1_7,
						   ST_MEN_XX_SPEED_1_8,
						   ST_MEN_XX_SPEED_1_9,
						   ST_MEN_XX_SPEED_2_0,
						   ST_MEN_XX_SPEED_2_1,
						   ST_MEN_XX_SPEED_2_2,
						   ST_MEN_XX_SPEED_2_3,
						   ST_MEN_XX_SPEED_2_4,
						   ST_MEN_XX_SPEED_2_5,
						   ST_MEN_XX_SPEED_2_6,
						   ST_MEN_XX_SPEED_2_7,
						   ST_MEN_XX_SPEED_2_8,
						   ST_MEN_XX_SPEED_2_9} task_menu_st_t;

typedef struct
{
	uint32_t		tick;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool			flag;
	bool			flag_lcd;
} task_menu_dta_t;

typedef struct
{
	bool			power; //on = true
	bool			spin; //right = true
	uint32_t		speed; //0 to 9
} motor_dta_t;



/********************** external data declaration ****************************/
extern task_menu_dta_t task_menu_dta;
extern motor_dta_t	motor_dta_list[];

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
