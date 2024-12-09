/******************************************************************************
 *  Copyright 2015 - 2016 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.1
 *  Application version 2.1.0
 *
 *  FILE
 *      main_app.c
 *
 *  DESCRIPTION
 *      This file implements the CSR Mesh application.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#ifdef CSR101x_A05
#include <ls_app_if.h>
#include <config_store.h>
#include <csr_ota.h>
#else
#include <uart.h>           /* Functions to interface with the UART */
#include <uart_sdk.h>       /* Enums to interface with the UART */
#include <configstore_api.h>
#include <configstore_id.h> 
#include <mem.h>
#endif /* !CSR101x_A05 */
#include <timer.h>
#include <uart.h>
#include <uart_sdk.h> 
#include <pio.h>
#include <nvm.h>
#include <debug.h>
#include <bluetooth.h>
#include <ble_direct_test.h>
#include <random.h>
/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "app_debug.h"
#include "battery_hw.h"
#include "app_hw.h"
#include "nvm_access.h"
#include "main_app.h"
#include "gap_service.h"
#include "gatt_service.h"
#include "mesh_control_service.h"
#include "advertisement_handler.h"
#include "connection_handler.h"
#include "mesh_control_service.h"
#include "conn_param_update.h"
#include "uart_queue.h"
#include "extension_model_handler.h"
#include "user_config.h"
#include "app_mesh_handler.h"
#include "reset.h"

#if !defined(CSR101x_A05)
#include "gaia_service.h"
#ifdef GAIA_OTAU_SUPPORT
#include "gaia_otau_api.h"
#include "app_otau_handler.h"
#endif /* GAIA_OTAU_SUPPORT  */
#ifdef GAIA_OTAU_RELAY_SUPPORT
#include "gaia_client_service.h"
#include "app_otau_client_handler.h"
#endif
#else
#include "csr_ota_service.h"
#endif /* (!CSR101x_A05) */
        
#include "ONESECOND.h"
/*============================================================================*
 *  Private Definitions
 *============================================================================*/
#ifndef CSR101x_A05
/* Standard setup of CSR102x boards */
#define UART_PIO_TX                    (8)
#define UART_PIO_RX                    (9)
#define UART_PIO_RTS                   (PIO_NONE)
#define UART_PIO_CTS                   (PIO_NONE)
#define USER_KEY1                      (69)
#define bdaddr                         (1)
#else
/* The User key index where the application config flags are stored */
#define CSKEY_INDEX_USER_FLAGS         (0)
#endif /* !CSR101x_A05 */

/* NVM Store ID */
#define NVM_ID                          (2)

/*#define TIMER_TIMEOUT1 (100 * MILLISECOND)
#define TIMER_TIMEOUT2 (1 * SECOND)
#define TIMER_TIMEOUT3 (500 * MILLISECOND)*/
CsrUint16 last2digit;
CsrUint16 last2digit1;
CsrUint16 last2digit2;

#define TIMER_TIMEOUT1      (100* MILLISECOND)
#define TIMER_TIMEOUT2      (1 * SECOND)
#define TIMER_TIMEOUT3      (500 * MILLISECOND)
#define TIMER_TIMEOUT_1MS   (1 * MILLISECOND)
/*============================================================================*
 *  Private Data
 *===========================================================================*/
/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

static uint16 appUserReadIndCallback(void   *p_rx_buffer, uint16  length);
        
CSR_MESH_DEVICE_APPEARANCE_T appearance;

DIMMER_HANDLER_DATA_T                    dimmer_hdlr_data;

#define MAX_LENGTH_RX_DATA_TO_RECEIVE    15
CsrUint8 DataRX[MAX_LENGTH_RX_DATA_TO_RECEIVE];
CsrUint8 DATA;
uint16 packetRX = 0;
bool firstFFfound = FALSE;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* UART Receive callback */
#if defined(CSR101x_A05) && defined(DEBUG_ENABLE)
static uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
                                   uint16* p_num_additional_words );
