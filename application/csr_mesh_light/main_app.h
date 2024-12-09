/******************************************************************************
 *  Copyright 2015 - 2016 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.1
 *  Application version 2.1.0
 *
 *  FILE
 *      main_app.h
 *
 *  DESCRIPTION
 *      Header definitions for CSR Mesh application file
 *
 ******************************************************************************/

#ifndef __MAIN_APP_H__
#define __MAIN_APP_H__

 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"

/*============================================================================*
 *  CSR Mesh Header Files
 *============================================================================*/
#include "csr_mesh.h"
#include "csr_sched.h"
#include "cm_types.h"
#include "csr_mesh_nvm.h"
/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Configuration bits on the User Key */
#define CSKEY_RELAY_ENABLE_BIT         (1)
#define CSKEY_BRIDGE_ENABLE_BIT        (2)
#define CSKEY_RANDOM_UUID_ENABLE_BIT   (4)
#define UUID_LENGTH_WORDS              (8)
#define AUTH_CODE_LENGTH_WORDS         (4)

/* Maximum number of timers */
#define MAX_APP_TIMERS                 (20)

/* Magic value to check the sanity of NVM region used by the application */
#define NVM_SANITY_MAGIC               (0xAB90)

/* NVM Offset for RGB data */
#define NVM_RGB_DATA_OFFSET            (CSR_MESH_NVM_SIZE)

/* Size of RGB Data in Words */
#define NVM_RGB_DATA_SIZE              (3)

#define NVM_OFFSET_BUTTON_STATE        (NVM_RGB_DATA_OFFSET + \
                                        sizeof(NVM_RGB_DATA_SIZE))
/*Size of the Button State in Words*/
#define NVM_BUTTON_STATE_SIZE          (20)

#define NVM_OFFSET_TIME_INTERVAL       (NVM_OFFSET_BUTTON_STATE + NVM_BUTTON_STATE_SIZE)

#ifdef ENABLE_TIME_MODEL
#define NVM_TIME_INTERVAL_SIZE         (1)
#else
#define NVM_TIME_INTERVAL_SIZE         (0)
#endif

#define NVM_OFFSET_ACTION_MODEL_DATA   ((NVM_OFFSET_TIME_INTERVAL + \
                                         NVM_TIME_INTERVAL_SIZE))

#ifdef ENABLE_ACTION_MODEL
#define ACTION_SIZE                    (16)
#define NVM_ACTIONS_SIZE               (MAX_ACTIONS_SUPPORTED * ACTION_SIZE)

/* Get NVM Offset of a specific action from its index. */
#define GET_ACTION_NVM_OFFSET(idx)     (NVM_OFFSET_ACTION_MODEL_DATA + \
                                       ((idx) * (ACTION_SIZE)))
#else
#define NVM_ACTIONS_SIZE               (0)
#endif

#define NVM_OFFSET_EXTENSION_MODEL_DATA     (NVM_OFFSET_ACTION_MODEL_DATA + \
                                         NVM_ACTIONS_SIZE)

#ifdef ENABLE_EXTENSION_MODEL
#define EXTENSION_SIZE                  (2)
#define NVM_EXTENSION_SIZE               (MAX_EXT_OPCODE_RANGE_SUPPORTED * EXTENSION_SIZE)

/*Get NVM Offset of a specific action from its index */ 
#define GET_EXTENSION_NVM_OFFSET(idx)  (NVM_OFFSET_EXTENSION_MODEL_DATA + \
                                        ((idx) * (EXTENSION_SIZE)))

#else
#define NVM_EXTENSION_SIZE              (0)  
#endif

#define NVM_OFFSET_ASSET_MODEL_DATA     (NVM_OFFSET_EXTENSION_MODEL_DATA + \
                                         NVM_EXTENSION_SIZE)

#ifdef ENABLE_ASSET_MODEL
#define NVM_ASSET_SIZE                  (5)
#else
#define NVM_ASSET_SIZE                  (0)
#endif

#define NVM_OFFSET_TRACKER_MODEL_DATA   (NVM_OFFSET_ASSET_MODEL_DATA + \
                                         NVM_ASSET_SIZE)

#ifdef ENABLE_TRACKER_MODEL
#define NVM_TRACKER_SIZE                (8)
#else
#define NVM_TRACKER_SIZE                (0)
#endif

#define NVM_OFFSET_LIGHT_MODEL_GROUPS   (NVM_OFFSET_TRACKER_MODEL_DATA + \
                                         NVM_TRACKER_SIZE)

#ifdef ENABLE_LIGHT_MODEL
#define SIZEOF_LIGHT_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_LIGHT_MODEL_GROUPS       (0)
#endif /* ENABLE_LIGHT_MODEL */

#define NVM_OFFSET_EXTENSION_MODEL_GROUPS  (NVM_OFFSET_LIGHT_MODEL_GROUPS + \
                                        SIZEOF_LIGHT_MODEL_GROUPS)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_EXTENSION_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_EXTENSION_MODEL_GROUPS       (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_appearancevalue  (NVM_OFFSET_EXTENSION_MODEL_GROUPS + \
                                        SIZEOF_EXTENSION_MODEL_GROUPS)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_appearancevalue       (1)
#else
#define SIZEOF_appearancevalue       (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_Location  (NVM_OFFSET_appearancevalue + \
                                        SIZEOF_appearancevalue)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_Location       (1)
