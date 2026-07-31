#include "slave.h"
#include "bootloader.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t register_map[REG_MAP_SIZE] = {0}; // Register map storage
static uint8_t new_segment_available = 0;
static uint8_t reg_index = 0;              // Current write address

static uint32_t error = HAL_OK;
static uint8_t error_state = 0;

static uint8_t bootloader_magic_buf[4] = {0};
static uint8_t receiving_boot_magic = 0;   // Set while a BOOTLOADER_TRIGGER_I2C_ADDR write is in flight

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
                HAL_StatusTypeDef ret;

                // AddrMatchCode is delivered pre-shifted by 1 (same convention as
                // Init.OwnAddress1/OwnAddress2), not the raw 7-bit address.
                if (AddrMatchCode == (BOOTLOADER_TRIGGER_I2C_ADDR << 1)) {
                        receiving_boot_magic = 1;
                        ret = HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, bootloader_magic_buf, sizeof(bootloader_magic_buf), I2C_LAST_FRAME);
                } else {
                        ret = HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, &reg_index, 1, I2C_NEXT_FRAME);
                }

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

        if (receiving_boot_magic) {
                receiving_boot_magic = 0;

                uint32_t magic = ((uint32_t)bootloader_magic_buf[0] << 24)
                                | ((uint32_t)bootloader_magic_buf[1] << 16)
                                | ((uint32_t)bootloader_magic_buf[2] << 8)
                                |  (uint32_t)bootloader_magic_buf[3];

                if (magic == BOOTLOADER_TRIGGER_MAGIC) {
                        resetToSystemBootLoader(); // Does not return
                }
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

void Slave_HandleComplete()
{
        if (reg_index == 0x26 /*&& (register_map[0x24] & 0x0F) > 0*/)
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

        if (receiving_boot_magic) {
                // Truncated bootloader-trigger write; discard it rather than
                // let stale reg_index state be mistaken for a completed
                // register-map transfer below.
                receiving_boot_magic = 0;
        } else {
                Slave_HandleComplete();
        }

        Slave_Start();

}

uint8_t Slave_RegRead(uint8_t addr) { return register_map[addr % REG_MAP_SIZE]; }
void Slave_RegWrite(uint8_t addr, uint8_t val) { register_map[addr % REG_MAP_SIZE] = val; }

uint8_t Slave_IsNewSegmentAvailable(void) {
        if (new_segment_available) {
                new_segment_available = 0;
                return 1;
        }
        return 0;
}

uint8_t Slave_HardError(void) {
        return error_state;
}