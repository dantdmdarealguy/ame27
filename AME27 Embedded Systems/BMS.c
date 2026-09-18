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
    
    uint8_t new_active = 0;
    int32_t current_mA = latest_current_mA;
    if (max_voltage > 4.2f) new_active |= FAULT_CELL_OVER_VOLTAGE;
    if (min_voltage < 2.5f) new_active |= FAULT_CELL_UNDER_VOLTAGE;
    if (max_temperature > 60.0f) new_active |= FAULT_CELL_OVER_TEMP;
    if ((max_voltage - min_voltage) > 0.2f) new_active |= FAULT_CELL_DELTA_EXCEEDED;
    if (current_mA > 200000 || current_mA < -200000) new_active |= FAULT_PACK_OVER_CURRENT;
    
    active_faults = new_active;
    latched_faults |= new_active;
    if (clear_requested)
    {
        latched_faults &= new_active;
        clear_requested = false;
    }
    
    if (latched_faults != 0)
    {
        HAL_SetSDC(false);
    }
    else
    {
        HAL_SetSDC(true);
    }
    
    uint8_t tx_data[CAN_LEN] = {0};
    tx_data[0] = active_faults;
    tx_data[1] = latched_faults;
    HAL_SendCanMsg(0x0B0, tx_data);
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
