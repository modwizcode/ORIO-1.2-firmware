//**************************************************************************
//
//  Open Robot I/O
//
//    Copyright (C) 2019 John Winans
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//**************************************************************************

#include <stdio.h>

#include "fsl_sctimer.h"
#include "fsl_ctimer.h"
#include "peripherals.h"
#include "clock_config.h"
#include "orio_pwm.h"


// the following assumes that the SCTIMER0 & CTIMER0 clocks are running at 1MHZ
#define SCT_COUNTER_LIMIT (1000000/50)


// The PWM outputs go low when the counters are reset and high
// at the end of their cycle.  This makes the PWMs generated by CTIMERs
// and SCTIMERs work with the same timer count values.

// Note that the pwmDefault value is one past the period.  It is used 
// to prevent any PWM output from pulsing.
void orio_pwm_Init()
{
	SCT_Type *base = SCTIMER_1_PERIPHERAL;

	base->LIMIT = SCT_LIMIT_LIMMSK_L(1<<2);
	base->HALT = 0;					// never halt
	base->STOP = 0;					// never stop
	base->START = 0;				// never start (automatically)
	base->COUNT = 0;				// clear the counter
	base->REGMODE = 0;				// all MAT regs are for matching (not capture)
	base->OUTPUT = 0;				// init all output bits to low
	base->OUTPUTDIRCTRL = 0;		// no direction sensitivity
	base->RES = 0xaaaaaaaa;			// if conflicting output, clear it
	base->DMA0REQUEST	= 0;		// never trigger DMA
	base->DMA1REQUEST	= 0;		// never trigger DMA
	base->EVEN = 0;					// no event-based IRQs
	base->CONEN = 0;				// no conflict-related IRQs
	base->STATE = 0;				// we are using no states

	uint32_t pwmDefault = SCT_COUNTER_LIMIT+1;	// a value that will generate no PWM pulses (off)

	for (int i=0; i<10; ++i)
	{
		if (i==2)
		{
			base->SCTMATCH[i] = SCT_COUNTER_LIMIT;		// channel 2 is used to reset the timer
			base->SCTMATCHREL[i] = SCT_COUNTER_LIMIT;	// channel 2 is used to reset the timer

			base->OUT[i].CLR = 0;			// output 2 does not get set
			base->OUT[i].SET = 0;			// output 2 does not get cleared
		}
		else
		{
			base->SCTMATCH[i] = pwmDefault;		// default for all channels
			base->SCTMATCHREL[i] = pwmDefault;	// default for all channels

			base->OUT[i].CLR = 1<<2;		// event 2 resets all PWM outputs
			base->OUT[i].SET = 1<<i;		// event n sets PWM output n
		}

		base->EVENT[i].STATE = 0xffff;
		base->EVENT[i].CTRL = SCT_EVENT_CTRL_COMBMODE(1)|i;
	}

	base->CTRL &= ~(SCT_CTRL_HALT_L_MASK);	// start the counter running


	// Complete the set up of CTIMER0 to cover 'channel 2'

	CTIMER_SetupPwmPeriod(CTIMER_0_PERIPHERAL, kCTIMER_Match_2, SCT_COUNTER_LIMIT, pwmDefault, false);

	CTIMER_0_PERIPHERAL->MCR |= CTIMER_MCR_MR2RL(1);	// reload MR2 from SHADOW register
	CTIMER_0_PERIPHERAL->MSR[2] = pwmDefault;

	CTIMER_StartTimer(CTIMER_0_PERIPHERAL);		// start the timer running
}



static void orio_pwm_set_ctimer(int32_t usec)
{
	CTIMER_0_PERIPHERAL->MSR[2] = SCT_COUNTER_LIMIT-usec;
}

void orio_pwm_Set(uint8_t channel, int32_t usec)
{
	if (channel > 9)
		return;			// invalid channel number

//	printf("pwm %d=%d, limit=%d, ", channel, usec, SCT_COUNTER_LIMIT);

	if (usec < -1)
		usec = -1;
	if (usec > SCT_COUNTER_LIMIT)
		usec = SCT_COUNTER_LIMIT;

	//	printf("val = %d\n", usec);

	if (channel == 2)
		orio_pwm_set_ctimer(usec);
	else
		SCTIMER_1_PERIPHERAL->SCTMATCHREL[channel] = SCT_COUNTER_LIMIT-usec;
}
