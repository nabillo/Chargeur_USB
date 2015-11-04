/* 
 * File:   battery.h
 * Author: nabillo
 *
 * Created on 30 septembre 2014, 19:29
 */

#ifndef BATTERY_H
#define	BATTERY_H

struct Chg_Mode_Struct{
	short chg_Mode;
	float balance_Start;
	float balance_Diff;
	float balance_Set_Point;
	short balance_Done_Delay;
};

struct Charge_Struct{
	float chg_Current;
	struct Chg_Mode_Struct chg_Mode;
	float chg_End_Current;
	float chg_Cell_Volt;
	float restore_Lowest_Voltage;
	unsigned int restore_Charge_Time;
	float restore_Charge_Current;
	unsigned short keep_Charging;
	unsigned short sensitivity;
	unsigned short allow_0V;
	float trickle_Current;
	unsigned int trickle_Timeout;
	unsigned short cut_Temp;
	unsigned short max_Capacity;
	unsigned short safety_Timer;
};

struct Discharge_Struct{
	float discharge_Current;
	float discharge_Voltage;
	float end_Current;
	unsigned short balance_Enable;
	unsigned short cut_Temp;
	unsigned short max_Capacity;
	unsigned short safety_Timer;
};

struct Cycle_Struct{
	unsigned short cycle_Count;
	unsigned short delay_Time;
};

typedef struct {
	char program_name[10];
	char battery_type[5];
	unsigned short number_of_cells;
	unsigned short battery_capacity;
	struct Charge_Struct charge;
	float storage_Cell_Voltage;
	struct Discharge_Struct discharge;
	struct Cycle_Struct cycle;
}Battery;




#endif	/* BATTERY_H */

