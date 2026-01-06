
#include "stm32g031xx.h"
#include "bootloader.h"

typedef void 		(*pFunction)(void);

static pFunction 	JumpToBootLoader;
static uint32_t 	JumpAddress;

extern int 			_estack;
static uint32_t* 	bootloader_flag = (uint32_t*) (&_estack - SYSBOOTLOADER_FLAG_OFFSET);

uint32_t calculateCrc32(uint8_t *data, uint32_t len) {
    if (len == 0) {
        return 0xFFFFFFFF;
    }

    /* Configure and reset CRC peripheral: use byte-wise input and reversed output */
    CRC->CR = CRC_CR_RESET | CRC_CR_REV_IN_0 | CRC_CR_REV_OUT;

    /* Feed data as bytes so peripheral processes one byte per write */
    uint8_t *p = (uint8_t *)data;
    for (uint32_t i = 0; i < len; i++) {
        *((volatile uint8_t *)&CRC->DR) = p[i];
    }

    /* Final XOR and return */
    return (CRC->DR ^ 0xFFFFFFFF);
}

void resetToSystemBootLoader()
{
	*bootloader_flag = SYSBOOTLOADER_FLAG_MAGIC;
	NVIC_SystemReset();
}


void handleSystemBootLoader()
{
	if( *bootloader_flag == SYSBOOTLOADER_FLAG_MAGIC )
	{
		*bootloader_flag = 0; // so next boot won't be affected

		/* Jump to system memory bootloader */
		JumpAddress = *(uint32_t*) (SYSBOOTLOADER_OFFSET + 4);

		JumpToBootLoader = (pFunction) JumpAddress;
		JumpToBootLoader();
	}
}

void jump(uint32_t addr) {
    pFunction app_entry;
    // Setup pointers to vector table start + reset handler
    uint32_t sp = *(uint32_t*)addr;
    uint32_t reset = *(uint32_t*)(addr + 4);

    // Disable all interrupts
    __disable_irq();

    // Disable GPIO
    RCC->IOPENR &= ~RCC_IOPENR_GPIOAEN;
    // Disable CRC
    RCC->AHBENR &= ~RCC_AHBENR_CRCEN;

    // Disable Systick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Disable interrupts and clear pending ones
    for (unsigned int i = 0; i < sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    /* Sanity check: SP must point to SRAM */
    if ((sp & 0x2FFE0000U) != 0x20000000U)
        return;

    // Re-enable interrupts
    __enable_irq();

    // Set vector table base offset
    SCB->VTOR = addr;

    // Switch to Main Stack Pointer (in case it was using the Process Stack Pointer)
    __set_CONTROL(0);
    
    // Instruction synchronization barrier
    __ISB();

    __set_MSP(sp);

    __DSB(); // Data synchronization barrier
    __ISB(); // Instruction synchronization barrier

    app_entry = (pFunction)reset;
    app_entry();
}

bool validateTombstone(uint32_t tombstone_address, uint32_t data_offset, const uint8_t magic[4]) {
    tombstone_t *ts = (tombstone_t *)tombstone_address;

    if (!(ts->magic[0] == magic[0] &&
        ts->magic[1] == magic[1] &&
        ts->magic[2] == magic[2] &&
        ts->magic[3] == magic[3])) {
        return false;
    }

    uint32_t calculated_crc32 = calculateCrc32((uint8_t *)data_offset, ts->size);

    if (calculated_crc32 != ts->crc) {
        return false;
    }

    return true;
}

bool validateIaplTombstone() {
    return validateTombstone(TOMBSTONE_OFFSET_IAPL, PRELOADER_ENTRY_POINT, TOMBSTONE_IAPL_MAGIC);
}

bool validateUappTombstone() {
    return validateTombstone(TOMBSTONE_OFFSET_UAPP, APP_ENTRY_POINT, TOMBSTONE_UAPP_MAGIC);
}