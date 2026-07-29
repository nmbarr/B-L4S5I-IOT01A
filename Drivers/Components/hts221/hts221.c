#include "hts221.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_i2c.h"
#include <stdint.h>

HAL_StatusTypeDef HTS221_ReadReg(HTS221_Handle_t *pHandle, uint8_t RegAddress, uint8_t *pBuffer, uint16_t Length) {
    
    // We OR the RegAddress with the auto-increment bit value to set up multi-read bytes
    uint8_t ActualAddress = RegAddress | HTS221_AUTO_INCREMENT_BIT;

    // Call Mem_Read from the STM32 HAL library
    return HAL_I2C_Mem_Read(pHandle->hi2c, 
        HTS221_I2C_ADDR, 
        ActualAddress, 
        I2C_MEMADD_SIZE_8BIT, 
        pBuffer, 
        Length, 
        HAL_MAX_DELAY
    );
}

HAL_StatusTypeDef HTS221_ReadCalibration(HTS221_Handle_t *pHandle) {

    // The byte we are about to read
    uint8_t CalibrationByte;

    // The return type of reading the register
    HAL_StatusTypeDef status;

    status = HTS221_ReadReg(pHandle, HTS221_REG_H0_RH_X2, &CalibrationByte, 1);
    if (status != HAL_OK) {
        return  status;
    }
    pHandle->H0_rH_x2 = CalibrationByte;

    status = HTS221_ReadReg(pHandle, HTS221_REG_H1_RH_X2, &CalibrationByte, 1);
    if (status != HAL_OK) {
        return  status;
    }
    pHandle->H1_rH_x2 = CalibrationByte;

    // Need a 16-bit to store the 10-bit combined value
    uint16_t T0DegC;

    // Stores the LSB
    uint8_t T0Lsb;

    // Stores the MSB
    uint8_t T0Msb;

    status = HTS221_ReadReg(pHandle, HTS221_REG_T0_DEGC_X8, &T0Lsb, 1);
    if (status != HAL_OK) {
        return  status;
    }
    
    status = HTS221_ReadReg(pHandle, HTS221_REG_T0_T1_MSB, &T0Msb, 1);
    if (status != HAL_OK) {
        return  status;
    }

    // Extract T0's MSB's from the byte (located at [1:0])
    T0Msb &= HTS221_T0_MSB_MASK;

    // Combine 8-bit LSB with 2-bit MSB
    T0DegC = (T0Msb << 8) | T0Lsb;

    // Update typedef with combined 10-bit value
    pHandle->T0_degC_x8 = T0DegC;

    // Need a 16-bit to store the 10-bit combined value
    uint16_t T1DegC;

    // Stores the LSB
    uint8_t T1Lsb;

    // Stores the MSB
    uint8_t T1Msb;

    status = HTS221_ReadReg(pHandle, HTS221_REG_T1_DEGC_X8, &T1Lsb, 1);
    if (status != HAL_OK) {
        return  status;
    }
    
    status = HTS221_ReadReg(pHandle, HTS221_REG_T0_T1_MSB, &T1Msb, 1);
    if (status != HAL_OK) {
        return  status;
    }

    // Extract T1's MSB's from the byte (located at [3:2])
    T1Msb = (T1Msb & HTS221_T1_MSB_MASK) >> HTS221_T1_MSB_SHIFT;

    // Combine 8-bit LSB with 2-bit MSB
    T1DegC = (T1Msb << 8) | T1Lsb;

    // Update typedef with combined 10-bit value
    pHandle->T1_degC_x8 = T1DegC;

    // Buffer to hold the 2 bytes for the _OUT registers
    uint8_t buffer[2];

    int16_t out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_H0_T0_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->H0_T0_OUT = out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_H1_T0_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->H1_T0_OUT = out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_T0_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->T0_OUT = out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_T1_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->T1_OUT = out;

    return HAL_OK;
}

HAL_StatusTypeDef HTS221_ReadRawOutput(HTS221_Handle_t *pHandle) {
    
    // The return type of reading the register
    HAL_StatusTypeDef status;

    // Buffer to hold the 2 bytes for the _OUT registers
    uint8_t buffer[2];

    int16_t out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_H_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->H_OUT = out;

    status = HTS221_ReadReg(pHandle, HTS221_REG_T_OUT, buffer, 2);
    if (status != HAL_OK) {
        return status;
    }

    out = (buffer[1] << 8) | buffer[0];
    pHandle->T_OUT = out;

    return HAL_OK;
}

bool HTS221_CheckWhoAmI(HTS221_Handle_t *pHandle) {

    // The byte we are about to read and where data will be written to
    uint8_t WhoAmIByte;

    // Call ReadReg to read the data from the register
    HAL_StatusTypeDef status = HTS221_ReadReg(pHandle, HTS221_REG_WHO_AM_I, &WhoAmIByte, 1);

    // If the mem read didnt return good data or errored out, dont try to compare and just return early
    if (status != HAL_OK) {
        return false;
    }

    return WhoAmIByte == HTS221_WHO_AM_I_VALUE;
}

float HTS221_ReadTemperature(HTS221_Handle_t *pHandle) {
    float temp;
    float T0_degC = pHandle->T0_degC_x8 / 8.0f;
    float T1_degC = pHandle->T1_degC_x8 / 8.0f;

    HAL_StatusTypeDef status = HTS221_ReadRawOutput(pHandle);
    if (status != HAL_OK) {
        temp = -999.0f;  // sentinel: read failed, ignore this value
    }
    else {
        temp = T0_degC + (pHandle->T_OUT - pHandle->T0_OUT) * (T1_degC - T0_degC) / (pHandle->T1_OUT - pHandle->T0_OUT);
    }

    return temp;
}

float HTS221_ReadHumidity(HTS221_Handle_t *pHandle) {
    float humidity;
    float H0_rH = pHandle->H0_rH_x2 / 2.0f;
    float H1_rH = pHandle->H1_rH_x2 / 2.0f;

    HAL_StatusTypeDef status = HTS221_ReadRawOutput(pHandle);
    if (status != HAL_OK) {
        humidity = -999.0f;  // sentinel: read failed, ignore this value
    }
    else {
        humidity = H0_rH + (pHandle->H_OUT - pHandle->H0_T0_OUT) * (H1_rH - H0_rH) / (pHandle->H1_T0_OUT - pHandle->H0_T0_OUT);
    }

    return humidity;
}