#include "hal.h"

static uint8_t active_faults = 0;
static uint8_t latched_faults = 0;

enum faultstore
{
    FAULT_CELL_OVER_VOLTAGE = 1 << 0,
    FAULT_CELL_UNDER_VOLTAGE = 1 << 1,
    FAULT_CELL_OVER_TEMP = 1 << 2,
    FAULT_CELL_DELTA_EXCEEDED = 1 << 3,
    FAULT_PACK_OVER_CURRENT = 1 << 4,
};

static volatile int32_t latest_current_mA = 0;
static volatile bool clear_requested = false;

void Init() {
    
}

void Iter() {
    // This function runs periodically at ~20Hz
}

void RxCan() {
    // Called every time a CAN frame is received on the bus, using an interrupt.
    // Keep in mind, this can be called at any point in the execution of your program.
    // You may not use any HAL_* functions here except HAL_RecvCanMsg,
    // which is how you can pull the message from the bus.
    // An example for pulling a CAN frame is shown below.

    uint8_t data[CAN_LEN];
    uint16_t id;
    HAL_RecvCanMsg(&id, data);
}
