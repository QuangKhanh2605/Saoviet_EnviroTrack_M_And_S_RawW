#include "user_app_rs485.h"
#include "user_modbus_rtu.h"
#include "user_internal_mem.h"
#include "user_rs485.h"
#include "user_app_wm.h"
#include "user_define.h"
#include "user_convert_variable.h"
/*=========================Fucntion Static=========================*/
static uint8_t fevent_rs485_entry(uint8_t event);
static uint8_t fevent_rs485_transmit(uint8_t event);
static uint8_t fevent_rs485_receive_handle(uint8_t event);
static uint8_t fevent_rs485_receive_complete(uint8_t event);

static uint8_t fevent_rs485_wait_calib(uint8_t event);
static uint8_t fevent_rs485_refresh(uint8_t event);
/*==============================Struct=============================*/
sEvent_struct               sEventAppRs485[]=
{
  {_EVENT_RS485_ENTRY,              1, 5, 30000,            fevent_rs485_entry},            //Doi slave khoi dong moi truyen opera
  {_EVENT_RS485_TRANSMIT,           0, 0, 1500,             fevent_rs485_transmit},
  {_EVENT_RS485_RECEIVE_HANDLE,     0, 5, 5,                fevent_rs485_receive_handle},
  {_EVENT_RS485_RECEIVE_COMPLETE,   0, 5, 500,              fevent_rs485_receive_complete},
  
  {_EVENT_RS485_WAIT_CALIB,         0, 5, 5000,             fevent_rs485_wait_calib},
  {_EVENT_RS485_REFRESH,            0, 5, 60000,            fevent_rs485_refresh},
};
uint16_t CountBufferHandleRecv = 0;

Struct_KindMode485  sKindMode485=
{
    .Trans = _RS485_SS_PH_OPERA,
};
extern sData sUart485;

uint8_t Kind_Trans_Calib = 0;

Struct_Data_Sensor_Measure  sDataSensorMeasure = {0};
Struct_Hanlde_RS485         sHandleRs485 = {0};

sStruct_RS485_pH            sRs485_pH = {0};
sStruct_RS485_COD           sRs485_COD = {0};
sStruct_RS485_TSS           sRs485_TSS = {0};
sStruct_RS485_NH4           sRs485_NH4 = {0};
sStruct_RS485_DO            sRs485_DO = {0};
/*========================Function Handle========================*/
static uint8_t fevent_rs485_entry(uint8_t event)
{
    fevent_enable(sEventAppRs485, _EVENT_RS485_TRANSMIT);
    fevent_enable(sEventAppRs485, _EVENT_RS485_REFRESH);
    return 1;
}

static uint8_t fevent_rs485_transmit(uint8_t event)
{
/*--------------------Hoi du lieu tu Slave--------------------*/
    uint8_t Frame[20]={0};
    sData sFrame = {&Frame[0], 0};

    Handle_Data_Trans_Sensor(&sFrame, sKindMode485.Trans);
    
    sKindMode485.Recv = sKindMode485.Trans;
    
    //Transmit RS485
    Send_RS458_Sensor(sFrame.Data_a8, sFrame.Length_u16);
    
    if(sKindMode485.Trans < _RS485_SS_DO_OPERA)
    {
        sKindMode485.Trans++;
    }
    else
    {
        sKindMode485.Trans = _RS485_SS_PH_OPERA;
    }

    fevent_active(sEventAppRs485, _EVENT_RS485_RECEIVE_HANDLE);
    fevent_enable(sEventAppRs485, _EVENT_RS485_RECEIVE_COMPLETE);
    fevent_enable(sEventAppRs485, event);
    return 1;
}

static uint8_t fevent_rs485_receive_handle(uint8_t event)
{
/*-----------------Kiem tra da nhan xong tu 485------------*/
    if(sUart485.Length_u16 != 0)
    {
        if(CountBufferHandleRecv == sUart485.Length_u16)
        {
            CountBufferHandleRecv = 0;
            fevent_active(sEventAppRs485, _EVENT_RS485_RECEIVE_COMPLETE);
            return 1;
        }
        else
        {
            CountBufferHandleRecv = sUart485.Length_u16;
        }
    }
    
    fevent_enable(sEventAppRs485, event);
    return 1;
}

