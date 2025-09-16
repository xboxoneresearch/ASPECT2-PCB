# STM32 firmware for MAX6958A emulation

This firmware lets the STM32 pose as a MAX6958A 7-Segment-Display-Controller to the Xbox.
It then interprets the incoming data and renders output to a SSD1306 OLED display. 

## Get latest firmware

Download from the latest release archive: [Release](https://github.com/xboxoneresearch/ASPECT2-PCB/releases) 

## Flashing firmware

See datasheet for the particular STM32 MCU.

- TODO

## Development

### VS Code

- Download the [stm32-for-vscode extension](https://marketplace.visualstudio.com/items?itemName=bmd.stm32-for-vscode).
- Open the `firmware/` folder from this repo in VS Code.
- Change to the STM32-tab in VS Code and hit `Build`. 

### Manual

(for linux hosts)

- Ensure to have `make` installed
- Get an `arm-none-eabi` toolchain (f.e. from [developer.arm.com](https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz))
- Extract the archive and add the `bin/` directory to your `$PATH` environment variable
- Build firmware via `make`

For Windows, this should help: [mydeardoctor/STM32_VSCode_Make_Windows_Tutorial](https://github.com/mydeardoctor/STM32_VSCode_Make_Windows_Tutorial)