#endif /* defined(CSR101x_A05) && defined(DEBUG_ENABLE) */

#ifndef CSR101x_A05

/* Cached Value of UUID. */
uint16 cached_uuid[UUID_LENGTH_WORDS];

/* Cached Value of Authorization Code. */
#ifdef USE_AUTHORISATION_CODE
uint16 cached_auth_code[AUTH_CODE_LENGTH_WORDS];
#endif /* USE_AUTHORISATION_CODE */

#endif /* !CSR101x_A05 */

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
CsrUint8 HextoASCII (CsrUint8 a)
{
    if(a <= 9)
    {
        return (a + 48);
    }
    else
    {
        return (a + 55);
    }
}
/*-----------------------------------------------------------------------------*
 *  NAME
 *      UartDataRxCallback
 *
 *  DESCRIPTION
 *      This callback is issued when data is received over UART. Application
 *      may ignore the data, if not required. For more information refer to
 *      the API documentation for the type "uart_data_out_fn"
 *
 *  RETURNS
 *      The number of words processed, return data_count if all of the received
 *      data had been processed (or if application don't care about the data)
 *
 *----------------------------------------------------------------------------*/
#if defined(CSR101x_A05) && defined(DEBUG_ENABLE)
static uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
        uint16* p_num_additional_words )
{
    /* Application needs 1 additional data to be received */
    *p_num_additional_words = 1;

    return data_count;
}
#endif /* defined(CSR101x_A05) && defined(DEBUG_ENABLE) */

#ifndef CSR101x_A05
/*----------------------------------------------------------------------------*
 *  NAME
 *      configStoreProcessEvent
 *
 *  DESCRIPTION
 *      This function handles the config store messages
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void configStoreProcessEvent(msg_t *msg)
{
    configstore_msg_t *cs_msg = (configstore_msg_t*)msg;
    switch(msg->header.id)
    {
        case CONFIG_STORE_READ_KEY_CFM:
        {
            configstore_read_key_cfm_t *read_key_cfm =
                    (configstore_read_key_cfm_t*)&cs_msg->body;
            
            if(STATUS_SUCCESS == read_key_cfm -> status)
            {
                if((bdaddr == read_key_cfm ->id) && (0 != read_key_cfm->value_len))
                {
                   last2digit = read_key_cfm->value[0];
                   last2digit1 = read_key_cfm->value[1];
                   last2digit2 = read_key_cfm->value[2];
                   appearance.shortName[3] = HextoASCII((last2digit>>12));
                   appearance.shortName[4] = HextoASCII((last2digit>>8) - ((last2digit>>12) <<4));
                   appearance.shortName[5] = HextoASCII((last2digit>>4) - ((last2digit>>8) <<4));
                   appearance.shortName[6] = HextoASCII(last2digit - ((last2digit>>4)<<4));
                }
            }
            if (STATUS_SUCCESS == read_key_cfm->status)
            {
                if ((USER_KEY1 == read_key_cfm->id) && (0 != read_key_cfm->value_len))
                {
                    /* Copy flags. */
                    g_cskey_flags = read_key_cfm->value[0];

                    /* Copy UUID if Random UUID is not used. */
                    if (0 == (g_cskey_flags & CSKEY_RANDOM_UUID_ENABLE_BIT))
                    {
                        if (read_key_cfm->value_len >= (UUID_LENGTH_WORDS + 1))
                        {
                            MemCopy(cached_uuid, &read_key_cfm->value[1],
                                    UUID_LENGTH_WORDS);
                        }

                        /* Copy Authorization Code if enabled. */
#ifdef USE_AUTHORISATION_CODE
                        if (read_key_cfm->value_len >= (UUID_LENGTH_WORDS + AUTH_CODE_LENGTH_WORDS + 1))
                        {
                            MemCopy(cached_auth_code, &read_key_cfm->value[1 + UUID_LENGTH_WORDS],
                                    AUTH_CODE_LENGTH_WORDS);
                        }
#endif /* USE_AUTHORISATION_CODE */
                    }
                    /* Initialize the nvm offset to max words taken by  
                     * The CM is stores its data after this.
                     */        
                    g_app_nvm_offset = NVM_OFFSET_SANITY_WORD;

                   /* Initialise the NVM. AppNvmReady is called when the NVM 
                    * initialisation complete.
                    */
                    Nvm_Init(NVM_ID,NVM_SANITY_MAGIC, &g_app_nvm_offset);
                }
                else if(CS_ID_APP_STORE_ID == read_key_cfm->id)
                {
#ifdef GAIA_OTAU_SUPPORT
                    GaiaOtauConfigStoreMsg(msg);
#endif /* GAIA_OTAU_SUPPORT */
#ifdef GAIA_OTAU_RELAY_SUPPORT
                    GaiaOtauClientConfigStoreMsg(msg);
#endif /* GAIA_OTAU_RELAY_SUPPORT */
                }
            }
        }
        break;
        default:
        break;
    }
}
#endif

