

#ifndef USER_APP_SENSOR_H_
#define USER_APP_SENSOR_H_

#define USING_APP_SENSOR

#define USING_APP_SENSOR_DEBUG

#define TIME_RESEND_WARNING         5

#define SCALE_SENSOR_DEFAULT        0xFE


#include "event_driven.h"
#include "user_util.h"
#include "user_app_rs485.h"

#define RESET_VALUES(sensor, ...) do {                              \
    float *vars[] = { __VA_ARGS__ };                                \
    for (int i = 0; i < (int)(sizeof(vars)/sizeof(vars[0])); i++) { \
        *vars[i] = 0;                                               \
    }                                                               \
} while(0)

#define SENSOR_CHECK(sensor, ...) do {                              \
    if ((sensor).nConnect_u8 == 0) {                                \
        (sensor).State_Connect_u8 = _SENSOR_DISCONNECT;             \
        RESET_VALUES(sensor, __VA_ARGS__);                          \
    } else if ((sensor).nConnect_u8 >= 3) {                         \
        (sensor).State_Connect_u8 = _SENSOR_CONNECT;                \
    }                                                               \
} while(0)

typedef enum
{
    _EVENT_SENSOR_ENTRY,
    _EVENT_SENSOR_LOG_TSVH,
    
    _EVENT_SENSOR_DATA_MEASURE,
    _EVENT_SENSOR_HANDLE_STATE,
    
    _EVENT_SENSOR_END,
}eKindEventSensor;

typedef enum 
{
    _OFFSET_CLO,            //Offset Clo
    _OFFSET_PH,             //Offset pH
    _OFFSET_TURB,           //Offset Turbidity
    _OFFSET_EC,             //Offset EC
    _OFFSET_SAL,            //Offset Salinity (do man)
    _OFFSET_TEMP,           //Offset Temperature
}eKindOffsetMeasure;

typedef enum
{
    _INACTIVE_SENSOR,       //Non active sensor
    _ACTIVE_SENSOR,         //Active sensor
}eKindDCU_UserSensor;

typedef enum
{
    _SS_POWER = 0,
    _SS_PH,
    _SS_CLO,
    _SS_EC,
    _SS_TURB,
    _SS_COD,
    _SS_TSS,
    _SS_NH4,
    _SS_DO,
    _SS_SALT,
    _SS_TDS,
    _SS_NO3,
    
    _END_SENSOR,
}eKind_Sensor;

typedef uint8_t (*_Cb_Handler_Sensor_Connect) (uint8_t);

typedef struct
{
    uint8_t e_Name;
    uint8_t *State_Active;
    uint8_t *State_Connect_Now;
    uint8_t State_Connect_Befor;
    uint32_t Gettick_Handle;
    _Cb_Handler_Sensor_Connect 	_Cb_Handler_SS_Connect;
}Struct_SensorWarning;

typedef struct
{
    uint8_t e_Name;
    Struct_RS485_Measure   *sMeasureSensor;
    Struct_SS_Value         *sMeasureHanlde;
    uint8_t Scale_1;
    float   ParaScale;
    uint8_t Scale_2;
}Struct_MeasureHanle;

extern sEvent_struct                sEventAppSensor[];
extern Struct_RS485_Measure            s485Measure[];
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
