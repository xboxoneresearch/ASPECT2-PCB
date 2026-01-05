#include "slave.h"
#include "display.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t register_map[REG_MAP_SIZE] = {0}; // Register map storage
static uint8_t new_segment_available = 0;
static uint8_t reg_index = 0;              // Current write address

static uint32_t error = HAL_OK;
static uint8_t error_state = 0;

void Slave_Start(void);

/**
 * @brief Start listening for addr
 *
 */
void Slave_Init(void)
{
        Slave_Start();
}

void Slave_Start(void)
{
        while (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_BUSY) {}

        if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_LISTEN) {
                HAL_I2C_DisableListen_IT(&hi2c1);
        }
        while (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_BUSY) {}

        HAL_I2C_EnableListen_IT(&hi2c1);
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
        if (hi2c != &hi2c1) {
                return;
        }
        error_state = 0;

        // Check if master->slave transmission
        if (TransferDirection == I2C_DIRECTION_TRANSMIT)
        {
                HAL_StatusTypeDef ret = HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, &reg_index, 1, I2C_NEXT_FRAME);
                if (ret != HAL_OK) {
                        Error_Handler();
                }
        }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        Slave_Start();
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        if (HAL_I2C_Slave_Seq_Receive_IT(hi2c, &register_map[reg_index++], 1, I2C_NEXT_FRAME) != HAL_OK) {
                Error_Handler();
        }
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        Error_Handler();
}

void handle_complete()
{
        if (reg_index == 0x26 && (register_map[0x24] & 0x1))
                new_segment_available = 1;
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        error = HAL_I2C_GetError(&hi2c1);

        switch (error) {
                case HAL_I2C_ERROR_AF:
                        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);
                        break;
                case HAL_I2C_ERROR_BERR:
                        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
                        error_state = 1;
                        break;
                default:
                        Error_Handler();
        }
        
        hi2c->ErrorCode = 0;
        handle_complete();

        Slave_Start();

}

uint8_t slave_reg_read(uint8_t addr) { return register_map[addr % REG_MAP_SIZE]; }
void slave_reg_write(uint8_t addr, uint8_t val) { register_map[addr % REG_MAP_SIZE] = val; }

uint8_t slave_is_new_segment_available(void) {
        if (new_segment_available) {
                new_segment_available = 0;
                return 1;
        }
        return 0;
}

uint8_t slave_hard_error(void) {
        return error_state;
}