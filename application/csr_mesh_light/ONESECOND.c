/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
 
#include <main.h>           /* Functions relating to powering up the device */
/*#include <ls_app_if.h>*/      /* Link Supervisor application interface */
#include <timer.h>          /* Chip timer functions */
#include <panic.h>          /* Support for applications to panic */
#include <pio.h>
#include <uart.h>
#include <ble_direct_test.h>
#include <random.h>
#include <mem.h>
/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "ONESECOND.h"
#include "extension_model_handler.h"
#include "app_mesh_handler.h"
#include "nvm_access.h"
#include "main_app.h"
#include "iot_hw.h"
#include "uart_queue.h"
#include "reset.h"
/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* First timeout at which the timer has to fire a callback */
#define TIMER_TIMEOUT1      (100 * MILLISECOND)
#define TIMER_TIMEOUT2      (1 * SECOND)
#define TIMER_TIMEOUT3      (260 * MILLISECOND)
#define TIMER_TIMEOUT_1MS   (1 * MILLISECOND)
#define ON  (1)
#define OFF (0)

/*============================================================================*
 *  Private Data
 *============================================================================*/
CSR_MESH_DEVICE_APPEARANCE_T  appearance;

/* Pointer to dimmer handler data */
 DIMMER_HANDLER_DATA_T                      dimmer_hdlr_data;
  
 GROUP_DATA_INFO group_data;
 GROUP_NUM_INFO numgroup_data;
 BATTERY_DATA_INFO battery_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
timer_id tId;
timer_id tId_UART_TX_ACK = TIMER_INVALID;

/*============================================================================*
 *  Private declaration variable
 *============================================================================*/
CsrUint16 last2digit;
CsrUint16 last2digit1;
CsrUint16 last2digit2;

MESH_HANDLER_DATA_T                     g_mesh_handler_data;
timer_id                                gatt_advert_tid;

uint8 gotTxAckFromESP = 0; /* flag if got TX acknowledgement from ESP after CSR TXed */
//#define READY   0
//#define SUCCESS 1
//#define TIMEOUT 2
//#define WAITING 3

uint8 *prevTXBuff = 0;
uint8 *thisTXBuff = 0;
uint16 Get_Group_count = 50;
/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      DELETE
 *
 *  DESCRIPTION
 *      Reset all group model to 0
 *
 * PARAMETERS
 *      Nothing
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DELETE(void)
{
    int index = 35;   
    if(MAX_MODEL_GROUPS == 340)
    {
        while(index < 330)
        {
            extension_model_groups[index] = 0;
            Nvm_Write(&extension_model_groups[index],sizeof(uint16), NVM_OFFSET_EXTENSION_MODEL_GROUPS + index);
            index++;
        }
    }
    else if(MAX_MODEL_GROUPS == 35)
    {
        while(index < 35)
        {
            extension_model_groups[index] = 0;
            Nvm_Write(&extension_model_groups[index],sizeof(uint16), NVM_OFFSET_EXTENSION_MODEL_GROUPS + index);
            index++;
        }
    }
    return;
}

