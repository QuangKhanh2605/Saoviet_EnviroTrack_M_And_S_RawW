
#include "user_app_sensor.h"
#include "user_define.h"
#include "user_convert_variable.h"
#include "math.h"

/*============== Function static ===============*/
static uint8_t fevent_sensor_entry(uint8_t event);
static uint8_t fevent_sensor_log_tsvh(uint8_t event);

static uint8_t fevent_sensor_data_measure(uint8_t event);
static uint8_t fevent_sensor_handle_state(uint8_t event);
/*=================== struct ==================*/
sEvent_struct               sEventAppSensor[] = 
{
  {_EVENT_SENSOR_ENTRY,              1, 5, 60000,                fevent_sensor_entry},
  {_EVENT_SENSOR_LOG_TSVH,           0, 0, 500,                  fevent_sensor_log_tsvh},
  
  {_EVENT_SENSOR_DATA_MEASURE,       1, 5, 500,                  fevent_sensor_data_measure}, 
  {_EVENT_SENSOR_HANDLE_STATE,       0, 5, 500,                  fevent_sensor_handle_state},
};
uint8_t DurationTimeWarningSensor = 0;

uint8_t _Cb_Handle_Detect_Power(uint8_t State);
uint8_t _Cb_Handle_SS_pH(uint8_t State);
uint8_t _Cb_Handle_SS_Clo(uint8_t State);
uint8_t _Cb_Handle_SS_EC(uint8_t State);
uint8_t _Cb_Handle_SS_Turb(uint8_t State);

uint8_t _Cb_Handle_SS_COD(uint8_t State);
uint8_t _Cb_Handle_SS_TSS(uint8_t State);
uint8_t _Cb_Handle_SS_NH4(uint8_t State);
uint8_t _Cb_Handle_SS_DO(uint8_t State);

uint8_t _Cb_Handle_SS_SALT(uint8_t State);
uint8_t _Cb_Handle_SS_TDS(uint8_t State);
uint8_t _Cb_Handle_SS_NO3(uint8_t State);

Struct_RS485_Measure            s485Measure[]=
{
  {_SS_POWER},
  {_SS_PH},
  {_SS_CLO},
  {_SS_EC},
  {_SS_TURB},
  {_SS_COD},
  {_SS_TSS},
  {_SS_NH4},
  {_SS_DO},
  {_SS_SALT},
  {_SS_TDS},
  {_SS_NO3},
};

Struct_SensorWarning        sSensorWarning[] = 
{
    //e_Name        State_Active             State_Connect_Now                   State_Connect_Befor    Gettick_Handle  _Cb_Handler_SS_Connect
  {_SS_POWER,   NULL,                         NULL,                               _SENSOR_CONNECT,       0,               _Cb_Handle_Detect_Power},
  {_SS_PH,      &s485Measure[_SS_PH].sUser,   &s485Measure[_SS_PH].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_pH},
  {_SS_CLO,     &s485Measure[_SS_CLO].sUser,  &s485Measure[_SS_CLO].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_Clo},
  {_SS_EC,      &s485Measure[_SS_EC].sUser,   &s485Measure[_SS_EC].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_EC},
  {_SS_TURB,    &s485Measure[_SS_TURB].sUser, &s485Measure[_SS_TURB].sConnect_u8, _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_Turb},
  
  {_SS_COD,     &s485Measure[_SS_COD].sUser,  &s485Measure[_SS_COD].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_COD},
  {_SS_TSS,     &s485Measure[_SS_TSS].sUser,  &s485Measure[_SS_TSS].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_TSS},
  {_SS_NH4,     &s485Measure[_SS_NH4].sUser,  &s485Measure[_SS_NH4].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_NH4},
  {_SS_DO,      &s485Measure[_SS_DO].sUser,   &s485Measure[_SS_DO].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_DO},
  {_SS_SALT,    &s485Measure[_SS_SALT].sUser, &s485Measure[_SS_SALT].sConnect_u8, _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_SALT},
  
  {_SS_TDS,     &s485Measure[_SS_TDS].sUser,  &s485Measure[_SS_TDS].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_TDS},
  {_SS_NO3,     &s485Measure[_SS_NO3].sUser,  &s485Measure[_SS_NO3].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_NO3},
};

