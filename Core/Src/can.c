#include "can.h"
#include "stm32f0xx_hal.h"

// CAN control modes
//#define CAN_CTRLMODE_NORMAL             0x00
#define USB_8DEV_CAN_MODE_SILENT        0x01
#define USB_8DEV_CAN_MODE_LOOPBACK      0x02
#define USB_8DEV_MODE_ONESHOT           0x04

// Not supported
//#define CAN_CTRLMODE_3_SAMPLES          0x04
//#define CAN_CTRLMODE_BERR_REPORTING     0x10
//#define CAN_CTRLMODE_FD                 0x20
//#define CAN_CTRLMODE_PRESUME_ACK        0x40
//#define CAN_CTRLMODE_FD_NON_ISO         0x80

extern CAN_HandleTypeDef hcan;

static uint8_t enabled; /*< Indicates if CAN interface in enabled. */
static CAN_FilterTypeDef sFilterConfig;

static void can_interrupts_enable();
static void can_interrupts_disable();

/**
 * Initialize CAN interface.
 *
 * @return 0 if OK
 */
uint8_t can_init() {
    enabled = 0;
    return 0;
}

/**
 * Request to open the CAN interface.
 *
 * Set up a request and corresponding initialization data to start the CAN
 * interface.
 *
 * @param can_bittiming CAN bit timings to configure CAN.
 * @param ctrlmode flag setting CAN control modes e.g. @see
 * USB_8DEV_CAN_MODE_SILENT
 */
void can_open_req(Can_BitTimingTypeDef* can_bittiming, uint8_t ctrlmode) {
    /* See datasheet p836 on bit timings for SJW, BS1 and BS2
     * tq = (BRP+1).tpclk
     * tsjw = tq.(SJW+1)
     * tbs1 = tq.(TS1+1)
     * tbs2 = tq.(TS2+1)
     * baud = 1/(tsjw+tbs1+tbs2) = 1/(tq.((SJW+1)+(TS1+1)+(TS2+1)))
     */

    // Configure the CAN peripheral
    hcan.Instance = CANx;

    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = DISABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = DISABLE;
    if (ctrlmode & USB_8DEV_MODE_ONESHOT) {
        hcan.Init.AutoRetransmission = ENABLE;
    }
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    if (ctrlmode & USB_8DEV_CAN_MODE_SILENT) {
        hcan.Init.Mode |= CAN_MODE_SILENT;
    }
    if (ctrlmode & USB_8DEV_CAN_MODE_LOOPBACK) {
        hcan.Init.Mode |= CAN_MODE_LOOPBACK;
    }
    // The shift is needed because that's how CAN_SJW_xTQ,CAN_BS1_xTQ and
    // CAN_BS2_xTQ are defined
    hcan.Init.SyncJumpWidth = can_bittiming->sjw << (6 * 4);
    hcan.Init.TimeSeg1 = can_bittiming->ts1 << (4 * 4);
    hcan.Init.TimeSeg2 = can_bittiming->ts2 << (5 * 4);
    hcan.Init.Prescaler = can_bittiming->brp;

    // Configure the CAN Filter, needed to receive CAN data
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
}

/**
 * Open the CAN interface.
 *
 * Initialize and configure the filters for the CAN interface.
 *
 * @return 0 if OK
 */
uint8_t can_open() {
    if (HAL_CAN_Init(&hcan)) {
        return 1;
    }
    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig)) {
        return 2;
    }

    HAL_CAN_Start(&hcan);

    /*if (HAL_CAN_Receive_IT(&hcan, CAN_FIFO0)) {*/
        /*return 3;*/
    /*}*/
    can_interrupts_enable();
    enabled = 1;
    return 0;
}

/**
 * Close the CAN interface
 */
uint8_t can_close() {
    can_interrupts_disable();
    if (HAL_CAN_DeInit(&hcan)) {
        return 1;
    }
    enabled = 0;
    return 0;
}

/**
 * Check if there are CAN messages pending in receive FIFO.
 *
 * @return Number of messages pending.
 */
uint8_t can_msg_pending() {
    if (!enabled) {
        return 0;
    } else {
    	return(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0));
    }
}

static void can_interrupts_enable() {
    /* Enable FIFO0 overrun interrupt */
    // TODO not easily handled by HAL
    //__HAL_CAN_ENABLE_IT(&hcan, CAN_IT_FOV0);

    /* Enable error warning interrupt */
    __HAL_CAN_ENABLE_IT(&hcan, CAN_IT_ERROR_WARNING);

    /* Enable error passive interrupt */
    __HAL_CAN_ENABLE_IT(&hcan, CAN_IT_ERROR_PASSIVE);

    /* Enable bus-off interrupt */
    __HAL_CAN_ENABLE_IT(&hcan, CAN_IT_BUSOFF);

    /* Enable last error code interrupt */
    __HAL_CAN_ENABLE_IT(&hcan, CAN_IT_LAST_ERROR_CODE);

    /* Enable error interrupt */
    __HAL_CAN_ENABLE_IT(&hcan, CAN_IT_ERROR);
}

static void can_interrupts_disable() {
    /* Disable FIFO0 overrun interrupt */
    //__HAL_CAN_ENABLE_IT(&hcan, CAN_IT_FOV0);

    /* Disable error warning interrupt */
    __HAL_CAN_DISABLE_IT(&hcan, CAN_IT_ERROR_WARNING);

    /* Disable error passive interrupt */
    __HAL_CAN_DISABLE_IT(&hcan, CAN_IT_ERROR_PASSIVE);

    /* Disable bus-off interrupt */
    __HAL_CAN_DISABLE_IT(&hcan, CAN_IT_BUSOFF);

    /* Disable last error code interrupt */
    __HAL_CAN_DISABLE_IT(&hcan, CAN_IT_LAST_ERROR_CODE);

    /* Disable error interrupt */
    __HAL_CAN_DISABLE_IT(&hcan, CAN_IT_ERROR);
}
