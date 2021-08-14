/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_cdc_if.h"
#include "M90E32.h"
#include "ssd1306_tests.h"
#include "ssd1306.h"
#include "usbd_cdc_if.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define ON_rly	GPIO_PIN_RESET
#define OFF_rly	GPIO_PIN_SET
#define OFF_BUZZER GPIO_PIN_SET
#define ON_BUZZER GPIO_PIN_RESET

#define SELECT_NON		0
#define SELECTSOURCE1	1
#define SELECTSOURCE2	2

#define NETWORK3P4W		0
#define NETWORK1P2W		1

#define MENUTIMEOUT		60
#define CTRL_ATS_TIMEOUT		5000

#define INIT_STARTDELAY	3

/*        fLASH            */
char Flashdata[48];
#define FLASH_PAGE_START_ADDRESS    0x0801F800
#define FLASH_PAGE_END_ADDRESS      0x0801FFFF
#define FLASH_PAGE_size             2048

uint8_t FlashErase(void);
uint8_t FlashWrite(uint32_t Address, uint8_t *Data, uint32_t Length);
void EEPROMWriteInt(uint32_t addr, uint16_t Value);

#define UnderSet_addr 0x00
#define OverSet_addr 0x02
#define UnderResSet_addr 0x04
#define OverResSet_addr 0x06
#define UnderTimSet_addr 0x08
#define OverTimSet_addr 0x0A
#define UnderResTimSet_addr 0x0C
#define OverResTimSet_addr 0x0E
#define SourceSelect_addr 0x10
#define NetworkSelect_addr 0x12 	
#define ModeSelect_addr 0x14
#define system_addr 0x16

#define FreqUnderSet_addr 0x18
#define FreqUnderReturnSet_addr 0x1A
#define FreqOverSet_addr 0x1C
#define FreqOverReturnSet_addr 0x1E
#define FreqABNormalTimeSet_addr 0x20
#define FreqNormalTimeSet_addr 0x22

#define GesScheduleEnable_addr	0x24
#define GesScheduleEvery_addr	0x26
#define GesScheduleDate_addr	0x28
#define GesScheduleDayofweek_addr	0x2A
#define GesScheduleHour_addr	0x2C
#define GesScheduleMinute_addr	0x2E
#define GesScheduleTime_addr	0x30
	
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const char mountname0[] 	= 	"";
const char mountname1[] 	= 	"Jan";
const char mountname2[] 	= 	"Feb";
const char mountname3[] 	= 	"Mar";
const char mountname4[] 	= 	"Apr";
const char mountname5[] 	= 	"May";
const char mountname6[] 	= 	"Jun";
const char mountname7[] 	= 	"Jul";
const char mountname8[] 	= 	"Aug";
const char mountname9[] 	= 	"Sep";
const char mountname10[] 	= 	"Oct";
const char mountname11[] 	= 	"Nov";
const char mountname12[] 	= 	"Dec";
const char* const mountname[] = {mountname0, mountname1, mountname2, mountname3, mountname4, mountname5 ,mountname6, 
																	mountname7, mountname8, mountname9, mountname10, mountname11, mountname12};
const char dayname0[] 	= 	"";
const char dayname1[] 	= 	"Sun";
const char dayname2[] 	= 	"Mon";	
const char dayname3[] 	= 	"Tue";	
const char dayname4[] 	= 	"Wed";	
const char dayname5[] 	= 	"Thu";	
const char dayname6[] 	= 	"Fri";	
const char dayname7[] 	= 	"Sat";
const char* const dayname[] = {dayname0, dayname1, dayname2, dayname3, dayname4, dayname5 ,dayname6, dayname7};																	

const char main1[] 	= 	"1.VoltUnderConfig";
const char main2[] 	= 	"2.VoltOverConfig";
const char main3[] 	= 	"3.PriorityConfig";
const char main4[] 	= 	"4.ACSystemConfig";
const char main5[] 	= 	"5.DateTimeConfig";
const char main6[] 	= 	"6.System Type";
const char main7[] 	= 	"7.FrequencyConfig";
const char main8[] 	= 	"8.GenSchedule";
const char main9[] 	= 	"9.Exit";

const char* const mainmenu[] = { main1, main2, main3, main4, main5 ,main6, main7, main8, main9};

const char undermenu1[] 	= 	"1.VoltUnderCutoff";
const char undermenu2[] 	= 	"2.VoltUnderReturn";
const char undermenu3[] 	= 	"3.TimeUnderCutoff";
const char undermenu4[] 	= 	"4.TimeUnderReturn";
const char undermenu5[] 	= 	"5.Exit";
const char* const undermenu[] = { undermenu1, undermenu2, undermenu3, undermenu4, undermenu5};
		//    0
enum{	NONselect,
			//  1                 2              3               4
			VoltUnderSet,VoltUnderReturnSet,TimeUnderSet,TimeUnderReturnSet,
			//  5                6               7              8
			VoltOverSet, VoltOverReturnSet, TimeOverSet, TimeOverReturnSet,
			//9        10        11
			DateSet, MonthSet, YearSet,
			// 12       13         14
			HoursSet, MinuteSet, SecondsSet, 
			//	 15              16
			FreqUnderSet, FreqUnderReturnSet,
			//	 17              18
			FreqOverSet, FreqOverReturnSet,
			//	 17              				18
			FreqABNormalTimeSet, FreqNormalTimeSet, 
			//			19      							20
			GenScheduleEnableSet , SchedulePeriodSet,
			//				21											22
			ScheduleSetDateTimeSet , ScheduleStartTimeSet
						
};
volatile signed char setvalueselect = NONselect;

const char overmenu1[] 	= 	"1.VoltOverCutoff";
const char overmenu2[] 	= 	"2.VoltOverReturn";
const char overmenu3[] 	= 	"3.TimeOverCutoff";
const char overmenu4[] 	= 	"4.TimeOverReturn";
const char overmenu5[] 	= 	"5.Exit";
const char* const overmenu[] = { overmenu1, overmenu2, overmenu3, overmenu4, overmenu5};

const char sourceselectmenu1[] 	= 	"1.Source1";
const char sourceselectmenu2[] 	= 	"2.Source2";
const char sourceselectmenu3[] 	= 	"";
const char sourceselectmenu4[] 	= 	"";
const char sourceselectmenu5[] 	= 	"*ENT Save&Exit";
const char* const sourceselectmenu[] = { sourceselectmenu1, sourceselectmenu2, sourceselectmenu3, sourceselectmenu4, sourceselectmenu5};
enum{selecsourceNON,selecsource1,selecsource2};

const char networksystemmenu1[] 	= 	"1.3P4W";
const char networksystemmenu2[] 	= 	"2.1P2W";
const char networksystemmenu3[] 	= 	"";
const char networksystemmenu4[] 	= 	"";
const char networksystemmenu5[] 	= 	"*ENT Save&Exit";
const char* const networksystemmenu[] = {networksystemmenu1, networksystemmenu2, networksystemmenu3, networksystemmenu4, networksystemmenu5};
enum{sys3P4W,sys1P2W};

const char datetimemenu1[] 	= 	"1.Set Date";
const char datetimemenu2[] 	= 	"2.Set Month";
const char datetimemenu3[] 	= 	"3.Set Year";
const char datetimemenu4[] 	= 	"4.Set Hours";
const char datetimemenu5[] 	= 	"5.Set Minute";
const char datetimemenu6[] 	= 	"6.Set Dayofweek";
//const char datetimemenu6[] 	= 	"6.Set Seconds";
const char datetimemenu7[] 	= 	"7.Exit";
const char* const datetimemenu[] = { datetimemenu1, datetimemenu2, datetimemenu3, datetimemenu4, datetimemenu5, datetimemenu6, datetimemenu7};

const char systemtypemenu1[] 	= 	"1. 1#Main 2#Gens";
const char systemtypemenu2[] 	= 	"2. 1#Main 2#Main";
const char systemtypemenu3[] 	= 	"";
const char systemtypemenu4[] 	= 	"";
const char systemtypemenu5[] 	= 	"*ENT Save&Exit";
const char* const systemtypemenu[] = {systemtypemenu1, systemtypemenu2, systemtypemenu3, systemtypemenu4, systemtypemenu5};
enum{main_gens,main_main};

const char freqmenu1[] 	= 	"1.FreqUnderCutoff";
const char freqmenu2[] 	= 	"2.FreqUnderReturn";
const char freqmenu3[] 	= 	"3.FreqOverCutoff";
const char freqmenu4[] 	= 	"4.FreqOverReturn";
const char freqmenu5[] 	= 	"5.FreqTimeAbNorm";
const char freqmenu6[] 	= 	"6.FreqTimeReturn";
const char freqmenu7[] 	= 	"7.Exit";
const char* const frequencymenu[] = {freqmenu1, freqmenu2, freqmenu3, freqmenu4, freqmenu5, freqmenu6, freqmenu7};

const char GenSchedulemenu1[] 	= 	"1.Disable/Enable";
const char GenSchedulemenu2[] 	= 	"2.Period";
const char GenSchedulemenu3[] 	= 	"3.Date/Time";
const char GenSchedulemenu4[] 	= 	"4.StartTime";
const char GenSchedulemenu5[] 	= 	"5.Exit";
const char* const GenSchedulemenu[] = { GenSchedulemenu1, GenSchedulemenu2, GenSchedulemenu3, GenSchedulemenu4, GenSchedulemenu5};
enum{ScheduleEnable_T,SchedulePeriod_T,ScheduleSetDateTime_T,ScheduleStartTime_T,ScheduleGoback_T};

const char GenStartEnablemenu1[] 	= 	"1.Disable";
const char GenStartEnablemenu2[] 	= 	"2.Enable";
const char GenStartEnablemenu3[] 	= 	"";
const char GenStartEnablemenu4[] 	= 	"";
const char GenStartEnablemenu5[] 	= 	"*ENT Save&Exit";
const char* const GenStartEnablemenu[] = {GenStartEnablemenu1, GenStartEnablemenu2, GenStartEnablemenu3, GenStartEnablemenu4, GenStartEnablemenu5};
enum{SCHEDULE_EN, SCHEDULE_DIS};

const char Periodmenu1[] 	= 	"1.Daily";
const char Periodmenu2[] 	= 	"2.Weekly";
const char Periodmenu3[] 	= 	"3.Monthly";
const char Periodmenu4[] 	= 	"";
const char Periodmenu5[] 	= 	"*ENT Save&Exit";
const char* const Periodmenu[] = { Periodmenu1, Periodmenu2, Periodmenu3, Periodmenu4, Periodmenu5};
enum{DAILY, WEEKLY, MONTHLY};

const char GenStartDateTimemenu1[] 	= 	"1.Date";
const char GenStartDateTimemenu2[] 	= 	"2.Dayofweek";
const char GenStartDateTimemenu3[] 	= 	"3.Hours";
const char GenStartDateTimemenu4[] 	= 	"4.Minute";
const char GenStartDateTimemenu5[] 	= 	"5.Exit";
const char* const GenStartDateTimemenu[] = { GenStartDateTimemenu1, GenStartDateTimemenu2, GenStartDateTimemenu3, GenStartDateTimemenu4, GenStartDateTimemenu5};
enum{ScheduleDate_T,ScheduleDayofweek_T,ScheduleSetHours_T,ScheduleMinute_T,Schedulegoback_T};

const char statusmenu0[] 	= 	"Normal";
const char statusmenu1[] 	= 	"Under";
const char statusmenu2[] 	= 	"Under";
const char statusmenu3[] 	= 	"UnderReturn";
const char statusmenu4[] 	= 	"";
const char statusmenu5[] 	= 	"Over";
const char statusmenu6[] 	= 	"Over";
const char statusmenu7[] 	= 	"OverReturn";
const char statusmenu8[] 	= 	"";
const char* const statusmenu[] = { statusmenu0, statusmenu1, statusmenu2, statusmenu3, statusmenu4, statusmenu5, statusmenu6, statusmenu7, statusmenu8};

#define MAXLENGHT 17   //Font_7x10
																	
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t loopcount;

float source1_A;
float source1_B;
float source1_C;
float freqS1;									

uint16_t V1_A;
uint16_t V1_B;
uint16_t V1_C;
uint16_t F_S1;

float source2_A;
float source2_B;
float source2_C;
float freqS2;

uint16_t V2_A;
uint16_t V2_B;
uint16_t V2_C;
uint16_t F_S2;

//				0					1								2								3						4						5          6						7
enum{UnderSet_T, OvererSet_T, MainselectSet_T, ConfigSet_T, TimeSet_T, SystemSet_T, FreqSet_T, Schedule_T};
enum{VoltCut_T,VoltReturn_T,TimeCut_T,TimeReturn_T,Goback_T};

enum{modeauto,modemanual};

enum{State_nor,State_PreUnder,State_Under,State_PreUnderRes,State_UnderRes,State_PreOver,State_Over,State_PreOverRes,State_OverRes};
enum{nor, UnderSet, OverSet,UnderResSet,OverResSet,UnderTimSet,OverTimSet,UnderResTimSet,OverResTimSet};
enum{normal, SetUnder, SetUnderRes,SetUnderTim,SetUnderResTim,SetSource};

volatile int8_t State = State_nor;
//volatile int16_t selecsource = selecsource1;
volatile int8_t lcdflag ;

//volatile signed char SubMenu1=0,SubMenu2=0,SubMenu3=0,SubMenu4=0;
//				0						1						2						3						4
enum{mainpage_T,Pagemenu1_T,Pagemenu2_T,Pagemenu3_T,Pagemenu4_T};	
volatile signed char PageMenuCount = mainpage_T;
volatile signed char Submenu1Count =0;
volatile signed char Submenu2Count =0;
volatile signed char Submenu3Count =0;

enum{Display1_T, Display2_T, Display3_T, Display4_T};
volatile signed char DisplayMain = Display1_T;
	
volatile int16_t 	 UnderValue, UnderResValue, UnderTimSetValue, UnderResTimSetValue;
volatile int16_t 	 UnderValue_compare, UnderResValue_compare, UnderTimSetValue_compare, UnderResTimSetValue_compare;

volatile int16_t   OverValue, OverResValue, OverTimSetValue,  OverResTimSetValue;
volatile int16_t   OverValue_compare, OverResValue_compare, OverTimSetValue_compare,  OverResTimSetValue_compare;

volatile uint16_t  SourceSelectValue, NetworkSelectValue;
volatile uint16_t  SourceSelectValue_compare, NetworkSelectValue_compare;
volatile uint16_t  workmodeValue ,workmodeValue_compare;

volatile uint16_t  systemValue , systemValue_compare;

volatile int16_t 	 freqUnderValue, freqUnderResValue;
volatile int16_t 	 freqUnderValue_compare, freqUnderResValue_compare;

volatile int16_t   freqOverValue, freqOverResValue;
volatile int16_t   freqOverValue_compare, freqOverResValue_compare;

volatile int8_t 	 freqUnderflag = 0 ,freqOverflag = 0;

volatile int16_t 	 freqABnormalTimeSetValue, freqNormalTimeSetValue ;
volatile int16_t 	 freqABnormalTimeSetValue_compare, freqNormalTimeSetValue_compare ;

volatile int16_t   UnderTimeCount =0 , UnderResTimeCount =0;

volatile int16_t   freqABnormalTimeCount =0 , freqNormalTime =0;

volatile int16_t   genstarttimeValue, genstarttimeValue_compare ;

volatile int16_t   OverTimeCount =0 , OverResTimeCount =0;

volatile int16_t   GenstartTimeCount =0;

volatile signed char Timer_flag =0;

volatile int16_t PhaseSequenceerror , Status0;
//volatile int16_t StartMeasureCount = 5000;

volatile signed char initstartdelaycount = INIT_STARTDELAY, start_ats = 0;

RTC_TimeTypeDef Timeupdate = {0};
RTC_DateTypeDef Dateupdate = {0};
RTC_TimeTypeDef Timeset = {0};
RTC_DateTypeDef Dateset = {0};

RTC_TimeTypeDef TimeGenStartset = {0};
RTC_DateTypeDef DateGenStartset = {0};

uint8_t clockdata[4] = {0};
uint8_t datedata[4] = {0};
uint8_t timedata[8] = {0};