#else
#define SIZEOF_Location       (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_Starttimer  (NVM_OFFSET_Location + \
                                        SIZEOF_Location)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_Starttimer     (1)
#else
#define SIZEOF_Starttimer     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_power       (NVM_OFFSET_Starttimer +\
                                        SIZEOF_Starttimer)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_power          (1)
#else
#define SIZEOF_power          (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_ID_Starttimer  (NVM_OFFSET_power + \
                                        SIZEOF_power)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_ID_Starttimer   (1)
#else
#define SIZEOF_ID_Starttimer   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_DayStarttimer  (NVM_OFFSET_ID_Starttimer + \
                                        SIZEOF_ID_Starttimer)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_DayStarttimer   (1)
#else
#define SIZEOF_DayStarttimer   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_masterfunction  (NVM_OFFSET_DayStarttimer + \
                                        SIZEOF_DayStarttimer)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_masterfunction   (1)
#else
#define SIZEOF_masterfunction   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_groupmaster  (NVM_OFFSET_masterfunction + \
                                        SIZEOF_masterfunction)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_groupmaster   (1)
#else
#define SIZEOF_groupmaster   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_productID    (NVM_OFFSET_groupmaster +\
                                        SIZEOF_groupmaster)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_productID     (1)
#else
#define SIZEOF_productID     (0)
#endif

#define NVM_OFFSET_SchDaystart  (NVM_OFFSET_productID + \
                                        SIZEOF_productID)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchDaystart   (5)
#else
#define SIZEOF_SchDaystart   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchHourstart  (NVM_OFFSET_SchDaystart + \
                                        SIZEOF_SchDaystart)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchHourstart   (5)
#else
#define SIZEOF_SchHourstart   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchMinstart  (NVM_OFFSET_SchHourstart + \
                                        SIZEOF_SchHourstart)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchMinstart   (5)
#else
#define SIZEOF_SchMinstart   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchHourstop  (NVM_OFFSET_SchMinstart + \
                                        SIZEOF_SchMinstart)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchHourstop   (5)
#else
#define SIZEOF_SchHourstop   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchMinstop  (NVM_OFFSET_SchHourstop + \
                                        SIZEOF_SchHourstop)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchMinstop   (5)
#else
#define SIZEOF_SchMinstop   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchDimlevel  (NVM_OFFSET_SchMinstop + \
                                        SIZEOF_SchMinstop)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchDimlevel   (5)
#else
#define SIZEOF_SchDimlevel   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SchCCTlevel    (NVM_OFFSET_SchDimlevel +\
                                         SIZEOF_SchDimlevel)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SchCCTlevel     (5)
#else
#define SIZEOF_SchCCTlevel     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_previouslocation    (NVM_OFFSET_SchCCTlevel +\
                                         SIZEOF_SchCCTlevel)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_previouslocation     (1)
#else
#define SIZEOF_previouslocation     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_warmresetflag    (NVM_OFFSET_previouslocation +\
                                         SIZEOF_previouslocation)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_warmresetflag     (1)
#else
#define SIZEOF_warmresetflag     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_CircadianMode    (NVM_OFFSET_warmresetflag +\
                                         SIZEOF_warmresetflag)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_CircadianMode     (1)
#else
#define SIZEOF_CircadianMode     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_OFFSETclock   (NVM_OFFSET_CircadianMode +\
                                         SIZEOF_CircadianMode)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_OFFSETclock     (1)
#else
#define SIZEOF_OFFSETclock     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_offsetcount  (NVM_OFFSET_OFFSETclock +\
                                         SIZEOF_OFFSETclock)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_offsetcount     (1)
#else
#define SIZEOF_offsetcount     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_Circadian_ID    (NVM_OFFSET_offsetcount +\
                                         SIZEOF_offsetcount)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_Circadian_ID     (1)
#else
#define SIZEOF_Circadian_ID     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_Circadianlevel    (NVM_OFFSET_Circadian_ID +\
                                         SIZEOF_Circadian_ID)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_Circadianlevel     (1)
#else
#define SIZEOF_Circadianlevel     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_CircadianCCTlevel    (NVM_OFFSET_Circadianlevel +\
                                         SIZEOF_Circadianlevel)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_CircadianCCTlevel     (1)
#else
#define SIZEOF_CircadianCCTlevel     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_dimmingmin    (NVM_OFFSET_CircadianCCTlevel +\
                                         SIZEOF_CircadianCCTlevel)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_dimmingmin            (1)
#else
#define SIZEOF_dimmingmin            (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_dimmingmax   (NVM_OFFSET_dimmingmin +\
                                         SIZEOF_dimmingmin)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_dimmingmax     (1)
#else
#define SIZEOF_dimmingmax     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_countflag (NVM_OFFSET_dimmingmax + \
                                        SIZEOF_dimmingmax)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_countflag     (1)
