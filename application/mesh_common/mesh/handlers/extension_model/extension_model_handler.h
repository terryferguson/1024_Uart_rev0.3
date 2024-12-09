/******************************************************************************
 *  Copyright 2016 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.1
 *  Application version 2.1.0
 *
 *  FILE
 *      extension_model_handler.h
 *
 ******************************************************************************/
#ifndef __EXTENSION_MODEL_HANDLER_H__
#define __EXTENSION_MODEL_HANDLER_H__

#include <csr_types.h>
#include "user_config.h"
#include "group_client.h"
#include "group_model.h"
/*============================================================================*
 *  Public Data
 *============================================================================*/
/*Manual edit for scheduler*/
/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 ID;  
    CsrUint8 StartDay ; 
    CsrUint8 StartHour ;
    CsrUint8 StartMin ;
    CsrUint8 EndHour;
    CsrUint8 EndMin;
    CsrUint8 DimmingLevel;
        
} CSRMESH_SCHEDULER_T;*/

/*Manual edit for clock*/
/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 Day;
    CsrUint8 Hour;
    CsrUint8 Min;
    CsrUint8 Sec;
    CsrUint8 Selection;
   
} CSRMESH_CLOCK_T;*/

/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 AwayMin;
    CsrUint8 AwaySec;
    CsrUint8 AwayEnable;
          
} CSRMESH_AWAY_T;*/

/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 power; 
    CsrUint8 level; 
    CsrUint8 tid;
          
} CSRMESH_DIMMER_STATE_T;*/

/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 dimmingcondition;
    
}CSRMESH_DIMMING_STATE_T;*/

typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    
}CSRMESH_REQUEST_T;

/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 brightness_id;
    CsrUint8 tid;
    
}CSRMESH_SWITCH_REQUEST_T;*/
        
/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 ID;
    CsrUint8 level;
    
} CSRMESH_SCENE_STATE_T;*/

typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 newlocation_idHigh;
    CsrUint8 newlocation_idLow;
    
}CSRMESH_LOCATION_DATA_T;

/*typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    CsrUint8 ID;
    CsrUint8 group_numHigh;
    CsrUint8 group_numLow;         
}CSRMESH_SET_GROUP_T;*/

typedef struct
{
    CsrUint8 location_idHigh;
    CsrUint8 location_idLow;
    
}CSRMESH_DELETE_T;

typedef struct
{
    CsrUint8 productID1_High;
    CsrUint8 productID1_Low;
    CsrUint8 productID2_High;
    CsrUint8 productID2_Low;
    CsrUint8 productID3_High;
    CsrUint8 productID3_Low;
    CsrUint8 productID4_High;
    CsrUint8 productID4_Low;
    CsrUint8 idx_grp;
}CSRMESH_SETGROUP_T;


typedef struct
{
    uint8 message[13];
    
}CSRMESH_RECEIVE_DATA_T;

/* Application Model Handler Data Structure SCHEDULAR_HANDLER_DATA_T */
typedef struct
{
    /*CSRMESH_SCHEDULER_T            scheduler;  
    CSRMESH_CLOCK_T                clock; 
    CSRMESH_AWAY_T                 away; */
    /*CSRMESH_DIMMER_STATE_T         dimmer_model;*/ 
    /*CSRMESH_SCENE_STATE_T         scene;*/ 
    CSRMESH_REQUEST_T              request;
    CSRMESH_LOCATION_DATA_T        location;
    CSRMESH_RECEIVE_DATA_T         receive;
    /*CSRMESH_DIMMING_STATE_T        dimming_state;
    CSRMESH_SWITCH_REQUEST_T       switch_request;*/ 
    /*CSRMESH_SET_GROUP_T                group;*/
    CSRMESH_DELETE_T               del_group;
    CSRMESH_SETGROUP_T             set_group;
    
}DIMMER_HANDLER_DATA_T;

typedef struct
{ 
    CsrUint8 opcode;
    CsrUint16 DeviceID;
    CsrUint8 model; 
    CsrUint8 groupindex; 
    CsrUint8 instance; 
    CsrUint16 groupid; 
    CsrUint8 tid;  
    
}GROUP_DATA_INFO;

typedef struct
{
    CsrUint8 opcode;
    CsrUint16 DeviceID;
    CsrUint8 model;
    CsrUint8 num;
    CsrUint8 tid;
}GROUP_NUM_INFO;

typedef struct
{
    CsrUint16 opcode;
    CsrUint16 DeviceID;
    CsrUint8 batterylevel;
    CsrUint8 batterystate;
    CsrUint8 tid;
}BATTERY_DATA_INFO;
/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* The function initialises the extension model handler */
extern void ExtensionModelHandlerInit(CsrUint8 nw_id,
                                      CsrUint16 model_groups[],
                                      CsrUint16 num_groups);

/* The function initialises the group model handler */
extern void GroupModelHandlerInit(CsrUint8 nw_id,CsrUint16 model_grps[],CsrUint16 num_groups);

/* The function initialises the battery model handler */
extern void BatteryModelHandlerInit_A(CsrUint8 nw_id,CsrUint16 model_grps[],CsrUint16 num_groups);

/* The function initialises the extension model data in the handler */
extern void ExtensionModelDataInit(void);

/* The function reads the extension model data from NVM */
extern void ReadExtensionModelDataFromNVM(CsrUint16 offset);

/* The function writes the extension model data onto NVM */
extern void WriteExtensionModelDataOntoNVM(CsrUint16 offset);

extern uint8 ProcessRxData(CsrUint8 *byteParam);

#endif /* __EXTENSION_MODEL_HANDLER_H__ */