typedef struct gen{
	volatile int16_t genschedule_every;
  volatile int16_t genschedule_enable;
  volatile int16_t genschedule_hour;
  volatile int16_t genschedule_minute;
  volatile int16_t genschedule_dayofweek;
	volatile int16_t genschedule_date;
	volatile int16_t genschedule_time;
} genschedulestart_t;
genschedulestart_t genschedulestart ,genschedulestart_compare ;
	
uint8_t source1OK , source2OK;

volatile uint8_t source_out;

volatile uint8_t releaserelay =0;
volatile uint8_t Checksource1OK =0;
volatile uint8_t Checksource2OK =0;

enum{GENSTOP , GENSTART};
volatile uint8_t genstart = GENSTOP;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Beep(void);
void ReadSetting(void);
void buttonRead(void);
void lcdupdate(void);
void cleardisplay(void);
void readvolt(void);
uint8_t comparesettingvalue(void);
void storecomparevalue(void);
void restorevalue(void);
void system_init(void);
uint8_t checkauxinput(void);
void ats_process(void);
void check_releaserelay(void);
void checkgenpromp(void);
void checkgenschedule(void);

volatile int16_t systickcount =0;
volatile signed char beepcount = 0;
volatile signed int ctrlATScount = 0;
void HAL_SYSTICK_Callback()
{	
	systickcount++;
	
	// off beep
	if(beepcount)
	{
		if(--beepcount <=0)
		{
			HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,OFF_BUZZER);
		}
	}
	if(ctrlATScount)
	{
		if(--ctrlATScount <=0)
		{
			HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
			HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
		}
	}
	
	/*     Timer Count      */
	if(Timer_flag)
	{
		/////////////// Under //////////////////////
		//if((UnderTimeCount)&&(State == State_PreUnder))
		if(UnderTimeCount)
		{	
			if(--UnderTimeCount <=0)
			{
				UnderTimeCount = 0;
				if(SourceSelectValue == SELECTSOURCE1)
				{
					source1OK = 0;
					if(systemValue == main_main)
					{
						if(source2OK)
						{
							if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
								source_out = selecsource2;
								releaserelay =1;
							}
						}
						else{
							Checksource2OK = 1;
						}
						

					}
					else //(main_gens)
					{
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
						genstart = GENSTART;
					}
					if(NetworkSelectValue == sys3P4W){
						if((F_S1 <= freqUnderValue)&& (V1_A > UnderValue) && (V1_B > UnderValue) && (V1_C > UnderValue)){
							freqUnderflag = 1;
						}
					}
					else{ // sys1P2W
						if((F_S1 <= freqUnderValue)&& (V1_A > UnderValue)){
							freqUnderflag = 1;
						}
					}
					
					
				}
				else // (SourceSelectValue == SELECTSOURCE2)
				{
					source2OK = 0;
					if(systemValue == main_main)
					{
						if(source1OK)
						{
							if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
								source_out = selecsource1;
								releaserelay =1;
							}
						}
						else{
							Checksource1OK = 1;
						}
					}
					else //(main_gens)
					{
							
					}
					if(NetworkSelectValue == sys3P4W){
						if((F_S2 <= freqUnderValue)&& (V2_A > UnderValue) && (V2_B > UnderValue) && (V2_C > UnderValue)){
							freqUnderflag = 1;
						}
					}
					else{ // sys1P2W
						if((F_S2 <= freqUnderValue)&& (V2_A > UnderValue)){
							freqUnderflag = 1;
						}
					}
				}		
				State = State_Under;
				if((UnderResTimeCount ==0) && (OverTimeCount ==0)&& (OverResTimeCount ==0))
				{
					Timer_flag = 0; // stop timer
				}
				
			}
		}
		//if((UnderResTimeCount)&&(State == State_PreUnderRes))
		if(UnderResTimeCount)
		{
			if(--UnderResTimeCount <=0)
			{
				UnderResTimeCount = 0;
				if(SourceSelectValue == SELECTSOURCE1)
				{
					source1OK = 1;
					State = State_nor;
					if(systemValue == main_main)
					{
						if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							source_out = selecsource1;
							releaserelay =1;
						}
					}
					else //(main_gens)
					{
						if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							source_out = selecsource1;
							releaserelay =1;
						}
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
						genstart = GENSTOP;
					}
					if(F_S1 >= freqUnderResValue){
						freqUnderflag = 0 ;
					}
				}
				else // (SourceSelectValue == SELECTSOURCE2)
				{
					source2OK = 1;
				}
				if(State == State_PreUnderRes)/**/
				{
					State = State_nor;
					if(systemValue == main_main)
					{
						if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
							source_out = selecsource2;
							releaserelay =1;
						}
					}
					else //(main_gens)
					{
						// impossible
					}
					if(F_S2 >= freqUnderResValue){
						freqUnderflag = 0 ;
					}
				}
				if((UnderTimeCount ==0) && (OverTimeCount ==0)&& (OverResTimeCount ==0))
				{
					Timer_flag = 0; // stop timer
				}
			}
		}
		/////////////// Over //////////////////////
		//if((OverTimeCount)&&(State == State_PreOver))
		if(OverTimeCount)
		{		
			if(--OverTimeCount <=0)
			{
				OverTimeCount = 0;
				if(SourceSelectValue == SELECTSOURCE1)
				{
					source1OK = 0;
					if(systemValue == main_main)
					{
						if(source2OK)
						{
							if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
								source_out = selecsource2;
								releaserelay =1;
							}
						}
						else{
							Checksource2OK = 1;
						}
					}
					else //(main_gens)
					{
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
						genstart = GENSTART;
					}
					if(F_S1 >= freqOverValue){
						freqOverflag = 1;
					}
					
				}
				else // (SourceSelectValue == SELECTSOURCE2)
				{
					source2OK = 0;
					if(systemValue == main_main)
					{
						if(source1OK)
						{
							if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
								source_out = selecsource1;
								releaserelay =1;
							}
						}
						else
						{
							Checksource1OK = 1;
						}
					}
					else //(main_gens)
					{
							
					}
					if(F_S2 >= freqOverValue){
						freqOverflag = 1;
					}
				}		
				State = State_Over;
				if((UnderResTimeCount ==0) && (UnderResTimeCount ==0)&& (OverResTimeCount ==0))
				{
					Timer_flag = 0; // stop timer
				}
			}
		}
		//if((OverResTimeCount)&&(State == State_PreOverRes))
		if(OverResTimeCount)
		{
			if(--OverResTimeCount <=0)
			{
				OverResTimeCount = 0;
				if(SourceSelectValue == SELECTSOURCE1)
				{
					source1OK = 1;
					State = State_nor;
					if(systemValue == main_main)
					{
						if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							source_out = selecsource1;
							releaserelay =1;
						}
					}
					else //(main_gens)
					{
						if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							source_out = selecsource1;
							releaserelay =1;
						}
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
						genstart = GENSTOP;
					}
					if(F_S1 <= freqOverResValue){
						freqOverflag = 0;
					}	
				}
				else // (SourceSelectValue == SELECTSOURCE2)
				{
					source2OK = 1;
					State = State_nor;
					if(systemValue == main_main)
					{
						if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
							source_out = selecsource2;
							releaserelay =1;
						}
					}
					else //(main_gens)
					{
						if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							source_out = selecsource1;
							releaserelay =1;
						}
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
						genstart = GENSTOP;
					}
					if(F_S2 <= freqOverResValue){
						freqOverflag = 0;
					}	
				}
				if(State == State_PreUnderRes)/**/
				{
					State = State_nor;
					if(systemValue == main_main)
					{
						if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
						{
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
							source_out = selecsource2;
							releaserelay =1;
						}
					}
					else //(main_gens)
					{
						// impossible
					}	
				}
				if((UnderTimeCount ==0) && (UnderResTimeCount ==0) && (OverTimeCount ==0))
				{
					Timer_flag = 0; // stop timer
				}
			}
		}
	}
	//////////////////////////////
}

volatile int16_t exticount =0;
volatile uint8_t exit1_flag=0, exit2_flag=0 ;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if((exit1_flag==0)&&(GPIO_Pin == GPIO_PIN_1))
	{
		exit1_flag = 255;
	}
	if((exit2_flag==0)&&(GPIO_Pin == GPIO_PIN_0))
	{
		exit2_flag = 255;
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi)
{
	if(lcdflag)
	{
		HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
		lcdflag = 0;
	}
}

volatile signed char menucount = 0;
//Interrupt TIM Overflow routine 1 Sec..
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	
	if(htim->Instance == TIM7)
	{
		// CHECK MENU TIMEOUT
		if(menucount)
		{
			if(--menucount <=0)
			{
				menucount =0;
				PageMenuCount = mainpage_T;
				Submenu1Count = 0;
				Submenu2Count = 0;
				setvalueselect = NONselect;
				//restorevalue();
				if(comparesettingvalue())
				{
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
				}
			}
		}
		if(initstartdelaycount)
		{
			if(--initstartdelaycount <=0)
			{
				initstartdelaycount =0;
				start_ats = 1;
				
			}
		}
		if(GenstartTimeCount)
		{
			if(--GenstartTimeCount <=0)
			{
				GenstartTimeCount =0;
				HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
				
			}
		}
	}
}





void checkgenschedule(void)
{
	if((genschedulestart.genschedule_enable)&&(systemValue == main_gens))
	{
		if(genschedulestart.genschedule_every == DAILY)
		{
			if((genschedulestart.genschedule_hour == Timeupdate.Hours) && (genschedulestart.genschedule_minute == Timeupdate.Minutes))
			{
				if(GenstartTimeCount == 0){
					HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
					GenstartTimeCount = genschedulestart.genschedule_time * 60;
				}
			}
		}
		else if(genschedulestart.genschedule_every == WEEKLY){
			if(genschedulestart.genschedule_dayofweek == Dateupdate.WeekDay){
				if((genschedulestart.genschedule_hour == Timeupdate.Hours) && (genschedulestart.genschedule_minute == Timeupdate.Minutes))
				{
					if(GenstartTimeCount == 0){
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
						GenstartTimeCount = genschedulestart.genschedule_time * 60;
					}
				}
			}
		}
		else if(genschedulestart.genschedule_every == MONTHLY){
			if(genschedulestart.genschedule_date == Dateupdate.Date){
				if((genschedulestart.genschedule_hour == Timeupdate.Hours) && (genschedulestart.genschedule_minute == Timeupdate.Minutes))
				{
					if(GenstartTimeCount == 0)
					{
						HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
						GenstartTimeCount = genschedulestart.genschedule_time * 60;
					}
				}
			}
		}
	}
}

