

#ifndef USER_APP_SENSOR_H_
#define USER_APP_SENSOR_H_

#define USING_APP_SENSOR

#define USING_APP_SENSOR_DEBUG

#define TIME_RESEND_WARNING         5

#define SCALE_SENSOR_DEFAULT        0xFE


#include "event_driven.h"
#include "user_util.h"
#include "user_app_rs485.h"

typedef enum
{
    _EVENT_SENSOR_ENTRY,
    
    _EVENT_SENSOR_HANDLE_STATE,
    
    _EVENT_SENSOR_END,
}eKindEventSensor;

typedef enum 
{
    _OFFSET_COD,            //Offset COD
    _OFFSET_PH,             //Offset pH
    _OFFSET_NH4,           //Offset Turbidity
    _OFFSET_TSS,            //Offset TSS
    _OFFSET_DO,            //Offset NH4
    _OFFSET_TEMP,             //Offset DO
}eKindOffsetMeasure;

typedef enum
{
    _INACTIVE_SENSOR,       //Non active sensor
    _ACTIVE_SENSOR,         //Active sensor
}eKindDCU_UserSensor;

typedef enum
{
    _ACTIVE_PH,             //Active sensor pH
    _ACTIVE_COD,            //Active sensor COD
    _ACTIVE_TSS,            //Active sensor TSS
    _ACTIVE_NH4,            //Active sensor NH4
    _ACTIVE_DO,             //Active sensor DO
}eKindDCU_StateSensor;

typedef enum
{
    _DETECT_POWER,
    _SENSOR_PH,
    _SENSOR_COD,
    _SENSOR_TSS,
    _SENSOR_NH4,
    _SENSOR_DO,
    
    _END_SENSOR,
}eKind_Sensor;

typedef struct
{
    float COD_f;            //Value float offset Clo
    float pH_f;             //Value float offset pH
    float NH4_f;           //Value float offset Turbidity
    float TSS_f;             //Value float offset EC
    float DO_f;            //Value float offset Salinity
    float Temp_f;           //Value float offset Temperature
}Struct_Offset_Measure;

typedef struct
{
    uint8_t User_pH;            //Chon su dung pH hay khong
    uint8_t User_COD;           //Chon su dung Clo hay khong
    uint8_t User_TSS;            //Chon su dung EC hay khong
    uint8_t User_NH4;          //Chon su dung Turb 10 NTU hay khong
    uint8_t User_DO;          //Chon su dung Turb 10 NTU hay khong
}Struct_UserSensor;

typedef uint8_t (*_Cb_Handler_Sensor_Connect) (uint8_t);

typedef struct
{
    uint8_t e_Name;
    uint8_t State_Active;
    uint8_t State_Connect_Now;
    uint8_t State_Connect_Befor;
    uint32_t Gettick_Handle;
    _Cb_Handler_Sensor_Connect 	_Cb_Handler_SS_Connect;
}Struct_SensorWarning;

extern sEvent_struct                sEventAppSensor[];
extern Struct_Offset_Measure        sOffsetMeasure;
extern Struct_UserSensor            sUserSensor;
/*=============== Function handle ================*/

uint8_t     AppSensor_Task(void);
void        Init_AppSensor(void);

void Log_EventWarnig(uint8_t Obis, uint8_t LengthData, uint8_t *aDataWaring);
void Save_TimeWarningSensor(uint8_t Duration);
void Init_TimeWarningSensor(void);

void Save_OffsetMeasure(uint8_t KindOffset, float Var_Offset_f);
void Init_OffsetMeasure(void);

void Save_UserSensor(uint8_t KindSensor, uint8_t State);
void Init_UserSensor(void);
#endif