static uint8_t fevent_rs485_receive_complete(uint8_t event)
{
/*------------------Xu ly chuoi nhan duoc----------------*/
    uint16_t Crc_Check = 0;
    uint16_t Crc_Recv  = 0;

    if(sUart485.Length_u16 > 2)
    {
        Crc_Recv = (sUart485.Data_a8[sUart485.Length_u16-1] << 8) |
                   (sUart485.Data_a8[sUart485.Length_u16-2]);
        Crc_Check = ModRTU_CRC(sUart485.Data_a8, sUart485.Length_u16 - 2);
        if(Crc_Check == Crc_Recv)
        {
            fevent_enable(sEventAppRs485, _EVENT_RS485_REFRESH);
            
            if(sUart485.Data_a8[0] == ID_DEFAULT_SS_PH ||
               sUart485.Data_a8[0] == ID_DEFAULT_SS_COD ||
               sUart485.Data_a8[0] == ID_DEFAULT_SS_TSS ||
               sUart485.Data_a8[0] == ID_DEFAULT_SS_NH4 ||
               sUart485.Data_a8[0] == ID_DEFAULT_SS_DO)
            {
                Handle_State_Sensor(sKindMode485.Recv, _RS485_RESPOND);
                Handle_Data_Recv_Sensor(sUart485, sKindMode485.Recv);
            }
            else
            {
                Handle_State_Sensor(sKindMode485.Recv, _RS485_UNRESPOND);
            }
        }
        else
        {
            Handle_State_Sensor(sKindMode485.Recv, _RS485_UNRESPOND);
        } 
    }
    else
    {
        Handle_State_Sensor(sKindMode485.Recv, _RS485_UNRESPOND);
    } 
    
    Handle_Data_Measure(sKindMode485.Recv);

    fevent_disable(sEventAppRs485, _EVENT_RS485_RECEIVE_HANDLE);
    return 1;
}

static uint8_t fevent_rs485_wait_calib(uint8_t event)
{
    if(sHandleRs485.State_Wait_Calib != _STATE_CALIB_DONE)
    {
        sHandleRs485.State_Wait_Calib = _STATE_CALIB_ERROR;
    }
    return 1;
}

static uint8_t fevent_rs485_refresh(uint8_t event)
{
    Init_UartRs485();
    fevent_enable(sEventAppRs485, event);
    return 1;
}
/*==================Handle Sensor pH===================*/
void Handle_Data_Trans_SS_pH(sData *sFrame, uint8_t KindTrans)
{
    switch(KindTrans)
    {
        //Trans Opera
        case _RS485_SS_PH_OPERA:
            ModRTU_Master_Read_Frame(sFrame, ID_DEFAULT_SS_PH, 0x03, 0x0002, 0x04);
            break;
      
        default:
          break;
    }
}

void Handle_Data_Recv_SS_pH(sData sDataRS485, uint8_t KindRecv)
{
    uint16_t Pos = 0;
    uint32_t Stamp_Hex = 0;
    switch(KindRecv)
    {
        //Recv Opera
        case _RS485_SS_PH_OPERA:
          Pos = 3;
            
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_pH.sPH_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_pH.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_pH.pH_Value_f);
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_pH.sTemperature_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_pH.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_pH.Temp_Value_f);
          break;
          
        default:
          break;
    }
}

void Handle_State_SS_pH(uint8_t KindRecv, uint8_t KindDetect)
{
    switch(KindRecv)
    {
        case _RS485_SS_PH_OPERA:
          if(KindDetect == _RS485_RESPOND)
          {
            sRs485_pH.Count_Disconnect = 0;
            sRs485_pH.State_Connect_u8 = _SENSOR_CONNECT;
            
            if(KindRecv == _RS485_SS_PH_OPERA)
              sRs485_pH.State_Recv_Data = 1;
          }
          else
          {
            if(sRs485_pH.Count_Disconnect < 3)
                sRs485_pH.Count_Disconnect++;
          }
          break;
          
        default:
          break;
    }
    
    if(sRs485_pH.Count_Disconnect == 0)
    {
        
    }
    else if(sRs485_pH.Count_Disconnect >=3)
    {
        sRs485_pH.State_Connect_u8 = _SENSOR_DISCONNECT;
        sRs485_pH.pH_Value_f = 0;
        sRs485_pH.Temp_Value_f = 0;
        sRs485_pH.State_Recv_Data = 0;
    }
}

