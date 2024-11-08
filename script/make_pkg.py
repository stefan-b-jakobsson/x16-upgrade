import os
from datetime import datetime, timezone
from intelhex import IntelHex

# -----------------------------------------------
# PACKAGE CONFIG
# -----------------------------------------------

# Package file format version
pkg_version = 1

# Package file information
pkg_info = "R48 Unofficial test package"
pkg_created_by = "X16 Community"
pkg_created_on = datetime.now(timezone.utc).strftime("%Y%m%d%H%M%S")

# Firmware versions
rom_version = 48
vera_version = [47,0,2]
smc_version = [47,2,4]

# Firmware resources
rom_path = "res/rom.bin"
vera_path = "res/VERA_47.0.2.bin"
smc_path = "res/x16-smc.ino.hex"

# Output path
pkg_path = "build/X16-R48.PKG"

# -----------------------------------------------

BLOB_ENTRY_SIZE = 16

BLOBTYPE_TEXT = 0
BLOBTYPE_X16_ROM = 1
BLOBTYPE_X16_VERA = 2
BLOBTYPE_X16_SMC = 3

def petscii_encode(str):
    ba = bytearray()
    for c in str:
        b = ord(c)
        if b < 65: ba.append(b)
        elif b < 96: ba.append(b+128)
        else: ba.append(b-32)
    return ba

def crc16(crc, data):
    p = 0x1021
    crc ^= (data << 8)
    for i in range(8):
        if crc & 0x8000:
            crc = (crc << 1) ^ p
        else:
            crc = crc << 1
    return (crc & 0xffff)

def crc16_from_file(src):
    crc = 0xffff
    src = open(src, "rb")
    ba = src.read()
    while ba:
        for b in ba:
            crc = crc16(crc, b)
        ba = src.read()
    src.close()
    return crc

def file_concat(dst, src):
    src = open(src, "rb")
    ba = src.read()
    while ba:
        dst.write(ba)
        ba = src.read()
    src.close()

# Load SMC intelhex file
smc = IntelHex()
smc.loadhex(smc_path)
smc_bin = smc.tobinarray()

# Get file sizes
rom_size = os.stat(rom_path).st_size
vera_size = os.stat(vera_path).st_size
smc_size = len(smc_bin)

# Check file sizes
if rom_size > 0x80000:
    print ("ROM file too large")
    exit(1)

if (smc_size) > 0x1e00:
    print ("SMC firmware file overflows into bootloader area, starting at 0x1E00.")
    exit(1)

# Calculate CRCs
rom_crc = crc16_from_file(rom_path)
vera_crc = crc16_from_file(vera_path)
smc_crc = 0xffff
for b in smc_bin:
    smc_crc = crc16(smc_crc, b)

# Create fix part of header
header = bytearray()

header += petscii_encode("X16PKG")          # Magic string

header.append(pkg_version)                  # Package file format version

pkg_info = pkg_info[0:63]                   # Description
header += petscii_encode(pkg_info)
header += bytes(64-len(pkg_info))

pkg_created_by = pkg_created_by[0:15]       # Created by
header += petscii_encode(pkg_created_by)
header += bytes(16-len(pkg_created_by))

header += bytes(pkg_created_on, "ascii")    # Created on (UTC)

header.append(3)                            # BLOB count (16 bit)
header.append(0)

# BLOB entry 1: Kernal ROM
header.append(BLOBTYPE_X16_ROM)             # Type

header.append(rom_version)                  # ROM version
header.append(0)
header.append(0)

header.append((rom_size & 255))             # ROM size
header.append((rom_size >> 8) & 255)
header.append((rom_size >> 16) & 255)

header.append(rom_crc & 255)                # CRC
header.append((rom_crc >> 8) & 255)

header += bytes(7)                          # Reserved

# BLOB entry 2: VERA
header.append(BLOBTYPE_X16_VERA)            # Type

header.append(vera_version[0])              # VERA version
header.append(vera_version[1])
header.append(vera_version[2])

header.append(vera_size & 255)              # VERA size
header.append((vera_size >> 8) & 255)
header.append((vera_size >> 16) & 255)

header.append(vera_crc & 255)               # CRC
header.append((vera_crc >> 8) & 255)

header += bytes(7)                          # Reserved

# BLOB entry 3: SMC
header.append(BLOBTYPE_X16_SMC)             # Type

header.append(smc_version[0])               # SMC version
header.append(smc_version[1])
header.append(smc_version[2])

header.append(smc_size & 255)               # SMC version
header.append((smc_size >> 8) & 255)
header.append((smc_size >> 16) & 255)

header.append((smc_crc & 255))
header.append((smc_crc >> 8) & 255)

header += bytes(7)                          # Reserved

# Header CRC
crc_header = 0xffff                         # Offset 0x: Header CRC-16
for b in header:
    crc_header = crc16(crc_header, b)
header.append(crc_header & 255)
header.append(crc_header >> 8)

# Create package file
f = open(pkg_path, "wb")
f.write(header)
file_concat(f, rom_path)
file_concat(f, vera_path)
f.write(smc_bin)
f.close()
