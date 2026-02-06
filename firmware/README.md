# STM32 firmware for MAX6958A emulation

This firmware lets the STM32 pose as a MAX6958A 7-Segment-Display-Controller to the Xbox.
It then interprets the incoming data and renders output to a SSD1306 OLED display. 

## Firmware Layout

This project is split into two independent components:

* **IAPL (In-App preloader) @ `preloader/`**

  * Lives at flash base (`0x0800_0000`).
  * Tombstone info located at the end, at `0x0800_07E0-0x0800_0800`
  * Responsible for:

    * Checking a GPIO pin for 200 ms after reset.
    * If pressed → jump to the STM32 system ROM bootloader.
    * If not pressed → jump to the user application at `0x0800_0800`.
  * Contains a fixed **tombstone** block at the end of its flash region, with build metadata (magic, version, date).

* **UAPP (User-app) @ `userapp/`**

  * Section starts at `0x0800_0800` (tombstone: `0x0800_0800`, code: `0x0800_0A00`)
  * Normal user firmware, built with its own vector table and linker script.
  * At the very start, contains a **tombstone** block, to identify itself.
  * Uses the tombstone info (if needed) to check validity or report version.

**Flash Memory Layout**

```
+--------------------------+ 0x0800_0000
| Preloader Vector         |
| & Code                   |
|  (2 KB-32 bytes)         |
+--------------------------+ 0x0800_07E0
| Tombstone info IAPL      |
| (32 bytes)               |
+--------------------------+ 0x0800_0800
| Tombstone info UAPP      |
| (32 bytes + padding)     |
+--------------------------+ 0x0800_0A00
| User App Vector          |
| & Code                   |
| (up to 30KB-0x200 bytes) |
+--------------------------+ end of flash
```

* **Preloader region**: `0x0800_0000 .. 0x0800_07E0`
* **Tombstone IAPL**: `0x0800_07E0 .. 0x0800_0800`
* **Tombstone UAPP**: `0x0800_0800 .. 0x0800_0820`
* **User application**: `0x0800_0A00 ..`

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

1. Disconnect PCB from console and PC!
2. Press and hold the button on the PCB.
3. Connect PCB to PC, keep button pressed for a second.
4. Use custom `aspect2-stm32-updater` tool from this repo: [repo](https://github.com/xboxoneresearch/libaspect2)

4.1. Preloader first

```
$ ./aspect2-stm32-updater flash preloader preloader.bin`
* Using File=Ok("preloader.bin"), Size=2048 bytes
[*] About to write offset 0x8000000 - 0x8000800 (0x800 bytes)
[*] Start page: 0, count: 1
[+] Chip ID: 0x466
[+] Erasing flash...
[+] Writing firmware...
[00:00:18, eta:2m] Writing █████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 256 B / 2.00 KiB (13 B/s)
```

At the end, this should appear:

```
[+] Writing firmware...
[+] Verifying firmware...
[00:02:17, eta:0s] Writing ████████████████████████████████████████ 2.00 KiB / 2.00 KiB (15 B/s)
[00:02:17, eta:0s] Verifying ████████████████████████████████████████ 2.00 KiB / 2.00 KiB (15 B/s)
[*] Done
```

The info command is supposed to show

```
$ ./aspect2-stm32-updater info
Magic 'IAPL' @ 0x80007E0
Version=1.1, Size=0x7E0, CRC32=0xC2A290A3
Hash valid: YES
No firmware / tombstone found @ 0x8000800
```

4.2. Now userapp (and yes, it is veeeery slow)

```
./aspect2-stm32-updater flash user-app userapp.bin
* Using File=Ok("userapp.bin"), Size=24104 bytes
[*] About to write offset 0x8000800 - 0x8006628 (0x5E28 bytes)
[*] Start page: 1, count: 12
[+] Chip ID: 0x466
[+] Erasing flash...
[+] Writing firmware...
[00:00:11, eta:37m] Writing ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 128 B / 23.54 KiB (10 B/s)
```

Firmware got written successfully

```
[+] Writing firmware...
[00:26:33, eta:0s] Writing ████████████████████████████████████████ 23.54 KiB / 23.54 KiB (14 B/s)
[+] Verifying firmware...
[00:26:33, eta:0s] Verifying ████████████████████████████████████████ 23.54 KiB / 23.54 KiB (14 B/s)
[*] Done
```

Let's check the info command once again

```
$ ./aspect2-stm32-updater info
Magic 'IAPL' @ 0x80007E0
Version=1.1, Size=0x7E0, CRC32=0xC2A290A3
Hash valid: YES
Magic 'UAPP' @ 0x8000800
Version=1.1, Size=0x5C28, CRC32=0xC8961915
Hash valid: YES
```

5. We are all done!


## Development

### Manual

(for linux hosts)

- Ensure to have `make` installed
- Get an `arm-none-eabi` toolchain (f.e. from [developer.arm.com](https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz))
- Extract the archive and add the `bin/` directory to your `$PATH` environment variable
- Build firmware via `make`