/*==================Handle Sensor COD===================*/

void Handle_Data_Trans_SS_COD(sData *sFrame, uint8_t KindTrans)
{
    switch(KindTrans)
    {
        //Trans Opera
        case _RS485_SS_COD_OPERA:
            ModRTU_Master_Read_Frame(sFrame, ID_DEFAULT_SS_COD, 0x03, 0x0002, 0x04);
            break;
        
        default:
          break;
    }
}

void Handle_Data_Recv_SS_COD(sData sDataRS485, uint8_t KindRecv)
{
    uint16_t Pos = 0;
    uint32_t Stamp_Hex = 0;
    
    switch(KindRecv)
    {
        //Recv Opera 
        case _RS485_SS_COD_OPERA:
          Pos = 3;
            
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Clo.sClo_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Clo.sClo_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_COD.COD_Value_f);

          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Clo.sTemperature_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Clo.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_COD.Temp_Value_f);
          break;

        default:
          break;
    }
}

void Handle_State_SS_COD(uint8_t KindRecv, uint8_t KindDetect)
{
    switch(KindRecv)
    {
        case _RS485_SS_COD_OPERA:
          if(KindDetect == _RS485_RESPOND)
          {
             sRs485_COD.Count_Disconnect = 0;
             sRs485_COD.State_Connect_u8 = _SENSOR_CONNECT;
             
             if(KindRecv == _RS485_SS_COD_OPERA)
             {
                 sRs485_COD.State_Recv_Data = 1;
             }
          }
          else
          {
             if(sRs485_COD.Count_Disconnect <3)
                sRs485_COD.Count_Disconnect++;
          }
          break;
          
        default:
          break;
    }
    
    if(sRs485_COD.Count_Disconnect == 0)
    {
        
    }
    else if(sRs485_COD.Count_Disconnect >=3)
    {
        sRs485_COD.State_Connect_u8 = _SENSOR_DISCONNECT;
        sRs485_COD.COD_Value_f = 0;
        sRs485_COD.Temp_Value_f = 0;
        
        sRs485_COD.State_Recv_Data = 0;
    }
}

/*==================Handle Sensor TSS===================*/
void Handle_Data_Trans_SS_TSS(sData *sFrame, uint8_t KindTrans)
{
    switch(KindTrans)
    {
        //Send Opera
        case _RS485_SS_TSS_OPERA:
            ModRTU_Master_Read_Frame(sFrame, ID_DEFAULT_SS_TSS, 0x03, 0x0002, 4);
            break;
      
        default:
          break;
    }
}

void Handle_Data_Recv_SS_TSS(sData sDataRS485, uint8_t KindRecv)
{
    uint16_t Pos = 0;
    uint32_t Stamp_Hex = 0;
    switch(KindRecv)
    {
        //Recv Opera
        case _RS485_SS_TSS_OPERA:
          Pos = 3;

          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_EC.sResistivity_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_EC.sResistivity_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_TSS.TSS_Value_f);
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_EC.sTemperature_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_EC.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_TSS.Temp_Value_f);
          break;
      
        default:
          break;
    }
}

void Handle_State_SS_TSS(uint8_t KindRecv, uint8_t KindDetect)
{
    switch(KindRecv)
    {
        case _RS485_SS_TSS_OPERA:
          if(KindDetect == _RS485_RESPOND)
          {
             sRs485_TSS.Count_Disconnect = 0;
             sRs485_TSS.State_Connect_u8 = _SENSOR_CONNECT;
             
             if(KindRecv == _RS485_SS_TSS_OPERA)
             {
                sRs485_TSS.State_Recv_Data = 1;
             }
          }
          else
          {
             if(sRs485_TSS.Count_Disconnect <3)
               sRs485_TSS.Count_Disconnect++;
          }
          break;
          
        default:
          break;
    }
    
    if(sRs485_TSS.Count_Disconnect == 0)
    {
        
    }
    else if(sRs485_TSS.Count_Disconnect >=3)
    {
        sRs485_TSS.State_Connect_u8 = _SENSOR_DISCONNECT;
        
        sRs485_TSS.TSS_Value_f = 0;
        sRs485_TSS.Temp_Value_f = 0;
        sRs485_TSS.TDS_Value_f = 0;
        sRs485_TSS.Sal_Value_f = 0;
        
        sRs485_TSS.State_Recv_Data = 0;
    }
}