Struct_MeasureHanle         sMeasureHandle [] =
{
    //e_Name        sMeasureSensor            sMeasureHanlde              Scale_1  ParaScale  Scale_2
  {_SS_POWER,   NULL,                        NULL,                         0x00,       0,      0x00},
  {_SS_PH,      &s485Measure[_SS_PH],        &sDataSensorMeasure.spH,      0xFE,       0,      0xFE},
  {_SS_CLO,     &s485Measure[_SS_CLO],       &sDataSensorMeasure.sClo,     0xFE,       0,      0xFE},
  {_SS_EC,      &s485Measure[_SS_EC],        &sDataSensorMeasure.sEC,      0xFE,       0,      0xFE},
  {_SS_TURB,    &s485Measure[_SS_TURB],      &sDataSensorMeasure.sTurb,    0xFE,       10,     0xFE},
  
  {_SS_COD,     &s485Measure[_SS_COD],       &sDataSensorMeasure.sCOD,     0xFE,       0,      0xFE},
  {_SS_TSS,     &s485Measure[_SS_TSS],       &sDataSensorMeasure.sTSS,     0xFE,       0,      0xFE},
  {_SS_NH4,     &s485Measure[_SS_NH4],       &sDataSensorMeasure.sNH4,     0xFE,       0,      0xFE},
  {_SS_DO,      &s485Measure[_SS_DO],        &sDataSensorMeasure.sDO,      0xFE,       0,      0xFE},
  {_SS_SALT,    &s485Measure[_SS_SALT],      &sDataSensorMeasure.sSal,     0xFE,       0,      0xFE},
  
  {_SS_TDS,     &s485Measure[_SS_TDS],       &sDataSensorMeasure.sTDS,     0xFE,       0,      0xFE},
  {_SS_NO3,     &s485Measure[_SS_NO3],       &sDataSensorMeasure.sNO3,     0xFE,       0,      0xFE},
};

static inline void clear_measure_block(Struct_SS_Value *blk)
{
    blk->State_u8 = 0;
    blk->Value_i32 = 0;
    blk->Scale_u8 = 0;
}

//======================================================
static inline void set_measure_block(Struct_SS_Value *blk, float value, float offset, uint8_t scale)
{
    float stamp = value + offset;
    float factor = Calculator_Scale(scale);

    blk->State_u8 = 1;
    blk->Value_i32 = (int32_t)((stamp >= 0) ? (stamp * factor) : 0);
    blk->Scale_u8 = scale;
}

/*================= Function Handle ==============*/
static uint8_t fevent_sensor_entry(uint8_t event)
{
    fevent_enable(sEventAppSensor, _EVENT_SENSOR_HANDLE_STATE);
    return 1;
}

static uint8_t fevent_sensor_log_tsvh(uint8_t event)
{
    if (sRTC.year < 20)
    {
    	return 0;
    }
    
//    Average_One_Hour();
    
//    if(sRTC.min == 0)
//    AppSensor_Packet_TNMT();
    
    AppSensor_Log_Data_TSVH();
    
//    fevent_enable(sEventAppSensor,_EVENT_CONTROL_CLEAN_SENSOR);
    return 1;
}

static uint8_t fevent_sensor_data_measure(uint8_t event)
{
    float *ptemp = NULL;

    for(uint8_t i = _SS_POWER; i<_END_SENSOR; i++)
    {
        if (sMeasureHandle[i].sMeasureSensor->nConnect_u8 == 0) 
        {                                
            sMeasureHandle[i].sMeasureSensor->sConnect_u8 = _SENSOR_DISCONNECT;             
            sMeasureHandle[i].sMeasureSensor->Temp_f = 0;
            sMeasureHandle[i].sMeasureSensor->Value_f = 0;
        } else if (sMeasureHandle[i].sMeasureSensor->nConnect_u8 >= 3) 
        {
            sMeasureHandle[i].sMeasureSensor->sConnect_u8 = _SENSOR_CONNECT;                
        }   
        
        if (sMeasureHandle[i].sMeasureSensor->sConnect_u8 == _SENSOR_CONNECT)
        {
            uint8_t sc = (sMeasureHandle[i].sMeasureSensor->Value_f <= sMeasureHandle[i].ParaScale) ? sMeasureHandle[i].Scale_1 : sMeasureHandle[i].Scale_2;
            set_measure_block(sMeasureHandle[i].sMeasureHanlde, sMeasureHandle[i].sMeasureSensor->Value_f, sMeasureHandle[i].sMeasureSensor->Offset_f, sc);
            if (ptemp == NULL) ptemp = &sMeasureHandle[i].sMeasureSensor->Temp_f;
        }
        else clear_measure_block(sMeasureHandle[i].sMeasureHanlde);
    }

    //TEMP
    if (ptemp != NULL)
        set_measure_block(&sDataSensorMeasure.sTemp, *ptemp, 0, 0xFE);
    else
        clear_measure_block(&sDataSensorMeasure.sTemp);

    fevent_enable(sEventAppSensor, event);
    return 1;
}

