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
    active_faults = 0;
    latched_faults = 0;
    HAL_SetSDC(false);
}

void Iter() {
    float voltages[N_CELLS];
    float temperatures[N_CELLS];
    HAL_ReadVoltages(voltages);
    HAL_ReadTemperatures(temperatures);
    
    float min_voltage = voltages[0];
    float max_voltage = voltages[0];
    float max_temperature = temperatures[0];
    for (int i = 0; i < N_CELLS; i++)
    {
        float v = voltages[i];
        float t = temperatures[i];
    
        if (v > max_voltage) max_voltage = v;
        if (v < min_voltage) min_voltage = v;
        if (t > max_temperature) max_temperature = t;
    }
}

void RxCan() {
    uint8_t data[CAN_LEN];
    uint16_t id;
    HAL_RecvCanMsg(&id, data);

    switch (id)
    {
    case 0x511:
        latest_current_mA = (int32_t)((data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5]);
        break;
    case 0x1CF:
        clear_requested = true;
        break;
    default: 
        break;
    }
}