/*==================Handle Sensor NH4===================*/
void Handle_Data_Trans_SS_NH4(sData *sFrame, uint8_t KindTrans)
{
    switch(KindTrans)
    {
        //Send Opera
        case _RS485_SS_NH4_OPERA:
            ModRTU_Master_Read_Frame(sFrame, ID_DEFAULT_SS_NH4, 0x03, 0x0002, 0x04);
            break;
      
        default:
          break;
    }
}

void Handle_Data_Recv_SS_NH4(sData sDataRS485, uint8_t KindRecv)
{
    uint16_t Pos = 0;
    uint32_t Stamp_Hex = 0;
    switch(KindRecv)
    {
        //Recv Opera
        case _RS485_SS_NH4_OPERA:
          Pos = 3;
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Turbidity.sNTU_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Turbidity.sNTU_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_NH4.NH4_Value_f);
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Turbidity.sTemperature_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Turbidity.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_NH4.Temp_Value_f);
          break; 
          
        default:
          break;
    }
}
void Handle_State_SS_NH4(uint8_t KindRecv, uint8_t KindDetect)
{
    switch(KindRecv)
    {
        case _RS485_SS_NH4_OPERA:
          if(KindDetect == _RS485_RESPOND)
          {
            sRs485_NH4.Count_Disconnect = 0;
            sRs485_NH4.State_Connect_u8 = _SENSOR_CONNECT;
            
            if(KindRecv  == _RS485_SS_NH4_OPERA)
            {
                sRs485_NH4.State_Recv_Data = 1;
            }
          }
          else
          {
            if(sRs485_NH4.Count_Disconnect <3)
            sRs485_NH4.Count_Disconnect++;
          }
          break;
          
        default:
          break;
    }
    
    if(sRs485_NH4.Count_Disconnect == 0)
    {
        
    }
    else if(sRs485_NH4.Count_Disconnect >=3)
    {
        sRs485_NH4.State_Connect_u8 = _SENSOR_DISCONNECT;
        
        sRs485_NH4.NH4_Value_f = 0;
        sRs485_NH4.Temp_Value_f = 0;
        
        sRs485_NH4.State_Recv_Data = 0;
    }
}

/*==================Handle Sensor DO===================*/
void Handle_Data_Trans_SS_DO(sData *sFrame, uint8_t KindTrans)
{
    uint8_t aData[4] = {0};
    float  Send_f = 0;
    uint32_t Send_u32 = 0;
    switch(KindTrans)
    {
        //Send Opera
        case _RS485_SS_DO_SEND_SALT:
//            if(sRs485_pH.State_Connect_u8 == _SENSOR_CONNECT)
//                ph_Send_f = sRs485_pH.pH_Value_f;
//            else
                Send_f = 0;
              
            Send_u32 = Handle_Float_To_hexUint32(Send_f);
            aData[0] = Send_u32 >> 8;
            aData[1] = Send_u32;
            aData[2] = Send_u32 >> 24;
            aData[3] = Send_u32 >> 16;
            ModRTU_Master_Write_Frame(sFrame, ID_DEFAULT_SS_DO, 0x10, 0x0008, 2, aData);
            break;
      
        case _RS485_SS_DO_OPERA:
            ModRTU_Master_Read_Frame(sFrame, ID_DEFAULT_SS_DO, 0x03, 0x0002, 0x04);
            break;
      
        default:
          break;
    }
}

