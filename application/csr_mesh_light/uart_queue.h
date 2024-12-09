/*******************************************************************************
 *  Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  Part of Bluetooth Low Energy CSR102x SDK 3.1.1
 *  Application version 3.1.1.0
 *
 *  FILE
 *      uart_queue.h
 *
 *  DESCRIPTION
 *      Interface to circular buffer implementation.
 *
 ******************************************************************************/

#ifndef __UART_QUEUE_H__
#define __UART_QUEUE_H__

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/
 
#include <types.h>         /* Commonly used type definitions */
#include <uart.h> 
#include <uart_sdk.h> 
#include <timer.h> 

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/
#define READY   0
#define SUCCESS 1
#define TIMEOUT 2
#define WAITING 3

#define PACKET_SIZE         15
#define PACKET_TOTAL_NUMBER 20

/* Define the required UART baud rate */
#define APP_UART_RATE UART_RATE_230K4

/* Definition of user callback function type */
typedef uint16 (*user_read_callback_fn)(void *p_rx_buffer, uint16 length);

/*  Run the startup routine  */
extern void UARTMCUinit(user_read_callback_fn cb, uint16 baud_rate);

/*  This function handles any UART specific messages  */
extern void AppUartProcessEvent(msg_t *msg);


/*  Queue the supplied data if there is sufficient space available.
 *  If there is not enough space FALSE is returned instead.
 */
extern bool UQ_SafeQueueBytes(const uint8 *p_data, uint16 len);

/*  Queue the supplied data. If there is not enough space then data at the
 *  head of the queue is overwritten and the head of the queue moved to
 *  the end of the new data.
 */
extern void UQ_ForceQueueBytes(const uint8 *p_data, uint16 len);

/*  Return the total size of the buffer  */
extern uint16 UQ_BufferCapacity(void);

/*  Return the amount of data currently in the queue  */
extern uint16 UQ_DataAvailable(void);

/*  Return the amount of free space available in the buffer  */
extern uint16 UQ_SpaceAvailable(void);

/*  Reset queue pointers leaving the queue empty  */
extern void UQ_Reset(void);

/*  Peek up to the specified number of bytes from the queue, without
 *  modifying the buffer. If not enough data is held in the queue then
 *  the function returns immediately with whatever data is available.
 */
extern uint8* UQ_Peek(uint16 *len);

/*  Remove from the queue the data that was returned in the last call to
 *  UQPeekBytes.
 */
extern void UQ_CommitLastPeek(void);

/* Transmit waiting data over UART1*/
extern  void sendPendingData1(void);

extern void copyToTXPacketBuffer(const uint8 *p_data);

extern uint8 *getTXPacketBuff(void);

extern void mainUartTxHandler(timer_id const id); 

extern void setUartTxAckFlag(int flag);

extern uint8 genUartCheckSum(uint8 *byte, uint8 len);

extern void sendTxData(uint8 *byte, uint8 len);
       
#endif /* __UART_QUEUE_H__ */