/*----------------------------------------------------------------------------*
 *  NAME
 *      readUserKeys
 *
 *  DESCRIPTION
 *      This function reads the user keys from config file
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
static void readUserKeys(void)
{
#ifndef CSR101x_A05
    /* Read payload filler from User key index1 */
    ConfigStoreReadKey(USER_KEY1 ,STORE_ID_UNUSED);
    /* Read payload filler from User bdaddr*/
    ConfigStoreReadKey(bdaddr,STORE_ID_UNUSED);
    
#else
    g_cskey_flags = CSReadUserKey(CSKEY_INDEX_USER_FLAGS);

    /* Initialize the nvm offset to max words taken by application. The CM is
     * stores its data after this.
     */
    g_app_nvm_offset = NVM_OFFSET_SANITY_WORD;

    /* Initialise the NVM. AppNvmReady is called when the NVM initialisation
     * complete.
     */
    Nvm_Init(NVM_SANITY_MAGIC, &g_app_nvm_offset);
#endif
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitialiseAppSuppServices
 *
 *  DESCRIPTION
 *      This function initialises the application supported services.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void InitAppSupportedServices(void)
{
#ifndef CSR101x_A05
    g_app_nvm_offset = NVM_OFFSET_GAIA_OTA_SERVICE;
    /* Initialize the Gaia OTA service. */
#ifdef RESET_NVM
    GAIAInitServerService(g_gaia_nvm_fresh, &g_app_nvm_offset);
#else
    GAIAInitServerService(g_app_nvm_fresh, &g_app_nvm_offset);
#endif /* RESET_NVM */
#ifdef GAIA_OTAU_SUPPORT
    AppGaiaOtauInit();
#endif /* GAIA_OTAU_SUPPORT */
#ifdef GAIA_OTAU_RELAY_SUPPORT 
    GaiaInitClientService(g_app_nvm_fresh);
    GaiaAppClientInit(g_app_nvm_fresh,&g_app_nvm_offset);
#endif  
    g_app_nvm_offset = NVM_OFFSET_MESH_APP_SERVICES;
#else
#ifdef OTAU_BOOTLOADER
    /* Initialize the OTA service. */
    OtaInitServerService(g_app_nvm_fresh, &g_app_nvm_offset);
#endif
#endif   
    
    /* Initialize the GATT service. */
    GattExtInitServerService(g_app_nvm_fresh, &g_app_nvm_offset, TRUE);

    /* Initialize the GAP service. */
    GapInitServerServiceNoBond();

    /* Initialize the Mesh control service */
    MeshControlInitServerService(g_app_nvm_fresh, &g_app_nvm_offset);
    
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise CSRmesh application
 *      data structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppDataInit(void)
{
    /* Stop the connection parameter update if its in progress */
    StopConnParamsUpdate();

    /* Initialises the GATT Data */
    InitialiseGattData();

    /* Initialize the Mesh Control Service Data Structure */
    MeshControlServiceDataInit();
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This user application function is called after a power-on reset
 *      (including after a firmware panic), after a wakeup from Hibernate or
 *      Dormant sleep states, or after an HCI Reset has been requested.
 *
 *      The last sleep state is provided to the application in the parameter.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after app_power_on_reset().
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppInit(sleep_state last_sleep_state)
{
    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);

    LightHardwareInit();
    
    /* Read the user keys */
    readUserKeys();
    
    UARTMCUinit(appUserReadIndCallback, APP_UART_RATE);
    
    TimerCreate(TIMER_TIMEOUT_1MS * 1000, TRUE, mainUartTxHandler);  
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      appUserReadIndCallback
 *
 *  DESCRIPTION
 *      This function is registered as a callback from the app_uart module
 *      In this app this message is received on receipt of every byte over
 *      the UART, see appUartStartReadEvents().
 *      Incoming data is queued for transmission back to the UART
 *      We process the incoming data to cater for special case characters.
 *
 * PARAMETERS
 *      void *p_rx_buffer  Pointer to the receive buffer (uint8 if 'unpacked'
 *                         or uint16 if 'packed' depending on the chosen UART
 *                         data mode - this application uses 'unpacked')
 *
 *      uint16 length      Number of bytes ('unpacked') or words ('packed')
 *                         received
 *
 * RETURNS
 *      The number of bytes ('unpacked') or words ('packed') that have been
 *      processed.
 *----------------------------------------------------------------------------*/
static uint16 appUserReadIndCallback(void   *p_rx_buffer,
                                     uint16  length)
{    
    /* Inform the UART driver that we'd like to receive another byte when it
     * becomes available
     */
    uint8 returnRequest = 1; 

    DATA = *(CsrUint8 *)p_rx_buffer;
       
    if((DATA == 0xFF || DATA == 0xFE || DATA == 0xFD) && !firstFFfound)
    {  
        packetRX = 0;        
        DataRX[packetRX++] = DATA;
        firstFFfound = TRUE;
    }   
    else if((DataRX[0] == 0xFF || DataRX[0] == 0xFE || DataRX[0] == 0xFD) && (packetRX < MAX_LENGTH_RX_DATA_TO_RECEIVE))
    {
        DataRX[packetRX++] = DATA;
    }
  
    if(packetRX == (MAX_LENGTH_RX_DATA_TO_RECEIVE - 1)) /* Since packetRX starts from 0 */    
    {                  
        returnRequest = ProcessRxData(DataRX);
        
        if(returnRequest)/* chksum is good*/
        {           
            PioSet(PIN_CSR_ACK, 1); /* inform sender that sync 14 is good - output 12 */           
            TimerCreate(TIMER_TIMEOUT_1MS * 75, TRUE, timerCallbackUartRxedPinToLow); /* need to re-LOW pin after HIGH */            
            
            if(returnRequest == 2)
            {
                DirectTestExtended(1);    
                TestEnableCarrierWave(20);
            }
            else if(returnRequest == 3)
            {
                WarmReset();
            }
            else if(returnRequest == 4)
            {
                ColdReset();
            }
        }
        else
        { /* chksum is not good, do nothing, will timeout at ESP */
   
            /* PioSet(PIN_CSR_LED, 0); */        
        }      
    }
    else
    {
        /*p_additional_req_data_length = (uint16)0;*/ 
        /* do not worry 
        request to read 14 always happen inside timer and timer is initiated by
        the frame rx signal from the transmitter (host)*/     
       
        length = 0;
    }
      
    //CSR1024 will have problem if the byte receive also 0xFF same as header 0xFF. This should fix the issue
    if( packetRX >= MAX_LENGTH_RX_DATA_TO_RECEIVE )
    {
        firstFFfound = FALSE;
        packetRX = 0;
    }
        
    /* Return the number of bytes that have been processed */
    return length;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppNvmReady
 *
 *  DESCRIPTION
 *      This function is called when the NVM is initialised
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppNvmReady(bool nvm_fresh, uint16 nvm_offset)
{
    /* Save the NVM data */
    g_app_nvm_fresh = nvm_fresh;
#ifdef CSR101x_A05
    g_app_nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
#else
    g_app_nvm_offset = NVM_OFFSET_CM_INITIALISATION;
    
/* If RESET_NVM flag is set, this means APP/Stack NVM offsets have changed 
 * between the  releases.In order for GAIA OTAu to complete successfully if it
 * is in post reboot phase of OTA, force the GAIA service to read from the old 
 * offset.All NVM data will be lost after this update.
 * Note: GAIA service offsets should be still same between the releases.
 */
#ifdef RESET_NVM
    if(GAIAInPostRebootPhase())
    {
        g_app_nvm_fresh = TRUE;
        g_gaia_nvm_fresh = FALSE;
    }
    else
    {
        g_gaia_nvm_fresh = g_app_nvm_fresh;
    }
#endif /* RESET_NVM */
#endif

    /* Initilize CM */
    AppCMInit(g_app_nvm_fresh, &g_app_nvm_offset);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppProcesSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch (id)
    {
        case sys_event_pio_changed:
        {
             HandlePIOChangedEvent((pio_changed_data*)data);
        }
        break;

        case sys_event_battery_low:
        {
            /* Battery low event received - Broadcast the battery state to the
             * network.
             */
            SendLowBatteryIndication();
        }
        break;

        default:
        break;
    }
}

#ifndef CSR101x_A05
/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessEvent
 *
 *  DESCRIPTION
 *      Handles the system events
 *
 *  RETURNS
 *     status_t: STATUS_SUCCESS if successful
 *
 *----------------------------------------------------------------------------*/

status_t AppProcessEvent(msg_t *msg)
{
    switch(GET_GROUP_ID(msg->header.id))
    {
        case SM_GROUP_ID:
        case LS_GROUP_ID:
        case GATT_GROUP_ID:
        {
            /* CM Handles the SM, LS and GATT Messages */
            CMProcessMsg(msg);
        }
        break;

        case USER_STORE_GROUP_ID:
        {
            NvmProcessEvent(msg);
        }
        break;

        case CONFIG_STORE_GROUP_ID:
        {
            configStoreProcessEvent(msg);
        }
        break;
        
#ifdef GAIA_OTAU_SUPPORT
        case STORE_UPDATE_GROUP_ID:
        GaiaOtauHandleStoreUpdateMsg(GetConnectedDeviceId(), (store_update_msg_t*)msg );
#ifdef GAIA_OTAU_RELAY_SUPPORT
        GaiaOtauClientHandleStoreUpdateMsg(GetConnectedDeviceId(), (store_update_msg_t*)msg );
#endif
        break;
#endif

        default:
        break;
    }    
    
    if ( GET_GROUP_ID(msg->header.id) == UART_GROUP_ID )
    {
        /* Pass UART events to app_uart */
        AppUartProcessEvent(msg);
    }
    

    return STATUS_SUCCESS;
}

#else
/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event is
 *      received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern bool AppProcessLmEvent(lm_event_code event_code,
                              LM_EVENT_T *p_event_data)
{
    /* CM Handles the SM, LS and GATT Messages */
    CMProcessMsg(event_code, p_event_data);
    return TRUE;
}

/*----------------------------------------------------------------------------*
 * NAME
 *   AppPowerOnReset
 *
 * DESCRIPTION
 *   This user application function is called just after a power-on reset
 *   (including after a firmware panic), or after a wakeup from Hibernate or
 *   Dormant sleep states.
 *
 *   At the time this function is called, the last sleep state is not yet
 *   known.
 *
 *   NOTE: this function should only contain code to be executed after a
 *   power-on reset or panic. Code that should also be executed after an
 *   HCI_RESET should instead be placed in the AppInit() function.
 *
 * RETURNS
 *   Nothing
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{
     /* Code that is only executed after a power-on reset or firmware panic 
      * should be implemented here.
      */
}
#endif