void Handle_Data_Recv_SS_DO(sData sDataRS485, uint8_t KindRecv)
{
    uint16_t Pos = 0;
    uint32_t Stamp_Hex = 0;
    switch(KindRecv)
    {
        //Recv Opera
        case _RS485_SS_DO_SEND_SALT:
            break;
        case _RS485_SS_DO_OPERA:
          Pos = 3;
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Turbidity.sNTU_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Turbidity.sNTU_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_DO.DO_Value_f);
          
          Stamp_Hex = Read_Register_Rs485(sDataRS485.Data_a8, &Pos, 4);
//          sSensor_Turbidity.sTemperature_Value.Value =  Handle_HexFloat_To_Int32_Round(Stamp_Hex, sSensor_Turbidity.sTemperature_Value.Scale);
          Convert_uint32Hex_To_Float(Stamp_Hex, &sRs485_DO.Temp_Value_f);
          break; 
          
        default:
          break;
    }
}
void Handle_State_SS_DO(uint8_t KindRecv, uint8_t KindDetect)
{
    switch(KindRecv)
    {
        case _RS485_SS_DO_OPERA:
          if(KindDetect == _RS485_RESPOND)
          {
            sRs485_DO.Count_Disconnect = 0;
            sRs485_DO.State_Connect_u8 = _SENSOR_CONNECT;
            
            if(KindRecv  == _RS485_SS_DO_OPERA)
            {
                sRs485_DO.State_Recv_Data = 1;
            }
          }
          else
          {
            if(sRs485_DO.Count_Disconnect <3)
            sRs485_DO.Count_Disconnect++;
          }
          break;
          
        default:
          break;
    }
    
    if(sRs485_DO.Count_Disconnect == 0)
    {
        
    }
    else if(sRs485_DO.Count_Disconnect >=3)
    {
        sRs485_DO.State_Connect_u8 = _SENSOR_DISCONNECT;
        
        sRs485_DO.DO_Value_f = 0;
        sRs485_DO.Temp_Value_f = 0;
        
        sRs485_DO.State_Recv_Data = 0;
    }
}
/*=====================Handle Sensor=====================*/
void Handle_Data_Trans_Sensor(sData *sFrame, uint8_t KindRecv)
{
    Handle_Data_Trans_SS_pH(sFrame, KindRecv);
    Handle_Data_Trans_SS_COD(sFrame, KindRecv);
    Handle_Data_Trans_SS_TSS(sFrame, KindRecv);
    Handle_Data_Trans_SS_NH4(sFrame, KindRecv);
    Handle_Data_Trans_SS_DO(sFrame, KindRecv);
}

void Handle_Data_Recv_Sensor(sData sDataRS485, uint8_t KindRecv)
{
    if(sDataRS485.Data_a8[0] == ID_DEFAULT_SS_PH)
        Handle_Data_Recv_SS_pH(sDataRS485, KindRecv);
    
    if(sDataRS485.Data_a8[0] == ID_DEFAULT_SS_COD)
        Handle_Data_Recv_SS_COD(sDataRS485, KindRecv);
    
    if(sDataRS485.Data_a8[0] == ID_DEFAULT_SS_TSS)
        Handle_Data_Recv_SS_TSS(sDataRS485, KindRecv);
    
    if(sDataRS485.Data_a8[0] == ID_DEFAULT_SS_NH4)
        Handle_Data_Recv_SS_NH4(sDataRS485, KindRecv);
    
    if(sDataRS485.Data_a8[0] == ID_DEFAULT_SS_DO)
        Handle_Data_Recv_SS_DO(sDataRS485, KindRecv);
}

void Handle_State_Sensor(uint8_t KindRecv, uint8_t KindDetect)
{
    Handle_State_SS_pH(KindRecv, KindDetect);
    Handle_State_SS_COD(KindRecv, KindDetect);
    Handle_State_SS_TSS(KindRecv, KindDetect);
    Handle_State_SS_NH4(KindRecv, KindDetect);
    Handle_State_SS_DO(KindRecv, KindDetect);
}

