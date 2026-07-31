#include "slave.h"
#include "bootloader.h"
#include "postcodes.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t register_map[REG_MAP_SIZE] = {0}; // Register map storage
static uint8_t new_segment_available = 0;
static uint8_t reg_index = 0;

static uint32_t error = HAL_OK;
static uint8_t error_state = 0;

static uint8_t bootloader_magic_buf[4] = {0};
static uint8_t receiving_boot_magic = 0;   // Set while a BOOTLOADER_TRIGGER_I2C_ADDR write is in flight

static uint8_t rx_scratch[REG_MAP_SIZE + 1]; // [0]=target register, [1..]=data

// Set by ISR callbacks, checked by Slave_Poll() from the main loop.
static volatile uint8_t need_restart = 0;

void Slave_Start(void);

/**
 * @brief Start listening for addr
 *
 */
void Slave_Init(void)
{
        Slave_Start();
}

void Slave_Poll(void)
{
        if (need_restart) {
                need_restart = 0;
                Slave_Start();
        }
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
                        // Single call for the whole transaction
                        ret = HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, rx_scratch, sizeof(rx_scratch), I2C_NEXT_FRAME);
                }

                if (ret != HAL_OK) {
                        need_restart = 1;
                }
        }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        need_restart = 1;
}

static void Slave_FlushRxScratch(void)
{
        if (hi2c1.pBuffPtr >= rx_scratch && hi2c1.pBuffPtr <= rx_scratch + sizeof(rx_scratch)) {
                uint16_t received = (uint16_t)(hi2c1.pBuffPtr - rx_scratch);

                if (received >= 1) {
                        uint8_t start_addr = rx_scratch[0];
                        uint16_t data_len = received - 1;

                        for (uint16_t i = 0; i < data_len; i++) {
                                register_map[(start_addr + i) % REG_MAP_SIZE] = rx_scratch[1 + i];
                        }
                        reg_index = (uint8_t)(start_addr + data_len);

                        // Did the received range contain Segment register?
                        if (start_addr <= REG_Segments && REG_Segments < start_addr + data_len) {
                                new_segment_available = 1;
                        }
                }
        }
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

        Slave_FlushRxScratch();
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        need_restart = 1;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
        if (hi2c != &hi2c1) {
                return;
        }

        error = HAL_I2C_GetError(&hi2c1);

        // error is a bitmask
        if (error & HAL_I2C_ERROR_AF) {
            __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);
        }
        if (error & HAL_I2C_ERROR_ARLO) {
            __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ARLO);
        }
        if (error & HAL_I2C_ERROR_OVR) {
            __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_OVR);
        }
        if (error & HAL_I2C_ERROR_BERR) {
            __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
            error_state = 1;
        }

        hi2c->ErrorCode = 0;

        if (receiving_boot_magic) {
                // Truncated bootloader-trigger write; discard it.
                receiving_boot_magic = 0;
        } else {
                Slave_FlushRxScratch();
        }

        need_restart = 1;
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