/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
  
#include <ble_direct_test.h>

#include <main.h>           /* Functions relating to powering up the device */
/*#include <sleep.h>  */        /* Control functions for sleep states */
#include <uart.h>           /* Functions to interface with the chip's UART */
#include <uart_sdk.h>       /* Enums to interface with the chip's UART */
#include <pio.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "uart_queue.h"
#include "extension_model_handler.h"
#include "user_config.h"
#include "app_mesh_handler.h"
#include "reset.h"
#include "ONESECOND.h"
#include "main_app.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Standard setup of CSR102x development boards */
#define UART_PIO_TX 8
#define UART_PIO_RX 9
#define UART_PIO_RTS PIO_NONE
#define UART_PIO_CTS PIO_NONE

/* The UART sub-system supports a set of buffer sizes that are referenced by a 
 * set of buffer size definitions UART_BUF_SIZE_BYTES_X   (64 In this case)
 * Always declare UART buffers using one of these values
 * For more details, see the uart_buf_size_bytes enumeration definition 
 * in the firmware library documentation for the uart.h file */

#define RX_BUFFER_SIZE      UART_BUF_SIZE_BYTES_32   
#define TX_BUFFER_SIZE      UART_BUF_SIZE_BYTES_32

#define TIMER_TIMEOUT_1MS   (1 * MILLISECOND)
/*============================================================================*
 *  Private Data
 *============================================================================*/
/* The application is required to create two buffers, one for receive, one
 * for transmit. 
 * The buffers need to meet alignment requirements therefore buffers
 * are declared using the UART_DECLARE_BUFFER definition
 * For more details, see the macro definition in uart.h
 */

/* Create 64-byte receive buffer for UART data */
UART_DECLARE_BUFFER(uart_rx_buffer, RX_BUFFER_SIZE);

/* Create 64-byte transmit buffer for UART data */
UART_DECLARE_BUFFER(uart_tx_buffer, TX_BUFFER_SIZE);

/* This needs to be bigger than the value requested in UartRead */
static uint8 uart_input[4];

/* Flag that we are waiting for the UART */
static uint16 uart_busy;
    
/* Pointer for user callback function, called on receipt of UART data */
user_read_callback_fn user_read_callback;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* UART callback, response to UART_READ_IND messages */
static void appUartProcessReadIndEvent(void   *p_rx_buffer,
                                       uint16  length);

/* Callback function, response to UART_WRITE_CFM messages */
static void appUartProcessWriteCfmEvent(sys_status status);


/*============================================================================*
 *  Private declaration variable
 *============================================================================*/
unsigned int data_num;


/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
 

/*-----------------------------------------------------------------------------*
 *  NAME
 *      appUartProcessReadIndEvent
 *
 *  DESCRIPTION
 *      This function is called when a UART_READ_IND message is received.
 *
 *  PARAMETERS
 *      void *p_rx_buffer  Pointer to the receive buffer (uint8 if 'unpacked'
 *                         or uint16 if 'packed' depending on the chosen UART
 *                         data mode - this application uses 'unpacked')
 *
 *      uint16 length      Number of bytes ('unpacked') or words ('packed')
 *                         received
 *
 *  RETURNS
 *      The number of bytes ('unpacked') or words ('packed') that have been
 *      processed.
 *----------------------------------------------------------------------------*/
static void appUartProcessReadIndEvent(void   *p_rx_buffer,
                                       uint16  length)
{
    /* If there is a registered callback */
    if ( user_read_callback != NULL )
    {
        /* Call the registered user callback function */
        length = user_read_callback(p_rx_buffer, length);
        
        /*
        if ( length > 0 )
        {
            // Submit any queued data
           
           sendPendingData1();
        }*/
    }
}
 
/*-----------------------------------------------------------------------------*
 *  NAME
 *      appUartProcessWriteCfmEvent
 *
 *  DESCRIPTION
 *      This function is called when a UART_WRITE_CFM message is received.
 *
 *  PARAMETERS
 *      sys_status  status   Result of previous UART write
 *      a = 1   UART Receive
 *      a = 0   UART Transmit
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appUartProcessWriteCfmEvent(sys_status status)
{
    /* Check status of previous operation */
    if ( sys_status_success == status)
    {
        /* Data accepted by UART tx buffer, update application queue */
        //UQ_CommitLastPeek();
    }
    
    /* UART not busy */
    //uart_busy = FALSE;

    /* Submit queued data */
    
    //sendPendingData1();
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppUartProcessEvent
 *
 *  DESCRIPTION
 *      This function handles any UART specific messages
 *
 *  PARAMETERS
 *      msg_t  msg      Message data
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void AppUartProcessEvent(msg_t *msg)
{
    /* UART message type */
    uart_msg_t *uart_msg;
    
    /* Cast to UART message type */
    uart_msg = (uart_msg_t*)msg;
    
    switch(msg->header.id)
    {
        /* This is the message we get with incomming data. */
        case UART_READ_IND:
            appUartProcessReadIndEvent(uart_msg->body.read_ind.buffer, 
                                       uart_msg->body.read_ind.length);
            /* Request callback on received data */
            UartRead((void*)uart_input,1,0);
            break;
            
        /* Data submitted with UartWrite() has been writen to the UART transmit
           buffer */
        case UART_WRITE_CFM:
            /* Handle write confirm message */
            appUartProcessWriteCfmEvent(uart_msg->body.write_cfm.status);
            break;
        default:
            break;
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      sendPendingData1
 *
 *  DESCRIPTION
 *      Submit queued data to the UART for transmission
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void sendPendingData1(void)
{
    /* Count of bytes to send */
    uint16 bytes_to_send;
    
    /* Pointer to data in UART queue */
    uint8* p_data;

    /* UART is busy, come back later */
    if ( uart_busy == FALSE )
    {
        /* Is there data in the UART Queue */
        if ( UQ_DataAvailable() > 0 )
        {
            /* Amount of data to extract from queue */
            bytes_to_send = data_num;
            
            /* Locate data in the queue */
            p_data = UQ_Peek(&bytes_to_send);
            if ( bytes_to_send > 0 )
            {
                /* Signal that the UART is now busy */
                uart_busy = TRUE;
                
                /* Submit data to UART */
                UartWrite(p_data, bytes_to_send);
            }
        }
    }
}

/*----------------------------------------------------------------------------*
*  NAME
*      UARTMCUinit 
*
*  DESCRIPTION
*      Init Uart
*
* RETURNS
*      Nothing
*---------------------------------------------------------------------------*/
void UARTMCUinit(user_read_callback_fn cb, uint16 baud_rate)
{
    /* Configuration structure for the UART */
    uart_pio_pins_t uart_pins;
    
    /* Configure the interface pins */
    uart_pins.rx  = UART_PIO_RX;
    uart_pins.tx  = UART_PIO_TX;
    uart_pins.rts = PIO_NONE;
    uart_pins.cts = PIO_NONE;
    
    /* Initialise the UART physical interface */
    UartInit(uart_rx_buffer, RX_BUFFER_SIZE,
             uart_tx_buffer, TX_BUFFER_SIZE,
             uart_data_unpacked,
             &uart_pins);
    
    UartConfig(baud_rate, UARTCFG_TWO_STOP_BITS);
    
    /* Enable UART */
    UartEnable(TRUE);
    
    /* UART not busy */
    uart_busy = FALSE;
    
    /* Set pointer to callback function */
    user_read_callback = cb;
   
    /* UART receive threshold is set to 1 byte, so that 1 byte
     * received will trigger the receiver callback */
    UartRead((void*)uart_input, 1, 0);   
}
 

