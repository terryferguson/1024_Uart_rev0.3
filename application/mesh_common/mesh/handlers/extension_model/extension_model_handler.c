/******************************************************************************
 *  Copyright 2015 - 2016 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.1
 *  Application version 2.1.0
 *
 *  FILE
 *      extension_model_handler.c
 *      
 ******************************************************************************/
 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <mem.h>
#include <pio.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "app_mesh_handler.h"
#include "extension_model_handler.h"
#include "extension_server.h"
#include "extension_client.h"
#include "app_mesh_handler.h"
#include "extension_server.h"
#include "extension_client.h"
#include "main_app.h"
#include "nvm_access.h"
#include "app_util.h"
#include "core_mesh_handler.h"
#include "ONESECOND.h"
#include "reset.h"
#include "uart_queue.h"
#include "group_client.h"
#include "group_model.h"
#include "group_server.h"
#include "battery_client.h"
#include "battery_model.h"
#include "battery_server.h"

#ifdef ENABLE_EXTENSION_MODEL
/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Macros for NVM access */
#define NVM_OFFSET_EXTENSION_OPCODE               (0)
#define NVM_OFFSET_EXTENSION_RANGE                (1)
#define TIMER_TIMEOUT_1MS   (1 * MILLISECOND)
/*#define MANUF_HASH_STRING_LENGTH                  (6)*/

/*#define COPY_OPCODE_2BYTE(buf, opcode) buf[0] = (opcode >> 8) & 0xFF; \
                                       buf[1] = (opcode & 0xFF)

#define WRITE_CsrUint16(p, v, offset, len) p[offset++] = v & 0x00FF; \
                                           p[offset++] = (v >> 8) & 0xFF
*/
typedef union
{
    CSRMESH_EXTENSION_CONFLICT_T                   ext_conflict;
} EXTENSION_MODEL_RSP_DATA_T;

typedef struct
{
    CsrUint16 opcode;
    CsrUint16 range;
/*    CsrUint8 manuf_hash_string[MANUF_HASH_STRING_LENGTH];*/
}EXT_MODEL_PROPOSED_OPCODE_T;

/* Application Model Handler Data Structure */
typedef struct
{
    /* extension model opcode data to be stored */
    EXT_MODEL_PROPOSED_OPCODE_T prop_opcode[MAX_EXT_OPCODE_RANGE_SUPPORTED];
}EXTENSION_HANDLER_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Pointer to extension handler data */
static EXTENSION_HANDLER_DATA_T                    ext_hdlr_data;

DIMMER_HANDLER_DATA_T                    dimmer_hdlr_data;                     

CSR_MESH_DEVICE_APPEARANCE_T   appearance;     

//CSRMESH_GROUP_MODEL_GROUPID_T set_group_data;
CSRMESH_GROUP_SET_MODEL_GROUPID_T set_group_data;
CSRMESH_GROUP_GET_MODEL_GROUPID_T get_group_data;
CSRMESH_GROUP_GET_NUMBER_OF_MODEL_GROUPIDS_T getnum_group_data;

CSRMESH_BATTERY_GET_STATE_T battery_get_data;

GROUP_DATA_INFO group_data;
GROUP_NUM_INFO numgroup_data;
BATTERY_DATA_INFO battery_data;

/* Model Response Common Data */
/*static EXTENSION_MODEL_RSP_DATA_T                  ext_model_rsp_data;*/

/*============================================================================*
 *  Public Data
 *============================================================================*/
bool enableFilteredSourceID = FALSE; 
unsigned int filteredSourceID = 0x0000;
bool enableFilteredLocationID = FALSE; 
unsigned int filteredLocationID = 0x0000;

