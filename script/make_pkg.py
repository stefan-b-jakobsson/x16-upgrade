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

# -----------------------------------------------
# CONFIG
# -----------------------------------------------

# Package file information
pkg_info = "Package Description"
pkg_created_by = "Name of Creator"

# Firmware versions
vera_version = [0,0,0]
smc_version = [0,0,0]

# Firmware resources
rom_path = "res/rom.bin"
vera_path = "res/VERA.BIN"
smc_path = "res/x16-smc.ino.hex"

# Output path
pkg_path = "build/X16_nn.PKG"

# -----------------------------------------------
# BUILD PACKAGE
# -----------------------------------------------

r = x16pkg.make_pkg(pkg_info, pkg_created_by, rom_path, vera_path, smc_path, vera_version, smc_version, pkg_path)
if r[0] != 0:
    print(r[1])