static uint8_t fevent_sensor_handle_state(uint8_t event)
{   
    static uint8_t i = _SS_POWER;
    static uint8_t Power_User = _ACTIVE_SENSOR;
    static uint8_t Power_Connect = _SENSOR_CONNECT;
    
    sSensorWarning[_SS_POWER].State_Active = &Power_User;
    sSensorWarning[_SS_POWER].State_Connect_Now = &Power_Connect;
    
    if(sVout.mVol_u32 < 5000)
        *sSensorWarning[_SS_POWER].State_Connect_Now = _SENSOR_DISCONNECT;
    else
        *sSensorWarning[_SS_POWER].State_Connect_Now = _SENSOR_CONNECT;
    
    if(*sSensorWarning[i].State_Active == _ACTIVE_SENSOR)
    {
        if(*sSensorWarning[i].State_Connect_Now != sSensorWarning[i].State_Connect_Befor)
        {
            sSensorWarning[i].Gettick_Handle = HAL_GetTick();
            if(*sSensorWarning[i].State_Connect_Now == _SENSOR_DISCONNECT)
                sSensorWarning[i]._Cb_Handler_SS_Connect(*sSensorWarning[i].State_Connect_Now);
            
            sSensorWarning[i].State_Connect_Befor = *sSensorWarning[i].State_Connect_Now;
        }
        
        if(sSensorWarning[i].State_Connect_Now == _SENSOR_DISCONNECT)
        {
            if(HAL_GetTick() - sSensorWarning[i].Gettick_Handle >= DurationTimeWarningSensor*60000)
            {
                sSensorWarning[i].Gettick_Handle = HAL_GetTick();
                sSensorWarning[i]._Cb_Handler_SS_Connect(*sSensorWarning[i].State_Connect_Now);
            }
        }
    }
    else
    {
        sSensorWarning[i].Gettick_Handle = HAL_GetTick();
    }
    
    if(i+1 < _END_SENSOR)
        i++;
    else
        i = _SS_POWER;
    
    if(sSensorWarning[_SS_POWER].State_Connect_Now == _SENSOR_DISCONNECT)
    {
        i = _SS_POWER;
        for(uint8_t j = _SS_POWER + 1; j < _END_SENSOR; j++)
        {
            sSensorWarning[j].Gettick_Handle = HAL_GetTick();
            sSensorWarning[j].State_Connect_Befor = _SENSOR_DISCONNECT;
        }
    }
    else
    {
        sSensorWarning[_SS_POWER].Gettick_Handle = 0;
    }
    
    fevent_enable(sEventAppSensor, event);
    return 0;
}
/*====================Function Handle====================*/
uint8_t _Cb_Handle_Detect_Power(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x00;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_DETECT_POWER, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_pH(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x00;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_Clo(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x02;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_EC(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x04;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_Turb(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x06;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_COD(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x08;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_TSS(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0A;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_NH4(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0C;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_DO(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0E;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_SALT(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x10;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_TDS(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x12;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_NO3(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x14;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}
/*=======================Handle Sensor======================*/
void Log_EventWarnig(uint8_t Obis, uint8_t LengthData, uint8_t *aDataWaring)
{
  Get_RTC();
  
  if(sRTC.year > 20)
  {
    uint8_t     aData[10]={0};
    uint16_t    length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    
    SV_Protocol_Packet_Data(aData, &length, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
    aData[length++] = Obis;
    aData[length++] = LengthData;
    
    for(uint8_t i = 0; i < LengthData; i++)
        aData[length++] = *(aDataWaring+i);
    
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= aData[i];

    aData[length-1] = TempCrc;
  
#ifdef USING_APP_MEM
    AppMem_Write_Data(sAppMem.RecMemType_u8, _MEM_DATA_EVENT_A, 0, &aData[0], length, sRecEvent.SizeRecord_u16);
#endif
  }
}
/*==================Handle Define AT command=================*/
#ifdef USING_AT_CONFIG
void AT_CMD_Get_Time_Warning_Sensor(sData *str, uint16_t Pos)
{
    uint8_t aTemp[50] = "TimeWarningSensor: ";   //13 ki tu dau tien
    sData StrResp = {&aTemp[0], 19}; 

    Convert_Uint64_To_StringDec (&StrResp, (uint64_t) (DurationTimeWarningSensor), 0);
    Insert_String_To_String(StrResp.Data_a8, &StrResp.Length_u16, (uint8_t*)" min",0 , 4);

	Modem_Respond(PortConfig, StrResp.Data_a8, StrResp.Length_u16, 0);
}

void AT_CMD_Set_Time_Warning_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t TempU32 = 0;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length++;
        }
        TempU32 = Convert_String_To_Dec(str_Receiv->Data_a8 , length);
        if(TempU32 <=255 && TempU32 >=1)
        {
            Save_TimeWarningSensor(TempU32);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

void AT_CMD_Get_State_Sensor(sData *str, uint16_t Pos)
{
    uint8_t aTemp[200] = "State: ";   //11 ki tu dau tien
    uint16_t length = 7;

    Insert_String_To_String(aTemp, &length, (uint8_t*)"PH:",0 , 3);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_PH].sConnect_u8), 0x00);

    Insert_String_To_String(aTemp, &length, (uint8_t*)" CLO: ",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_CLO].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" EC:",0 , 4);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_EC].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TURB:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TURB].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" COD:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_COD].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TSS].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NH4].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_DO].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" SALT:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_SALT].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TDS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TDS].sConnect_u8), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NO3:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NO3].sConnect_u8), 0x00);    
	
    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Value (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Measure_Value: ";   //11 ki tu dau tien
    uint16_t length = 15;

    Insert_String_To_String(aTemp, &length, (uint8_t*)"PH:",0 , 3);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_PH].Value_f*100), 0xFE);

    Insert_String_To_String(aTemp, &length, (uint8_t*)" CLO: ",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_CLO].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" EC:",0 , 4);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_EC].Value_f), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TURB:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TURB].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" COD:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_COD].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TSS].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NH4].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_DO].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" SALT:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_SALT].Value_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TDS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TDS].Value_f), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NO3:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NO3].Value_f*100), 0xFE);    

	Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Filter (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Measure_Filter: ";   //11 ki tu dau tien
    uint16_t length = 16;

    Insert_String_To_String(aTemp, &length, (uint8_t*)"PH:",0 , 3);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_PH].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_PH].sMeasureHanlde->Scale_u8);

    Insert_String_To_String(aTemp, &length, (uint8_t*)" CLO: ",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_CLO].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_CLO].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" EC:",0 , 4);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_EC].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_EC].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TURB:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_TURB].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_TURB].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" COD:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_COD].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_COD].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_TSS].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_TSS].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_NH4].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_NH4].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_DO].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_DO].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" SALT:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_SALT].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_SALT].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TDS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_TDS].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_TDS].sMeasureHanlde->Scale_u8);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NO3:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sMeasureHandle[_SS_NO3].sMeasureHanlde->Value_i32), sMeasureHandle[_SS_NO3].sMeasureHanlde->Scale_u8);

	Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_User_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[200] = "User: ";   //11 ki tu dau tien
    uint16_t length = 6;

    Insert_String_To_String(aTemp, &length, (uint8_t*)"PH:",0 , 3);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_PH].sUser), 0x00);

    Insert_String_To_String(aTemp, &length, (uint8_t*)" CLO: ",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_CLO].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" EC:",0 , 4);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_EC].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TURB:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TURB].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" COD:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_COD].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TSS].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NH4].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_DO].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" SALT:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_SALT].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TDS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TDS].sUser), 0x00);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NO3:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NO3].sUser), 0x00);    
	
    Modem_Respond(PortConfig, aTemp, length, 0);
}

            
void AT_CMD_Set_User_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t TempU32_1 = 255; 
    uint32_t TempU32_2 = 255;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length_1 = 0;
        uint8_t length_2 = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_1++;
        }
        if(length_1 > 0)
            TempU32_1 = Convert_String_To_Dec(str_Receiv->Data_a8 , length_1);

        for(uint8_t i = length_1+1; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_2++;
        }
        if(length_2 > 0)
            TempU32_2 = Convert_String_To_Dec(&str_Receiv->Data_a8[length_1+1] , length_2);
        
        if(TempU32_1 <= _END_SENSOR && TempU32_1 > 0 && TempU32_2 <= 1)
        {
            Save_UserSensor(TempU32_1, TempU32_2);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

void AT_CMD_Get_Offset_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[200] = "Offset: ";   //11 ki tu dau tien
    uint16_t length = 8;

    Insert_String_To_String(aTemp, &length, (uint8_t*)"PH:",0 , 3);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_PH].Offset_f*100), 0xFE);

    Insert_String_To_String(aTemp, &length, (uint8_t*)" CLO: ",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_CLO].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" EC:",0 , 4);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_EC].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TURB:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TURB].Offset_f*100), 0xFE);
 
    Insert_String_To_String(aTemp, &length, (uint8_t*)" COD:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_COD].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TSS].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NH4].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_DO].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" SALT:",0 , 6);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_SALT].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TDS:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_TDS].Offset_f*100), 0xFE);
    
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NO3:",0 , 5);
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(s485Measure[_SS_NO3].Offset_f*100), 0xFE);
    
	Modem_Respond(PortConfig, aTemp, length, 0);
}

            
void AT_CMD_Set_Offset_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t Temp_U32 = 0;
    int32_t  Temp_I32 = 0;
    float    Temp_f = 0;
    uint8_t checkTemp = 0;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length_1 = 0;
        uint8_t length_2 = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_1++;
        }
        if(length_1 > 0)
            Temp_U32 = Convert_String_To_Dec(str_Receiv->Data_a8 , length_1);
        
        if(str_Receiv->Data_a8[length_1 + 1] == '-')
            checkTemp = 1;

        for(uint8_t i = length_1+checkTemp+1; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_2++;
        }
        if(length_2 > 0)
            Temp_I32 = Convert_String_To_Dec(&str_Receiv->Data_a8[length_1+checkTemp+1] , length_2);
        
        if(checkTemp == 1)
          Temp_I32 = 0 - Temp_I32;

        if(Temp_U32 <= _END_SENSOR && Temp_U32 > 0 && length_2 > 0)
        {
            Temp_f = Handle_int32_To_Float_Scale(Temp_I32, 0xFE);
            Save_OffsetMeasure(Temp_U32, Temp_f);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

#endif

/*---------------------Save and Init User Sensor----------------------*/
void Save_UserSensor(uint8_t KindSensor, uint8_t State)
{
    uint8_t aData[30] = {0};
    uint8_t length = 0;
           
    if(KindSensor < _END_SENSOR && KindSensor > 0)
        s485Measure[KindSensor].sUser = State;
    
    for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
    {
        aData[length++] = s485Measure[i].sUser;
    }

    Save_Array(ADDR_USER_SENSOR, aData, length);
}

void Init_UserSensor(void)
{
#ifdef USING_INTERNAL_MEM
    if(*(__IO uint8_t*)(ADDR_USER_SENSOR) != FLASH_BYTE_EMPTY)
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            s485Measure[i].sUser = *(__IO uint8_t*)(ADDR_USER_SENSOR+ 2 + i - 1);
            if(s485Measure[i].sUser > 1)
                s485Measure[i].sUser = 0;
        }
    }
    else
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            s485Measure[i].sUser = 0;
        }
    }
