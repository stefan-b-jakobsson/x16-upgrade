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

import x16pkg
import re

print("Searching for latest Github releases...")

# Fetch Kernal ROM
rom_version = x16pkg.github_fetch_latest("X16Community", "x16-rom", "rom.bin$", "res/rom.bin")
if rom_version == None:
    print("Kernal ROM not found")
    quit()
else:
    print("Downloaded Kernal ROM " + rom_version + " -> res/rom.bin")
    rom_real_version = x16pkg.get_rom_version("res/rom.bin")

# Fetch VERA firmware
vera_version = x16pkg.github_fetch_latest("X16Community", "vera-module", ".bin$", "res/vera.bin")
if vera_version == None:
    print("VERA firmware not found")
    quit()
else:
    print("Downloaded VERA firmware " + vera_version + " -> res/vera.bin")
    vera_real_version = re.search("([0-9]+[.]{1}[0-9]+[.]{1}[0-9]+)", vera_version).group(1).split(".")
    for i in range(0,3):
        vera_real_version[i] = int(vera_real_version[i])

# Fetch SMC firmware
smc_version = x16pkg.github_fetch_latest("X16Community", "x16-smc", "x16-smc.ino.hex$", "res/x16-smc.ino.hex")
if smc_version == None:
    print("SMC firmware not found")
    quit()
else:
    print("Downloaded SMC firmware " + smc_version + " -> res/x16-smc.ino.hex")
    smc_real_version = re.search("([0-9]+[.]{1}[0-9]+[.]{1}[0-9]+)", smc_version).group(1).split(".")
    for i in range(0,3):
        smc_real_version[i] = int(smc_real_version[i])

# Make package
x16pkg.make_pkg("X16 Latest Official Releases", "X16 Community", "res/rom.bin", "res/vera.bin", "res/x16-smc.ino.hex", vera_real_version, smc_real_version, "build/x16-latest.pkg")
print("Created package -> build/x16-latest.pkg")
