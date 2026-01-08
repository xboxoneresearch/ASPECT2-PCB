#!/usr/bin/env python3
"""Fix size and CRC fields in a single binary's tombstone.

Usage examples:
  Fix an iapl (preloader) binary and write to a separate file:
    scripts/fix_size_crc.py iapl preloader.bin fixed_pre.bin 

The script implements ProgramType with values: UAPP, IAPL. Based on the
type it chooses the correct tombstone offset and data region for size/crc
calculation:
  - IAPL: expects a fixed preloader size (0x800). Tombstone at 0x7E0.
          CRC is computed over the preloader up to the tombstone.
  - UAPP: tombstone at file start (0x0). CRC/size computed from entry
          offset (0x200) to EOF.
"""

import argparse
import enum
import struct
from typing import Tuple

# Constants mirroring firmware common/bootloader.h
PRELOADER_SIZE = 0x800
TOMBSTONE_SIZE = 0x20
PRELOADER_TOMBSTONE_OFF = PRELOADER_SIZE - TOMBSTONE_SIZE  # 0x7E0
UAPP_TOMBSTONE_OFF = 0x0
UAPP_ENTRY_OFFSET = 0x200


class ProgramType(enum.Enum):
    UAPP = 'uapp'
    IAPL = 'iapl'


def write_le_u16(buf: bytearray, offset: int, value: int):
    buf[offset:offset+2] = struct.pack('<H', value & 0xFFFF)


def write_le_u32(buf: bytearray, offset: int, value: int):
    buf[offset:offset+4] = struct.pack('<I', value & 0xFFFFFFFF)


def crc32_final(data: bytes, crc: int = 0xffffffff) -> int:
    """
    CRC32/MPEG-2 with data fed in byte-reversed u32-chunks
    """
    U32_SZ = 4
    assert len(data) % 4 == 0
    for position in range(0, len(data), U32_SZ):
        uint32 = data[position:position+U32_SZ]
        # Reverse bytes
        uint32 = uint32[::-1]

        for val in uint32:
            crc ^= val << 24
            for _ in range(8):
                crc = crc << 1 if (crc & 0x80000000) == 0 else (crc << 1) ^ 0x104c11db7
    return crc


def compute_and_patch(buf: bytearray, ptype: ProgramType) -> Tuple[int, int, int]:
    """Compute size and crc for given program type and patch in-place.

    Returns (tombstone_offset, size, crc).
    """
    if ptype is ProgramType.IAPL:
        if len(buf) != PRELOADER_SIZE:
            raise SystemExit(f"IAPL/preloader expected size {PRELOADER_SIZE:#x}, got {len(buf):#x}")
        crc_region = bytes(buf[:PRELOADER_TOMBSTONE_OFF])
        size = len(crc_region)
        crc = crc32_final(crc_region)
        tombstone_off = PRELOADER_TOMBSTONE_OFF
    elif ptype is ProgramType.UAPP:
        if len(buf) < UAPP_ENTRY_OFFSET:
            raise SystemExit("UAPP binary smaller than expected entry offset; cannot compute tombstone")
        crc_region = bytes(buf[UAPP_ENTRY_OFFSET:])
        size = len(crc_region)
        crc = crc32_final(crc_region)
        tombstone_off = UAPP_TOMBSTONE_OFF
    else:
        raise SystemExit("Unsupported program type")
    
    tombstone = struct.unpack_from("<4sHH2s4s", buf, tombstone_off)
    magic, ver_maj, ver_min, size_tpl, crc_tpl = tombstone
    assert magic in [b"IAPL", b"UAPP"], f"Invalid magic in tombstone, got: {magic}"
    assert b"SZ" == size_tpl, f"Invalid size placeholder, got: {size_tpl}"
    assert b"CRC\x00" == crc_tpl, f"Invalid CRC placeholder, got: {crc_tpl}"

    print(f"Type={ptype} Version={ver_maj}.{ver_min}")

    # size @ +0x08 (u16), crc @ +0x0A (u32)
    write_le_u16(buf, tombstone_off + 0x08, size)
    write_le_u32(buf, tombstone_off + 0x0A, crc)

    return tombstone_off, size, crc


def parse_prog_type(s: str) -> ProgramType:
    s2 = s.strip().lower()
    if s2 in ('uapp', 'userapp'):
        return ProgramType.UAPP
    if s2 in ('iapl', 'preloader'):
        return ProgramType.IAPL
    raise argparse.ArgumentTypeError('must be one of: uapp, iapl')


def main():
    p = argparse.ArgumentParser(description='Fix size and CRC fields in a single binary tombstone')
    p.add_argument('type', type=parse_prog_type, help='program type: uapp or iapl')
    p.add_argument('input', help='input binary file')
    p.add_argument('out', help='output file path; if omitted, file is modified in-place')
    args = p.parse_args()

    with open(args.input, 'rb') as f:
        data = bytearray(f.read())

    tombstone_off, size, crc = compute_and_patch(data, args.type)

    with open(args.out, 'wb') as f:
        f.write(data)

    print(f"Patched {args.out}: tombstone_off=0x{tombstone_off:04X} size={size} crc=0x{crc:08X}")


if __name__ == '__main__':
    main()