#else
#define SIZEOF_countflag     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_SCENE_MODEL_GROUPS  (NVM_OFFSET_countflag + \
                                        SIZEOF_countflag)

#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_SCENE_MODEL_GROUPS   ((sizeof(uint16)*MAX_MODEL_SCENES)*2)
#else
#define SIZEOF_SCENE_MODEL_GROUPS   (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_producttype  (NVM_OFFSET_SCENE_MODEL_GROUPS + \
                                        SIZEOF_SCENE_MODEL_GROUPS)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_producttype    (1)
#else
#define SIZEOF_producttype     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_revision (NVM_OFFSET_producttype + \
                                        SIZEOF_producttype)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_revision    (1)
#else
#define SIZEOF_revision     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_DimmingState  (NVM_OFFSET_revision + \
                                        SIZEOF_revision)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_DimmingState   (2)
#else
#define SIZEOF_DimmingState1     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_CCTlevel  (NVM_OFFSET_DimmingState + \
                                        SIZEOF_DimmingState)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_CCTlevel   (2)
#else
#define SIZEOF_CCTlevel     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_CCTlevelState  (NVM_OFFSET_CCTlevel + \
                                        SIZEOF_CCTlevel)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_CCTlevelState    (1)
#else
#define SIZEOF_CCTlevelState      (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_externalpushmode  (NVM_OFFSET_CCTlevelState  + \
                                        SIZEOF_CCTlevelState )
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_externalpushmode    (1)
#else
#define SIZEOF_externalpushmode      (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_FadeMode  (NVM_OFFSET_externalpushmode  + \
                                        SIZEOF_externalpushmode )
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_FadeMode    (1)
#else
#define SIZEOF_FadeMode      (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_Bombvalue  (NVM_OFFSET_FadeMode  + \
                                        SIZEOF_FadeMode )
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_Bombvalue    (1)
#else
#define SIZEOF_Bombvalue     (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_dimmermode  (NVM_OFFSET_Bombvalue  + \
                                SIZEOF_Bombvalue)
#ifdef ENABLE_EXTENSION_MODEL
#define SIZEOF_dimmermode    (1)
#else
#define SIZEOF_dimmermode      (0)
#endif /*ENABLE_EXTENSION_MODEL*/

#define NVM_OFFSET_POWER_MODEL_GROUPS  (NVM_OFFSET_dimmermode   + \
                                        SIZEOF_dimmermode)

#ifdef ENABLE_POWER_MODEL
#define SIZEOF_POWER_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_POWER_MODEL_GROUPS       (0)
#endif /* ENABLE_POWER_MODEL */

#define NVM_OFFSET_ATT_MODEL_GROUPS    (NVM_OFFSET_POWER_MODEL_GROUPS + \
                                        SIZEOF_POWER_MODEL_GROUPS)

#ifdef ENABLE_ATTENTION_MODEL
#define SIZEOF_ATT_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_ATT_MODEL_GROUPS         (0)
#endif /* ENABLE_ATTENTION_MODEL */

#define NVM_OFFSET_DATA_MODEL_GROUPS   (NVM_OFFSET_ATT_MODEL_GROUPS + \
                                        SIZEOF_ATT_MODEL_GROUPS)

#ifdef ENABLE_DATA_MODEL
#define SIZEOF_DATA_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_DATA_MODEL_GROUPS       (0)
#endif /* ENABLE_DATA_MODEL */

#define NVM_OFFSET_LOT_MODEL_GROUPS   (NVM_OFFSET_DATA_MODEL_GROUPS + \
                                        SIZEOF_DATA_MODEL_GROUPS)

#ifdef ENABLE_LOT_MODEL
#define SIZEOF_LOT_MODEL_GROUPS        (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_LOT_MODEL_GROUPS        (0)
#endif /* ENABLE_LOT_MODEL */

/* NVM Offset for Application data */
#define NVM_MAX_APP_MEMORY_WORDS       (NVM_OFFSET_LOT_MODEL_GROUPS + \
                                        SIZEOF_LOT_MODEL_GROUPS)

#define LEVEL_DATA_OFFSET              (2)
uint16                                  g_app_nvm_offset;
bool                                    g_app_nvm_fresh;
uint16                                  g_cskey_flags;

#ifndef CSR101x
/* Cached Value of UUID. */
extern uint16 cached_uuid[UUID_LENGTH_WORDS];

/* Cached Value of Authorization Code. */
#ifdef USE_AUTHORISATION_CODE
extern uint16 cached_auth_code[AUTH_CODE_LENGTH_WORDS];
#endif /* USE_AUTHORISATION_CODE */

#ifdef RESET_NVM
bool                                    g_gaia_nvm_fresh;
#endif /* RESET_NVM */

#endif /* !CSR101x */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise application data */
extern void AppDataInit(void);

/* Initialise the Application supported services */
extern void InitAppSupportedServices(void);
#endif /* __MAIN_APP_H__ */

CsrUint8 HextoASCII(CsrUint8 a);