extern uint16 SelfMeshID;
uint16 Get_Group_count;
uint16 previoustid;
unsigned int dest_id;
/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/*============================================================================*
 *  Private Function Definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      extendedMsgHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Extension opcode messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void extendedMsgHandler(CSRMESH_MODEL_EVENT_T event_code,
                               CSRMESH_EVENT_DATA_T* data,
                               CsrUint16 length,
                               void **state_data)
{
    uint8 *dataPtr = data->data;
  
    /* Each time CSR receives mesh opcode, will filter based on opcode and put any necessary data to be passed to ESP32
       CSR will pass the data back to ESP through UART
       ESP will also use this data as sniff data and record to csrMessageWriteListPtr (see ESP gateway code)
      */
    
    /* NOTE:
       Reason this extendedMsgHandler for hub manages to receive all mesh is due at initializeSupportedModelGroups as one
       of the group is set to 0x8000. All dimmer devices broadcast mesh to address 0x8000 

       extendedMsgHandler default can receives mesh data intended to this BLE mesh id. Since for CSR's Unis has been modified
       to receive all dimmer mesh 0x8000 id, app need to cautiously when set Hub location (FF0F) or set Hub wifi (FF32), app needs
       to send these opcodes specifically to this CSR's hub mesh id so only this mesh id will take effect the location and wifi set
       */
    
    if( (enableFilteredLocationID == FALSE) || (filteredLocationID == (((uint16)dataPtr[0] << 8) | (uint16)dataPtr[1])))/* can select either filter location or not */
    {
        if( enableFilteredSourceID == FALSE || filteredSourceID == data->src_id)/* can select either filter device ID or not */
        {
            uint8 byte[16];
            
            byte[0] = 0xFF;  //upper event code  
            byte[1] = (uint8)event_code; //lower event code
            byte[2] = (uint8)(data->src_id >> 8);
            byte[3] = (uint8)(data->src_id & 0x00FF);
            byte[4] = (uint8)dataPtr[0]; //location id high
            byte[5] = (uint8)dataPtr[1]; //location id low
            byte[6] = (uint8)dataPtr[2];
            byte[7] = (uint8)dataPtr[3];
            byte[8] = (uint8)dataPtr[4];
            byte[9] = (uint8)dataPtr[5];
            byte[10] = (uint8)dataPtr[6];
            byte[11] = (uint8)dataPtr[7];
            byte[12] = (uint8)dataPtr[8];
            byte[13] = (uint8)(data->rx_rssi * -1);

            if( (byte[1] & 0x00FF) == 0x10) /* app request CSR device name */
            {
                byte[6] = appearance.shortName[1];
                byte[7] = appearance.shortName[2];
                byte[8] = appearance.shortName[3];
                byte[9] = appearance.shortName[4];
                byte[10] = appearance.shortName[5];
                byte[11] = appearance.shortName[6];
                byte[12] = 0;
                byte[13] = 0;
            }
             
            if( (byte[1] & 0x00FF) == 0x03) /* Use by Nasrul to sniff opcode FF03*/
            {
                byte[9] = (uint8)(data->dst_id >> 8);
                byte[10] = (uint8)(data->dst_id & 0x00FF);
                byte[11] = 0;
                byte[12] = 0;
                byte[13] = 0;
            }
                        
            if((byte[1] & 0x00FF) == 0x04)/* Use by Nasrul to sniff opcode FF04*/
            {
                byte[6] = (uint8)(data->dst_id >> 8);
                byte[7] = (uint8)(data->dst_id & 0x00FF);
                byte[8] = 0;
                byte[9] = 0;
                byte[10] = 0;
                byte[11] = 0;
                byte[12] = 0;
                byte[13] = 0;
            }
            
            if((byte[1] & 0x00FF) == 0x01)/* Use by Nasrul to sniff opcode FF01*/
            {
                byte[6] = (uint8)dataPtr[2];//Day
                byte[7] = (uint8)dataPtr[3];//Hour
                byte[8] = (uint8)dataPtr[4];//Minute
                byte[9] = (uint8)dataPtr[5];//Second
                byte[10] = (uint8)dataPtr[6];//Selection
                byte[11] = (uint8)(data->dst_id >> 8);
                byte[12] = (uint8)(data->dst_id & 0x00FF);
                byte[13] = 0;
            }
                        
            byte[(PACKET_SIZE - 1)] = genUartCheckSum( byte, (PACKET_SIZE - 1)); /* get 14 bytes of chksum  */  
            
            copyToTXPacketBuffer(byte);
         }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      extensionModelEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Extension Model messages
 *
 *  RETURNS
 *      CSRmeshResult : The status of the message handled.
 *
 *---------------------------------------------------------------------------*/
static CSRmeshResult extensionModelEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                                CSRMESH_EVENT_DATA_T* data,
                                                CsrUint16 length,
                                                void **state_data)
{ 
     /* Handle the extended opcode messages here */
    extendedMsgHandler(event_code, data, length,state_data);
    
    return CSR_MESH_RESULT_SUCCESS;
}

