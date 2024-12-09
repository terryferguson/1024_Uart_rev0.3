/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
 
#include <main.h>           /* Functions relating to powering up the device */
/*#include <ls_app_if.h>*/      /* Link Supervisor application interface */
#include <timer.h>          /* Chip timer functions */
#include <panic.h>          /* Support for applications to panic */
/*============================================================================*
 *  Local Header Files
 *============================================================================*/

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/*Define the INPUT PORT*/
/*#define PIO_BIT_MASK(pio)       (0x01 << (pio))*/ 

//#define RF_MODE     (13)          /*CSR1010 PIO 3*/
//#define Protection     (14)       /*CSR1010 PIO 4*/
/*
#define EXT1_SW     (14)
#define CURRENT_PROTECTION (14)
*/
/*
#define SW2_MASK    PIO_BIT_MASK(RF_MODE)
#define SW3_MASK    PIO_BIT_MASK(Protection)
*/
/*#define SW4_MASK    PIO_BIT_MASK(SW4_PIO)*/

/* Bit-mask of all the Switch PIOs used by the board. */
/*#define BUTTONS_BIT_MASK        (SW2_MASK | SW3_MASK)*/


/*Define the OUTPUT PORT*/
//#define OUTPUT1     (11)   /*CSR1010 PIO 10*/
//#define OUTPUT2     (12)   /*CSR1010 PIO 9*/


/* Comparison between CSR1010 & CSR1024 pinout */
    /* ESP sends to CSR:
       PIN_RX_FLAG(input)       1010:pin-10  1024:pin-11
       PIN_CSR_ACK(output)      1010:pin-9   1024:pin-12
       
       CSR sends to ESP:
       PIN_TX_FLAG(output)      1010:pin-11  1024:pin-10
       PIN_ESP_ACK(input)       1010:pin-3   1024:pin-13
     */
#define PIN_CSR_ACK     (12)
#define PIN_RX_FLAG     (11)
#define PIN_TX_FLAG     (10)
#define PIN_ESP_ACK     (13)
/*============================================================================*
 *  Private Data
 *============================================================================*/

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* Start timer 1[sec] */
extern void startTimer(uint32 timeout, timer_callback_arg handler);

extern void DELETE(void);
extern void timerCallbackUartRxedPinToLow(timer_id const id);
extern void timerCallbackUartTxedPinToLow(timer_id const id);
extern void timerCallbackWaitUartTxAck(timer_id const id);
extern void createUartTxAckTimeout(void);
extern void Group_Set_UartTxHandler(timer_id const id);