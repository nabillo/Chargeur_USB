/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include <stdio.h>
#include <adc.h>
#include <string.h>
#include <timers.h>
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>

#include "app_custom_cdc.h"
#include "user.h"
#include "battery.h"

extern enum State cur_State;
extern tPIParams PI;
extern Battery battery;
extern unsigned short long precondition_Time;
extern unsigned short long charge_Time;
extern unsigned short timeout;
extern long seconds;
extern char msg_info[256];


/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* Return difference (in minuts) from actual time (retrive it from timer 1)
 * to initial time (in seconds) */
unsigned int calc_Time(long initial)
{
    unsigned int interval = 0;

    if (seconds >= initial)
    {
        interval = seconds - initial;
        interval = (int) interval / 60;
    }

    return interval;
}

/* App initialisation
 * init ADC
 * init timer 1
 * init interrupts
 */
void InitCharger(void)
{
    cur_State = CHARGE_ERROR;
    TRISBbits.RB0 = 1;
    INTCONbits.INT0E = 1; //enable Interrupt 0 (RB0 as interrupt)
    INTCON2bits.INTEDG0 = 0; //cause interrupt at falling edge
    INTCONbits.INT0F = 0; //reset interrupt flag
    //ei();
    
    strncpy(battery.battery_type,"LIPO\0",5);
    battery.charge.restore_Lowest_Voltage = 5;
    battery.number_of_cells = 1;
    battery.charge.restore_Charge_Current = 0.5;
    seconds = 0;

    OpenADC( ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_6_TAD,
            ADC_CH0 & ADC_INT_OFF & ADC_REF_VDD_VSS,
            ADC_1ANA );
    /* open timers */
    OpenTimer1( TIMER_INT_ON &
                T1_8BIT_RW &
                T1_SOURCE_EXT &
                T1_PS_1_1 &
                T1_OSC1EN_ON &
                T1_SYNC_EXT_OFF );

    //ei();

    /* TODO init PI structure */
    PI.Ki = 2;
    PI.Kp = 35;
    /* TODO init analog port */
}

/* Eval voltage on a channel
 * result on volt
 */
short V_Eval(unsigned char channel,signed float *voltage)
{
    signed float temp = 0;
    SelChanConvADC(channel);
    timeout = ADC_TIMEOUT;
    while(BusyADC() && timeout > 0); //Wait here until conversion is finished

    if (timeout == 0)
    {
        return KO;
    }

    temp = ReadADC();
    *voltage = (temp / 1024.0) * 5.0;
    
    return OK;
}

/* Set I or V by selected channel */
short I_V_Set(unsigned char channel)
{
    short res;
    short count = 0;

    do
    {
        res = V_Eval(channel, &PI.Feedback);
        if (res != OK)
        {
            return KO;
        }
        CalcPI(&PI);
        /* TODO set duty cycle */
        /* TODO add a delay */
        count++;
    /* Stop if error is in a dedband or a fix number of steps */
    } while (((PI.Error > DEADBAND) || (PI.Error < -1*DEADBAND)) && (count < ERROR_STEP));

    return OK;
}

/* Proportional Integral regulator algorithm */
void CalcPI(tPIParams *PIdata)
{
    PIdata->Error = PIdata->Setpoint - PIdata->Feedback;
    // Deadband -- If the magnitude of the error is 2 or less,
    // then don't calculate the PI routine at all. This saves
    // processor time and avoids oscillation problems.
    if ((PIdata->Error > DEADBAND) || (PIdata->Error < -DEADBAND))
    {
        // If the PI controller is saturated, then don't do any
        // accumulation of the integral.
        if (PIdata->Saturated == 0)
        {
            // Do some boundary checking on the integral value
            // to avoid overflow. If the integral value is near
            // the limits, then we won't do the accumulation.
            if (PIdata->Error > 0)
            {
                if (PIdata->Integral < 32000)
                PIdata->Integral += PIdata->Error;
            }
            else
            {
                if (PIdata->Integral > -32000)
                    /* TODO : Verifie Intergal type int */
                PIdata->Integral += PIdata->Error;
            }
        }
        // Now, calculate the actual PI function here.
        PIdata->Output = (PIdata->Error * PIdata->Kp + PIdata->Integral * PIdata->Ki)/256;
        // Perform boundary checks on the PI controller output. If the
        // output limits are exceeded, then set output to the limit
        // and set flag.
        if (PIdata->Output > PIdata->OutMax)
        {
            PIdata->Saturated = 1;
            PIdata->Output = PIdata->OutMax;
        }
        else if (PIdata->Output < 0)
        {
            PIdata->Saturated = 1;
            PIdata->Output = 0;
        }
        else
            PIdata->Saturated = 0;
    }
}

/* */
short initialize(void)
{
    return OK;
}