/*----------------------------------------------------------------------------*
*  NAME
*      genUartCheckSum 
*
*  DESCRIPTION
*      Generate checksum to be transmitted together with the packet send out
*
* RETURNS
*      checksum value
*---------------------------------------------------------------------------*/
extern CsrUint8 genUartCheckSum(uint8 *byte, uint8 len)
{
    CsrUint8 chkSum=0;
    while(len)
    {
        /*
        chkSum = chkSum ^ *byte++;
        chkSum++;
        */
        chkSum = chkSum + ((*byte++) & 0x00FF);
        len--;
    }
    return (chkSum & 0x00FF);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      startTimer
 *
 *  DESCRIPTION
 *      Start a timer
 *
 * PARAMETERS
 *      timeout [in]    Timeout period in seconds
 *      handler [in]    Callback handler for when timer expires
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void startTimer(uint32 timeout, timer_callback_arg handler)
{
    /* Now starting a timer */
    tId = TimerCreate(timeout, TRUE, handler);
             
    /* If a timer could not be created, panic to restart the app */
    if (tId == TIMER_INVALID)
    {
      /* Panic with panic code 0xfe */
      Panic(0xfe);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      timerCallbackUartRxedPinToLow
 *
 *  DESCRIPTION
 *      Each time RX data, PIN_CSR_ACK pin sets HIGH after checksum OK. 
 *      Need to LOW again
 *
 * RETURNS
 *      Nothing
 *--------------------------------------------------------------------------*/ 
extern void timerCallbackUartRxedPinToLow(timer_id const id)
{    
    PioSet(PIN_CSR_ACK,0);
} 

/*----------------------------------------------------------------------------*
 *  NAME
 *      timerCallbackWaitUartTxAck 
 *
 *  DESCRIPTION
 *      Callback timeout when ESP not reply HIGH to CSR after CSR sends UART  
 *
 * RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void timerCallbackWaitUartTxAck(timer_id const id) /* timeout receiving an acknowledge from receiver*/
{
    setUartTxAckFlag(TIMEOUT); /* stamp with timeout flag, no ESP ack within time */
    TimerDelete(tId_UART_TX_ACK);
    tId_UART_TX_ACK = TIMER_INVALID;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      createUartTxAckTimeout 
 *
 *  DESCRIPTION
 *      To start timeout if ESP manages to receive UART data within time
 *      If ESP manages received within time, tId_UART_TX_ACK will be deleted
 *      when detect pin, halting the timeout, halting resends
 *
 * RETURNS
 *      Nothing
 *--------------------------------------------------------------------------*/ 
extern void createUartTxAckTimeout(void)
{    
    tId_UART_TX_ACK = TimerCreate(TIMER_TIMEOUT_1MS * 500, TRUE, timerCallbackWaitUartTxAck);
}  

/*----------------------------------------------------------------------------*
 *  NAME
 *      timerCallbackUartTxedPinToLow
 *
 *  DESCRIPTION
 *      Each time TX data, PIN_TX_FLAG pin sets HIGH. Need to LOW again
 *
 * RETURNS
 *      Nothing
 *--------------------------------------------------------------------------*/ 
extern void timerCallbackUartTxedPinToLow(timer_id const id)
{    
    PioSet(PIN_TX_FLAG,0);
} 

/*----------------------------------------------------------------------------*
 *  NAME
 *      sendTxData
 *
 *  DESCRIPTION
 *      Transmit data out through UART
 *
 * RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void sendTxData(uint8 *byte, uint8 len)
{
    PioSet(PIN_TX_FLAG, 1); 
    UartWrite( byte, len);
    TimerCreate(TIMER_TIMEOUT_1MS * 25, TRUE, timerCallbackUartTxedPinToLow);/* to LOW PIN_TX_FLAG again after HIGH */   
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      setUartTxAckFlag 
 *
 *  DESCRIPTION     
 *      To set the status of the transmit data out
 *
 *      flag:   #define READY   0
 *              #define SUCCESS 1
 *              #define TIMEOUT 2
 *              #define WAITING 3
 *
 * RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void setUartTxAckFlag(int flag)
{
    gotTxAckFromESP = flag;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      mainUartTxHandler
 *
 *  DESCRIPTION
 *      Main UART TX operation handler
 *      Perform checking status of UART transmit and calling operations 
 *      to sends data out
 *
 * RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void mainUartTxHandler(timer_id const id)
{    
    if(gotTxAckFromESP == READY || gotTxAckFromESP == SUCCESS) /*got ack within time */
    {
        thisTXBuff = getTXPacketBuff();  /*get next packet if any, return 0 if no data in UART TX buffer */ 
        if( thisTXBuff != 0) /*if there any packet available already */
        { 
            prevTXBuff = thisTXBuff;  /* record pointer, maybe need to resend if chksum error */  
            setUartTxAckFlag(WAITING);  /* Set flag as waiting ack from ESP  */
            sendTxData(thisTXBuff, PACKET_SIZE);  /* add data to UartWrite */         
            createUartTxAckTimeout(); /* Start timer timeout to check either ESP received within time */
        }
    }
    
    if(gotTxAckFromESP == TIMEOUT) /* got no ack from ESP after timeout */
    {
        thisTXBuff = prevTXBuff;  /* restore previous pointer of data */
        setUartTxAckFlag(WAITING); /*  Set flag as waiting ack from ESP */ 
        sendTxData(thisTXBuff, PACKET_SIZE); /* add data to UartWrite    */   
        createUartTxAckTimeout(); /* Start timer timeout to check either ESP received within time */
    }
    
    TimerCreate(TIMER_TIMEOUT_1MS * 50, TRUE, mainUartTxHandler);  /*Always loop main Tx Handler */
}

extern void Group_Set_UartTxHandler(timer_id const id)
{
     uint8 byte1[PACKET_SIZE];
     MemSet(byte1, 0, PACKET_SIZE - 1);
     /* Must refer to the Hub's main chip RX uart position 
        Byte 0 - Opcode High
        Byte 1 - Opcode Low
        Byte 2 - Device Id High
        Byte 3 - Device Id Low
        Byte 4 - Location Id High
        Byte 5 - Location Id Low
        Byte 6-13 - Data
        Byte 14 - Checksum
     */
     
     if (group_data.opcode == CSRMESH_GROUP_MODEL_GROUPID)
     {
         byte1[0] = 0xFD;
         byte1[1] = group_data.opcode;//0x11
         byte1[2] = group_data.DeviceID >> 8;
         byte1[3] = group_data.DeviceID & 0xFF;
         byte1[6] = group_data.groupindex;
         byte1[7] = group_data.tid;
         byte1[8] = group_data.groupid >> 8;
         byte1[9] = group_data.groupid & 0xFF;
         byte1[10] = group_data.instance;
         byte1[14] = genUartCheckSum( byte1, (PACKET_SIZE - 1)); /* get 14 bytes of chksum  */
     }
     
     if (numgroup_data.opcode == CSRMESH_GROUP_NUMBER_OF_MODEL_GROUPIDS)
     {
        byte1[0] = 0xFD;
        byte1[1] = numgroup_data.opcode;
        byte1[2] = numgroup_data.DeviceID >> 8;
        byte1[3] = numgroup_data.DeviceID & 0xFF;
        byte1[6] = numgroup_data.model;
        byte1[7] = numgroup_data.num;
        byte1[8] = numgroup_data.tid;
        byte1[14] = genUartCheckSum( byte1, (PACKET_SIZE - 1)); /* get 14 bytes of chksum  */
     }
     
     if (battery_data.opcode == CSRMESH_BATTERY_STATE)
     {
        byte1[0] = 0xFC;
        byte1[1] = battery_data.opcode >> 8;
        byte1[2] = battery_data.DeviceID >> 8;
        byte1[3] = battery_data.DeviceID & 0xFF;
        byte1[6] = battery_data.batterylevel;
        byte1[7] = battery_data.batterystate;
        byte1[8] = battery_data.tid;
        byte1[14] = genUartCheckSum( byte1, (PACKET_SIZE - 1)); /* get 14 bytes of chksum  */
     }
     
     copyToTXPacketBuffer(byte1);
     
     if (Get_Group_count < 2)
     {
        Get_Group_count++;  
        TimerCreate(TIMER_TIMEOUT_1MS * 100, TRUE, Group_Set_UartTxHandler);
     }
     else
     {
        Get_Group_count = 50;
     }
}