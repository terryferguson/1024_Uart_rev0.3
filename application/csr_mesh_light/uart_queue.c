/*******************************************************************************
 *  Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  Part of Bluetooth Low Energy CSR102x SDK 3.1.1
 *  Application version 3.1.1.0
 *
 *  FILE
 *      uart_queue.c
 *
 *  DESCRIPTION
 *      Circular buffer implementation.
 *
 ******************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/
 
#include <mem.h>            /* Memory library */

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "uart_queue.h"     /* Interface to this source file */

/*=============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Intended buffer size in number of bytes */
#define BUFFER_SIZE 60

/* Largest amount of data that can be stored in the buffer */
#define BUFFER_LEN (BUFFER_SIZE - 1)

/* Length of data currently held in queue */
#define QUEUE_LENGTH \
       ((g_tail >= g_head) ? g_tail - g_head : BUFFER_SIZE - g_head + g_tail)
           
/* Amount of free space left in queue (= BUFFER_LEN - QUEUE_LENGTH) */
#define QUEUE_FREE \
       ((g_tail >= g_head) ? BUFFER_LEN - g_tail + g_head : g_head - g_tail - 1)

/*=============================================================================*
 *  Private Data
 *============================================================================*/

/* Circular buffer */
static uint8 g_queue[BUFFER_SIZE];

/* Pointer to head of queue (next byte to be read out) */
static uint16 g_head = 0;

/* Pointer to head of queue after committing most recent peek */
static uint16 g_peek = 0;

/* Pointer to tail of queue (next byte to be inserted) */
static uint16 g_tail = 0;

/* Circular packet buffer */
uint8 u_queue[PACKET_SIZE * PACKET_TOTAL_NUMBER];
uint8 *g_headPtr = u_queue; /* record the head */
uint8 *g_writePtr = u_queue;
uint8 *g_readPtr = u_queue;

/*=============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Append the supplied data to the queue */
static void copyIntoBuffer(const uint8 *p_data, uint16 len);

/* Read up to the requested number of bytes out of the queue */
static uint8* peekBuffer(uint16 *len);

/*=============================================================================*
 *  Private Function Implementations
 *============================================================================*/
 
