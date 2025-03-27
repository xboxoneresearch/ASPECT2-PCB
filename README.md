[![Build PCBs](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_pcbs.yml/badge.svg)](https://github.com/xboxoneresearch/ASPECT2-PCB/actions/workflows/build_pcbs.yml)

> [!WARNING]  
> To reach the state of ready-for-manufacturing, check out the [issues](https://github.com/xboxoneresearch/ASPECT2-PCB/issues)

# ASPECT2 PCB

![PCB - work in progress](./assets/pcb_work_in_progress.png)


This repo contains:

* KiCad files for the PCB / schematics
* EEPROM template for FT_PROG

## Directory structure

```text
|- Main Aspect2 PCB
├── eeprom
│   ├── Greybull_FabD+.xml - FT Prog template (official on-board FACET)
│   └── Facet2_fabA+.xml - FT Prog template (derived from onboard FTDI dump)
├── lib - additional KiCad symbols / footprints
└── references - Datasheets and original blurry PCB photos
```

## Requirements

### Design Circuit

* [KiCad 8.0](https://www.kicad.org/download/)
* [KiCad JLCPCB tools extension](https://github.com/Bouni/kicad-jlcpcb-tools) (generating manufacturing files -> Gerbers, BOM, CPL)

### Using the module

Before being able to communicate with the device, programming eeprom or regular usage, drivers need to be installed!

* Windows x64
* [FTDI Drivers](https://ftdichip.com/wp-content/uploads/2021/08/CDM212364_Setup.zip)
* [FT_PROG](https://ftdichip.com/utilities/#ft_prog) (Programming eeprom only)

## Programming the eeprom

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

## References

- [AssemblerGames](https://web.archive.org/web/20250327165519/https://assemblergames.org/viewtopic.php?p=870129) - first mentioning of FACET2 PCB
- [FACET on xosft wiki](https://xboxoneresearch.github.io/wiki/hardware/facet/) - Technical documentation of FACET port
- [libaspect2](https://github.com/xboxoneresearch/libaspect2) - Attempt at an open source library