/*---------------------------------------------------------------
 *  *  NAME
 *      GroupModelEventHandler
 *
 *  DESCRIPTION
 *      Group model messages 
 *          CSRMESH_GROUP_GET_NUMBER_OF_MODEL_GROUPIDS = 0x0D,
 *          CSRMESH_GROUP_NUMBER_OF_MODEL_GROUPIDS = 0x0E,
 *          CSRMESH_GROUP_SET_MODEL_GROUPID = 0x0F,
 *          CSRMESH_GROUP_GET_MODEL_GROUPID = 0x10,
 *          CSRMESH_GROUP_MODEL_GROUPID = 0x11,
 *
 *  RETURNS
 *      CSRmeshResult : The status of the message handled.
 * 
 *--------------------------------------------------------------*/                                                         
static CSRmeshResult GroupModelEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                            CSRMESH_EVENT_DATA_T* data,
                                            CsrUint16 length,
                                            void **state_data)
{ 
    uint16 *dataPtr1 = data->data;   
    /* Handle the group opcode messages here*/ 
    switch(event_code)
    {
        case CSRMESH_GROUP_NUMBER_OF_MODEL_GROUPIDS:
        {
            numgroup_data.opcode = (uint8)event_code;
            numgroup_data.DeviceID = data->src_id;
            numgroup_data.model = (uint8)dataPtr1[0];
            numgroup_data.num = (uint8)dataPtr1[1]; //num of group id in the device 
            numgroup_data.tid = (uint8)dataPtr1[2];
            
            if (numgroup_data.tid == 0 || numgroup_data.tid != previoustid)
            {
                previoustid = numgroup_data.tid;
                Get_Group_count = 0;
                TimerCreate(TIMER_TIMEOUT_1MS * 50, TRUE, Group_Set_UartTxHandler);
            }
        }
        break;
            
        /* This callback is called when GroupGetModelGroupid() is fired */
        case CSRMESH_GROUP_MODEL_GROUPID:
        {
            group_data.opcode = (uint8)event_code;
            group_data.DeviceID = data->src_id;
            group_data.model = (uint8)dataPtr1[0];
            group_data.groupindex = (uint8)dataPtr1[1];
            group_data.instance = (uint8)dataPtr1[2];
            group_data.groupid = (uint16)dataPtr1[3];
            group_data.tid = (uint8)dataPtr1[4];
            
            if (group_data.tid == 0 || group_data.tid != previoustid)
            {
                previoustid = group_data.tid;
                Get_Group_count = 0;
                TimerCreate(TIMER_TIMEOUT_1MS * 50, TRUE, Group_Set_UartTxHandler);
            }
        }
        break;
        
        default:
        break;
    }
    return CSR_MESH_RESULT_SUCCESS;
}

/*---------------------------------------------------------------
 *  *  NAME
 *      BatteryModelEventHandler
 *
 *  DESCRIPTION
 *    Battery model messages
 *      CSRMESH_BATTERY_GET_STATE = 0x8300,
 *      CSRMESH_BATTERY_STATE = 0x8301,
 *
 *  RETURNS
 *      CSRmeshResult : The status of the message handled.
 * 
 *--------------------------------------------------------------*/
static CSRmeshResult BatteryModelEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                              CSRMESH_EVENT_DATA_T* data,
                                              CsrUint16 length,
                                              void **state_data)
{ 
    uint16 *dataPtr2 = data->data;
    /* Handle the battery opcode messages here*/ 
    switch(event_code)
    {
        case CSRMESH_BATTERY_GET_STATE:
        break;
        
        case CSRMESH_BATTERY_STATE:
        {
            battery_data.opcode = event_code;
            battery_data.DeviceID = data ->src_id;
            battery_data.batterylevel = (uint8)dataPtr2[0];
            battery_data.batterystate = (uint8)dataPtr2[1];
            battery_data.tid = (uint8)dataPtr2[2];
            TimerCreate(TIMER_TIMEOUT_1MS * 50, TRUE, Group_Set_UartTxHandler);
        }
        break;
        
        default:
        break;
        
    }
    return CSR_MESH_RESULT_SUCCESS;
}

/*============================================================================*
 *  Public Function Definitions
 *============================================================================*/


