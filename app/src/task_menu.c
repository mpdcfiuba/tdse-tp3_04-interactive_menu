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
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

//#include "task_menu_statechart.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_XX_MAIN, EV_MEN_ENT_IDLE, false, true};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/*si bien deberiamos armar el task_actuator, lo dejamos aqui por simplicidad
para terminar de resolver el trabajo practico. deberia estar en su modulo correspondiente
junto con tdo lo relacionado a motor */
motor_dta_t	motor_dta_list[] = {
	{false,true,0},{false,true,0}
};

#define MOTOR_DTA_QTY	(sizeof(motor_dta_list)/sizeof(motor_dta_t))

/********************** internal functions declaration ***********************/
void task_menu_statechart(void);

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	uint32_t index;
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;

	bool b_event;
	bool c_event;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - %s", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_INFO("  %s is a %s", GET_NAME(task_menu), p_task_menu_);

	/* Init & Print out: Task execution counter */
	g_task_menu_cnt = G_TASK_MEN_CNT_INI;
	LOGGER_INFO("   %s = %lu", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	/* Init & Print out: Task execution FSM */
	state = ST_MEN_XX_MAIN;
	p_task_menu_dta->state = state;

	event = EV_MEN_ENT_IDLE;
	p_task_menu_dta->event = event;

	b_event = false;
	p_task_menu_dta->flag = b_event;

	c_event = true;
	p_task_menu_dta->flag_lcd = c_event;

	for (index = 0; MOTOR_DTA_QTY > index; index++)
		{
			motor_dta_list[index].power = false; //false = off
			motor_dta_list[index].spin = true; // true = right
			motor_dta_list[index].speed = 0; // 0 = 0 velocity

		}

	LOGGER_INFO(" ");
	LOGGER_INFO("   %s = %lu   %s = %lu   %s = %s",
				 GET_NAME(state), (uint32_t)state,
				 GET_NAME(event), (uint32_t)event,
				 GET_NAME(b_event), (b_event ? "true" : "false"));

	/* Init & Print out: LCD Display */
	displayInit( DISPLAY_CONNECTION_GPIO_4BITS );
}

void task_menu_update(void *parameters)
{
	bool b_time_update_required = false;

	/* Protect shared resource */
	__asm("CPSID i");	/* disable interrupts */
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
		/* Update Tick Counter */
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts */

    while (b_time_update_required)
    {
		/* Update Task Counter */
		g_task_menu_cnt++;

		/* Run Task Menu Statechart */
    	task_menu_statechart();

    	/* Protect shared resource */
		__asm("CPSID i");	/* disable interrupts */
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			/* Update Tick Counter */
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts */
	}
}