void Handle_Data_Measure(uint8_t KindRecv)
{
    uint8_t Recv_Temp = 0;
    float   Stamp = 0;
    if(sUserSensor.User_pH == _INACTIVE_SENSOR && sRs485_pH.State_Recv_Data == 1)
    {
        Stamp = sRs485_pH.pH_Value_f + sOffsetMeasure.pH_f;
        sDataSensorMeasure.spH.State_u8 = 1;
        sDataSensorMeasure.spH.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
        sDataSensorMeasure.spH.Scale_u8 = 0xFE;
        
        if(Recv_Temp == 0)
            Recv_Temp = 1;
    }
    else
    {
        sDataSensorMeasure.spH.State_u8 = 0;
        sDataSensorMeasure.spH.Value_i32 = 0;
        sDataSensorMeasure.spH.Scale_u8 = 0;
    }
    
    if(sUserSensor.User_COD == _INACTIVE_SENSOR && sRs485_COD.State_Recv_Data == 1)
    {
        Stamp = sRs485_COD.COD_Value_f + sOffsetMeasure.COD_f;
        sDataSensorMeasure.sCOD.State_u8 = 1;
        sDataSensorMeasure.sCOD.Value_i32 = (int32_t)(int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
        sDataSensorMeasure.sCOD.Scale_u8 = 0xFE;
        
        if(Recv_Temp == 0)
            Recv_Temp = 2;
    }
    else
    {
        sDataSensorMeasure.sCOD.State_u8 = 0;
        sDataSensorMeasure.sCOD.Value_i32 = 0;
        sDataSensorMeasure.sCOD.Scale_u8 = 0;
    }
    
    if(sUserSensor.User_TSS == _INACTIVE_SENSOR && sRs485_TSS.State_Recv_Data == 1)
    {
        Stamp = sRs485_TSS.TSS_Value_f + sOffsetMeasure.TSS_f;
        sDataSensorMeasure.sTSS.State_u8 = 1;
        sDataSensorMeasure.sTSS.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 10) : 0);
        sDataSensorMeasure.sTSS.Scale_u8 = 0xFF;
 
        if(Recv_Temp == 0)
            Recv_Temp = 3;
    }
    else
    {
        sDataSensorMeasure.sTSS.State_u8 = 0;
        sDataSensorMeasure.sTSS.Value_i32 = 0;
        sDataSensorMeasure.sTSS.Scale_u8 = 0;
    }
    
    if(sUserSensor.User_NH4 == _INACTIVE_SENSOR && sRs485_NH4.State_Recv_Data == 1)
    {
        Stamp = sRs485_NH4.NH4_Value_f + sOffsetMeasure.NH4_f;
        sDataSensorMeasure.sNH4.State_u8 = 1;
        if(sRs485_NH4.NH4_Value_f <= 10)
        {
            sDataSensorMeasure.sNH4.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sNH4.Scale_u8 = 0xFE;
        }
        else
        {
            sDataSensorMeasure.sNH4.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 10) : 0);
            sDataSensorMeasure.sNH4.Scale_u8 = 0xFF;
        }
        
        if(Recv_Temp == 0)
            Recv_Temp = 4;
    }
    else
    {
        sDataSensorMeasure.sNH4.State_u8 = 0;
        sDataSensorMeasure.sNH4.Value_i32 = 0;
        sDataSensorMeasure.sNH4.Scale_u8 = 0;
    }
    
    if(sUserSensor.User_DO == _INACTIVE_SENSOR && sRs485_DO.State_Recv_Data == 1)
    {
        Stamp = sRs485_DO.DO_Value_f + sOffsetMeasure.DO_f;
        sDataSensorMeasure.sDO.State_u8 = 1;
        if(sRs485_DO.DO_Value_f <= 10)
        {
            sDataSensorMeasure.sDO.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sDO.Scale_u8 = 0xFE;
        }
        else
        {
            sDataSensorMeasure.sDO.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 10) : 0);
            sDataSensorMeasure.sDO.Scale_u8 = 0xFF;
        }
        
        if(Recv_Temp == 0)
            Recv_Temp = 5;
    }
    else
    {
        sDataSensorMeasure.sDO.State_u8 = 0;
        sDataSensorMeasure.sDO.Value_i32 = 0;
        sDataSensorMeasure.sDO.Scale_u8 = 0;
    }
    
    switch(Recv_Temp)
    {
        case 1:
            Stamp = sRs485_pH.Temp_Value_f + sOffsetMeasure.Temp_f;
            sDataSensorMeasure.sTemp.State_u8 = 1;
            sDataSensorMeasure.sTemp.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sTemp.Scale_u8 = 0xFE;
            break;
            
        case 2:
            Stamp = sRs485_COD.Temp_Value_f + sOffsetMeasure.Temp_f;
            sDataSensorMeasure.sTemp.State_u8 = 1;
            sDataSensorMeasure.sTemp.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sTemp.Scale_u8 = 0xFE;
            break;
            
        case 3:
            Stamp = sRs485_TSS.Temp_Value_f + sOffsetMeasure.Temp_f;
            sDataSensorMeasure.sTemp.State_u8 = 1;
            sDataSensorMeasure.sTemp.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sTemp.Scale_u8 = 0xFE;
            break;
            
        case 4:
            Stamp = sRs485_NH4.Temp_Value_f + sOffsetMeasure.Temp_f;
            sDataSensorMeasure.sTemp.State_u8 = 1;
            sDataSensorMeasure.sTemp.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sTemp.Scale_u8 = 0xFE;
            break;
            
        case 5:
            Stamp = sRs485_DO.Temp_Value_f + sOffsetMeasure.Temp_f;
            sDataSensorMeasure.sTemp.State_u8 = 1;
            sDataSensorMeasure.sTemp.Value_i32 = (int32_t)((Stamp >= 0) ? (Stamp * 100) : 0);
            sDataSensorMeasure.sTemp.Scale_u8 = 0xFE;
            break;
            
        default:
            sDataSensorMeasure.sTemp.State_u8 = 0;
            sDataSensorMeasure.sTemp.Value_i32 = 0;
            sDataSensorMeasure.sTemp.Scale_u8 = 0;
            break;
    }
}
/*==========================Handle==========================*/

