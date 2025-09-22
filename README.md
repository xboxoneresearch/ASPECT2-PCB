[![Build PCBs](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_pcbs.yml/badge.svg)](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_pcbs.yml)
[![Build firmware](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_stm32_fw.yml/badge.svg)](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_stm32_fw.yml)
[![GitHub Tag](https://img.shields.io/github/v/tag/XboxOneResearch/ASPECT2-PCB)](https://github.com/xboxoneresearch/ASPECT2-PCB/releases)


# ASPECT2 PCB

Development board for Xbox One / Xbox Series consoles, based mostly on [blurry PCB images](https://xboxoneresearch.github.io/wiki/hardware/facet/#external-pcb).

Allows for [POST-Code](https://xboxoneresearch.github.io/wiki/hardware/post/) monitoring (via I2C) and reading flash via SPI.

On some development kits, there are more features available.

See [Functionality](#functionality) for more infos

![PCB - front](https://xboxoneresearch.github.io/ASPECT2-PCB/3D/FACET-3D_top.png)
![PCB - front](https://xboxoneresearch.github.io/ASPECT2-PCB/3D/FACET-3D_top30deg.png)
![PCB - front](https://xboxoneresearch.github.io/ASPECT2-PCB/3D/FACET-3D_bottom.png)


This repo contains:

* KiCad files for the PCB / schematics
* [EEPROM template](./eeprom/) for FT_PROG
* [STM32 firmware](./firmware/)

## Functionality

| Functionality | Description            | Tested / Working   | Notes                                               |
| ------------- | ---------------------- | ------------------ | --------------------------------------------------- |
| SPI           | Reading eMMC flash     | ✅                 | Could also support reading SPI NOR on series-family |
| POST          | POST-Code display      | ✅                 | STM32 renders codes to OLED display                 |
| JTAG          | Southbridge JTAG       | ✅                 | Devkit exclusive                                    |
| I2C           | SMBus communication    | ❌                 | Requires bitbanging driver @ libaspect2 (TODO)      |
| SWO           | ARM Single-Wire-Output | ❌                 | No success, Devkit exclusive (?)                    |
| UART (KRNL)   | Kernel Debugging       | ✅                 | Devkit exclusive                                    |
| UART (SMC)    | SMC/SB Debugging (?)   | ❌                 | No success, likely Devkit exclusive                 |


* Devkit exclusive: Requires specific flavor of bootloaders or certain [Certificate capabilities](https://xboxoneresearch.github.io/wiki/security/certificates/#capabilities).


### Software

There are two main repos with support software for this board.

- [dsmc-rs]() - A windows x64 tool that makes use of a proprietary DLL to read flash via SPI. Also supports reading **Expected1SMCBLDigest** from flash controller.
- [libaspect2](https://github.com/xboxoneresearch/libaspect2) - Attempt at an open source library

## Directory structure

```text
|- Main Aspect2 PCB
├── eeprom
│   ├── Greybull_FabD+.xml - FT Prog template (official on-board FACET)
│   └── Facet2_fabA+.xml - FT Prog template (derived from onboard FTDI dump)
├── firmware - STM32 firmware
├── lib - additional KiCad symbols / footprints
```

## Manufacturing / Development

### PCB Stackup

- 4 Layer board (for impedance control) - 1.6mm thickness
- Stackup:
  - F.Cu: Signal
  - In1.Cu: Power (GND)
  - In2.Cu: Power (GND)
  - B.Cu: Signal

This PCB was designed with the target stackup of **JLC04161H-3313** (JLCPCB). See [JLCPCB - Impedance](https://jlcpcb.com/impedance).

### Manufacturing files

Generally, [Github Pages](https://xboxoneresearch.github.io/ASPECT2-PCB/) hosts the most up-to-date manufacturing files.
These are also available for download on the [Releases](https://github.com/xboxoneresearch/ASPECT2-PCB/releases)-page.

Here is a list of the most important ones.

- Bill of materials (BOM): [ibom (Interactive HTML)](https://xboxoneresearch.github.io/ASPECT2-PCB/Assembly/FACET-ibom.html), [HTML](https://xboxoneresearch.github.io/ASPECT2-PCB/BoM/Generic/FACET-bom.html), [CSV](https://xboxoneresearch.github.io/ASPECT2-PCB/BoM/Generic/FACET-bom.csv)
- Schematic: [PDF](https://xboxoneresearch.github.io/ASPECT2-PCB/Schematic/FACET-schematic.pdf)
- Datasheets: [Here](https://xboxoneresearch.github.io/ASPECT2-PCB/Browse/FACET-navigate_Schematic_docs.html#basic_download_datasheets)

We recommend JLCPCB, due to the specific Layer-Stackup that was used.

JLCPCB MFG files

- [Gerbers](https://xboxoneresearch.github.io/ASPECT2-PCB/Manufacturers/FACET-_JLCPCB_compress.zip)
- [Pick and place / CPL (CSV)](https://xboxoneresearch.github.io/ASPECT2-PCB/Manufacturers/JLCPCB/FACET_cpl_jlc.csv)
- [BOM (CSV)](https://xboxoneresearch.github.io/ASPECT2-PCB/Manufacturers/JLCPCB/FACET_bom_jlc.csv)

### Software / plugins

* [KiCad 9.0](https://www.kicad.org/download/)
* [KiCad JLCPCB tools extension](https://github.com/Bouni/kicad-jlcpcb-tools) (for looking up / syncing JLCPCB part#)

## Module bring-up

Before being able to communicate with the device, programming eeprom or regular usage, drivers need to be installed!

* Windows x64
* [FTDI Drivers](https://ftdichip.com/wp-content/uploads/2021/08/CDM212364_Setup.zip)
* [FT_PROG](https://ftdichip.com/utilities/#ft_prog) (Programming eeprom only)

### Programming the eeprom

* Download & install drivers
* Download & open FT Prog
* Connect Aspect2 board
* Click "Scan & Parse" to enumerate connected device (Magnifying glass icon or "Devices -> Scan & Parse")
* Right-click on the found device, hit **Apply template**
  - Choose [Facet2_FabA+.xml](./eeprom/Facet2_FabA+.xml)
* Click Program (Lightning bolt icon or "Devices -> Program")
* Click Program-button in the dialog box
* Click Close-button, close FT Prog tool
* Re-plug Aspect2 board

### Programming the STM32 (POST Code display)

See [firmware/](./firmware/) for build & flash instructions.

## References

- [AssemblerGames](https://web.archive.org/web/20250327165519/https://assemblergames.org/viewtopic.php?p=870129) - first mentioning of FACET2 PCB
- [FACET on xosft wiki](https://xboxoneresearch.github.io/wiki/hardware/facet/) - Technical documentation of FACET port
- [POST code on xosft wiki](https://xboxoneresearch.github.io/wiki/hardware/post/) - Original MAX6958A circuit and error codes
- [POST code database](https://errors.xboxresearch.com)
