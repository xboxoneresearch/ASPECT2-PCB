SHELL := /bin/bash

.PHONY: all preloader userapp copy merge clean

PRE_DIR := preloader
APP_DIR := userapp
PRE_NAME := $(PRE_DIR)/build/preloader
APP_NAME := $(APP_DIR)/build/uapp_max6958_emul
PRE_BIN := $(PRE_NAME).bin
APP_BIN := $(APP_NAME).bin
PRE_ELF := $(PRE_NAME).elf
APP_ELF := $(APP_NAME).elf

OUT_DIR := out
OUT_BIN_PRE := $(OUT_DIR)/preloader.bin
OUT_BIN_APP := $(OUT_DIR)/userapp.bin
OUT_BIN_COMBINED := $(OUT_DIR)/combined.bin

all: merge

preloader:
	$(MAKE) -C $(PRE_DIR) all

userapp: preloader
	$(MAKE) -C $(APP_DIR) all

copy: userapp
	@cp $(PRE_ELF) $(OUT_DIR)/
	@cp $(APP_ELF) $(OUT_DIR)/ 

patch: copy
	@mkdir -p $(OUT_DIR)
	@python3 scripts/fix_size_crc.py iapl $(PRE_BIN) $(OUT_BIN_PRE)
	@python3 scripts/fix_size_crc.py uapp $(APP_BIN) $(OUT_BIN_APP)

merge: patch
	@cat $(OUT_BIN_PRE) $(OUT_BIN_APP) > $(OUT_BIN_COMBINED)

flash: merge
	@openocd -f common/openocd.cfg -c "program out/combined.bin verify reset exit 0x08000000"

clean:
	$(MAKE) -C $(PRE_DIR) clean
	$(MAKE) -C $(APP_DIR) clean
	-rm -rf $(OUT_DIR)