///*-----------------------Handle Send RS485-----------------------*/
///*

/*
    @brief Send 485 sensor
*/
void        Send_RS458_Sensor(uint8_t *aData, uint16_t Length_u16) 
{
//    HAL_GPIO_WritePin(RS485_TXDE_S_GPIO_Port, RS485_TXDE_S_Pin, GPIO_PIN_SET);
//    HAL_Delay(5);
//    HAL_UART_Transmit(&uart_485, aData, Length_u16, 1000);
//    
//    UTIL_MEM_set(sUart485.Data_a8 , 0x00, sUart485.Length_u16);
//    sUart485.Length_u16 = 0;
//    CountBufferHandleRecv = 0;
//    
//    HAL_GPIO_WritePin(RS485_TXDE_S_GPIO_Port, RS485_TXDE_S_Pin, GPIO_PIN_RESET);
  

    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    // Send
    RS485_Init_Data();
    HAL_UART_Transmit(&uart_rs485, aData , Length_u16, 1000); 
    
    //Dua DE ve Receive
    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_RESET);
}

/*----------------------Save and Init Id Slave---------------------*/
void        Save_IdSlave(uint8_t ID_Oxy, uint8_t ID_pH)
{
//    uint8_t aData[2] = {0};
//    uint8_t length = 0;
//    
//    sIdSlave485.Oxy = ID_Oxy;
//    sIdSlave485.pH  = ID_pH;
//    aData[length++] = sIdSlave485.Oxy ;
//    aData[length++] = sIdSlave485.pH;
//
//    Save_Array(ADDR_INFOR_SLAVE_RS485, aData, length);
}

void        Init_IdSlave(void)
{
//    if(*(__IO uint8_t*)(ADDR_INFOR_SLAVE_RS485) == BYTE_TEMP_FIRST)
//    {
//        sIdSlave485.Oxy = *(__IO uint8_t*)(ADDR_INFOR_SLAVE_RS485+2);
//        sIdSlave485.pH  = *(__IO uint8_t*)(ADDR_INFOR_SLAVE_RS485+3);
//    }
}

void        Init_Parameter_Sensor(void)
{

}
/*=========================Handle Data=======================*/
uint32_t Read_Register_Rs485(uint8_t aData[], uint16_t *pos, uint8_t LengthData)
{
    uint32_t stamp = 0;
    uint16_t length = *pos;
    if(LengthData == 4)
    {
        stamp = aData[length+2]<<8 | aData[length+3];
        stamp = (stamp << 16) | (aData[length]<<8 | aData[length+1]);
    }
    else if(LengthData == 2)
    {
        stamp = aData[length]<<8 | aData[length+1];
    }
    *pos = *pos + LengthData;
    return stamp;
}