/* */
short list_Battery(void)
{
    return OK;
}

/* */
void select_Battery(void)
{

}

/* */
short list_Action(void)
{
    return OK;
}

/* */
void select_Action(void)
{

}

/* */
short list_Program(void)
{
    return OK;
}

/* */
void select_Program(void)
{

}

/* */
short check_Lipo(void)
{
    /* TODO Check Imput voltage */

    /* TODO Check cells number */

    return OK;
}

/* Check if procondition is necessary */
short check_Precondition(short *precondition)
{
    signed float voltage = 0;
    short res;

    res = V_Eval(ADC_CH0,&voltage);
    if (res != OK)
    {
        return KO;
    }

    if (voltage < battery.charge.restore_Lowest_Voltage * battery.number_of_cells)
    {
        *precondition = true;
    }
    else
    {
        *precondition = false;
    }

    sprintf(msg_info,"voltage : %f, precondition : %hd",voltage,precondition);
    APP_USB_send(msg_info);

    return OK;
}

/* Start precondition cycle */
short start_Precondition()
{
    short res;

    PI.Setpoint = battery.charge.restore_Charge_Current;
    PI.Saturated = 0;
    res = I_V_Set(ADC_CH10);
    if (res != OK)
    {
        return KO;
    }
    precondition_Time = seconds;

    return OK;
}

/* Verifie the ends condition for precondition */
short verifie_Precondition(char *end)
{
    signed float voltage = 0;
    short res = KO;

    res = V_Eval(ADC_CH0,&voltage);
    if (res != OK)
    {
        return KO;
    }

    if (calc_Time(precondition_Time) >= battery.charge.restore_Charge_Time)
    {
        strncpy(end,"TIMER",5);
        return OK;
    }
    if (voltage >= battery.charge.restore_Lowest_Voltage * battery.number_of_cells)
    {
        strncpy(end,"END",3);
        return OK;
    }
    else
    {
        PI.Setpoint = battery.charge.restore_Charge_Current;
        res = I_V_Set(ADC_CH10);
        if (res != OK)
        {
            return KO;
        }
        strncpy(end,"PROGRESS",8);
        return OK;
    }
}

/* */
short start_CC()
{
    short res;

    PI.Setpoint = battery.charge.chg_Current;
    PI.Saturated = 0;
    res = I_V_Set(ADC_CH10);
    if (res != OK)
    {
        return KO;
    }
    charge_Time = seconds;

    return OK;
}

/* */
short verifie_Vbat(char *end)
{
    signed float voltage = 0;
    short res = KO;

    res = V_Eval(ADC_CH0,&voltage);
    if (res != OK)
    {
        return KO;
    }

    if (calc_Time(charge_Time) > battery.charge.safety_Timer)
    {
        strncpy(end,"TIMER",5);
        return OK;
    }
    if (voltage >= battery.charge.chg_Cell_Volt * battery.number_of_cells)
    {
        strncpy(end,"END",3);
        return OK;
    }
    else
    {
        PI.Setpoint = battery.charge.chg_Current;
        res = I_V_Set(ADC_CH10);
        if (res != OK)
        {
            return KO;
        }
        strncpy(end,"PROGRESS",8);
        return OK;
    }
}

/* */
short start_CV()
{
    short res;

    PI.Setpoint = battery.charge.chg_Cell_Volt * battery.number_of_cells;
    PI.Saturated = 0;
    res = I_V_Set(ADC_CH0);
    if (res != OK)
    {
        return KO;
    }

    return OK;
}

/* */
short verifie_Ibat(char *end)
{
    signed float current = 0;
    short res = KO;

    res = V_Eval(ADC_CH10,&current);
    if (res != OK)
    {
        return KO;
    }

    if (calc_Time(charge_Time) > battery.charge.safety_Timer)
    {
        strncpy(end,"TIMER",5);
        return OK;
    }
    if (current >= battery.charge.chg_End_Current)
    {
        strncpy(end,"END",3);
        return OK;
    }
    else
    {
        PI.Setpoint = battery.charge.chg_Cell_Volt * battery.number_of_cells;
        res = I_V_Set(ADC_CH0);
        if (res != OK)
        {
            return KO;
        }
        strncpy(end,"PROGRESS",8);
        return OK;
    }
}

/* */
short stop_CV()
{
    signed float voltage = 0;
    short res = KO;
    float v_bat = battery.charge.chg_Cell_Volt * battery.number_of_cells;

    /* TODO set duty cycle to 0 */
    res = V_Eval(ADC_CH0,&voltage);
    if (res != OK)
    {
        return KO;
    }

    if (voltage >= v_bat - (DELTA/1000) && voltage <= v_bat + (DELTA/1000))
    {
        return OK;
    }
    else
    {
        return KO;
    }

}

/* */
void end_Charge()
{

}