#endif    
}
/*===================Save and Init Offset Measure=================*/
void Save_OffsetMeasure(uint8_t KindOffset, float Var_Offset_f)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[300] = {0};
    uint8_t length = 0;
    
    uint32_t Stamp_Hex = 0;
   
    if(KindOffset < _END_SENSOR && KindOffset > 0)
        s485Measure[KindOffset].Offset_f = Var_Offset_f;
    
    for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
    {
        Stamp_Hex = Handle_Float_To_hexUint32(s485Measure[i].Offset_f);
        aData[length++] = Stamp_Hex >> 24;
        aData[length++] = Stamp_Hex >> 16;
        aData[length++] = Stamp_Hex >> 8;
        aData[length++] = Stamp_Hex ;
    }
    
    Save_Array(ADDR_OFFSET_MEASURE, aData, length);
#endif
}

void Init_OffsetMeasure(void)
{
#ifdef USING_INTERNAL_MEM
    uint32_t Stamp_Hex = 0;
    
    if(*(__IO uint8_t*)(ADDR_OFFSET_MEASURE) != FLASH_BYTE_EMPTY)
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            Stamp_Hex   = *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+2 + (i-1)*4) << 24;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+3 + (i-1)*4)<< 16;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+4 + (i-1)*4)<< 8;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+5 + (i-1)*4);
            if(Stamp_Hex != 0xFFFFFFFF)
                Convert_uint32Hex_To_Float(Stamp_Hex,  &s485Measure[i].Offset_f);
            else
                s485Measure[i].Offset_f = 0;
        }
    }
    else
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
            s485Measure[i].Offset_f = 0;
    }