/*==================Handle Define AT command=================*/
#ifdef USING_AT_CONFIG
void AT_CMD_Get_State_Sensor(sData *str, uint16_t Pos)
{
    uint8_t aTemp[60] = "State_Sensor COD:";   //11 ki tu dau tien
    uint16_t length = 17;

    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sRs485_COD.State_Connect_u8), 0x00);
    Insert_String_To_String(aTemp, &length, (uint8_t*)" pH:",0 , 4);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sRs485_pH.State_Connect_u8), 0x00);
    Insert_String_To_String(aTemp, &length, (uint8_t*)" TSS:",0 , 5);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sRs485_TSS.State_Connect_u8), 0x00);
    Insert_String_To_String(aTemp, &length, (uint8_t*)" NH4:",0 , 5);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sRs485_NH4.State_Connect_u8), 0x00);
    Insert_String_To_String(aTemp, &length, (uint8_t*)" DO:",0 , 4);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sRs485_DO.State_Connect_u8), 0x00);
//    Insert_String_To_String(aTemp, &length, (uint8_t*)" \r\n",0 , 3);

	Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Value (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[80] = "Measure_Value: COD=";   //11 ki tu dau tien
    uint16_t length = 19;

    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sCOD.Value_i32), sDataSensorMeasure.sCOD.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,pH=",0 , 8);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.spH.Value_i32), sDataSensorMeasure.spH.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)",Temp=",0 , 6);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sTemp.Value_i32), sDataSensorMeasure.sTemp.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"'C,TSS=",0 , 7);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sTSS.Value_i32), sDataSensorMeasure.sTSS.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,DO=",0 , 8);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sDO.Value_i32), sDataSensorMeasure.sDO.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,nh4=",0 , 9);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sNH4.Value_i32), sDataSensorMeasure.sNH4.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L",0 , 4);

	Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Filter (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[80] = "Measure_Filter: COD=";   //11 ki tu dau tien
    uint16_t length = 20;

    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sCOD.Value_i32), sDataSensorMeasure.sCOD.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,pH=",0 , 8);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.spH.Value_i32), sDataSensorMeasure.spH.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)",Temp=",0 , 6);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sTemp.Value_i32), sDataSensorMeasure.sTemp.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"'C,TSS=",0 , 7);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sTSS.Value_i32), sDataSensorMeasure.sTSS.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,DO=",0 , 8);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sDO.Value_i32), sDataSensorMeasure.sDO.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L,nh4=",0 , 9);
    
    Convert_Point_Int_To_String_Scale (aTemp, &length, (int)(sDataSensorMeasure.sNH4.Value_i32), sDataSensorMeasure.sNH4.Scale_u8);
    Insert_String_To_String(aTemp, &length, (uint8_t*)"mg/L",0 , 4);

	Modem_Respond(PortConfig, aTemp, length, 0);
}
#endif
/*==================Handle Task and Init app=================*/
void Init_UartRs485(void)
{
    RS485_Stop_RX_Mode();
    WM_DIG_Init_Uart(&uart_rs485, sWmDigVar.sModbInfor[0].MType_u8);
    RS485_Init_RX_Mode();
}

void       Init_AppRs485(void)
{
    Init_UartRs485();
    Init_IdSlave();
    Init_Parameter_Sensor();
#ifdef USING_AT_CONFIG
    /* regis cb serial */
    CheckList_AT_CONFIG[_GET_STATE_SENSOR].CallBack = AT_CMD_Get_State_Sensor;
    CheckList_AT_CONFIG[_GET_MEASURE_VALUE].CallBack = AT_CMD_Get_Measure_Value;
    CheckList_AT_CONFIG[_GET_MEASURE_FILTER].CallBack = AT_CMD_Get_Measure_Filter;
#endif
}

uint8_t        AppRs485_Task(void)
{
    uint8_t i = 0;
    uint8_t Result =  false;
    
    for(i = 0; i < _EVENT_RS485_END; i++)
    {
        if(sEventAppRs485[i].e_status == 1)
        {
            Result = true; 
            
            if((sEventAppRs485[i].e_systick == 0) ||
               ((HAL_GetTick() - sEventAppRs485[i].e_systick) >= sEventAppRs485[i].e_period))
            {
                sEventAppRs485[i].e_status = 0; //Disable event
                sEventAppRs485[i].e_systick= HAL_GetTick();
                sEventAppRs485[i].e_function_handler(i);
            }
        }
    }
    
    return Result;
}



