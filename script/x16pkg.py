# Copyright (c) 2024, Stefan Jakobsson
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:

# (1) Redistributions of source code must retain the above copyright notice, 
#     this list of conditions and the following disclaimer. 
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Python Standard Library
import os
from datetime import datetime, timezone
import urllib
from urllib import request
import ssl
import re
import zipfile

# Non-standard Libraries
try:
    from intelhex import IntelHex
except:
    print ("Non-standard library intelhex not installed. Install with: pip install intelhex")
    quit()

try:
    import certifi
    certifi_installed = True
except:
    certifi_installed = False
    
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

def get_rom_version(src):
    f = open(src, "rb")
    f.seek(0x3F80)  # = X16 address 00:FF80
    ba = f.read(1)
    f.close()
    return int(ba[0])

def get_vera_version(src):
    f = open(src, "rb")
    ba = f.read(32)
    f.close()

    if ba[2:6] == b"VERA":
        out = ""
        for c in ba[6:]:
            if c == 0:
                return out.strip()
            elif (c>=ord('0') and c<=ord('9')) or c==ord('.'):
                out += chr(c)
    return None

def make_pkg(pkg_info, pkg_created_by, rom_path, vera_path, smc_path, vera_version, smc_version, pkg_path):
    # Package file format version
    pkg_version = 1

    # Package timestamp (UTC)
    pkg_created_on = datetime.now(timezone.utc).strftime("%Y%m%d%H%M%S")

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
        return [1, "ROM file too large"]

    if (smc_size) > 0x1e00:
        return [1, "SMC file overflows into bootloader area"]

    # Calculate CRCs
    rom_crc = crc16_from_file(rom_path)
    vera_crc = crc16_from_file(vera_path)
    smc_crc = 0xffff
    for b in smc_bin:
        smc_crc = crc16(smc_crc, b)

    # Get firmware versions from file, when possible
    rom_version = get_rom_version(rom_path)

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

    return [0, "Package created"]

def github_fetch_latest(user, project, filename_filter, save_dir, save_name):
    # Abort if certifi not installed
    if certifi_installed == False:
        print("Non-standard library certifi not installed. Install with: pip install certifi")
        quit()

    # Create HTTPS context
    ctx = ssl.create_default_context(cafile=certifi.where())
    
    # Github server address
    server = "https://github.com"

    # Fetch latest release page, and find "expanded_assets" link
    req = urllib.request.urlopen(server + "/" + user + "/" + project + "/releases/latest", context=ctx)
    temp = req.read().decode("utf-8")
    assets = re.findall(server + "/" + user + "/" + project + "/releases/expanded_assets/[^\"\']*", temp, re.IGNORECASE)
    req.close()

    if len(assets) == 0:
        return None

    # Fetch assets page, and find "download" links
    req = urllib.request.urlopen(assets[0], context=ctx)
    temp = req.read().decode("utf-8")
    downloads = re.findall("/" + user + "/" + project + "/releases/download/[^\"\']*", temp, re.IGNORECASE)
    req.close()

    # Look for filename match in download links
    for a in downloads:
        rs = re.search("/([^/]+)/([^/]+$)", a, re.IGNORECASE)
        name = rs.group(2)
        version = rs.group(1)

        if re.search(".zip$", name, re.IGNORECASE) != None:
            # Handle zip files
            req = urllib.request.urlopen(server + a, context=ctx)
            f = open(os.path.join(save_dir, "temp.zip"), "wb")
            f.write(req.read())
            f.close()
            req.close()
            
            zip = zipfile.ZipFile(os.path.join(save_dir, "temp.zip"))
            for name in zip.namelist():
                if re.search(filename_filter, name, re.IGNORECASE) != None:
                    f = zip.open(name)
                    content = f.read()
                    f = open(os.path.join(save_dir, save_name), "wb")
                    f.write(content)
                    f.close()
                    return version

        if re.search(filename_filter, name, re.IGNORECASE) != None:
            # Handle other than zip files
            req = urllib.request.urlopen(server + a, context=ctx)
            f = open(os.path.join(save_dir, save_name), "wb")
            f.write(req.read())
            f.close()
            req.close()
            return version
    
    # No release download found, return False
    return None