/*-----------------------------------------------------------------------------*
 *  NAME
 *      copyIntoBuffer
 *
 *  DESCRIPTION
 *      Copy a given number of bytes in to the buffer. Assumes there is enough
 *      space available in the buffer. If not, the existing data will be
 *      overwritten to accommodate the new data.
 *
 *      At the end of the function g_head points to the oldest queue entry and
 *      g_tail the next insertion point.
 *
 *  PARAMETERS
 *      uint8  *p_data     Pointer to the data to be copied
 *      uint16  len        Number of bytes of data to be copied
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void copyIntoBuffer(const uint8 *p_data, uint16 len)
{
    /* Sanity check */
    if ( (len == 0) || (p_data == NULL) )
        return;
    
    /* No point copying more data into the queue than the queue can hold */
    if ( len > BUFFER_LEN )
    {
        /* Advance input pointer to the last BUFFER_LEN bytes */
        p_data += len - BUFFER_LEN;
        
        /* Adjust len */
        len = BUFFER_LEN;
    }
    
    /* Check whether the queue will overflow */
    if ( len > QUEUE_FREE )
    {
        /* Advance g_head to point to the oldest item, after the overflow */
        g_head += len - QUEUE_FREE;
        
        /* If this goes past the end of the buffer, wrap around */
        if ( g_head >= BUFFER_SIZE )
            g_head -= BUFFER_SIZE;
        
        /* Update g_peek similarly */
        g_peek = g_head;
    }
    
    /* Check whether we're going past the end of the buffer */
    if ( g_tail + len >= BUFFER_SIZE )
    {
        /* Calculate how much space there is till the end of the buffer */
        const uint16 available = BUFFER_SIZE - g_tail;
        
        /* Copy data into the queue up to end of buffer */
        MemCopy(&g_queue[g_tail], p_data, available);
        
        /* Update g_tail */
        g_tail = len - available;
        
        /* Copy data from start of buffer */
        MemCopy(g_queue, p_data + available, g_tail);
    }
    else
    {
        /* Append data to tail of the queue */
        MemCopy(&g_queue[g_tail], p_data, len);
        
        /* Update g_tail */
        g_tail += len;
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      peekBuffer
 *
 *  DESCRIPTION
 *      Read a given number of bytes from the buffer without removing any data.
 *      If more data is requested than is available, then only the available
 *      data is read.
 *
 *  PARAMETERS
 *      uint16  *len    Number of bytes of data to peek
 *
 *  RETURNS
 *      Number of bytes of data peeked.
 *----------------------------------------------------------------------------*/
static uint8 *peekBuffer(uint16 *len)
{
    /* Number of bytes of data peeked */
    uint16 peeked = *len;    
    
    /* Sanity check */
    if ( *len == 0 )
        return 0;
    
    /* Cannot peek more data than is available */
    if ( peeked > QUEUE_LENGTH )
        peeked = QUEUE_LENGTH;
    
    /* Check whether we're going past the end of the buffer */
    if ( g_head + peeked >= BUFFER_SIZE )
    {
        /* Calculate how much data there is till the end of the buffer */
        peeked = BUFFER_SIZE - g_head;
    }
        
    /* Update g_peek */
    g_peek = g_head + peeked;
        
    /* Store count of peeked bytes for return */
    *len = peeked;
    
    /* Reset to start of buffer if required */
    if ( g_peek >= BUFFER_SIZE )
        g_peek = 0;
    
    /* Return pointer to head of buffer */
    return ( &g_queue[g_head] );
}

/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_SafeQueueBytes
 *
 *  DESCRIPTION
 *      Queue the supplied data if there is sufficient space available.
 *      If there is not enough space FALSE is returned instead.
 *
 *  PARAMETERS
 *      uint8  *p_data    Pointer to the data to be queued
 *      uint16  len       Number of bytes of data to be queued
 *
 *  RETURNS
 *      TRUE if the data is queued successfully
 *      FALSE if there is not enough space in the queue
 *----------------------------------------------------------------------------*/
bool UQ_SafeQueueBytes(const uint8 *p_data, uint16 len)
{
    /* Check whether there is enough space available in the buffer */
    bool ret_val = (QUEUE_FREE >= len);
    
    /* If so, copy the data into the buffer */
    if ( ret_val )
        copyIntoBuffer(p_data, len);
    
    return ret_val;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_ForceQueueBytes
 *
 *  DESCRIPTION
 *      Queue the supplied data. If there is not enough space then data at the
 *      head of the queue is overwritten and the head of the queue moved to
 *      the end of the new data.
 *
 *  PARAMETERS
 *      uint8  *p_data     Pointer to the data to be queued
 *      uint16  len        Number of bytes of data to be queued
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void UQ_ForceQueueBytes(const uint8 *p_data, uint16 len)
{
    /* Copy data into the buffer whether or not space is available */
    copyIntoBuffer(p_data, len);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_BufferCapacity
 *
 *  DESCRIPTION
 *      Return the total size of the buffer.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Total buffer size in bytes
 *----------------------------------------------------------------------------*/
uint16 UQ_BufferCapacity(void)
{
    return BUFFER_LEN;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_DataAvailable
 *
 *  DESCRIPTION
 *      Return the amount of data currently in the queue.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Size of data currently stored in the queue in bytes.
 *----------------------------------------------------------------------------*/
uint16 UQ_DataAvailable(void)
{
    return QUEUE_LENGTH;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_SpaceAvailable
 *
 *  DESCRIPTION
 *      Return the amount of free space available in the buffer.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Size of free space available in the buffer in bytes.
 *----------------------------------------------------------------------------*/
uint16 UQ_SpaceAvailable(void)
{
    return QUEUE_FREE;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_Reset
 *
 *  DESCRIPTION
 *      Reset queue pointers leaving the queue empty.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void UQ_Reset(void)
{
    /* Reset queue pointers */
    g_head = g_peek = g_tail = 0;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_Peek
 *
 *  DESCRIPTION
 *      Peek up to the specified number of bytes from the queue, without
 *      modifying the buffer. If not enough data is held in the queue then
 *      the function returns immediately with whatever data is available.
 *
 *  PARAMETERS
 *      uint16  *len       Number of bytes of data to be peeked
 *
 *  RETURNS
 *      Number of bytes actually peeked, may be fewer than requested if not
 *      enough data is available.
 *----------------------------------------------------------------------------*/
uint8* UQ_Peek(uint16 *len)
{
    /* Peek into the buffer */
    return peekBuffer(len);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UQ_CommitLastPeek
 *
 *  DESCRIPTION
 *      Remove from the queue the data that was returned in the last call to
 *      UQ_Peek.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void UQ_CommitLastPeek(void)
{
    /* Update g_head to point to current g_peek location */
    g_head = g_peek;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      copyToTXPacketBuffer
 *
 *  DESCRIPTION
 *      Copy mesh data to UART buffer
 *
 * RETURNS
 *      Nothing
 *--------------------------------------------------------------------------*/
extern void copyToTXPacketBuffer(const uint8 *p_data)
{
    MemCopy(g_writePtr, p_data, PACKET_SIZE); /* actual copy / buffering*/
    
    g_writePtr = g_writePtr + PACKET_SIZE; /* increment write pointer */
    
    if( (g_writePtr - g_headPtr) >= (PACKET_SIZE * PACKET_TOTAL_NUMBER)) /* if overflow start again */
    {g_writePtr = g_headPtr;} /* reset writePtr back to the start head */  
}



/*----------------------------------------------------------------------------*
 *  NAME
 *      getTXPacketBuff
 *
 *  DESCRIPTION
 *      Check if the UART TX buffer got value. If yes return the pointer to
 *      the data. If no return 0
 *
 * RETURNS
 *      Return pointer to be read
 *--------------------------------------------------------------------------*/
extern uint8 *getTXPacketBuff(void)
{
    uint8 *p_data = g_readPtr; /* record the current read pointer before increment, this is the value to be returned*/
    
    if(g_readPtr == g_writePtr) /* if got no data in TX uart buffer, then return null to indicate no packet is ready */
    {return 0;}
    
    g_readPtr = g_readPtr + PACKET_SIZE; /* increment read pointer */
    
    if( (g_readPtr - g_headPtr) >= (PACKET_SIZE * PACKET_TOTAL_NUMBER)) /* if overflow start again */
    {g_readPtr = g_headPtr;} /* reset readPtr back to the start head */
    
    return p_data;
}