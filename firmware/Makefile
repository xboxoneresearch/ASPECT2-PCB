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

ARTIFACTS_DIR := artifacts

OUT_DIR := out
OUT_BIN := $(OUT_DIR)/combined.bin

all: merge

preloader:
	$(MAKE) -C $(PRE_DIR) all

userapp: preloader
	$(MAKE) -C $(APP_DIR) all

copy: userapp
	@mkdir -p $(ARTIFACTS_DIR)
	@cp $(PRE_ELF) $(ARTIFACTS_DIR)/
	@cp $(APP_ELF) $(ARTIFACTS_DIR)/ 

merge: copy
	@mkdir -p $(OUT_DIR)
	python3 scripts/merge_bins.py $(PRE_BIN) $(APP_BIN) $(OUT_BIN)

flash: merge
	@openocd -f common/openocd.cfg -c "program out/combined.bin verify reset exit 0x08000000"

clean:
	$(MAKE) -C $(PRE_DIR) clean
	$(MAKE) -C $(APP_DIR) clean
	-rm -rf $(OUT_DIR)
	-rm -rf $(ARTIFACTS_DIR)

