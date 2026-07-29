#ifndef HTS221_H
#define HTS221_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// I2C address (already left-shifted for the STM32 HAL)
// 7-bit address is 0x5F; write-form (0xBE) = 0x5F shifted left by 1
#define HTS221_I2C_ADDR             0xBE

// Register addresses
#define HTS221_REG_WHO_AM_I         0x0F

// Calibration register addresses
#define HTS221_REG_H0_RH_X2      0x30
#define HTS221_REG_H1_RH_X2      0x31
#define HTS221_REG_T0_DEGC_X8    0x32
#define HTS221_REG_T1_DEGC_X8    0x33
#define HTS221_REG_T0_T1_MSB     0x35
#define HTS221_REG_H0_T0_OUT     0x36  // + 0x37 for high byte
#define HTS221_REG_H1_T0_OUT     0x3A  // + 0x3B for high byte
#define HTS221_REG_T0_OUT        0x3C  // + 0x3D for high byte
#define HTS221_REG_T1_OUT        0x3E  // + 0x3F for high byte

// Output register addresses
#define HTS221_REG_H_OUT         0x28  // + 0x29 for high byte
#define HTS221_REG_T_OUT         0x2A  // + 0x2B for high byte

// Bit mask values
#define HTS221_T0_MSB_MASK   0x03
#define HTS221_T1_MSB_MASK   0x0C
#define HTS221_T1_MSB_SHIFT  2

// Expected return value from WHO_AM_I
#define HTS221_WHO_AM_I_VALUE       0xBC

// Auto-increment byte for multi-byte reads (OR into register address)
#define HTS221_AUTO_INCREMENT_BIT   0x80

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t T0_degC_x8;
    uint16_t T1_degC_x8;
    uint8_t H0_rH_x2;
    uint8_t H1_rH_x2;
    int16_t H0_T0_OUT;
    int16_t H1_T0_OUT;
    int16_t T0_OUT;
    int16_t T1_OUT;
    int16_t H_OUT;
    int16_t T_OUT;
} HTS221_Handle_t;

// Function to read a register given a register address
HAL_StatusTypeDef HTS221_ReadReg(HTS221_Handle_t *pHandle, uint8_t RegAddress, uint8_t *pBuffer, uint16_t Length);

// Function to read calibration registers
HAL_StatusTypeDef HTS221_ReadCalibration(HTS221_Handle_t *pHandle);

// Function to read output registers
HAL_StatusTypeDef HTS221_ReadRawOutput(HTS221_Handle_t *pHandle);

// Function to verify the WHO_AM_I register returns the correct data
bool HTS221_CheckWhoAmI(HTS221_Handle_t *pHandle);

// Function to return the temperature in degC
float HTS221_ReadTemperature(HTS221_Handle_t *pHandle);

// Function to return the humidity
float HTS221_ReadHumidity(HTS221_Handle_t *pHandle);

#endif // HTS221_H