void checkgenpromp(void)
{
	if(NetworkSelectValue == sys3P4W)
	{
		if(source_out == selecsource1){
			if(((V2_A > UnderValue)&&(V2_A < OverValue)) && ((V2_B > UnderValue)&&(V2_B < OverValue)) && ((V2_C > UnderValue)&&(V2_C < OverValue))&& 
			(F_S2 > freqUnderValue) && (F_S2 < freqOverValue)  )
			{
				source2OK = 1;
				//HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
				ctrlATScount = CTRL_ATS_TIMEOUT;
				HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
				HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
				source_out = selecsource2;
				releaserelay =1;
			}
		}
	}
	else// if(NetworkSelectValue == sys1P2W)
	{
		if(source_out == selecsource1){
			if((V2_A > UnderValue)&&(V2_A < OverValue) &&
			(F_S2 > freqUnderValue) && (F_S2 < freqOverValue)  )
			{
				source2OK = 1;
				//HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
				ctrlATScount = CTRL_ATS_TIMEOUT;
				HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
				HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
				source_out = selecsource2;
				releaserelay =1;
			}
		}
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  MX_SPI2_Init();
  MX_RTC_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,OFF_BUZZER);
	HAL_GPIO_WritePin(SPI1_CS1_GPIO_Port,SPI1_CS1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port,SPI2_CS_Pin,GPIO_PIN_SET);
	HAL_Delay(1);
	
	InitEnergyIC(SOURCE2);
	InitEnergyIC(SOURCE1);
	
	ReadSetting();
	system_init();
	
	ssd1306_Init();
	ssd1306_Fill(Black);
	
	uint32_t delta;

/*
// off all relay
//	HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(LED_Manual_GPIO_Port,LED_Manual_Pin,GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(LED_Auto_GPIO_Port,LED_Auto_Pin,GPIO_PIN_SET);

		// write Rectangle
//	for(delta = 0; delta < 1; delta ++) {
//		ssd1306_DrawRectangle(1 + (5*delta),1 + (5*delta) ,SSD1306_WIDTH-1 - (5*delta),SSD1306_HEIGHT-1 - (5*delta),White);
//	}
*/	
	
	HAL_TIM_Base_Start_IT(&htim7);
	
	CommEnergyIC(SOURCE1, 0, EMMIntEn1, 0x7000);
	CommEnergyIC(SOURCE2, 0, EMMIntEn1, 0x7000);
	
	initstartdelaycount = INIT_STARTDELAY;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	//  ATS_process // jj // jj
  while (1)
  {
		loopcount++;
			
		buttonRead();
		check_releaserelay();
		if(genstart == GENSTART)
		{
			checkgenpromp();
		}
		if(workmodeValue == modemanual)
		{
			checkauxinput();
		}
		if(((loopcount % 100) == 0) && (lcdflag ==0)&& (start_ats))// 10.4 ms.
		{
			HAL_GPIO_TogglePin(LCD_D2_GPIO_Port,LCD_D2_Pin);	
			readvolt(); //1 ms.
		}
		
		if(((loopcount % 20000) == 0))// 120.4 ms.
		{
			hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
			if (HAL_SPI_Init(&hspi1) != HAL_OK)
			{
				Error_Handler();
			}
			
			lcdupdate(); // 13.5 ms.
			
			hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
			if (HAL_SPI_Init(&hspi1) != HAL_OK)
			{
				Error_Handler();
			}
		}
		
		if(Checksource2OK)
		{
			if(NetworkSelectValue == sys3P4W)
			{
					if((V2_A > UnderValue) && (V2_B > UnderValue) && (V2_C > UnderValue)&&
					(V2_A < OverValue) && (V2_B < OverValue) && (V2_C < OverValue) &&
					(F_S2 > freqUnderValue) && (F_S2 < freqOverValue) )
					{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
						source_out = selecsource2;
						releaserelay =1;
						Checksource2OK =0;
					}
			}
			else{ //sys1P2W
				if((V2_A > UnderValue) && (V2_A < OverValue) &&
					(F_S2 > freqUnderValue) && (F_S2 < freqOverValue)){
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
						source_out = selecsource2;
						releaserelay =1;
						Checksource2OK =0;
				}
			}
		}
		if(Checksource1OK)
		{
			if(NetworkSelectValue == sys3P4W)
			{
				if((V1_A > UnderValue) && (V1_B > UnderValue) && (V1_C > UnderValue)&&
				(V1_A < OverValue) && (V1_B < OverValue) && (V1_C < OverValue) &&
				(F_S1 > freqUnderValue) && (F_S1 < freqOverValue) )
				{
					ctrlATScount = CTRL_ATS_TIMEOUT;
					HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
					HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
					source_out = selecsource1;
					releaserelay =1;
					Checksource1OK =0;
				}
			}
			else //1P2W
			{
				if((V1_A > UnderValue) && (V1_A < OverValue) &&
				(F_S1 > freqUnderValue) && (F_S1 < freqOverValue) )
				{
					ctrlATScount = CTRL_ATS_TIMEOUT;
					HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
					HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
					source_out = selecsource1;
					releaserelay =1;
					Checksource1OK =0;
				}
			}
		}
		
		if((loopcount % 35000) == 0)// 1.5 sec.
		{
			checkgenschedule();
			HAL_GPIO_TogglePin(LED_HEALTY_GPIO_Port,LED_HEALTY_Pin);
		}
		//HAL_IWDG_Refresh(&hiwdg);
		
		//  3.370 us. per loop.
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

//--------------- FLASH fUNCTION ---------------------------------//
uint8_t FlashErase(void)
{
  uint8_t ret = 1;
  uint32_t Address;
  
  /* Unlock the Flash to enable the flash control register access *************/ 
	HAL_FLASH_Unlock();
  
  /* Erase the user Flash area ***********/

  /* Clear pending flags (if any) */  
  FLASH->SR = (FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  
  for(Address = FLASH_PAGE_START_ADDRESS; Address < FLASH_PAGE_END_ADDRESS; Address += FLASH_PAGE_size)
  {
    /* Wait for last operation to be completed */
    while((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY);
    
    if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
    {
      /* Write protected error */
      ret = 0;
      break;
    }
    
    if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
    {
      /* Programming error */
      ret = 0;
      break;
    }
    
    /* If the previous operation is completed, proceed to erase the page */
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = Address;
    FLASH->CR |= FLASH_CR_STRT;
      
    /* Wait for last operation to be completed */
    while((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY);
    
    if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
    {
      /* Write protected error */
      ret = 0;
      break;
    }
    
    if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
    {
      /* Programming error */
      ret = 0;
      break;
    }
      
    /* Disable the PER Bit */
    FLASH->CR &= ~FLASH_CR_PER;
  }
  
  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  /* Set the LOCK Bit to lock the FLASH control register and program memory access */
  FLASH->CR |= FLASH_CR_LOCK;
  
  return ret;
}

uint8_t FlashWrite(uint32_t Address, uint8_t *Data, uint32_t Length)
{
  uint8_t ret = 1;
  uint16_t TmpData;
  
  if(Address >= FLASH_PAGE_START_ADDRESS && Address <= FLASH_PAGE_END_ADDRESS)
  {
    /* Unlock the Flash to enable the flash control register access *************/ 
    HAL_FLASH_Unlock();
    
    /* Clear pending flags (if any) */  
    FLASH->SR = (FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    
    while(Length > 0)
    {
      if(Length == 1)
      {
        TmpData = Data[0] | (0x00 << 8 );
        Data = Data + 1;
        Length = Length - 1;
      }
      else
      {
        TmpData = Data[0] | (Data[1] << 8 );
        Data = Data + 2;
        Length = Length - 2;
      }
      
      /* Wait for last operation to be completed */
      while((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY);
      
      if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
      {
        /* Write protected error */
        ret = 0;
        break;
      }
      
      if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
      {
        /* Programming error */
        ret = 0;
        break;
      }
      
      /* If the previous operation is completed, proceed to program the new data */
      FLASH->CR |= FLASH_CR_PG;
      
      *(__IO uint16_t*)Address = TmpData;
      
      /* Wait for last operation to be completed */
      while((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY);
      
      if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
      {
        /* Write protected error */
        ret = 0;
        break;
      }
      
      if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
      {
        /* Programming error */
        ret = 0;
        break;
      }
      
      /* Disable the PG Bit */
      FLASH->CR &= ~FLASH_CR_PG;
      
      /* Next address */
      Address = Address + 2;
    }
    
    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) *********/
    /* Set the LOCK Bit to lock the FLASH control register and program memory access */
    FLASH->CR |= FLASH_CR_LOCK;
  }
  else
    ret = 0;
  
  return ret;
}

/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  */
void EEPROMWriteInt(uint32_t addr, uint16_t Value)
{
	Flashdata[1 + addr] = (uint8_t)Value;
	Value = Value>>8;
	Flashdata[0 + addr] = (uint8_t)Value;
}


/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  */
uint16_t EMMInt1 ,EMMInt2 ,EMMInt3,EMMInt4;
uint8_t freq1overcount =0 , freq2overcount =0;
void readvolt(void)
{
	if(NetworkSelectValue == sys3P4W)
	{
		source1_A = GetLineVoltageA(SOURCE1);
		source1_B = GetLineVoltageB(SOURCE1); 
		source1_C = GetLineVoltageC(SOURCE1);
		freqS1 = GetFrequency(SOURCE1);
		
		V1_A = (uint16_t)source1_A;
		V1_B = (uint16_t)source1_B;
		V1_C = (uint16_t)source1_C;
		
		if(V1_A <10)
			V1_A = 0;
		if(V1_B <10)
			V1_B = 0;
		if(V1_C <10)
			V1_C = 0;
		
		if((V1_A <10) && (V1_B <10) && (V1_C <10)){
			freqS1 = 0;
		}
		
		F_S1 = (uint16_t)freqS1;
		if(F_S1 >= freqOverValue)
		{
			if(++freq1overcount > 5){
				freq1overcount = 5;
			}
		}
		else
		{
			freq1overcount = 0;
		}
		
		if((V1_A > UnderValue) && (V1_B > UnderValue) && (V1_C > UnderValue)&&
		(V1_A < OverValue) && (V1_B < OverValue) && (V1_C < OverValue) &&
		(F_S1 > freqUnderValue) && (F_S1 < freqOverValue) )
	  {
			source1OK = 1;
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_SET);
		}
		else
		{
			source1OK = 0;
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_RESET);
		}

		source2_A = GetLineVoltageA(SOURCE2);
		source2_B = GetLineVoltageB(SOURCE2); 
		source2_C = GetLineVoltageC(SOURCE2);
		freqS2 = GetFrequency(SOURCE2);
			
		V2_A = (uint16_t)source2_A;
		V2_B = (uint16_t)source2_B;
		V2_C = (uint16_t)source2_C;
		
		if(V2_A <10)
			V2_A = 0;
		if(V2_B <10)
			V2_B = 0;
		if(V2_C <10)
			V2_C = 0;
		
		if((V2_A <10) && (V2_B <10) && (V2_C <10)){
			freqS2 = 0;
		}
		
		F_S2 = (uint16_t)freqS2;
		if(F_S2 >= freqOverValue)
		{
			if(++freq2overcount > 5){
				freq2overcount = 5;
			}
		}
		else
		{
			freq2overcount = 0;
		}
		
		if((V2_A > UnderValue) && (V2_B > UnderValue) && (V2_C > UnderValue)&&
			(V2_A < OverValue) && (V2_B < OverValue) && (V2_C < OverValue) &&
			(F_S2 > freqUnderValue) && (F_S2 < freqOverValue) )
		{
			source2OK = 1;
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
		}
		else
		{
			source2OK = 0;
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_RESET);
		}
		
		if((Checksource1OK) && (source1OK) && (source2OK))
		{
			Checksource1OK =0;
		}
		if((Checksource2OK) && (source2OK) && (source1OK))
		{
			Checksource2OK =0;
		}
		
		
		if(start_ats)
		{
			if(SourceSelectValue == SELECTSOURCE1)
			{
				/*****************UNDER**********************/
				if( (((V1_A <= UnderValue ) || (V1_B <= UnderValue ) || (V1_C <= UnderValue )) ||(F_S1 <= freqUnderValue) )&&
				((State == State_nor)||(State == State_PreOverRes)))
				{
					if((V1_A <= UnderValue ) || (V1_B <= UnderValue ) || (V1_C <= UnderValue ))
					{
						UnderTimeCount = UnderTimSetValue*1000;
					}
					else{
						UnderTimeCount = freqABnormalTimeSetValue*1000;
						if(UnderTimeCount ==0)
						{
							freqUnderflag = 1;
						}
					}
					
					State = State_PreUnder;
					if(UnderTimeCount ==0)
					{
						source1OK = 0;
						State = State_Under;
						if(systemValue == main_main)
						{
							if(source2OK)
							{
								if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else
							{
								Checksource2OK = 1;
							}
						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag = 1; // start timer
						}
					}
				}
				
				if( (((V1_A > UnderValue ) && (V1_B > UnderValue ) && (V1_C > UnderValue ))&&(F_S1 > freqUnderValue))&& (State == State_PreUnder))
				{
					UnderTimeCount = 0;
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				// UNDER RETURN
				if( (((V1_A >= UnderResValue ) && (V1_B >= UnderResValue ) && (V1_C >= UnderResValue )) &&(F_S1 >= freqUnderResValue))&& (State == State_Under) )
				{
					if((V1_A >= UnderResValue ) && (V1_B >= UnderResValue ) && (V1_C >= UnderResValue )&&(!freqUnderflag))
					{
						UnderResTimeCount = UnderResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else
					{
						UnderResTimeCount = freqNormalTimeSetValue*1000;
						if(!UnderResTimeCount)
						{
							freqUnderflag = 0 ;
						}
					}
					
					State = State_PreUnderRes;
					if(UnderResTimeCount == 0)
					{
						source1OK = 1; 
						if(State == State_PreUnderRes)/**/
						{
							
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
									if(Checksource2OK)
									{
										Checksource2OK =0;
									}
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
							
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}	
				}
				if( (((V1_A <= UnderResValue) || (V1_B <= UnderResValue) || (V1_C <= UnderResValue)) ||(F_S1 <= freqUnderResValue))&& (State == State_PreUnderRes) )
				{
					State = State_Under;
					UnderResTimeCount = 0;
					Timer_flag = 0; // stop timer
				}
				
				/*****************OVER**********************/
				if((((V1_A >= OverValue )||(V1_B >= OverValue )||(V1_C >= OverValue ))||(freq1overcount >=5))&& ((State == State_nor)||(State == State_PreUnderRes)) )
				{
					if(((V1_A >= OverValue )||(V1_B >= OverValue )||(V1_C >= OverValue )) && (freq1overcount <5)){
						OverTimeCount = OverTimSetValue*1000;
					}
					else{
						OverTimeCount = freqABnormalTimeSetValue*1000;
						if(!OverTimeCount){
							freqOverflag = 1;
						}
					}
					
					State = State_PreOver;
					if(OverTimeCount ==0)
					{
						source1OK = 0;
						State = State_Over;
						if(systemValue == main_main)
						{
							if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
								source_out = selecsource2;
								releaserelay =1;
							}

						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; //(start timer)
						}
					}
				}
				
				if( (((V1_A <= OverValue) && (V1_B <= OverValue) && (V1_C <= OverValue)) &&(F_S1 <= freqOverValue))&& (State == State_PreOver) )
				{
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				//over return
				if( (((V1_A <= OverResValue) && (V1_B <= OverResValue) && (V1_C <= OverResValue)) &&(F_S1 <= freqOverResValue))&& (State == State_Over) )
				{
					if(((V1_A <= OverResValue) && (V1_B <= OverResValue) && (V1_C <= OverResValue)) &&(!freqOverflag)){
						OverResTimeCount = OverResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						OverResTimeCount = freqNormalTimeSetValue*1000;
						if(!OverResTimeCount){
							freqOverflag = 0;
						}
					}
					
					State = State_PreOverRes;
					if(OverResTimeCount == 0)
					{
						source1OK = 1; 
						if(State == State_PreOverRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
					
				}
				if( (((V1_A >= OverResValue)||(V1_B >= OverResValue)||(V1_C >= OverResValue))||(F_S1 >= freqOverResValue) )&& ((State == State_nor)&&(State == State_PreOverRes)) )
				{
					State = State_Over;
					Timer_flag = 0; // stop timer
				}
				
					
			}
			else //*****(SourceSelectValue == SELECTSOURCE2)
			{
				/*****************UNDER**********************/
				if( (((V2_A <= UnderValue ) || (V2_B <= UnderValue ) || (V2_C <= UnderValue )) ||(F_S2 <= freqUnderValue) )&&
				((State == State_nor)||(State == State_PreOverRes)))
				{
					if((V2_A <= UnderValue ) || (V2_B <= UnderValue ) || (V2_C <= UnderValue )){
						UnderTimeCount = UnderTimSetValue*1000;
					}
					else{
						UnderTimeCount = freqABnormalTimeSetValue*1000;
						if(UnderTimeCount ==0)
						{
							freqUnderflag = 1;
						}
					}
					
					State = State_PreUnder;
					if(UnderTimeCount ==0)
					{
						source2OK = 0;
						State = State_Under;
						if(systemValue == main_main)
						{
							if(source1OK)
							{
								if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
								}
							}
							else
							{
								Checksource1OK = 1;
							}
						}
						else //(main_gens)
						{
							// impossible
						}	
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag = 1; // start timer
						}
					}
					
				}
				
				if( (((V2_A > UnderValue ) && (V2_B > UnderValue ) && (V2_C > UnderValue )) &&(F_S2 > freqUnderValue))&& (State == State_PreUnder))
				{
					UnderTimeCount = 0;
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				
				// UNDER RETURN
				if( (((V2_A >= UnderResValue ) && (V2_B >= UnderResValue ) && (V2_C >= UnderResValue )) &&(F_S2 >= freqUnderResValue))&& (State == State_Under) )
				{
					if(((V2_A >= UnderResValue ) && (V2_B >= UnderResValue ) && (V2_C >= UnderResValue )) && (!freqUnderflag)){
						UnderResTimeCount = UnderResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						UnderResTimeCount = freqNormalTimeSetValue*1000;
						if(!UnderResTimeCount)
						{
							freqUnderflag = 0 ;
						}
					}
					
					State = State_PreUnderRes;
					if(UnderResTimeCount == 0)
					{
						source2OK = 1; 
						if(State == State_PreUnderRes)/**/
						{
							
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
									if(Checksource1OK)
									{
										Checksource1OK =0;
									}
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}		
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}		
				}
				if( (((V2_A <= UnderResValue) || (V2_B <= UnderResValue) || (V2_C <= UnderResValue)) || (F_S2 <= freqUnderResValue))&& (State == State_PreUnderRes) )
				{
					State = State_Under;
					UnderResTimeCount = 0;
					Timer_flag = 0; // stop timer
				}
				
				/*****************OVER**********************/
				if( (((V2_A >= OverValue )||(V2_B >= OverValue )||(V2_C >= OverValue ))|| (freq2overcount >=5))&& ((State == State_nor)||(State == State_PreUnderRes)) )
				{
					if(((V2_A >= OverValue )||(V2_B >= OverValue )||(V2_C >= OverValue )) && (freq2overcount <5) ){
						OverTimeCount = OverTimSetValue*1000;
					}
					else{
						OverTimeCount = freqABnormalTimeSetValue*1000;
						if(!OverTimeCount){
							freqOverflag = 1;
						}
					}
					
					State = State_PreOver;
					if(OverTimeCount ==0)
					{
						source2OK = 0;
						State = State_Over;
						if(systemValue == main_main)
						{
							if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
							{
								ctrlATScount = CTRL_ATS_TIMEOUT;
								HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
								HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
								source_out = selecsource1;
								releaserelay =1;
							}

						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
				}
				
				if( (((V2_A <= OverValue) && (V2_B <= OverValue) && (V2_C <= OverValue)) && (F_S2 <= freqOverValue))&& (State == State_PreOver) )
				{
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				//over return
				if( (((V2_A <= OverResValue) && (V2_B <= OverResValue) && (V2_C <= OverResValue))&&(F_S2 <= freqOverResValue) )&& (State == State_Over) )
				{
					if(((V2_A <= OverResValue) && (V2_B <= OverResValue) && (V2_C <= OverResValue)) &&(!freqOverflag)){
						OverResTimeCount = OverResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						OverResTimeCount = freqNormalTimeSetValue*1000;
						if(!OverResTimeCount){
							freqOverflag = 0;
						}
					}
					
					State = State_PreOverRes;
					if(OverResTimeCount == 0)
					{
						source2OK = 1; 
						if(State == State_PreOverRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
					
				}
				if( (((V2_A >= OverResValue)||(V2_B >= OverResValue)||(V2_C >= OverResValue)) ||(F_S2 >= freqOverValue)) && ((State == State_nor)&&(State == State_PreOverRes)) )
				{
					State = State_Over;
					Timer_flag = 0; // stop timer
				}
					
			}
			
		}//
	}
	else if(NetworkSelectValue == sys1P2W)
	{
		source1_A = GetLineVoltageA(SOURCE1);
		freqS1 = GetFrequency(SOURCE1);
		V1_A = (uint16_t)source1_A;
			
		if(V1_A <10){
			V1_A = 0;
			if(freqS1 >0)
				freqS1 =0;
		}
		F_S1 = (uint16_t)freqS1;
		if(F_S1 >= freqOverValue)
		{
			if(++freq1overcount > 5){
				freq1overcount = 5;
			}
		}
		else
		{
			freq1overcount = 0;
		}
		
		if((V1_A > UnderValue)&&(V1_A < OverValue) && (F_S1 > freqUnderValue) && (F_S1 < freqOverValue))
		{
			source1OK = 1;
		}
		
		source2_A = GetLineVoltageA(SOURCE2);
		freqS2 = GetFrequency(SOURCE2);
		V2_A = (uint16_t)source2_A;
		
		if(V2_A <10){
			V2_A = 0;
			if(freqS2 >0)
				freqS2 =0;
		}
		F_S2 = (uint16_t)freqS2;
		if(F_S2 >= freqOverValue)
		{
			if(++freq2overcount > 5){
				freq2overcount = 5;
			}
		}
		else
		{
			freq2overcount = 0;
		}
		
		if((V2_A > UnderValue)&&(V2_A < OverValue) && (F_S2 > freqUnderValue) && (F_S2 < freqOverValue)  )
		{
			source2OK = 1;
		}
		
		if((Checksource1OK) && (source1OK) && (source2OK))
		{
			Checksource1OK =0;
		}
		if((Checksource2OK) && (source2OK) && (source1OK))
		{
			Checksource2OK =0;
		}

		if(start_ats)
		{
			if(SourceSelectValue == SELECTSOURCE1)
			{
				/*****************UNDER**********************/
				if( ((V1_A <= UnderValue )||(F_S1 <= freqUnderValue))&&((State == State_nor)||(State == State_PreOverRes)))
				{
					if(V1_A <= UnderValue){
						UnderTimeCount = UnderTimSetValue*1000;
					}
					else{
						UnderTimeCount = freqABnormalTimeSetValue*1000;
					}
					
					State = State_PreUnder;
					if(UnderTimeCount ==0)
					{
						source1OK = 0;
						State = State_Under;
						if(systemValue == main_main)
						{
							if(source2OK)
							{
								if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else
							{
								Checksource2OK = 1;
							}
						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}	
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag = 1; // start timer
						}
					}	
				}
				
				if( ((V1_A > UnderValue )&&(F_S1 > freqUnderValue)) && (State == State_PreUnder))
				{
					UnderTimeCount = 0;
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				
				if( ((V1_A >= UnderResValue )&&(F_S1 >= freqUnderResValue)) && (State == State_Under) )
				{
					if((V1_A >= UnderResValue)&&(!freqUnderflag))
					{
						UnderResTimeCount = UnderResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						UnderResTimeCount = freqNormalTimeSetValue*1000;
					}
					State = State_PreUnderRes;
					if(UnderResTimeCount == 0)
					{
						source1OK = 1; 
						if(State == State_PreUnderRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}		
				}
				if(((V1_A <= UnderResValue)||(F_S1 <= freqUnderResValue)) && (State == State_PreUnderRes) )
				{
					State = State_Under;
					UnderResTimeCount = 0;
					Timer_flag = 0; // stop timer
				}
				
				/*****************OVER**********************/
				if( ((V1_A >= OverValue )||(freq1overcount >=5)) && ((State == State_nor)||(State == State_PreUnderRes)) )
				{
					if((V1_A >= OverValue) && (freq1overcount < 5) ){
						OverTimeCount = OverTimSetValue*1000;
					}
					else{
						OverTimeCount = freqABnormalTimeSetValue*1000;
						if(!OverTimeCount){
							freqOverflag = 1;
						}
					}
					State = State_PreOver;
					if(OverTimeCount ==0)
					{
						source1OK = 0;
						State = State_Over;
						if(systemValue == main_main)
						{
							if(source2OK){
								if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else{
								Checksource2OK = 1;
							}
						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}	
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
				}
				
				if( ((V1_A <= OverValue)&&(F_S1 <= freqOverValue)) && (State == State_PreOver) )
				{
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				//over return
				if( ((V1_A <= OverResValue)&&(F_S1 <= freqOverResValue)) && (State == State_Over) )
				{
					if((V1_A <= OverResValue)&& (!freqOverflag) ){
						OverResTimeCount = OverResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						OverResTimeCount = freqNormalTimeSetValue * 1000;
						if(!OverResTimeCount){
							freqOverflag = 0;
						}
					}
					
					State = State_PreOverRes;
					if(OverResTimeCount == 0)
					{
						source1OK = 1; 
						if(State == State_PreOverRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}		
				}
				if( ((V1_A >= OverResValue)||(F_S1 >= freqOverResValue)) && ((State == State_nor)&&(State == State_PreOverRes)) )
				{
					State = State_Over;
					Timer_flag = 0; // stop timer
				}
				
				if((V1_A > UnderValue) && (V1_A < OverValue) &&
					(F_S1 > freqUnderValue) && (F_S1 < freqOverValue))
				{
					source1OK = 1;
					HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_SET);
				}
				else
				{
					source1OK = 0;
					HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_RESET);
				}
				if((V2_A > UnderValue)&&(V2_A < OverValue) &&
				(F_S2 > freqUnderValue) && (F_S2 < freqOverValue)  )
				{
					source2OK = 1;
					HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
				}
				else
				{
					source2OK = 0;
					HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_RESET);
				}
			}
			else //*****(SourceSelectValue == SELECTSOURCE2)
			{
				/*****************UNDER**********************/
				if( ((V2_A <= UnderValue )||(F_S2 <= freqUnderValue))&&((State == State_nor)||(State == State_PreOverRes)))
				{
					
					if(V2_A <= UnderValue){
						UnderTimeCount = UnderTimSetValue*1000;
					}
					else{
						UnderTimeCount = freqABnormalTimeSetValue*1000;
						if(UnderTimeCount ==0)
						{
							freqUnderflag = 1;
						}
					}
					State = State_PreUnder;
					if(UnderTimeCount ==0)
					{
						source2OK = 0;
						State = State_Under;
						if(systemValue == main_main)
						{
							if(source1OK)
							{
								if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay =1;
								}
							}
							else
							{
								Checksource1OK = 1;
							}
						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}	
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag = 1; // start timer
						}
					}
					
				}
				
				if( ((V2_A > UnderValue )&&(F_S2 > freqUnderValue) ) && (State == State_PreUnder))
				{
					UnderTimeCount = 0;
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				// UNDER Return
				if( ((V2_A >= UnderResValue ) && (F_S2 >= freqUnderResValue)) && (State == State_Under) )
				{
					if((V2_A >= UnderResValue ) && (!freqUnderflag))
					{
						UnderResTimeCount = UnderResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						UnderResTimeCount = freqNormalTimeSetValue*1000;
						if(!UnderResTimeCount)
						{
							freqUnderflag = 0 ;
						}
					}
					State = State_PreUnderRes;
					if(UnderResTimeCount == 0)
					{
						source2OK = 1; 
						if(State == State_PreUnderRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
							
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}		
				}
				if( ((V2_A <= UnderResValue)||(F_S2 <= freqUnderResValue)) && (State == State_PreUnderRes) )
				{
					State = State_Under;
					UnderResTimeCount = 0;
					Timer_flag = 0; // stop timer
				}
				
				/*****************OVER**********************/
				if( ((V2_A >= OverValue )||(freq2overcount >=5)) && ((State == State_nor)||(State == State_PreUnderRes)) )
				{
					if((V2_A >= OverValue) && (freq2overcount <5)){
						OverTimeCount = OverTimSetValue*1000;
					}
					else{
						OverTimeCount = freqABnormalTimeSetValue*1000;
						if(!OverTimeCount){
							freqOverflag = 1;
						}
					}
					State = State_PreOver;
					if(OverTimeCount ==0)
					{
						source2OK = 0;
						State = State_Over;
						if(systemValue == main_main)
						{
							if(source1OK)
							{
								if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
									source_out = selecsource1;
									releaserelay = 1;
								}
							}
							else
							{
								Checksource1OK = 1;
							}
						}
						else //(main_gens)
						{
							HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,ON_rly);
							genstart = GENSTART;
						}
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
				}
				
				if( ((V2_A <= OverValue)&&(F_S2 <= freqOverValue)) && (State == State_PreOver) )
				{
					State = State_nor;
					Timer_flag = 0; // stop timer
				}
				//over return
				if( ((V2_A <= OverResValue)&&(F_S2 <= freqOverResValue)) && (State == State_Over) )
				{
					if((V2_A <= OverResValue) && (!freqOverflag)){
						OverResTimeCount = OverResTimSetValue*1000; //1000*1ms = 1 Sec
					}
					else{
						OverResTimeCount = freqNormalTimeSetValue*1000;
						if(!OverResTimeCount){
							freqOverflag = 0;
						}
					}
					State = State_PreOverRes;
					if(OverResTimeCount == 0)
					{
						source2OK = 1; 
						if(State == State_PreOverRes)/**/
						{
							State = State_nor;
							if(systemValue == main_main)
							{
								if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
								{
									ctrlATScount = CTRL_ATS_TIMEOUT;
									HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
									HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
									source_out = selecsource2;
									releaserelay =1;
								}
							}
							else //(main_gens)
							{
								HAL_GPIO_WritePin(RLY_GENS_Port,RLY_GENS_Pin,OFF_rly);
								genstart = GENSTOP;
							}	
						}		
					}
					else
					{
						if(Timer_flag ==0)
						{
							Timer_flag =1; // start timer
						}
					}
					
				}
				if( ((V2_A >= OverResValue)||(F_S2 >= freqOverResValue)) && ((State == State_nor)&&(State == State_PreOverRes)) )
				{
					State = State_Over;
					Timer_flag = 0; // stop timer
				}
				
				if((V1_A > UnderValue) && (V1_A < OverValue) &&
					(F_S1 > freqUnderValue) && (F_S1 < freqOverValue) ){
					source1OK = 1;
					HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_SET);
				}
				else
				{
					source1OK = 0;
					HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_RESET);
				}
				if((V2_A > UnderValue)&&(V2_A < OverValue) &&
				(F_S2 > freqUnderValue) && (F_S2 < freqOverValue) )
				{
					source2OK = 1;
					HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
				}
				else
				{
					//source2OK = 0;
					HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_RESET);
				}

			} // (SourceSelectValue == SELECTSOURCE2)
		}// start_ats
		
	}
	
/*	
	source2_A = GetLineVoltageA(SOURCE2);
	if(NetworkSelectValue == sys3P4W)
	{
		source2_B = GetLineVoltageB(SOURCE2);
		source2_C = GetLineVoltageC(SOURCE2);
	}
	
	freqS2 = GetFrequency(SOURCE2);
	
	EMMInt1 = CommEnergyIC(SOURCE2, 1, EMMIntState1, 0xFFFF);
	EMMInt2 = CommEnergyIC(SOURCE2, 1, EMMState1, 0xFFFF);
	EMMInt3 = CommEnergyIC(SOURCE2, 1, PhaseLossTh, 0xFFFF);
	EMMInt4 = CommEnergyIC(SOURCE2, 1, SagTh, 0xFFFF);
	
	V1_A = (uint16_t)source1_A;
	if(NetworkSelectValue == sys3P4W)
	{
		V1_B = (uint16_t)source1_B;
		V1_C = (uint16_t)source1_C;
	}
	
	if(V1_A <10)
		V1_A = 0;
	if(NetworkSelectValue == sys3P4W)
	{
		if(V1_B <10)
			V1_B = 0;
		if(V1_C <10)
			V1_C = 0;
	}
	
	
	V2_A = (uint16_t)source2_A;
	if(NetworkSelectValue == sys3P4W)
	{
		V2_B = (uint16_t)source2_B;
		V2_C = (uint16_t)source2_C;
	}
	
	if(V2_A <10)
		V2_A = 0;
	if(NetworkSelectValue == sys3P4W)
	{
		if(V2_B <10)
			V2_B = 0;
		if(V2_C <10)
			V2_C = 0;
	}
*/

	//ats_process();

/*	
	if(NetworkSelectValue == sys3P4W)
	{
		if(((V1_A >= UnderValue) && (V1_A <= OverValue)) && 
			((V1_B >= UnderValue) && (V1_B <= OverValue)) && 
			((V1_C >= UnderValue) && (V1_C <= OverValue)) )
		{
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_SET);
			CommEnergyIC(SOURCE2, 0, EMMIntState1, 0x7000);	// jj	
	//		exit1_flag = 0;
		}
		else{
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_RESET);
		}
	}
	else // sys1P2W
	{
		if(V1_A >= UnderValue)
		{
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_SET);
			CommEnergyIC(SOURCE2, 0, EMMIntState1, 0x7000);	// jj	
		}
		else{
			HAL_GPIO_WritePin(LED_S1_GPIO_Port,LED_S1_Pin,GPIO_PIN_RESET);
		}
		
	}
	
	if(NetworkSelectValue == sys3P4W)
	{
		if(((V2_A >= UnderValue) && (V2_A <= OverValue)) && 
			((V2_B >= UnderValue) && (V2_B <= OverValue)) && 
			((V2_C >= UnderValue) && (V2_C <= OverValue)) )
		{
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
			CommEnergyIC(SOURCE2, 0, EMMIntState1, 0x7000);	// jj	
			exit1_flag = 0;
		}
		else
		{
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_RESET);
		}
	}
	else//sys1P2WS
	{
		if(V2_A >= UnderValue)
		{
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_SET);
			CommEnergyIC(SOURCE2, 0, EMMIntState1, 0x7000);	// jj	
			exit1_flag = 0;
		}
		else{
			HAL_GPIO_WritePin(LED_S2_GPIO_Port,LED_S2_Pin,GPIO_PIN_RESET);
		}
	}
*/
	
	
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  */

#define rd(j,k)  HAL_GPIO_ReadPin(j, k)
void buttonRead(void)
{
	enum{st1, st2, st3, st4,st5};
  static uint32_t state ,deb;  //deb == Debount
  uint16_t tempValue;
	char datalcd[10];

  if(rd(btn_UP_GPIO_Port,btn_UP_Pin)&&rd(btn_DW_GPIO_Port,btn_DW_Pin)&&rd(btn_EN_GPIO_Port,btn_EN_Pin)&&rd(btn_MODE_GPIO_Port,btn_MODE_Pin))
  {    
    state = st1;  
    return;
  }

  if(((!rd(btn_UP_GPIO_Port,btn_UP_Pin))||(!rd(btn_DW_GPIO_Port,btn_DW_Pin))||(!rd(btn_EN_GPIO_Port,btn_EN_Pin))||(!rd(btn_MODE_GPIO_Port,btn_MODE_Pin)))&&state == st1)
  {
    state = st2;
    deb = 2500;
    return;
  }

  if(((!rd(btn_UP_GPIO_Port,btn_UP_Pin))||(!rd(btn_DW_GPIO_Port,btn_DW_Pin))||(!rd(btn_EN_GPIO_Port,btn_EN_Pin))||(!rd(btn_MODE_GPIO_Port,btn_MODE_Pin)))&&state == st2)
  {
    if(deb)
    {
      deb--;
      return;
    }
    else
    {
			if(!rd(btn_UP_GPIO_Port,btn_UP_Pin))
			{
				if(workmodeValue == modemanual)
				{
					ctrlATScount = CTRL_ATS_TIMEOUT;
					HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
					HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
					HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
					SourceSelectValue = selecsource1;
					EEPROMWriteInt(SourceSelect_addr, SourceSelectValue);
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
					releaserelay =1;
				}
				else //Mode Auto
				{	
					switch (PageMenuCount)
          {
          	case mainpage_T:
							if(NetworkSelectValue == sys3P4W)
							{
								if(++DisplayMain >Display2_T)
									DisplayMain = Display1_T;
							}
          		break;
          	case Pagemenu1_T:
							if(++Submenu1Count > 8)
							{
								Submenu1Count =0;
							}
          		break;
						case Pagemenu2_T:
							//UnderSet_T,OvererSet_T,MainselectSet_T,ConfigSet_T,TimeSet_T
							switch (Submenu1Count)
              {
              	case UnderSet_T:
									if(++Submenu2Count > 4)
									{
										Submenu2Count =0;
									}
              		break;
              	case OvererSet_T:
									if(++Submenu2Count > 4)
									{
										Submenu2Count =0;
									}
              		break;
								case MainselectSet_T:
									if(++Submenu2Count > 1)
									{
										Submenu2Count =0;
									}
              		break;
              	case ConfigSet_T:
									if(++Submenu2Count > 1)
									{
										Submenu2Count =0;
									}
              		break;
								case TimeSet_T:
									if(++Submenu2Count > 6)
									{
										Submenu2Count =0;
									}
              		break;
								case SystemSet_T:
									if(++Submenu2Count > main_main)
									{
										Submenu2Count = main_gens;
									}
									break;
								//break;
								case FreqSet_T:
									if(++Submenu2Count > 6)
									{
										Submenu2Count =0;
									}
									break;
								case Schedule_T:
									if(++Submenu2Count > 4)
									{
										Submenu2Count =0;
									}
									break;
								
								//break;

              	default:
              		break;
              }
          		break;
							
						case Pagemenu3_T:
							switch (setvalueselect)
							{
								case VoltUnderSet:
									UnderValue++;
									break;
								case VoltUnderReturnSet:
									UnderResValue++;
									break;
								case TimeUnderSet:
									UnderTimSetValue++;
									break;
								case TimeUnderReturnSet:
									UnderResTimSetValue++;
									break;
								case VoltOverSet:
									OverValue++;
									break;
								case VoltOverReturnSet:
									OverResValue++;
									break;
								case TimeOverSet:
									OverTimSetValue++;
									break;
								case TimeOverReturnSet:
									OverResTimSetValue++;
									break;
								case DateSet:
									if(++Dateset.Date > 31 ) Dateset.Date = 1 ;
									break;
								case MonthSet:
									if(++Dateset.Month > 12 ) Dateset.Month = 1 ;
									break;
								case YearSet:
									if(++Dateset.Year > 99 ) Dateset.Year = 0 ;
									break;
								case HoursSet:
									if(++Timeset.Hours > 23) Timeset.Hours = 0;
									break;
								case MinuteSet:
									if(++Timeset.Minutes >59) Timeset.Minutes = 0;
									break;
								case SecondsSet:
									if(++Dateset.WeekDay >7) Dateset.WeekDay = 1;
									break;
								
								case FreqUnderSet:
									if(++freqUnderValue > 50)
									{
										freqUnderValue =50;
									}
									break;
									
								case FreqUnderReturnSet:
									if(++freqUnderResValue > 50)
									{
										freqUnderResValue =50;
									}
									break;
									
								case FreqOverSet:
									if(++freqOverValue > 60)
									{
										freqOverValue =60;
									}
									break;
										
								case FreqOverReturnSet:
									if(++freqOverResValue > 60)
									{
										freqOverResValue =60;
									}
									break;
											
								case FreqABNormalTimeSet:
									if(++freqABnormalTimeSetValue > 60)
									{
										freqABnormalTimeSetValue =60;
									}
									break;
											
								case FreqNormalTimeSet:
									if(++freqNormalTimeSetValue  > 60)
									{
										freqNormalTimeSetValue  =60;
									}
									break;
								case GenScheduleEnableSet:
									if(++Submenu3Count > 1)
									{
										Submenu3Count =0;
									}
								break;
								case SchedulePeriodSet:
									if(++Submenu3Count > 2)
									{
										Submenu3Count =0;
									}
								break;
								case ScheduleSetDateTimeSet:
									if(++Submenu3Count > 4)
									{
										Submenu3Count =0;
									}
								break;
								case ScheduleStartTimeSet:
									if(++genschedulestart.genschedule_time  > 360)
									{
										genschedulestart.genschedule_time  =360;
									}
								break;
										
								default:
									break;
							}

							break;
						case Pagemenu4_T:
							switch (Submenu3Count)
              {
              	case ScheduleDate_T:
									if(++genschedulestart.genschedule_date > 31){
										genschedulestart.genschedule_date = 1;
									}
              		break;
              	case ScheduleDayofweek_T:
									if(++genschedulestart.genschedule_dayofweek > 7){
											genschedulestart.genschedule_dayofweek = 1;
									}
              		break;
								case ScheduleSetHours_T:
									if(++genschedulestart.genschedule_hour > 23){
											genschedulestart.genschedule_hour = 0;
									}
              		break;
								case ScheduleMinute_T:
									if(++genschedulestart.genschedule_minute > 59){
											genschedulestart.genschedule_minute = 0;
									}
              		break;
								
              	default:
              		break;
              }
							break;
						
						default:
							break;
						}
					
				}
				state = st3;
			}
			else if(!rd(btn_DW_GPIO_Port,btn_DW_Pin))
			{
				if(workmodeValue == modemanual)
				{
					HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
					HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
					HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
					SourceSelectValue = selecsourceNON;
					EEPROMWriteInt(SourceSelect_addr, SourceSelectValue);
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
					releaserelay =1;
				}
				else //Mode Auto
				{
					switch (PageMenuCount)
          {
          	case mainpage_T:
							if(NetworkSelectValue == sys3P4W){
								if(--DisplayMain < Display1_T)
									DisplayMain = Display2_T;
							}
          		break;
          	case Pagemenu1_T:
							if(--Submenu1Count < 0)
							{
								Submenu1Count = 8;
							}
          		break;
						case Pagemenu2_T:
							switch (Submenu1Count)
              {
              	case UnderSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 4;
									}
              		break;
              	case OvererSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 4;
									}
              		break;
								case MainselectSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 1;
									}
              		break;
              	case ConfigSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 1;
									}
              		break;
								case TimeSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 6;
									}
              		break;
								case SystemSet_T:
									if(--Submenu2Count < main_gens )
									{
										Submenu2Count = main_main ;
									}
								break;
								case FreqSet_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count = 6;
									}
							break;
								case Schedule_T:
									if(--Submenu2Count < 0)
									{
										Submenu2Count =4;
									}
									break;
								
              	default:
              		break;
              }
          		break;
							
						case Pagemenu3_T:
							switch (setvalueselect)
							{
								case VoltUnderSet:
									UnderValue--;
									if(UnderValue  <0)
									{
										UnderValue =0;
									}
									break;
								case VoltUnderReturnSet:
									UnderResValue--;
									if(UnderResValue  <0)
									{
										UnderResValue =0;
									}
									break;
								case TimeUnderSet:
									UnderTimSetValue--;
									if(UnderTimSetValue <0)
									{
										UnderTimSetValue =0;
									}
									break;
								case TimeUnderReturnSet:
									UnderResTimSetValue--;
									if(UnderResTimSetValue <0)
									{
										UnderResTimSetValue =0;
									}
									break;
								case VoltOverSet:
									OverValue--;
									if(OverValue <0)
									{
										OverValue =0;
									}
									break;
								case VoltOverReturnSet:
									OverResValue--;
									if(OverResValue <0)
									{
										OverResValue =0;
									}
									break;
								case TimeOverSet:
									OverTimSetValue--;
									if(OverTimSetValue <0)
									{
										OverTimSetValue =0;
									}
									break;
								case TimeOverReturnSet:
									OverResTimSetValue--;
									if(OverResTimSetValue <0)
									{
										OverResTimSetValue =0;
									}
									break;
								case DateSet:
									if(--Dateset.Date < 1) Dateset.Date = 31 ;
									break;
								case MonthSet:
									if(--Dateset.Month < 1 ) Dateset.Month = 12 ;
									break;
								case YearSet:
									if(--Dateset.Year < 1 ) Dateset.Year = 99 ;
									break;
								case HoursSet:
									if(--Timeset.Hours >= 24) Timeset.Hours = 0;
									break;
								case MinuteSet:
									if(--Timeset.Minutes >= 60) Timeset.Minutes = 0;
									break;
								case SecondsSet:
									if(--Dateset.WeekDay <= 0) Dateset.WeekDay = 7;
								default:
									break;
								
								case FreqUnderSet:
									if(--freqUnderValue < 40)
									{
										freqUnderValue =40;
									}
									break;
									
								case FreqUnderReturnSet:
									if(--freqUnderResValue < 40)
									{
										freqUnderResValue =40;
									}
									break;
									
								case FreqOverSet:
									if(--freqOverValue < 50)
									{
										freqOverValue =50;
									}
									break;
										
								case FreqOverReturnSet:
									if(--freqOverResValue < 50)
									{
										freqOverResValue =50;
									}
									break;
											
								case FreqABNormalTimeSet:
									if(--freqABnormalTimeSetValue < 0)
									{
										freqABnormalTimeSetValue =0;
									}
									break;
											
								case FreqNormalTimeSet:
									if(--freqNormalTimeSetValue  < 0)
									{
										freqNormalTimeSetValue  =0;
									}
									break;
								case GenScheduleEnableSet:
									if(--Submenu3Count < 0)
									{
										Submenu3Count =1;
									}
								break;
								case SchedulePeriodSet:
									if(--Submenu3Count < 0)
									{
										Submenu3Count =2;
									}
								break;
								case ScheduleSetDateTimeSet:
									if(--Submenu3Count < 0)
									{
										Submenu3Count =4;
									}
								break;
								case ScheduleStartTimeSet:
									if(--genschedulestart.genschedule_time < 1)
									{
										genschedulestart.genschedule_time =1;
									}
								break;
							}
							break;
							
							case Pagemenu4_T:
								switch (Submenu3Count)
								{
									case ScheduleDate_T:
										if(--genschedulestart.genschedule_date < 1){
											genschedulestart.genschedule_date = 31;
										}
										break;
									case ScheduleDayofweek_T:
										if(--genschedulestart.genschedule_dayofweek < 1){
												genschedulestart.genschedule_dayofweek = 7;
										}
										break;
									case ScheduleSetHours_T:
										if(--genschedulestart.genschedule_hour < 1){
												genschedulestart.genschedule_hour = 23;
										}
										break;
									case ScheduleMinute_T:
										if(--genschedulestart.genschedule_minute < 1){
												genschedulestart.genschedule_minute = 59;
										}
										break;
									
									default:
										break;
								}
							break;
						
							
          	default:
          		break;
          }
		
				}
				state = st3;
			}
			else if(!rd(btn_EN_GPIO_Port,btn_EN_Pin))
			{
				if(workmodeValue == modemanual)
				{
					ctrlATScount = CTRL_ATS_TIMEOUT;
					HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
					HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);	
					HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
					SourceSelectValue = selecsource2;
					EEPROMWriteInt(SourceSelect_addr, SourceSelectValue);
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
					system_init();
					releaserelay =1;
				}
				else //Mode Auto
				{
					if((PageMenuCount == Pagemenu1_T) && (Submenu1Count == 8) )
					{	
						if(comparesettingvalue())
						{
							FlashErase();
							FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
						}
						/* Save data to Flash */
						PageMenuCount--;
					}
					else if(PageMenuCount == Pagemenu2_T)
					{
						// go back
						switch (Submenu1Count)
						{
							case UnderSet_T: //0
								switch (Submenu2Count)
								{
									case 0:
										setvalueselect = VoltUnderSet;
										break;
									case 1:
										setvalueselect = VoltUnderReturnSet;
										break;
									case 2:
										setvalueselect = TimeUnderSet;
										break;
									case 3:
										setvalueselect = TimeUnderReturnSet;
										break;
									case 4:
										PageMenuCount = mainpage_T;
										setvalueselect = NONselect;
										break;
									default:
										break;
								}
								break;
							case OvererSet_T: //1
								switch (Submenu2Count)
								{
									case 0:
										setvalueselect = 	VoltOverSet;						
										break;
									case 1:
										setvalueselect = VoltOverReturnSet;
										break;
									case 2:
										setvalueselect = TimeOverSet;
										break;
									case 3:
										setvalueselect = TimeOverReturnSet;
										break;
									case 4:
										PageMenuCount = mainpage_T;
										setvalueselect = NONselect;
										break;
									default:
										break;
								}

								break;
							case MainselectSet_T: //2
								// Save befor back
								SourceSelectValue = Submenu2Count+1;
								EEPROMWriteInt(SourceSelect_addr, SourceSelectValue);
								PageMenuCount = mainpage_T;
								setvalueselect = NONselect;
								switch (SourceSelectValue)
								{
									case SELECTSOURCE1:
										if(NetworkSelectValue == sys3P4W)
										{
											if((V1_A > UnderValue) && (V1_B > UnderValue) && (V1_C > UnderValue)&&
											(V1_A < OverValue) && (V1_B < OverValue) && (V1_C < OverValue) &&
											(F_S1 > freqUnderValue) && (F_S1 < freqOverValue) )
											{
												ctrlATScount = CTRL_ATS_TIMEOUT;
												HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
												HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
												HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
												HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
												SourceSelectValue = selecsource1;
												releaserelay =1;
											}
											else{
												Checksource1OK = 1;
											}			
										}
										else{ //sys1P2W
											if((V1_A > UnderValue) && (V1_A < OverValue) &&
											(F_S1 > freqUnderValue) && (F_S1 < freqOverValue))
											{
												ctrlATScount = CTRL_ATS_TIMEOUT;
												HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
												HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
												HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
												HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
												SourceSelectValue = selecsource1;
												releaserelay =1;
											}
											else{
												Checksource1OK = 1;
											}
										}
										break;
									case SELECTSOURCE2:
										if(NetworkSelectValue == sys3P4W)
										{
											if((V2_A > UnderValue) && (V2_B > UnderValue) && (V2_C > UnderValue)&&
											(V2_A < OverValue) && (V2_B < OverValue) && (V2_C < OverValue) &&
											(F_S2 > freqUnderValue) && (F_S2 < freqOverValue) )
											{
												ctrlATScount = CTRL_ATS_TIMEOUT;
												HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
												HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
												HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
												HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
												SourceSelectValue = selecsource2;
												releaserelay =1;
											}
											else
											{
												Checksource2OK = 1;
											}
										}
										else //sys1P2W
										{ 
											if((V2_A > UnderValue) && (V2_A < OverValue) &&
											(F_S2 > freqUnderValue) && (F_S2 < freqOverValue))
											{
												ctrlATScount = CTRL_ATS_TIMEOUT;
												HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
												HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
												HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
												HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
												SourceSelectValue = selecsource2;
												releaserelay =1;
											}
											else
											{
												Checksource2OK = 1;
											}
										}
										break;
									default:
										break;
								}
								break;
							case ConfigSet_T: //3
								// Save befor back
								NetworkSelectValue = Submenu2Count;
								EEPROMWriteInt(NetworkSelect_addr, NetworkSelectValue);
								PageMenuCount = mainpage_T;
								setvalueselect = NONselect;
								break;
							case TimeSet_T: //4
								switch (Submenu2Count)
								{
									case 0:
										setvalueselect = 	DateSet;						
										break;
									case 1:
										setvalueselect = MonthSet;
										break;
									case 2:
										setvalueselect = YearSet;
										break;
									case 3:
										setvalueselect = HoursSet;
										break;
									case 4:
										setvalueselect = MinuteSet;
										break;
									case 5:
										setvalueselect = SecondsSet;
										break;
									case 6:
										// Save and Goback
										HAL_RTC_SetTime(&hrtc, &Timeset, RTC_FORMAT_BIN);
										HAL_RTC_SetDate(&hrtc, &Dateset, RTC_FORMAT_BIN);
										PageMenuCount = mainpage_T;
										setvalueselect = NONselect;
										break;
									default:
										break;
								}
								break;
							case SystemSet_T: //5
								systemValue = Submenu2Count;
								EEPROMWriteInt(system_addr, systemValue);
								PageMenuCount = mainpage_T;
								setvalueselect = NONselect;		
							break;
							case FreqSet_T: //6
								switch (Submenu2Count)
								{
									case 0:
										setvalueselect = 	FreqUnderSet;						
										break;
									case 1:
										setvalueselect = FreqUnderReturnSet;
										break;
									case 2:
										setvalueselect = FreqOverSet;
										break;
									case 3:
										setvalueselect = FreqOverReturnSet;
										break;
									case 4:
										setvalueselect = FreqABNormalTimeSet;
										break;
									case 5:
										setvalueselect = FreqNormalTimeSet;
										break;
									case 6:
										EEPROMWriteInt(FreqUnderSet_addr , freqUnderValue);
										EEPROMWriteInt(FreqUnderReturnSet_addr , freqUnderResValue);
										EEPROMWriteInt(FreqOverSet_addr , freqOverValue);
										EEPROMWriteInt(FreqOverReturnSet_addr , freqOverResValue);
										EEPROMWriteInt(FreqABNormalTimeSet_addr , freqABnormalTimeSetValue);
										EEPROMWriteInt(FreqNormalTimeSet_addr , freqNormalTimeSetValue);
										PageMenuCount = mainpage_T;
										setvalueselect = NONselect;
										break;
									
									default:
										break;
								}		
							break;
							case Schedule_T: //7
								switch (Submenu2Count)
                {
                	case ScheduleEnable_T: //0
										setvalueselect = 	GenScheduleEnableSet;
										Submenu3Count = genschedulestart.genschedule_enable;
                		break;
                	case SchedulePeriod_T: //1
										setvalueselect = 	SchedulePeriodSet;
										Submenu3Count = genschedulestart.genschedule_every;
                		break;
									case ScheduleSetDateTime_T: //2
										setvalueselect = 	ScheduleSetDateTimeSet;
										Submenu3Count = 0;
                		break;
									case ScheduleStartTime_T: //3
										setvalueselect = 	ScheduleStartTimeSet;
										Submenu3Count = 0;
                		break;
									case ScheduleGoback_T: //4
										// Save befor goback
										EEPROMWriteInt(GesScheduleEnable_addr , genschedulestart.genschedule_enable);
										EEPROMWriteInt(GesScheduleEvery_addr , genschedulestart.genschedule_every);
										EEPROMWriteInt(GesScheduleDate_addr , genschedulestart.genschedule_date);
										EEPROMWriteInt(GesScheduleDayofweek_addr , genschedulestart.genschedule_dayofweek);
										EEPROMWriteInt(GesScheduleHour_addr , genschedulestart.genschedule_hour);
										EEPROMWriteInt(GesScheduleMinute_addr , genschedulestart.genschedule_minute);
										EEPROMWriteInt(GesScheduleTime_addr , genschedulestart.genschedule_time);									
										
										PageMenuCount = Pagemenu1_T;
//										if (Submenu2Count == ScheduleGoback_T)
//                    {
//											PageMenuCount = 0;
//                    }
//                    else
//                    {
//											PageMenuCount--;
//                    }
										//PageMenuCount = 0;
										
										
										//PageMenuCount = mainpage_T;
										PageMenuCount--;
										setvalueselect = NONselect;
                		break;
                	default:
                		break;
                }
								//Submenu3Count = 0;
								break;
							default:
								break;
						}
						PageMenuCount++;
					}
					else if(PageMenuCount == Pagemenu3_T)
					{
						switch (setvalueselect)
            {
            	case VoltUnderSet:
								EEPROMWriteInt(UnderSet_addr, UnderValue);
            		break;
            	case VoltUnderReturnSet:
								EEPROMWriteInt(UnderResSet_addr, UnderResValue);
            		break;
							case TimeUnderSet:
								EEPROMWriteInt(UnderTimSet_addr, UnderTimSetValue);
            		break;
            	case TimeUnderReturnSet:
								EEPROMWriteInt(UnderResTimSet_addr,UnderResTimSetValue );
            		break;
							case VoltOverSet:
								EEPROMWriteInt(OverSet_addr, OverValue);
            		break;
            	case VoltOverReturnSet:
								EEPROMWriteInt(OverResSet_addr, OverResValue);
            		break;
							case TimeOverSet:
								EEPROMWriteInt(OverTimSet_addr, OverTimSetValue);
            		break;
            	case TimeOverReturnSet:
								EEPROMWriteInt(OverResTimSet_addr, OverResTimSetValue);
            		break;
							case GenScheduleEnableSet:
								if(Submenu3Count){
									genschedulestart.genschedule_enable = 1;
								}
								else{
									genschedulestart.genschedule_enable = 0;
								}
								break;
							case SchedulePeriodSet:
								switch(Submenu3Count)
                {
                	case DAILY:
										genschedulestart.genschedule_every = DAILY;
                		break;
                	case WEEKLY:
										genschedulestart.genschedule_every = WEEKLY;
                		break;
									case MONTHLY:
										genschedulestart.genschedule_every = MONTHLY;
                		break;
                	default:
                		break;
                }
								break;
							case ScheduleSetDateTimeSet:
								switch (Submenu3Count)
                {
                	case ScheduleDate_T:
										
                		break;
                	case ScheduleDayofweek_T:
										
                		break;
									case ScheduleSetHours_T:
										
                		break;
                	case ScheduleMinute_T:
										
                		break;
									case Schedulegoback_T:
										//Submenu2Count = 0;
										PageMenuCount--;
										//setvalueselect = NONselect;
                		break;
                	default :
                		break;
                }
								if (Submenu3Count < Schedulegoback_T)
                {
									PageMenuCount = Pagemenu4_T;
                }
                else
                {
									PageMenuCount++;
                }
								break;

            	default:
            		break;
            }
						if (PageMenuCount == Pagemenu4_T)
            {
							
            }
            else
            {
							setvalueselect = NONselect;
							PageMenuCount--;
            }
						
					}
					else
					{
						//PageMenuCount++;
						if(PageMenuCount == mainpage_T)
						{
							Submenu1Count = 0;
							// store value
							storecomparevalue();	
						}
						else if(PageMenuCount == Pagemenu1_T)
						{
							if(Submenu1Count == SystemSet_T){
								Submenu2Count = systemValue;
							}
							else if(Submenu1Count == MainselectSet_T){
								Submenu2Count = SourceSelectValue -1; // -1 for eject non_select to menu setup
							}
							else if(Submenu1Count == ConfigSet_T){
								Submenu2Count = NetworkSelectValue;
							}
							else{
								Submenu2Count = 0;
							}
							
						}
						if(++PageMenuCount >Pagemenu3_T)
						{
							PageMenuCount = Pagemenu3_T;
						}
					}
				}
				state = st3;
			}
			else if(!rd(btn_MODE_GPIO_Port,btn_MODE_Pin))
			{
				if(workmodeValue == modeauto)
				{
					workmodeValue = modemanual;
					HAL_GPIO_WritePin(LED_Manual_GPIO_Port,LED_Manual_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED_Auto_GPIO_Port,LED_Auto_Pin,GPIO_PIN_RESET);
					EEPROMWriteInt(ModeSelect_addr, modemanual);
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
				}
				else //workmodeValue = modemanual;
				{
					workmodeValue = modeauto;
					HAL_GPIO_WritePin(LED_Manual_GPIO_Port,LED_Manual_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED_Auto_GPIO_Port,LED_Auto_Pin,GPIO_PIN_SET);
					EEPROMWriteInt(ModeSelect_addr, modeauto);
					FlashErase();
					FlashWrite(FLASH_PAGE_START_ADDRESS, (uint8_t*)Flashdata, 128);
					
					switch (SourceSelectValue)
					{
						case SELECTSOURCE1:
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
							HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
							HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
							SourceSelectValue = selecsource1;
							releaserelay =1;
							break;
						case SELECTSOURCE2:
							ctrlATScount = CTRL_ATS_TIMEOUT;
							HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
							HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);	
							HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
							SourceSelectValue = selecsource2;
							releaserelay =1;
							break;
						default:
							break;
					}
				}
				
				state = st3;
			}
			
			lcdupdate();
			menucount = MENUTIMEOUT;
			Beep();
			//++++++++++END MENU+++++++++++++++++++//
    } // End of else
  }

  
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  */
char numofstring;
void lcdupdate(void)
{
	char buff[32];
	char strbuff[20];
	//static char pageold;
	static uint16_t toggletime =0; 

//	if(pageold != PageMenuCount)
//	{
//		cleardisplay();
//	}
	cleardisplay();
	
	//  Show Setting Value
	if((setvalueselect > NONselect) && (setvalueselect <= FreqNormalTimeSet))
	{	
		switch (setvalueselect)
		{
    	case VoltUnderSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("VoltUnder", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", UnderValue);
				ssd1306_WriteString(buff, Font_11x18, White);
    		break;
    	case VoltUnderReturnSet:
				ssd1306_SetCursor(15, 3);
				ssd1306_WriteString("VoltUnderReturn", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", UnderResValue);
				ssd1306_WriteString(buff, Font_11x18, White);			
    		break;
			case TimeUnderSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("TimeUnder", Font_7x10, White);		
				
				snprintf(buff, 4, "%d  ", UnderTimSetValue);
				numofstring = 64 - (((strlen(buff)/2)*7)+3);
				ssd1306_SetCursor(numofstring , 3+15);
				ssd1306_WriteString(buff, Font_11x18, White);
				
				//ssd1306_SetCursor(47, 3+15);
				//snprintf(buff, 4, "%d  ", UnderTimSetValue);
				//ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
    	case TimeUnderReturnSet:
				ssd1306_SetCursor(15, 3);
				ssd1306_WriteString("TimeUnderReturn", Font_7x10, White);	
			
				snprintf(buff, 4, "%d  ", UnderResTimSetValue);
				numofstring = 64 - (((strlen(buff)/2)*7)+3);
				ssd1306_SetCursor(numofstring , 3+15);
				ssd1306_WriteString(buff, Font_11x18, White);
				
//				ssd1306_SetCursor(47, 3+15);
//				snprintf(buff, 4, "%d  ", UnderResTimSetValue);
//				ssd1306_WriteString(buff, Font_11x18, White);		
			
    		break;
			case VoltOverSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("VoltOver", Font_7x10, White);		
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", OverValue);
				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
    	case VoltOverReturnSet:
				ssd1306_SetCursor(15, 3);
				ssd1306_WriteString("VoltOverReturn", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", OverResValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
			
    		break;
			case TimeOverSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("TimeOver", Font_7x10, White);	

				snprintf(buff, 4, "%d  ", OverTimSetValue);
				numofstring = 64 - (((strlen(buff)/2)*7)+3);
				ssd1306_SetCursor(numofstring , 3+15);
				ssd1306_WriteString(buff, Font_11x18, White);
				
//				ssd1306_SetCursor(47, 3+15);
//				snprintf(buff, 4, "%d  ", OverTimSetValue);
//				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
    	case TimeOverReturnSet:
				ssd1306_SetCursor(15, 3);
				ssd1306_WriteString("TimeOverReturn", Font_7x10, White);

				snprintf(buff, 4, "%d  ", OverResTimSetValue);
				numofstring = 64 - (((strlen(buff)/2)*7)+3);
				ssd1306_SetCursor(numofstring , 3+15);
				ssd1306_WriteString(buff, Font_11x18, White);
				
//				ssd1306_SetCursor(47, 3+15);
//				snprintf(buff, 4, "%d  ", OverResTimSetValue);
//				ssd1306_WriteString(buff, Font_11x18, White);		
			
    		break;
			case DateSet:
				ssd1306_SetCursor(39, 3);
				ssd1306_WriteString("SetDate", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",Dateset.Date);
				ssd1306_WriteString(buff, Font_11x18, White);	
    		break;
    	case MonthSet:
				ssd1306_SetCursor(39, 3);
				ssd1306_WriteString("SetMont", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%s  ",mountname[Dateset.Month]);
				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
			case YearSet:
				ssd1306_SetCursor(39, 3);
				ssd1306_WriteString("SetYear", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",Dateset.Year);
				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
    	case HoursSet:
				ssd1306_SetCursor(32, 3);
				ssd1306_WriteString("SetHours", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",Timeset.Hours);
				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
			case MinuteSet:
				ssd1306_SetCursor(32, 3);
				ssd1306_WriteString("SetMinute", Font_7x10, White);		
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",Timeset.Minutes);
				ssd1306_WriteString(buff, Font_11x18, White);	
			
    		break;
    	case SecondsSet:
				ssd1306_SetCursor(43, 3);
				ssd1306_WriteString("SetDay", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%s  ",dayname[Dateset.WeekDay]);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqUnderSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("FreqUnder", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",freqUnderValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqUnderReturnSet:
				ssd1306_SetCursor(5, 3);
				ssd1306_WriteString("SetFreqUnderReturn", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", freqUnderResValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqOverSet:
				ssd1306_SetCursor(36, 3);
				ssd1306_WriteString("FreqOver", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ",freqOverValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqOverReturnSet:
				ssd1306_SetCursor(5, 3);
				ssd1306_WriteString("SetFreqOverReturn", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", freqOverResValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqABNormalTimeSet:
				ssd1306_SetCursor(5, 3);
				ssd1306_WriteString("FreqAbNormalTime", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", freqABnormalTimeSetValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			case FreqNormalTimeSet:
				ssd1306_SetCursor(15, 3);
				ssd1306_WriteString("FreqNormalTime", Font_7x10, White);	
				
				ssd1306_SetCursor(47, 3+15);
				snprintf(buff, 4, "%d  ", freqNormalTimeSetValue);
				ssd1306_WriteString(buff, Font_11x18, White);		
				break;
			
    	default:
    		break;
    }
	}
	else
	{
		switch (PageMenuCount)
		{
			case mainpage_T:
				//snprintf(buff, sizeof(buff), "%02f Volt", source2_A);
				ssd1306_SetCursor(3, 5);
				if(DisplayMain == Display1_T)
				{
					ssd1306_WriteString("U1[LN]", Font_7x10, White);	
				}
				else if(DisplayMain == Display2_T)
				{
					ssd1306_WriteString("U1[LL]", Font_7x10, White);	
				}
				
				if(DisplayMain == Display1_T)
				{
					snprintf(buff, 4, "%d  ", V1_A);
				}
				else if(DisplayMain == Display2_T)
				{
					snprintf(buff, 4, "%f  ", (V1_A*1.732));
				}
				if(NetworkSelectValue == sys3P4W)
				{
					//ssd1306_SetCursor(3+(6*7), 5);
					ssd1306_SetCursor(45, 5);
					ssd1306_WriteString(buff, Font_7x10, White);
				}
				else//sys1P2W
				{
					//ssd1306_SetCursor(3+((7*3)*1)+(7*7), 5);
					ssd1306_SetCursor(73, 5);
					ssd1306_WriteString(buff, Font_7x10, White);
				}
				
				if(NetworkSelectValue == sys3P4W)
				{
	
					if(DisplayMain == Display1_T)
					{
						snprintf(buff, 4, "%d  ", V1_B);
					}
					else if(DisplayMain == Display2_T)
					{
						snprintf(buff, 4, "%f  ", (V1_B*1.732));
					}
					//ssd1306_SetCursor(3+((7*3)*1)+(7*7), 5);
					ssd1306_SetCursor(73, 5);
					ssd1306_WriteString(buff, Font_7x10, White);
					
					if(DisplayMain == Display1_T)
					{
						snprintf(buff, 4, "%d  ", V1_C);
					}
					else if(DisplayMain == Display2_T)
					{
						snprintf(buff, 4, "%f  ", (V1_C*1.732));
					}
					//ssd1306_SetCursor(3+((7*3)*2)+(8*7), 5);
					ssd1306_SetCursor(101, 5);
					ssd1306_WriteString(buff, Font_7x10, White);
				}
				
				ssd1306_SetCursor(3, 17);
				if(DisplayMain == Display1_T)
				{
					ssd1306_WriteString("U2[LN]", Font_7x10, White);	
				}
				else if(DisplayMain == Display2_T)
				{
					ssd1306_WriteString("U2[LL]", Font_7x10, White);	
				}
				
				if(DisplayMain == Display1_T)
				{
					snprintf(buff, 4, "%d  ", V2_A);
				}
				else if(DisplayMain == Display2_T)
				{
					snprintf(buff, 4, "%f  ", (V2_A*1.732));
				}
				
				if(NetworkSelectValue == sys3P4W)
				{
						//ssd1306_SetCursor(3+(6*7), 17);
					ssd1306_SetCursor(45, 17);
					ssd1306_WriteString(buff, Font_7x10, White);
				}
				else//sys1P2W
				{
					//ssd1306_SetCursor(3+((7*3)*1)+(7*7), 17);
					ssd1306_SetCursor(73, 17);
					ssd1306_WriteString(buff, Font_7x10, White);
				}
				
				
				if(NetworkSelectValue == sys3P4W)
				{
					
					if(DisplayMain == Display1_T)
					{
						snprintf(buff, 4, "%d  ", V2_B);
					}
					else if(DisplayMain == Display2_T)
					{
						snprintf(buff, 4, "%f  ", (V2_B*1.732));
					}
					//ssd1306_SetCursor(3+((7*3)*1)+(7*7), 17);
					ssd1306_SetCursor(73, 17);
					ssd1306_WriteString(buff, Font_7x10, White);
					
					if(DisplayMain == Display1_T)
					{
						snprintf(buff, 4, "%d  ", V2_C);
					}
					else if(DisplayMain == Display2_T)
					{
						snprintf(buff, 4, "%f  ", (V2_C*1.732));
					}
					//ssd1306_SetCursor(3+((7*3)*2)+(8*7), 17);
					ssd1306_SetCursor(101, 17);
					ssd1306_WriteString(buff, Font_7x10, White);
				}	
				
				
				
				//ssd1306_SetCursor(3, 17+12);
				ssd1306_SetCursor(3, 29);
				ssd1306_WriteString("F1", Font_7x10, White);	
				
				snprintf(buff, 5, "%.1f", freqS1);
				//ssd1306_SetCursor(3+((7*2)+5), 17+12);
				ssd1306_SetCursor(22, 29);
				ssd1306_WriteString(buff, Font_7x10, White);
				
				//ssd1306_SetCursor(3+((7*8)+3), 29);
				ssd1306_SetCursor(62, 17+12);
				ssd1306_WriteString("F2", Font_7x10, White);
				
				snprintf(buff, 5, "%.1f", freqS2);
				//ssd1306_SetCursor(3+((7*10)+6+2), 17+12);
				ssd1306_SetCursor(81, 29);
				ssd1306_WriteString(buff, Font_7x10, White);
				
				//ssd1306_SetCursor(3+((7*14)+6+4), 17+12);
				ssd1306_SetCursor(111, 29);
				ssd1306_WriteString("Hz", Font_7x10, White);

				HAL_RTC_GetTime(&hrtc, &Timeupdate, FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc, &Dateupdate, FORMAT_BIN);
				Timeset = Timeupdate;
				Dateset = Dateupdate;
				
				//ssd1306_SetCursor(3, 17+12+12);
				ssd1306_SetCursor(3, 41);
				if(workmodeValue)
				{
					ssd1306_WriteString("Mode:Man", Font_7x10, White);
				}
				else
				{
					ssd1306_WriteString("Mode:Auto", Font_7x10, White);
				}
				
				//ssd1306_SetCursor(3+(9*8), 17+12+12);
				ssd1306_SetCursor(75, 41);
				switch (source_out)
				{
					case SELECT_NON:
						ssd1306_WriteString("OUT:NON", Font_7x10, White);
						break;
					case SELECTSOURCE1:
						ssd1306_WriteString("OUT:U1", Font_7x10, White);
						break;
					case SELECTSOURCE2:
							ssd1306_WriteString("OUT:U2", Font_7x10, White);
						break;
					default:
						break;
				}
				
				uint8_t timecountdisplay;
				if(State){ // stste = ab normal
					if(State <= State_Under){
						if(UnderTimeCount)
						{
							timecountdisplay = (UnderTimeCount/1000)+1;
							sprintf(buff,"%s: %d",statusmenu[State],timecountdisplay );
						}
						else{
							sprintf(buff,"%s",statusmenu[State]);
						}
					}
					//               2                          4
					if((State > State_Under) && (State <= State_UnderRes))
					{
						if(UnderResTimeCount)
						{
							timecountdisplay = (UnderResTimeCount/1000)+1;
							sprintf(buff,"%s: %d",statusmenu[State],timecountdisplay );
						}else{
							sprintf(buff,"%s",statusmenu[State]);
						}
					}
					
					if((State >= State_PreOver) && (State <= State_Over))
					{
						if(OverTimeCount)
						{
							timecountdisplay = (OverTimeCount/1000)+1;
							sprintf(buff,"%s: %d",statusmenu[State],timecountdisplay );
						}
						else
						{
							sprintf(buff,"%s",statusmenu[State]);
						}
					}
					if((State >= State_PreOverRes) && (State <= State_OverRes))
					{
						if(OverResTimeCount)
						{
							timecountdisplay = (OverResTimeCount/1000)+1;
							sprintf(buff,"%s: %d",statusmenu[State],timecountdisplay );
						}
						else
						{
							sprintf(buff,"%s",statusmenu[State]);
						}
					}
				}
				else { // state = normal
					if(++toggletime % 2)
					{
						sprintf(buff,"%d/%s/%d %d %d %s", Dateupdate.Date,mountname[Dateupdate.Month],Dateupdate.Year,Timeupdate.Hours,Timeupdate.Minutes,dayname[Dateupdate.WeekDay]);
					}
					else
					{
						sprintf(buff,"%d/%s/%d %d:%d %s", Dateupdate.Date,mountname[Dateupdate.Month],Dateupdate.Year,Timeupdate.Hours,Timeupdate.Minutes,dayname[Dateupdate.WeekDay]);
					}
				}		
				numofstring = 64 - (((strlen(buff)/2)*7)+3);
				//ssd1306_SetCursor(3+(14), 17+12+12+12);
				//ssd1306_SetCursor(17, 53);
				ssd1306_SetCursor(numofstring , 53);
				ssd1306_WriteString(buff, Font_7x10, White);

				break;
			case Pagemenu1_T:
				ssd1306_SetCursor(50, 3);
				ssd1306_WriteString("MENU", Font_7x10, White);	
				for(char i=0; i<5; i++)
				{
					ssd1306_SetCursor(5, 3+10+(i*10));
					if(Submenu1Count <5)
					{
						strcpy(buff, mainmenu[i]);
						if(Submenu1Count == i)
						{
							ssd1306_WriteString(buff, Font_7x10, Black);	
						}
						else
						{
							ssd1306_WriteString(buff, Font_7x10, White);	
						}
					}
					else
					{
						if((5 + i) > 8)
						{
							break;
						}
						strcpy(buff, mainmenu[i+5]);
						if(Submenu1Count == i+5){
							ssd1306_WriteString(buff, Font_7x10, Black);	
						}
						else{
							ssd1306_WriteString(buff, Font_7x10, White);	
						}
					}		
				}
				break;
			case Pagemenu2_T:
				
				for(char i=0; i<5; i++)
				{
					//UnderSet_T,OvererSet_T,MainselectSet_T,ConfigSet_T,TimeSet_T
					switch (Submenu1Count)
					{
						case UnderSet_T:
							ssd1306_SetCursor(50, 3);
							ssd1306_WriteString("UNDER", Font_7x10, White);
						
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, undermenu[i]);
								if(Submenu2Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, undermenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
							
						case OvererSet_T:
							ssd1306_SetCursor(50, 3);
							ssd1306_WriteString("OVER", Font_7x10, White); 
						
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, overmenu[i]);
								if(Submenu2Count == i){
								ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, overmenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
						case MainselectSet_T:
							ssd1306_SetCursor(30, 3);
							ssd1306_WriteString("MainSelect", Font_7x10, White);
						
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, sourceselectmenu[i]);
								if(Submenu2Count == i){
								ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, sourceselectmenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
						case ConfigSet_T:
							ssd1306_SetCursor(30, 3);
							ssd1306_WriteString("ConfigNetwork", Font_7x10, White);
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, networksystemmenu[i]);
								if(Submenu2Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{	
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, networksystemmenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
						case TimeSet_T:
							ssd1306_SetCursor(30, 3);
							ssd1306_WriteString("DateTime", Font_7x10, White);
						
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, datetimemenu[i]);
								if(Submenu2Count == i){
								ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								if((5 + i) > 6)
								{
									break;
								}
								strcpy(buff, datetimemenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
							
						case SystemSet_T:
							ssd1306_SetCursor(30, 3);
							ssd1306_WriteString("SystemConfig", Font_7x10, White);
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, systemtypemenu[i]);
								if(Submenu2Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, systemtypemenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							break;
							
						case FreqSet_T:
							ssd1306_SetCursor(30, 3);
							ssd1306_WriteString("Frequency", Font_7x10, White);
						
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, frequencymenu[i]);
								if(Submenu2Count == i){
								ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								if((5 + i) > 6)
								{
									break;
								}
								strcpy(buff, frequencymenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}			
							break;
						case Schedule_T:
							ssd1306_SetCursor(8, 3);
							ssd1306_WriteString("GenStartSchedule", Font_7x10, White);	
							
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu2Count <5){
								strcpy(buff, GenSchedulemenu[i]);
								if(Submenu2Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{					
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, GenSchedulemenu[i+5]);
								if(Submenu2Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}	
						
							break;
						default:
							break;
					}												
				}
				
				break;
				
			case Pagemenu3_T:
				for(char i=0; i<5; i++)
        {
					switch (setvalueselect)
					{
						case GenScheduleEnableSet:
							ssd1306_SetCursor(15, 3);
							ssd1306_WriteString("GenStartEnable", Font_7x10, White);	
							
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu3Count <5){
								strcpy(buff, GenStartEnablemenu[i]);
								if(Submenu3Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, GenStartEnablemenu[i+5]);
								if(Submenu3Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}	
							break;
						case SchedulePeriodSet:
							ssd1306_SetCursor(15, 3);
							ssd1306_WriteString("GenStartPeriod", Font_7x10, White);	
							
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu3Count <5){
								strcpy(buff, Periodmenu[i]);
								if(Submenu3Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, Periodmenu[i+5]);
								if(Submenu3Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}	
							break;
						case ScheduleSetDateTimeSet:
							ssd1306_SetCursor(8, 3);
							ssd1306_WriteString("GenStartDateTime", Font_7x10, White);	
							
							ssd1306_SetCursor(5, 3+10+(i*10));
							if(Submenu3Count <5){
								strcpy(buff, GenStartDateTimemenu[i]);
								if(Submenu3Count == i){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}
							else
							{
								
								if((5 + i) > 4)
								{
									break;
								}
								strcpy(buff, GenStartDateTimemenu[i+5]);
								if(Submenu3Count == i+5){
									ssd1306_WriteString(buff, Font_7x10, Black);	
								}
								else{
									ssd1306_WriteString(buff, Font_7x10, White);	
								}
							}	
							break;
						case ScheduleStartTimeSet:
							ssd1306_SetCursor(5, 3);
							ssd1306_WriteString("GenStartTime(Min)", Font_7x10, White);	
						
							ssd1306_SetCursor(47, 3+15);
							snprintf(buff, 4, "%d  ", genschedulestart.genschedule_time);
							ssd1306_WriteString(buff, Font_11x18, White);		
						
							break;
						default:
							break;
					}
        }
				
				break;
			case Pagemenu4_T:
				switch (Submenu3Count)
        {
        	case ScheduleDate_T:
						ssd1306_SetCursor(11, 3);
						ssd1306_WriteString("SetStartGenDate", Font_7x10, White);	
						
						ssd1306_SetCursor(47, 3+15);
						snprintf(buff, 4, "%d  ",genschedulestart.genschedule_date);
						ssd1306_WriteString(buff, Font_11x18, White);	
        		break;
        	case ScheduleDayofweek_T:
						ssd1306_SetCursor(11, 3);
						ssd1306_WriteString("SetStartGenDay", Font_7x10, White);	
						
						ssd1306_SetCursor(47, 3+15);
						snprintf(buff, 4, "%s  ",dayname[genschedulestart.genschedule_dayofweek]);
						ssd1306_WriteString(buff, Font_11x18, White);		
        		break;
					case ScheduleSetHours_T:
						ssd1306_SetCursor(8, 3);
						ssd1306_WriteString("SetStartGenHours", Font_7x10, White);	
						
						ssd1306_SetCursor(47, 3+15);
						snprintf(buff, 4, "%d  ",genschedulestart.genschedule_hour);
						ssd1306_WriteString(buff, Font_11x18, White);	
        		break;
					case ScheduleMinute_T:
						ssd1306_SetCursor(8, 3);
						ssd1306_WriteString("SetStartGenMinute", Font_7x10, White);	
						
						ssd1306_SetCursor(47, 3+15);
						snprintf(buff, 4, "%d  ",genschedulestart.genschedule_minute);
						ssd1306_WriteString(buff, Font_11x18, White);	
        		break;
        	default:
        		break;
        }
			
				break;
			default:
				break;
		}
	}
	
	
	
	ssd1306_UpdateScreen();
	//pageold = PageMenuCount;
	
}


void ReadSetting(void)
{
	UnderValue = *(uint8_t *)0x801F800;
	UnderValue = UnderValue<<8;
	UnderValue |= *(uint8_t *)0x801F801;
	
	OverValue = *(uint8_t *)0x801F802;
	OverValue = OverValue<<8;
	OverValue |= *(uint8_t *)0x801F803;
	
	UnderResValue = *(uint8_t *)0x801F804;
	UnderResValue = UnderResValue<<8;
	UnderResValue |= *(uint8_t *)0x801F805;
	
	OverResValue = *(uint8_t *)0x801F806;
	OverResValue = OverResValue<<8;
	OverResValue |= *(uint8_t *)0x801F807;
	
	UnderTimSetValue = *(uint8_t *)0x801F808;
	UnderTimSetValue = UnderTimSetValue<<8;
	UnderTimSetValue |= *(uint8_t *)0x801F809;
	
	OverTimSetValue = *(uint8_t *)0x801F80A;
	OverTimSetValue = OverTimSetValue<<8;
	OverTimSetValue |= *(uint8_t *)0x801F80B;
	
	UnderResTimSetValue = *(uint8_t *)0x801F80C;
	UnderResTimSetValue = UnderResTimSetValue<<8;
	UnderResTimSetValue |= *(uint8_t *)0x801F80D;
	
	OverResTimSetValue = *(uint8_t *)0x801F80E;
	OverResTimSetValue = OverResTimSetValue<<8;
	OverResTimSetValue |= *(uint8_t *)0x801F80F;
	
	SourceSelectValue = *(uint8_t *)0x801F810;
	SourceSelectValue = SourceSelectValue<<8;
	SourceSelectValue |= *(uint8_t *)0x801F811;
	
	NetworkSelectValue = *(uint8_t *)0x801F812;
	NetworkSelectValue = NetworkSelectValue<<8;
	NetworkSelectValue |= *(uint8_t *)0x801F813;
	
	workmodeValue = *(uint8_t *)0x801F814;
	workmodeValue = workmodeValue<<8;
	workmodeValue |= *(uint8_t *)0x801F815;
	
	systemValue = *(uint8_t *)0x801F816;
	systemValue = systemValue<<8;
	systemValue |= *(uint8_t *)0x801F817;
	
	freqUnderValue = *(uint8_t *)0x801F818;
	freqUnderValue = freqUnderValue<<8;
	freqUnderValue |= *(uint8_t *)0x801F819;
	
	freqUnderResValue = *(uint8_t *)0x801F81A;
	freqUnderResValue = freqUnderResValue<<8;
	freqUnderResValue |= *(uint8_t *)0x801F81B;
	
	freqOverValue = *(uint8_t *)0x801F81C;
	freqOverValue = freqOverValue<<8;
	freqOverValue |= *(uint8_t *)0x801F81D;
	
	freqOverResValue = *(uint8_t *)0x801F81E;
	freqOverResValue = freqOverResValue<<8;
	freqOverResValue |= *(uint8_t *)0x801F81F;
	
	freqABnormalTimeSetValue = *(uint8_t *)0x801F820;
	freqABnormalTimeSetValue = freqABnormalTimeSetValue<<8;
	freqABnormalTimeSetValue |= *(uint8_t *)0x801F821;
	
	freqNormalTimeSetValue = *(uint8_t *)0x801F822;
	freqNormalTimeSetValue = freqNormalTimeSetValue<<8;
	freqNormalTimeSetValue |= *(uint8_t *)0x801F823;
	
	genschedulestart.genschedule_enable	 = *(uint8_t *)0x801F824;
	genschedulestart.genschedule_enable = genschedulestart.genschedule_enable<<8;
	genschedulestart.genschedule_enable |= *(uint8_t *)0x801F825;
	
	genschedulestart.genschedule_every = *(uint8_t *)0x801F826;
	genschedulestart.genschedule_every = genschedulestart.genschedule_every<<8;
	genschedulestart.genschedule_every |= *(uint8_t *)0x801F827;
	
	genschedulestart.genschedule_date = *(uint8_t *)0x801F828;
	genschedulestart.genschedule_date = genschedulestart.genschedule_date<<8;
	genschedulestart.genschedule_date |= *(uint8_t *)0x801F829;
	
	genschedulestart.genschedule_dayofweek = *(uint8_t *)0x801F82A;
	genschedulestart.genschedule_dayofweek = genschedulestart.genschedule_dayofweek<<8;
	genschedulestart.genschedule_dayofweek |= *(uint8_t *)0x801F82B;
	
	genschedulestart.genschedule_hour = *(uint8_t *)0x801F82C;
	genschedulestart.genschedule_hour = genschedulestart.genschedule_hour<<8;
	genschedulestart.genschedule_hour |= *(uint8_t *)0x801F82D;
	
	genschedulestart.genschedule_minute = *(uint8_t *)0x801F82E;
	genschedulestart.genschedule_minute = genschedulestart.genschedule_minute<<8;
	genschedulestart.genschedule_minute |= *(uint8_t *)0x801F82F;
	
	genschedulestart.genschedule_time = *(uint8_t *)0x801F830;
	genschedulestart.genschedule_time = genschedulestart.genschedule_time<<8;
	genschedulestart.genschedule_time |= *(uint8_t *)0x801F831;
	
	//SourceSelectValue, NetworkSelectValue;
	
	if((UnderValue > 220)||(UnderValue < 150))
		UnderValue = 200;
	if((OverValue > 280)||(OverValue <235))
		OverValue = 240;
	if((UnderResValue >220)||(UnderResValue < UnderValue+5))
		UnderResValue = UnderValue +5;
	if((OverResValue >OverValue-5)||(OverResValue < 220))
		OverResValue = OverValue - 5;
	
	if((UnderTimSetValue >60)||(UnderTimSetValue <0))
		UnderTimSetValue = 0;
	if((OverTimSetValue >60)||(OverTimSetValue <0))
		OverTimSetValue = 0;
	if((UnderResTimSetValue >60)||(UnderResTimSetValue <0))
		UnderResTimSetValue = 5;
	if((OverResTimSetValue >60)||(OverResTimSetValue <0))
		OverResTimSetValue = 5;
	
	if(NetworkSelectValue > NETWORK1P2W){
		NetworkSelectValue = NETWORK3P4W;
	}
	if((systemValue > main_main)||(systemValue < main_gens)){
		systemValue = main_gens ;
	}
	
	if(systemValue == main_gens)
	{
		SourceSelectValue = SELECTSOURCE1;
		source_out = selecsource1;
	}
	else{ // mains-main
		if(SourceSelectValue > SELECTSOURCE2){
			SourceSelectValue = SELECTSOURCE1;
			source_out = selecsource1;
			
		}
		else
		{
			if(SourceSelectValue == SELECTSOURCE1){
				source_out = selecsource1;
			}
			else if(SourceSelectValue == SELECTSOURCE2){
				source_out = selecsource2;
			}
			else if(SourceSelectValue == SELECT_NON){
				source_out = selecsourceNON;
			}
		}
	}
	
	
	
	if((freqUnderValue >70)||(freqUnderValue <40))
		freqUnderValue = 47;
	if((freqUnderResValue >70)||(freqUnderResValue <40))
		freqUnderResValue = 48;
	
	if((freqOverValue >70)||(freqOverValue <50))
		freqOverValue = 53;
	if((freqOverResValue >70)||(freqOverResValue <50))
		freqOverResValue = 52;
	
	if((freqABnormalTimeSetValue >60)||(freqABnormalTimeSetValue <0))
		freqABnormalTimeSetValue = 5;
	if((freqNormalTimeSetValue >60)||(freqNormalTimeSetValue <0))
		freqNormalTimeSetValue = 5;
	
	if((genschedulestart.genschedule_enable >1)||(genschedulestart.genschedule_enable <0))
		genschedulestart.genschedule_enable = 0;
	if((genschedulestart.genschedule_every >2)||(genschedulestart.genschedule_every <0))
		genschedulestart.genschedule_every = 2;
	if((genschedulestart.genschedule_date >31)||(genschedulestart.genschedule_date <1))
		genschedulestart.genschedule_date = 1;
	if((genschedulestart.genschedule_dayofweek >7)||(genschedulestart.genschedule_dayofweek <1))
		genschedulestart.genschedule_dayofweek = 7;
	if((genschedulestart.genschedule_hour >23)||(genschedulestart.genschedule_hour <0))
		genschedulestart.genschedule_hour = 8;
	if((genschedulestart.genschedule_time >360)||(genschedulestart.genschedule_time <1))
		genschedulestart.genschedule_time = 5;
	
	EEPROMWriteInt(UnderSet_addr, UnderValue);
	EEPROMWriteInt(UnderResSet_addr, UnderResValue);
	EEPROMWriteInt(UnderTimSet_addr, UnderTimSetValue);
	EEPROMWriteInt(UnderResTimSet_addr,UnderResTimSetValue );
	EEPROMWriteInt(OverSet_addr, OverValue);
	EEPROMWriteInt(OverResSet_addr, OverResValue);
	EEPROMWriteInt(OverTimSet_addr, OverTimSetValue);
	EEPROMWriteInt(OverResTimSet_addr, OverResTimSetValue);
	EEPROMWriteInt(SourceSelect_addr, SourceSelectValue);
	EEPROMWriteInt(NetworkSelect_addr, NetworkSelectValue);
	EEPROMWriteInt(ModeSelect_addr, workmodeValue);
	EEPROMWriteInt(system_addr, systemValue);
	
	EEPROMWriteInt(FreqUnderSet_addr , freqUnderValue);
	EEPROMWriteInt(FreqUnderReturnSet_addr , freqUnderResValue);
	EEPROMWriteInt(FreqOverSet_addr , freqOverValue);
	EEPROMWriteInt(FreqOverReturnSet_addr , freqOverResValue);
	EEPROMWriteInt(FreqABNormalTimeSet_addr , freqABnormalTimeSetValue);
	EEPROMWriteInt(FreqNormalTimeSet_addr , freqNormalTimeSetValue);
	
	EEPROMWriteInt(GesScheduleEnable_addr , genschedulestart.genschedule_enable);
	EEPROMWriteInt(GesScheduleEvery_addr , genschedulestart.genschedule_every);
	EEPROMWriteInt(GesScheduleDate_addr , genschedulestart.genschedule_date);
	EEPROMWriteInt(GesScheduleDayofweek_addr , genschedulestart.genschedule_dayofweek);
	EEPROMWriteInt(GesScheduleHour_addr , genschedulestart.genschedule_hour);
	EEPROMWriteInt(GesScheduleMinute_addr , genschedulestart.genschedule_minute);
	EEPROMWriteInt(GesScheduleTime_addr , genschedulestart.genschedule_time);			
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  */
void storecomparevalue(void)
{
	UnderValue_compare = UnderValue;
	UnderResValue_compare = UnderResValue;
	UnderTimSetValue_compare = UnderTimSetValue;
	UnderResTimSetValue_compare = UnderResTimSetValue;
	
	OverValue_compare = OverValue;
	OverResValue_compare = OverResValue;
	OverTimSetValue_compare = OverTimSetValue;
	OverResTimSetValue_compare = OverResTimSetValue;
	
	SourceSelectValue_compare = SourceSelectValue;
	NetworkSelectValue_compare = NetworkSelectValue;
	systemValue_compare = systemValue;
	
	freqUnderValue_compare = freqUnderValue;
	freqUnderResValue_compare = freqUnderResValue;
	freqOverValue_compare = freqOverValue;
	freqOverResValue_compare = freqOverResValue;
	freqABnormalTimeSetValue_compare = freqABnormalTimeSetValue;
	freqNormalTimeSetValue_compare = freqNormalTimeSetValue;
	
	genschedulestart_compare.genschedule_enable = genschedulestart.genschedule_enable;
	genschedulestart_compare.genschedule_every = genschedulestart.genschedule_every;
	genschedulestart_compare.genschedule_date = genschedulestart.genschedule_date;
	genschedulestart_compare.genschedule_dayofweek = genschedulestart.genschedule_dayofweek;
	genschedulestart_compare.genschedule_hour = genschedulestart.genschedule_hour;
	genschedulestart_compare.genschedule_minute = genschedulestart.genschedule_minute;
	genschedulestart_compare.genschedule_time = genschedulestart.genschedule_time;

}

void restorevalue(void)
{
	UnderValue = UnderValue_compare;
	UnderResValue = UnderResValue_compare;
	UnderTimSetValue = UnderTimSetValue_compare;
	UnderResTimSetValue = UnderResTimSetValue_compare;
	OverValue = OverValue_compare;
	OverResValue = OverResValue_compare;
	OverTimSetValue = OverTimSetValue_compare;
	OverResTimSetValue = OverResTimSetValue_compare;
	SourceSelectValue = SourceSelectValue_compare;
	NetworkSelectValue = NetworkSelectValue_compare;
	systemValue = systemValue_compare;
	
	freqUnderValue = freqUnderValue_compare;
	freqUnderResValue = freqUnderResValue_compare;
	freqOverValue = freqOverValue_compare;
	freqOverResValue = freqOverResValue_compare;
	freqABnormalTimeSetValue = freqABnormalTimeSetValue_compare;
	freqNormalTimeSetValue = freqNormalTimeSetValue_compare;
	
	genschedulestart.genschedule_enable = genschedulestart_compare.genschedule_enable;
	genschedulestart.genschedule_every = genschedulestart_compare.genschedule_every;
	genschedulestart.genschedule_date = genschedulestart_compare.genschedule_date;
	genschedulestart.genschedule_dayofweek = genschedulestart_compare.genschedule_dayofweek;
	genschedulestart.genschedule_hour = genschedulestart_compare.genschedule_hour;
	genschedulestart.genschedule_minute = genschedulestart_compare.genschedule_minute;
	genschedulestart.genschedule_time = genschedulestart_compare.genschedule_time;
}

uint8_t comparesettingvalue(void)
{
	if((UnderValue_compare != UnderValue) ||
		(UnderResValue_compare != UnderResValue)||
		(UnderTimSetValue_compare != UnderTimSetValue)||
		(UnderResTimSetValue_compare != UnderResTimSetValue)||
		(OverValue_compare != OverValue)||
		(OverResValue_compare != OverResValue)||
		(OverTimSetValue_compare != OverTimSetValue)||
		(OverResTimSetValue_compare != OverResTimSetValue)||
		(SourceSelectValue_compare != SourceSelectValue)||
		(NetworkSelectValue_compare != NetworkSelectValue)||
		(systemValue != systemValue_compare)||
	
		(freqUnderValue != freqUnderValue_compare)||
		(freqUnderResValue != freqUnderResValue_compare)||
		(freqOverValue != freqOverValue_compare)||
		(freqOverResValue != freqOverResValue_compare)||
		(freqABnormalTimeSetValue != freqABnormalTimeSetValue_compare)||
		(freqNormalTimeSetValue != freqNormalTimeSetValue_compare)||
	
		(genschedulestart.genschedule_enable != genschedulestart_compare.genschedule_enable)||
		(genschedulestart.genschedule_every != genschedulestart_compare.genschedule_every)||
		(genschedulestart.genschedule_date != genschedulestart_compare.genschedule_date)||
		
		(genschedulestart.genschedule_dayofweek != genschedulestart_compare.genschedule_dayofweek)||
		(genschedulestart.genschedule_hour != genschedulestart_compare.genschedule_hour)||
		(genschedulestart.genschedule_minute != genschedulestart_compare.genschedule_minute)||
		(genschedulestart.genschedule_time != genschedulestart_compare.genschedule_time))
	{
		if(systemValue == main_gens)
		{
			SourceSelectValue = SELECTSOURCE1;
			source_out = selecsource1;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void system_init(void)
{
	if(systemValue == main_gens)
	{
		SourceSelectValue = SELECTSOURCE1;
		source_out = selecsource1;
	}
	switch (SourceSelectValue)
  {
  	case SELECT_NON:
			HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
			HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
			HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
			SourceSelectValue = selecsourceNON;
			releaserelay =1;
  		break;
  	case SELECTSOURCE1:
			ctrlATScount = CTRL_ATS_TIMEOUT;
			HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
			HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
			HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
			SourceSelectValue = selecsource1;
			releaserelay =1;
  		break;
		case SELECTSOURCE2:
			ctrlATScount = CTRL_ATS_TIMEOUT;
			HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
			HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);	
			HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
			SourceSelectValue = selecsource2;
			releaserelay =1;
  		break;
  	default:
  		break;
  }
	switch (workmodeValue)
	{
		case modemanual:
			HAL_GPIO_WritePin(LED_Manual_GPIO_Port,LED_Manual_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_Auto_GPIO_Port,LED_Auto_Pin,GPIO_PIN_RESET);
			break;
		case modeauto:
			HAL_GPIO_WritePin(LED_Manual_GPIO_Port,LED_Manual_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_Auto_GPIO_Port,LED_Auto_Pin,GPIO_PIN_SET);
			break;
		default:
			break;
	}
	
	ctrlATScount = CTRL_ATS_TIMEOUT;
	
}

//  GPIO_PIN_RESET = 0U,
//  GPIO_PIN_SET
GPIO_PinState s1_input, s2_input, s3_input, s4_input;
uint8_t checkauxinput(void)
{
	s1_input = HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin);
	s2_input = HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin);
	if(ctrlATScount)
	{
		if((SourceSelectValue == selecsource1)&&(s1_input == GPIO_PIN_RESET))
		{
			HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
		}
		else if((SourceSelectValue == selecsource2)&&(s2_input == GPIO_PIN_RESET))
		{
			HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
		}
		
	}
}

void cleardisplay(void)
{
	uint32_t delta;
	ssd1306_Fill(Black);
	if(PageMenuCount == mainpage_T)
	{
// write Rectangle
//		for(delta = 0; delta < 1; delta ++) {
//			ssd1306_DrawRectangle(1 + (5*delta),1 + (5*delta) ,SSD1306_WIDTH-1 - (5*delta),SSD1306_HEIGHT-1 - (5*delta),White);
//		}	
	}
	
}
void Beep(void)
{
	HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,ON_BUZZER);
	beepcount = 50;
	//HAL_Delay(50);
	//HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,OFF_BUZZER);
}
void check_releaserelay(void)
{
	if(releaserelay)
	{
		if(source_out == selecsource1)
		{
			if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin))
			{
				HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
				HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
				HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
				releaserelay =0;
			}
		}
		else if(source_out == selecsource2)
		{
			if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
			{
				HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
				HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
				HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
				releaserelay =0;
			}
		}
	}
}

// ATS Process Function
void ats_process(void)
{
	if((workmodeValue == modeauto) && (start_ats == 1 ) )
	{
		if (SourceSelectValue == SELECTSOURCE1)
    {
			if(systemValue == main_main)
			{
				if((!source1OK)&&(source2OK) && (!UnderTimeCount))
        {
					if(HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin))
					{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
						source_out = selecsource2;
					}
					else
					{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
						HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
						
					}	
        }
        else if ((source1OK) && (!UnderResTimeCount))// Return to normal
        {
					if(!HAL_GPIO_ReadPin(Digital_In2_GPIO_Port, Digital_In2_Pin)){
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
						source_out = selecsource1;
					}
					else{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
						HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
						HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
						
					}
        }
			}
			else //(main_gens)
			{

			}				
    }
    else //(SourceSelectValue == SELECTSOURCE2)
    {
			if(systemValue == main_main)
			{
				if ((!source2OK)&&(source1OK) )
        {
					if(HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin)){
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,ON_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
					}
					else{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
						HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_SET);
						HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_RESET);
						source_out = selecsource1;
					}
					

        }
        else if ((source2OK)) // Return to normal
        {
					if(!HAL_GPIO_ReadPin(Digital_In1_GPIO_Port, Digital_In1_Pin)){
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,ON_rly);
					}
					else{
						ctrlATScount = CTRL_ATS_TIMEOUT;
						HAL_GPIO_WritePin(SOURCE1_GPIO_Port,SOURCE1_Pin,OFF_rly);
						HAL_GPIO_WritePin(SOURCE2_GPIO_Port,SOURCE2_Pin,OFF_rly);
						HAL_GPIO_WritePin(LED_S1ON_GPIO_Port,LED_S1ON_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(LED_S2ON_GPIO_Port,LED_S2ON_Pin,GPIO_PIN_SET);
						source_out = selecsource2;
					}
        }
			}
			else //(main_gens)
			{

			}
    }
		
	}
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
