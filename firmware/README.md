# STM32 firmware for MAX6958A emulation

This firmware lets the STM32 pose as a MAX6958A 7-Segment-Display-Controller to the Xbox.
It then interprets the incoming data and renders output to a SSD1306 OLED display. 

## Firmware Layout

This project is split into two independent components:

* **IAPL (In-App preloader) @ `preloader/`**

  * Lives at flash base (`0x0800_0000`).
  * Tombstone info located at the end, at `0x0800_07E0-0x0800_0800`
  * Responsible for:

    * Checking a GPIO pin for 2 s after reset.
    * If pressed → jump to the STM32 system ROM bootloader.
    * If not pressed → jump to the user application at `0x0800_0800`.
  * Contains a fixed **tombstone** block at the end of its flash region, with build metadata (magic, version, date).

* **UAPP (User-app) @ `userapp/`**

  * Section starts at `0x0800_0800` (tombstone: `0x0800_0800`, code: `0x0800_0820`)
  * Normal user firmware, built with its own vector table and linker script.
  * At the very start, contains a **tombstone** block, to identify itself.
  * Uses the tombstone info (if needed) to check validity or report version.

**Flash Memory Layout**

```
+-----------------------+ 0x0800_0000
| Preloader Vector      |
| & Code                |
|  (2 KB-32 bytes)      |
+-----------------------+ 0x0800_07E0
| Tombstone info IAPL   |
| (32 bytes, fixed info)|
+-----------------------+ 0x0800_0800
| Tombstone info UAPP   |
| (32 bytes, fixed info)|
+-----------------------+ 0x0800_0820
| User App Vector       |
| & Code                |
| (up to 30KB-32 bytes) |
+-----------------------+ end of flash
```

* **Preloader region**: `0x0800_0000 .. 0x0800_07E0`
* **Tombstone IAPL**: `0x0800_07E0 .. 0x0800_0800`
* **Tombstone UAPP**: `0x0800_0800 .. 0x0800_0820`
* **User application**: `0x0800_0820 ..`

**Build/Flash**

* Build preloader in `preloader/` → flash at `0x0800_0000`.
* Build user app in `userapp/` → flash at `0x0800_0800`.

## Get latest firmware

Download from the latest release archive: [Release](https://github.com/xboxoneresearch/ASPECT2-PCB/releases) 

## Flashing firmware

### External programming

- Use SWD programmer and connect 3V3, GND, SWDIO, SWDCLK to the MCU.
- Flash via openocd or official stm32flash utility.

### In-field update

1. Disconnect PCB from console!
2. Connect PCB to PC
3. Use custom stm32flash tool from this [repo](https://github.com/xboxoneresearch/libaspect2)

## Development

### Manual

(for linux hosts)

- Ensure to have `make` installed
- Get an `arm-none-eabi` toolchain (f.e. from [developer.arm.com](https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz))
- Extract the archive and add the `bin/` directory to your `$PATH` environment variable
- Build firmware via `make`