#endif    
}

/*====================Save and Init Time Warning==================*/
void Save_TimeWarningSensor(uint8_t Duration)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[8] = {0};
    uint8_t length = 0;
    
    DurationTimeWarningSensor = Duration;
    
    aData[length++] = DurationTimeWarningSensor;

    Save_Array(ADDR_TIME_WARNING_SENSOR, aData, length);
#endif
}

void Init_TimeWarningSensor(void)
{
#ifdef USING_INTERNAL_MEM
    if(*(__IO uint8_t*)(ADDR_TIME_WARNING_SENSOR) != FLASH_BYTE_EMPTY)
    {
        DurationTimeWarningSensor = *(__IO uint8_t*)(ADDR_TIME_WARNING_SENSOR+2);
    }
    else
    {
        DurationTimeWarningSensor = TIME_RESEND_WARNING;
    }
#endif    
}

/*=====================Handle Task and Init app===================*/
void Init_AppSensor(void)
{
    Init_TimeWarningSensor();
    Init_OffsetMeasure();
    Init_UserSensor();
#ifdef USING_AT_CONFIG
    /* regis cb serial */
    CheckList_AT_CONFIG[_GET_FREQ_WARNING_SENSOR].CallBack = AT_CMD_Get_Time_Warning_Sensor;
    CheckList_AT_CONFIG[_SET_FREQ_WARNING_SENSOR].CallBack = AT_CMD_Set_Time_Warning_Sensor;
    
    CheckList_AT_CONFIG[_GET_STATE_SENSOR].CallBack = AT_CMD_Get_State_Sensor;
    
    CheckList_AT_CONFIG[_GET_MEASURE_VALUE].CallBack = AT_CMD_Get_Measure_Value;
    CheckList_AT_CONFIG[_GET_MEASURE_FILTER].CallBack = AT_CMD_Get_Measure_Filter;
    
    CheckList_AT_CONFIG[_GET_USER_SENSOR].CallBack = AT_CMD_Get_User_Sensor;
    CheckList_AT_CONFIG[_SET_USER_SENSOR].CallBack = AT_CMD_Set_User_Sensor;
    
    CheckList_AT_CONFIG[_GET_OFFSET_SENSOR].CallBack = AT_CMD_Get_Offset_Sensor;
    CheckList_AT_CONFIG[_SET_OFFSET_SENSOR].CallBack = AT_CMD_Set_Offset_Sensor;
    
    
#endif
}

uint8_t AppSensor_Task(void)
{
    uint8_t i = 0;
    uint8_t Result = false;
    for( i = 0; i < _EVENT_SENSOR_END; i++)
    {
        if(sEventAppSensor[i].e_status == 1)
        {
            Result = true;
            if((sEventAppSensor[i].e_systick == 0) ||
                ((HAL_GetTick() - sEventAppSensor[i].e_systick) >= sEventAppSensor[i].e_period))
            {
                sEventAppSensor[i].e_status = 0; //Disable event
                sEventAppSensor[i].e_systick= HAL_GetTick();
                sEventAppSensor[i].e_function_handler(i);
            }
        }
    }
    
    return Result;
}
