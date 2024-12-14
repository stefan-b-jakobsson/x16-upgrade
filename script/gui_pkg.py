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

import os
import tkinter as tk
from tkinter import filedialog as fd
import x16pkg

def rom_get_file():
    dialog = fd.askopenfilename(filetypes=[(".bin files", "*.bin")])
    if dialog:
        # Set path
        rom_path.set(dialog)
        rom_update_version()

def rom_update_version(self):
    try:
        ver = x16pkg.get_rom_version(txtROM.get())
        if ver < 128:
            rom_ver.set("R" + str(ver))
        elif ver != 255:
            rom_ver.set("R" + str((ver^0xff)+1) + " (Pre-release)")
        else:
            rom_ver.set("Custom build")
    except:
        rom_ver.set("")

def vera_get_file():
    dialog = fd.askopenfilename(filetypes=[(".bin files", "*.bin")])
    vera_path.set(dialog)

def vera_version_array():
    try:
        a = vera_ver.get().split(".")
        if len(a) != 3:
            return False
        return [int(a[0]), int(a[1]), int(a[2])]
    except:
        return False

def smc_get_file():
    dialog = fd.askopenfilename(filetypes=[(".hex files", "*.hex")])
    smc_path.set(dialog)

def smc_version_array():
    try:
        a = smc_ver.get().split(".")
        if len(a) != 3:
            return False
        return [int(a[0]), int(a[1]), int(a[2])]
    except:
        return False

def btnCreate_click():
    # Validate input
    if len(txtDescription.get()) < 1:
        tk.messagebox.Message(message="Package description missing").show()
        return
    
    if len(txtCreatedBy.get()) == 0:
        tk.messagebox.Message(message="Package created by missing").show()
        return

    if os.path.isfile(rom_path.get()) == False:
        tk.messagebox.Message(message="ROM file not found").show()
        return
    
    if os.path.isfile(vera_path.get()) == False:
        tk.messagebox.Message(message="VERA file not found").show()
        return
    
    if os.path.isfile(smc_path.get()) == False:
        tk.messagebox.Message(message="SMC file not found").show()
        return

    if vera_version_array() == False:
        tk.messagebox.Message(message="VERA version format error. Use major.minor.patch, example 47.0.0").show()
        return

    if smc_version_array() == False:
        tk.messagebox.Message(message="SMC version format error. Use major.minor.patch, example 47.0.0").show()
        return

    # Get output file name
    pkg_path = fd.asksaveasfile().name

    # Make file
    r = x16pkg.make_pkg(txtDescription.get(), txtCreatedBy.get(), rom_path.get(), vera_path.get(), smc_path.get(), vera_version_array(), smc_version_array(), pkg_path)
    tk.messagebox.Message(message=r[1]).show()


# Window
window = tk.Tk()
window.title("Create X16 package file")
window.eval('tk::PlaceWindow . center')
window.minsize(500,400)

window.columnconfigure(0, weight=1)
window.columnconfigure(1, weight=50)
window.columnconfigure(2, weight=1)


rom_path = tk.StringVar()
vera_path = tk.StringVar()
smc_path = tk.StringVar()

rom_ver = tk.StringVar()
vera_ver = tk.StringVar()
smc_ver = tk.StringVar()

# Description
lblDescription = tk.Label(window, text="Package description")
lblDescription.grid(row=0, column=0, sticky="w")

txtDescription = tk.Entry(window)
txtDescription.grid(row=0, column=1, sticky="we")

# Created by
lblCreatedBy = tk.Label(window, text="Created by")
lblCreatedBy.grid(row=1, column=0, sticky="w")

txtCreatedBy = tk.Entry(window)
txtCreatedBy.grid(row=1, column=1, sticky="we")

# Files
lblFiles = tk.Label(window, text="Files:")
lblFiles.grid(row=2, column=0, sticky="w")

lblROM = tk.Label(window, text="ROM")
lblROM.grid(row=3, column=0, sticky="w", padx="10")

txtROM = tk.Entry(window, textvariable=rom_path)
txtROM.grid(row=3, column=1, sticky="we")
txtROM.bind('<FocusOut>', rom_update_version)

btnROM = tk.Button(window, text="...", command=rom_get_file)
btnROM.grid(row=3, column=2)

lblVERA = tk.Label(window, text="VERA")
lblVERA.grid(row=4, column=0, sticky="w", padx="10")

txtVERA = tk.Entry(window, textvariable=vera_path)
txtVERA.grid(row=4, column=1, sticky="we")

btnVERA = tk.Button(window, text="...", command=vera_get_file)
btnVERA.grid(row=4, column=2)

lblSMC = tk.Label(window, text="SMC")
lblSMC.grid(row=5, column=0, sticky="w", padx="10")

txtSMC = tk.Entry(window, textvariable=smc_path)
txtSMC.grid(row=5, column=1, sticky="we")

btnSMC = tk.Button(window, text="...", command=smc_get_file)
btnSMC.grid(row=5, column=2)

# Firmware versions

lblFiles = tk.Label(window, text="Firmware versions:")
lblFiles.grid(row=6, column=0, columnspan=2, sticky="w")

lblROMver = tk.Label(window, text="ROM")
lblROMver.grid(row=7, column=0, sticky="w", padx="10")

txtROMver = tk.Entry(window, textvariable=rom_ver)
txtROMver.grid(row=7, column=1, sticky="we")
txtROMver.config(state=tk.DISABLED)

lblVERAver = tk.Label(window, text="VERA")
lblVERAver.grid(row=8, column=0, sticky="w", padx="10")

txtVERAver = tk.Entry(window, textvariable=vera_ver)
txtVERAver.grid(row=8, column=1, sticky="we")

lblSMC = tk.Label(window, text="SMC")
lblSMC.grid(row=9, column=0, sticky="w", padx="10")

txtSMC = tk.Entry(window, textvariable=smc_ver)
txtSMC.grid(row=9, column=1, sticky="we")

lblFiller = tk.Label(window)
lblFiller.grid(row=10,column=0)

# Buttons

frame = tk.Frame(window)

btnCancel = tk.Button(frame, text="Cancel", command=quit)
btnCancel.pack(side=tk.LEFT)

btnCreate = tk.Button(frame, text="Create Package", command=btnCreate_click)
btnCreate.pack(side=tk.RIGHT)

frame.grid(row=11, column=0, columnspan=3, sticky="we", padx=20, pady=20)

# Enter window main loop

window.mainloop()