/*----------------------------------------------------------------------------*
 *  NAME
 *      ExtensionModelHandlerInit
 *
 *  DESCRIPTION
 *      The Application function Initilises the extension model handler.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void ExtensionModelHandlerInit(CsrUint8 nw_id,
                                     CsrUint16 model_grps[],
                                     CsrUint16 num_groups)
{
    ext_hdlr_data.prop_opcode[0].opcode = 0xFF00;
    ext_hdlr_data.prop_opcode[0].range = 200;
   
    /* Initialize extension Model */
    ExtensionModelInit(nw_id, 
                       model_grps,
                       num_groups,
                       extensionModelEventHandler);

    /* Initialize the extension opcode range structure with the server */
    ExtensionServerSetupOpcodeList((uint16*) ext_hdlr_data.prop_opcode, 
                                    MAX_EXT_OPCODE_RANGE_SUPPORTED);

    /* Initialize the extension opcode range structure with the client */
    ExtensionClientSetupOpcodeList((uint16*) ext_hdlr_data.prop_opcode, 
                                    MAX_EXT_OPCODE_RANGE_SUPPORTED);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GroupModelHandlerInit
 *
 *  DESCRIPTION
 *      This function initialises the group model handler.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void GroupModelHandlerInit(CsrUint8 nw_id,
                                 CsrUint16 model_grps[],
                                 CsrUint16 num_groups)
{
    GroupModelClientInit(GroupModelEventHandler);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BatteryModelHandlerInit
 *
 *  DESCRIPTION
 *      This function initialises the battery model handler.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void BatteryModelHandlerInit_A(CsrUint8 nw_id,CsrUint16 model_grps[],CsrUint16 num_groups)
{
    BatteryModelClientInit(BatteryModelEventHandler);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ExtensionModelDataInit
 *
 *  DESCRIPTION
 *      This function initialises the Extension Model data.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void ExtensionModelDataInit(void)
{
    MemSet(&ext_hdlr_data, 0, sizeof(ext_hdlr_data));
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      ProcessRxData
 *
 *  DESCRIPTION
 *      To process the UART data received from gateway. If the 1st byte is 0xFF
 *      broadcast the opcode to other CSR. If the 1st byte is 0xFE, then this is
 *      the configuration to CSR to execute
 *---------------------------------------------------------------------------*/
extern uint8 ProcessRxData(CsrUint8 *byteParam)
{
   CsrUint8 chkSum;
   uint8 msg[12];
   uint8 rxData[24];
   uint16 grpIdx;
   uint16 grpVal;
   uint16 dest;
   uint16 indexGroup = 0;
   
   MemCopy(rxData, byteParam, 15);
   
   chkSum = genUartCheckSum(byteParam, 14); /* calculate checksum up until byteLength received from hub */
   if(chkSum == byteParam[14])
   {
       if( (byteParam[0] & 0x00FF) == 0xFE)
       {              
            if( (byteParam[1] & 0x00FF) == 0x00) /* write to specific index of group model and its default value */
            {                
                /* Note:
                   Set this value if to hear mesh from app to dimmer */                
                grpIdx =  (((uint16)byteParam[2])<<8 )|(uint16)byteParam[3];  
                grpVal =  (((uint16)byteParam[4])<<8 )|(uint16)byteParam[5];
                
                extension_model_groups[grpIdx] = grpVal;
            }
            else if( byteParam[1] == 0x01) /* Direct Test Mode */
            {
                return 2; 
            }
            else if( byteParam[1] == 0x02)/* Warm Reset CSR */
            {
                return 3;
            }
            else if( byteParam[1] == 0x03)/* Cold Reset CSR */
            {
                return 4; 
            }  
            
            else if( byteParam[1] == 0x04)
            {
                enableFilteredSourceID = FALSE;
            }   
            else if( byteParam[1] == 0x05) /* enable the filter device ID and set the device ID*/
            {    
                enableFilteredSourceID = TRUE;
                filteredSourceID = ((unsigned int)byteParam[2])<<8 | ((unsigned int)byteParam[3]);
            }
            
            else if( byteParam[1] == 0x06)
            {
                enableFilteredLocationID = FALSE;
            }           
            else if( byteParam[1] == 0x07) /* enable the filter location ID and set the location ID*/
            {    
                enableFilteredLocationID = TRUE;
                filteredLocationID = ((unsigned int)byteParam[2])<<8 | ((unsigned int)byteParam[3]);
            }
            
            else if( byteParam[1] == 0x08) /* reset all group model */
            {    
                for(indexGroup = 0; indexGroup < 301; indexGroup++)
                {
                    extension_model_groups[indexGroup] = 0;
                }
            }
            
            else if( byteParam[1] == 0x09) /* Get Software version, appearance ID and mesh ID */
            {
                #define byteSize    16                
                uint8 byte[byteSize];
                
                MemSet(byte, 0, sizeof(byteSize) - 1);
                
                byte[0] = 0xFF;  /* upper event code */  
                byte[1] = 0xFE; /* lower event code */
                byte[2] = APP_MAJOR_VERSION;
                byte[3] = APP_MINOR_VERSION;
                byte[4] = APP_NEW_VERSION;
                byte[5] = appearance.shortName[1];
                byte[6] = appearance.shortName[2];
                byte[7] = appearance.shortName[3];
                byte[8] = appearance.shortName[4];
                byte[9] = appearance.shortName[5];
                byte[10] = appearance.shortName[6];
                byte[11] = SelfMeshID >> 8;
                byte[12] = SelfMeshID & 0xFF;
                byte[13] = 0;
                byte[14] = genUartCheckSum( byte, (PACKET_SIZE - 1)); /* get 14 bytes of chksum  */
                
                copyToTXPacketBuffer(byte);
            }
       }
       
       else if( byteParam[0] == 0xFF) /* Broadcast normally to CSR mesh */
       {
            msg[0] = byteParam[0];//opCodeHigh                
            msg[1] = byteParam[1];//opCodeLow;
            msg[2] = byteParam[2];//thiLoc_ID_High;
            msg[3] = byteParam[3];//thiLoc_ID_Low;
            dest = ((uint16)byteParam[4]<<8)|(uint16)byteParam[5];            
            msg[4] = byteParam[6];
            msg[5] = byteParam[7];
            msg[6] = byteParam[8];
            msg[7] = byteParam[9];
            msg[8] = byteParam[10];
            msg[9] = byteParam[11];
            msg[10] = byteParam[12];
            
            ExtensionSendMessage(DEFAULT_NW_ID, dest, AppGetCurrentTTL(), msg, byteParam[13]); /* length has to be send exactly the size */
        }

        else if((byteParam[0] & 0x00FF) == 0xFD) /*group model handler*/
        {
            if (byteParam[1] == 0x01) /*set group id*/
            {               
                CsrUint16 destId = ((uint16)byteParam[4]<<8)|(uint16)byteParam[5];
                
                set_group_data.model = CSRMESH_EXTENSION_MODEL;
                set_group_data.groupindex = (CsrUint8)byteParam[6];
                set_group_data.tid = (CsrUint8)byteParam[7];         
                set_group_data.groupid = ((uint16)byteParam[8]<<8)|(uint16)byteParam[9];
                set_group_data.instance = (CsrUint8)byteParam[10];//always zero
                
                GroupSetModelGroupid(DEFAULT_NW_ID, destId, AppGetCurrentTTL(), &set_group_data);                
            }
            else if (byteParam[1] == 0x02) /*Get group id*/
            {
                CsrUint16 destId = ((uint16)byteParam[4]<<8)|(uint16)byteParam[5];
                
                get_group_data.model = CSRMESH_EXTENSION_MODEL;   
                get_group_data.groupindex =(CsrUint8)byteParam[6];
                get_group_data.tid = (CsrUint8)byteParam[7];
                GroupGetModelGroupid(DEFAULT_NW_ID, destId, AppGetCurrentTTL(), &get_group_data);
            }
            else if(byteParam[1] == 0x03) /*Get total group number*/
            {
                CsrUint16 destId = ((uint16)byteParam[4]<<8)|(uint16)byteParam[5];
                
                getnum_group_data.model = CSRMESH_EXTENSION_MODEL;
                getnum_group_data.tid = (CsrUint8)byteParam[6];
                GroupGetNumberOfModelGroupids(DEFAULT_NW_ID, destId, AppGetCurrentTTL(), &getnum_group_data);
            }
            else if(byteParam[1] == 0x04) //Battery Model API
            {
                CsrUint16 destId = ((uint16)byteParam[4]<<8)|(uint16)byteParam[5];
                
                battery_get_data.tid = byteParam[6];
                BatteryGetState(DEFAULT_NW_ID, destId, AppGetCurrentTTL(), &battery_get_data);
            }          
        }
        
        else{}
        
        return 1;
    }
    return 0;
}

#endif /* ENABLE_EXTENSION_MODEL */
