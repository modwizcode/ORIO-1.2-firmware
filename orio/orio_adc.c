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

#include "orio_adc.h"

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC54606.h"

#include "fsl_power.h"

/**
 * This will turn on the analog power and clock to the ADC.
 *
 * I can't BELIEVE that this is not part of the generated code 
 * from Xpresso.
 ******************************************************************/
void orio_adc_EnablePower(void)
{
    POWER_DisablePD(kPDRUNCFG_PD_VDDA);
    POWER_DisablePD(kPDRUNCFG_PD_ADC0);
    POWER_DisablePD(kPDRUNCFG_PD_VD2_ANA);
    POWER_DisablePD(kPDRUNCFG_PD_VREFP);
    POWER_DisablePD(kPDRUNCFG_PD_TS);

    CLOCK_EnableClock(kCLOCK_Adc0);
}