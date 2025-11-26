
#ifndef USER_APP_RS485_H__
#define USER_APP_RS485_H__

#define USING_APP_RS485

#include "user_util.h"
#include "event_driven.h"

#define ID_DEFAULT_OXY          5
#define ID_DEFAULT_PH           3

#define ID_DEFAULT_SS_PH        1
#define ID_DEFAULT_SS_COD       5
#define ID_DEFAULT_SS_TSS       6
#define ID_DEFAULT_SS_NH4       7
#define ID_DEFAULT_SS_DO        8

#define DEFAULT_SCALE_COD           0xFE
#define DEFAULT_SCALE_PH            0xFE
#define DEFAULT_SCALE_NH4           0xFE
#define DEFAULT_SCALE_DO            0xFE
#define DEFAULT_SCALE_TEMPERATURE   0xFE
#define DEFAULT_SCALE_TSS           0x00

typedef enum
{
    _EVENT_RS485_ENTRY,
    
    _EVENT_RS485_TRANSMIT,
    _EVENT_RS485_RECEIVE_HANDLE,
    _EVENT_RS485_RECEIVE_COMPLETE,
    
    _EVENT_RS485_WAIT_CALIB,
    _EVENT_RS485_REFRESH,
    
    _EVENT_RS485_END,
}eKindEventRs485;

typedef enum
{
    _RS485_SS_PH_OPERA = 0,
    
    _RS485_SS_COD_OPERA,
    
    _RS485_SS_TSS_OPERA,
    
    _RS485_SS_NH4_OPERA,
    
    _RS485_SS_DO_SEND_SALT,
    _RS485_SS_DO_OPERA,
    
    _RS485_SS_END,
}eKindModeModbusRTU;

typedef enum
{
    _SENSOR_DISCONNECT = 0,
    _SENSOR_CONNECT,
}eKindStateSensor;

typedef enum
{
    _RS485_UNRESPOND = 0,
    _RS485_RESPOND,
}eKindStateRs485Respond;

typedef enum
{
    _STATE_CALIB_FREE = 0,
    _STATE_CALIB_ENTER,
    _STATE_CALIB_WAIT,
    _STATE_CALIB_DONE,
    _STATE_CALIB_ERROR,
}eKindStateSendCalib;

typedef struct
{   
    uint8_t State_u8;
    uint8_t Scale_u8;
    int32_t Value_i32;
}Struct_SS_Value;

typedef struct
{
    Struct_SS_Value sCOD;
    Struct_SS_Value spH;
    Struct_SS_Value sNH4;
    Struct_SS_Value sTSS;
    Struct_SS_Value sDO;
    Struct_SS_Value sTemp;
}Struct_Data_Sensor_Measure;

typedef struct 
{
    uint8_t CountDisconnectRS485_1;
    uint8_t CountDisconnectRS485_2;
  
    uint8_t State_Wait_Calib;
}Struct_Hanlde_RS485;

typedef struct
{
    uint8_t Trans;
    uint8_t Recv;
}Struct_KindMode485;

typedef struct
{
    uint8_t State_Connect_u8;
    uint8_t Count_Disconnect;
    uint8_t State_Recv_Data;
    
    float   pH_Value_f;
    float   Temp_Value_f;
}sStruct_RS485_pH;

typedef struct
{
    uint8_t State_Connect_u8;
    uint8_t Count_Disconnect;
    uint8_t State_Recv_Data;
    
    float   COD_Value_f;
    float   Temp_Value_f;
}sStruct_RS485_COD;

typedef struct
{
    uint8_t State_Connect_u8;
    uint8_t Count_Disconnect;
    uint8_t State_Recv_Data;
    
    float   TSS_Value_f;
    float   TDS_Value_f;
    float   Sal_Value_f;
    float   Temp_Value_f;
}sStruct_RS485_TSS;

typedef struct
{
    uint8_t State_Connect_u8;
    uint8_t Count_Disconnect;
    uint8_t State_Recv_Data;
    
    float   NH4_Value_f;
    float   Temp_Value_f;
}sStruct_RS485_NH4;

typedef struct
{
    uint8_t State_Connect_u8;
    uint8_t Count_Disconnect;
    uint8_t State_Recv_Data;
    
    float   DO_Value_f;
    float   Temp_Value_f;
}sStruct_RS485_DO;

extern sEvent_struct        sEventAppRs485[];
extern Struct_KindMode485   sKindMode485;

extern Struct_Data_Sensor_Measure  sDataSensorMeasure;
extern Struct_Hanlde_RS485         sHandleRs485;

extern sStruct_RS485_pH            sRs485_pH;
extern sStruct_RS485_COD           sRs485_COD;
extern sStruct_RS485_TSS           sRs485_TSS;
extern sStruct_RS485_NH4           sRs485_NH4;
extern sStruct_RS485_DO            sRs485_DO;
/*====================Function Handle====================*/

uint8_t    AppRs485_Task(void);
void       Init_AppRs485(void);

void       Save_IdSlave(uint8_t ID_Oxy, uint8_t ID_pH);
void       Init_IdSlave(void);

void       Init_Parameter_Sensor(void);

void       Init_UartRs485(void);
void       Send_RS458_Sensor(uint8_t *aData, uint16_t Length_u16);

uint32_t   Read_Register_Rs485(uint8_t aData[], uint16_t *pos, uint8_t LengthData);

void       Handle_Data_Trans_SS_pH(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Trans_SS_COD(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Trans_SS_TSS(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Trans_SS_NH4(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Trans_SS_DO(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Trans_Sensor(sData *sFrame, uint8_t KindRecv);
void       Handle_Data_Measure(uint8_t KindRecv);

void       Handle_Data_Recv_SS_pH(sData sDataRS485, uint8_t KindRecv);
void       Handle_Data_Recv_SS_COD(sData sDataRS485, uint8_t KindRecv);
void       Handle_Data_Recv_SS_TSS(sData sDataRS485, uint8_t KindRecv);
void       Handle_Data_Recv_SS_NH4(sData sDataRS485, uint8_t KindRecv);
void       Handle_Data_Recv_SS_DO(sData sDataRS485, uint8_t KindRecv);
void       Handle_Data_Recv_Sensor(sData sDataRS485, uint8_t KindRecv);

void       Handle_State_SS_pH(uint8_t KindRecv, uint8_t KindDetect);
void       Handle_State_SS_COD(uint8_t KindRecv, uint8_t KindDetect);
void       Handle_State_SS_TSS(uint8_t KindRecv, uint8_t KindDetect);
void       Handle_State_SS_NH4(uint8_t KindRecv, uint8_t KindDetect);
void       Handle_State_SS_DO(uint8_t KindRecv, uint8_t KindDetect);
void       Handle_State_Sensor(uint8_t KindRecv, uint8_t KindDetect);

#endif