void task_menu_statechart(void)
{
	task_menu_dta_t *p_task_menu_dta;

	char menu_str_1[32];
	char menu_str_2[32];

	p_task_menu_dta = &task_menu_dta;

	if (true == any_event_task_menu())
	{
		p_task_menu_dta->flag = true;
		p_task_menu_dta->flag_lcd = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	switch (p_task_menu_dta->state)
	{
		case ST_MEN_XX_MAIN:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_1, sizeof(menu_str_1), "Motor 1: %s, %lu, %s", (motor_dta_list[0].power ? "ON" : "OFF"),
						motor_dta_list[0].speed , (motor_dta_list[0].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_1);

				snprintf(menu_str_2, sizeof(menu_str_2), "Motor 2: %s, %lu, %s", (motor_dta_list[1].power ? "ON" : "OFF"),
						motor_dta_list[1].speed , (motor_dta_list[1].spin ? "L" : "R"));
				displayCharPositionWrite(0, 1);
				displayStringWrite(menu_str_2);

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter/Next/Escape   ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("                    ");

				p_task_menu_dta->flag_lcd = false;
			}
			break;

		case ST_MEN_XX_MOTOR_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				p_task_menu_dta->flag = false;
			}

			else if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_2;
				p_task_menu_dta->flag = false;
			}

			else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MAIN;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_1, sizeof(menu_str_1), "Motor 1: %s, %lu, %s", (motor_dta_list[0].power ? "ON" : "OFF"),
						motor_dta_list[0].speed , (motor_dta_list[0].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_1);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Motor 2     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1_ON;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_1, sizeof(menu_str_1), "Motor 1: %s, %lu, %s", (motor_dta_list[0].power ? "ON" : "OFF"),
						motor_dta_list[0].speed , (motor_dta_list[0].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_1);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Power |Next -> Spin ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1_L;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_1, sizeof(menu_str_1), "Motor 1: %s, %lu, %s", (motor_dta_list[0].power ? "ON" : "OFF"),
						motor_dta_list[0].speed , (motor_dta_list[0].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_1);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Spin |Next -> Speed ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_1, sizeof(menu_str_1), "Motor 1: %s, %lu, %s", (motor_dta_list[0].power ? "ON" : "OFF"),
						motor_dta_list[0].speed , (motor_dta_list[0].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_1);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Speed |Next -> Power");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_1_ON:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				motor_dta_list[0].power = true;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1_OFF;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Turn ON    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Turn OFF    ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_1_OFF:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				motor_dta_list[0].power = false;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1_ON;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Turn OFF   ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Turn ON     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_1_L:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1;
				motor_dta_list[0].spin = false;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1_R;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Right      ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Left        ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_1_R:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1;
				motor_dta_list[0].spin = true;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1_L;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Left       ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Right       ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_0:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 0    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 1     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 1    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 2     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_3;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 2    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 3     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_3:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 3;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_4;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 3    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 4     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_4:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 4;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_5;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 4    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 5     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_5:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 5;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_6;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 5    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 6     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_6:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 6;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_7;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 6    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 7     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_7:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 7;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_8;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 7    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 8     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_8:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 8;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_9;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 8    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 9     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_1_9:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				motor_dta_list[0].speed = 9;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1_0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_1;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 1: Speed 9    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 0     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_MOTOR_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				p_task_menu_dta->flag = false;
			}

			else if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_1;
				p_task_menu_dta->flag = false;
			}

			else if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MAIN;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_2, sizeof(menu_str_2), "Motor 2: %s, %lu, %s", (motor_dta_list[1].power ? "ON" : "OFF"),
						motor_dta_list[1].speed , (motor_dta_list[1].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_2);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Motor 1     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2_ON;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_2, sizeof(menu_str_2), "Motor 2: %s, %lu, %s", (motor_dta_list[1].power ? "ON" : "OFF"),
						motor_dta_list[1].speed , (motor_dta_list[1].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_2);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Power |Next -> Spin ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2_L;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_2, sizeof(menu_str_2), "Motor 2: %s, %lu, %s", (motor_dta_list[1].power ? "ON" : "OFF"),
						motor_dta_list[1].speed , (motor_dta_list[1].spin ? "L" : "R"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_2);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Spin |Next -> Speed ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_MOTOR_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				snprintf(menu_str_2, sizeof(menu_str_2), "Motor 2: %s, %lu, %s", (motor_dta_list[1].power ? "ON" : "OFF"),
						motor_dta_list[1].speed , (motor_dta_list[1].spin ? "R" : "L"));
				displayCharPositionWrite(0, 0);
				displayStringWrite(menu_str_2);

				displayCharPositionWrite(0, 1);
				displayStringWrite("Speed |Next -> Power");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to edit       ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_2_ON:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				motor_dta_list[1].power = true;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2_OFF;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Turn ON    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Turn OFF    ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_POWER_2_OFF:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				motor_dta_list[1].power = false;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2_ON;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_POWER_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Turn OFF   ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Turn ON     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_2_L:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2;
				motor_dta_list[1].spin = false;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2_R;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Right      ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Left        ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPIN_2_R:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2;
				motor_dta_list[1].spin = true;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2_L;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPIN_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Left       ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Right       ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_0:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 0    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 1     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_1:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 1;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 1    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 2     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_2:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 2;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_3;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 2    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 3     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_3:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 3;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_4;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 3    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 4     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_4:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 4;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_5;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 4    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 5     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_5:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 5;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_6;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 5    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 6     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_6:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 6;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_7;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 6    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 7     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_7:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 7;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_8;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 7    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 8     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_8:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 8;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_9;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 8    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 9     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		case ST_MEN_XX_SPEED_2_9:

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				motor_dta_list[1].speed = 9;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2_0;
				p_task_menu_dta->flag = false;
			}

			if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->state = ST_MEN_XX_SPEED_2;
				p_task_menu_dta->flag = false;
			}

			if (true == p_task_menu_dta->flag_lcd)
			{
				displayCharPositionWrite(0, 0);
				displayStringWrite("Motor 2: Speed 9    ");

				displayCharPositionWrite(0, 1);
				displayStringWrite("Next -> Speed 0     ");

				displayCharPositionWrite(0, 2);
				displayStringWrite("Enter to set        ");

				displayCharPositionWrite(0, 3);
				displayStringWrite("Escape to return    ");

				p_task_menu_dta->flag_lcd = false;
			}

			break;

		default:

			p_task_menu_dta->tick  = DEL_MEN_XX_MIN;
			p_task_menu_dta->state = ST_MEN_XX_MAIN;
			p_task_menu_dta->event = EV_MEN_ENT_IDLE;
			p_task_menu_dta->flag  = false;
			p_task_menu_dta->flag_lcd  = true;

			break;
	}

}

/********************** end of file ******************************************/
