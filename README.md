# X16-UPGRADE

**WARNING**: This tool is experimental.

## Introduction

This program upgrades the three major firmware components of the Commander X16:

- Kernal ROM
- VERA
- SMC

Keep this README accessible during updates for troubleshooting.

## User Interface

### General

The user interface of the program guides you through each step of the upgrade process with dedicated screens.

- Screen 1: Select package file
- Screen 2: Information about the selected package file
    - Custom: Manually select which components you want to update
- Screen 3: Pre-upgrade checks
- Screen 4: Disclaimer & warnings
- Screen 5: Updating firmware

You interact with the program through widgets, such as buttons and checkboxes.

The active widget is color highlighted. You move between widgets with the following keys:

- Tab or right cursor key moves to next widget
- Shift+Tab or left cursor key moves to previous widget

The active widget's action is performed when you press the Return key:

- A checkbox is toggled on or off
- A button is clicked


### Select Package File

At the first screen, you are prompted to enter the name of a package file.

The package file format is made especially for this tool. A package file contains all 
three firmware components (ROM, VERA, and SMC).

Package creators must ensure all included firmware components are compatible with each other.

You can create your own package files with tools included in this project:

- Type ```make latest``` in the project root folder. This command downloads the latest
releases of the ROM, the VERA and the SMC, and builds a package of them. You can optionally
specify the name of the creator of the package and a package description with
the command line arguments "createdby" and "desc", for example like this:
    - ```make latest createdby="My name" desc="My package description"```

- Type ```make package```in the project root folder. This opens up a GUI interface
that lets you select each of the three firmware files manually, and build a package
of them. The GUI inferface also supports downloading the latest releases from Github.

- You may also download the .py files in the script folder, and start the
GUI interface with ```python gui_pkg.py```

The python scripts in the scripts folder require two non-standard libraries. You can install
them with ```pip install -r script/requirements.txt``` or individually with ```pip install intelhex``` and
```pip install certifi```

More information on the package file format is found [here](doc/package-format.md).

### Package and Upgrade Summary

At the second screen, some general information about the selected package file is displayed:

- A short description of the package, for instance "R48 Official Relase"
- Name of the creator of the package
- Date and time when the package was created (UTC)

After that follows a list of firmware components to be installed and their
respective version numbers.

The Kernal ROM version is prefixed with "R" if it's an official release, and
"PR" if it's a pre-release. If it's a custom build, the word "Custom" is displayed".

VERA and SMC version numbers follow major.minor.patch (e.g. 47.0.0).

By default, the program selects all three components for updating. You may
press the Custom button to select only some of the components. 
Please note that a custom update requires that you verify yourself that the components 
to be updated are compatible with the components that are not updated.


### Pre-Upgrade Checks

The pre-upgrade checks are done before the update begins. No changes are
made to the installed components during this stage. The purpose of the pre-upgrade checks 
is to verify that it will actually be possible to install all the selected components.

It is not possible to proceed with the update if any of the pre-upgrade checks fail. In such
case you need to fix the issue, or unselect the component that fails the pre-upgrade
checks.

Some of the pre-upgrade checks raise warnings that you may ignore after considering the risks.

### Install Updates

After successful pre-upgrade checks, you may proceed to install the
updates. Follow the on-screen instructions.


## Errors, Warnings and Other Messages

### Common Messages

Selecting SD card... <br>
&nbsp;&nbsp;Remove jumper from position "JP1" on the VERA board
- This message is displayed if the program is to access the
SD card when a jumper is installed in JP1 on the VERA board.
- The SD card is disabled as long as the jumper is installed.
- Remove the jumper and proceed with the update.

Selecting VERA flash... <br>
&nbsp;&nbsp;Install jumper in position "JP1" on the VERA board
- This message is displayed if the program is to access the
VERA flash memory when a jumper is not installed in JP1 on the VERA board.
- Install the jumper to proceed.
- Later on, the program will prompt you to remove the jumper again in order
to restore the normal operation of the VERA and the SD card.

Loading... nn% Read error
- This error message is shown if the program could not read the
package file. Check that the SD card is working.

Loading... nn% Checksum error
- This error message is shown if the checksum of the loaded data
doesn't match the checksum stored in the package file header.
- The package file is faulty or corrupted. Replace the package file
and try again.

Validating data... nn% Checksum error
- If the new firmware was loaded into RAM during the pre-upgrade
checks, the validity of the data buffer is checked before the
update starts.
- If this check fails it is likely due to a bug in the upgrade program.

Writing... nn% FAIL
- This error message is displayed if updating the firmware
component did not succeed.
- If writing fails at the beginning (0%), it is likely that
the program could not make any changes to the current
firmware.
- If writing fails later on, there is a risk that the
firmware component needs to be restored with an external
programmer.

Verifying... nn% FAIL
- After finishing the update of a firmware component, the
update is verified by comparing the content of the target chip 
(ROM, VERA or SMC) with the new firmware that was intended to be
installed.
- If the verification fails, it is likely that computer
will not work properly, and that you will need an external
programmer to restore the firmware.

### Package File Header Messages

Unrecognized file header format error
- This error message is displayed if the package file doesn't start with
the expected magic string.
- The magic string in package file version 1 is PETSCII "X16PKG"
- In file version 2 it is PETSCII "x16pkg"

Header CRC-16 mismatch error
- This error message is displayed if the calculated checksum for the 
header doesn't match the checksum stored at the end of the header.
- The error implies that the package file is faulty or corrupted.

Unsupported package file version
- This error message is displayed if the update program doesn not
support the package file version. Check if there is a newer
version of the upgrade program and try again.

### ROM Messages

Write-enabled... FAIL <br>
&nbsp;&nbsp;Chip is write protected <br> 
&nbsp;&nbsp;Install jumper in position "J1" <br>
- The ROM chip is write-protected unless a jumper is installed in position "J1" on the main board.
- Install the jumper and try again. It is recommended that you install the jumper when the computer is disconnected from power.

Chip ID... FAIL
- This error message is displayed if the ROM chip doesn't return the expected Chip ID.
- Ensure that the ROM chip is an SST39SF040 and try again.

### SMC Messages

Firmware... FAIL <br>
&nbsp;&nbsp;Unsupported version
- The capability to start the firmware update procedure was added to firmware v 43.0.0.
- The error message is displayed if the current SMC firmware is older than that.
- You must update the SMC firmware using an external programmer.

Firmware... WARN <br>
&nbsp;&nbsp;Downgrading below v47.2.3 not recommended <br>
&nbsp;&nbsp;Press any key (ESC Abort)
- This warning is displayed if you are downgrading the SMC firmware and if the new firmware is below 47.2.3.
- Firmware versions before 47.2.3 cannot identify bootloader v3.
- Firmware versions before 45.1.0 cannot start bootloader v3.
- You may proceed after taking these limitations into account.

Write-enabled... FAIL
- This error message is displayed if the self-programming fuse bit of the
SMC chip (an ATtiny 861) is not enabled.
- Updating the SMC firmware is not possible unless the fuse bit is set up correctly.
- The fuse bits can only be setup with an external programmer.

Bootloader... FAIL <br>
&nbsp;&nbsp;No bootloader
- This error message means that there is no bootloader installed in the SMC.
- Without a bootloader it is not possible to update the SMC firmware.
- Use an external programmer to update the SMC.

Bootloader... FAIL <br>
&nbsp;&nbsp;Unsupported version
- The bootloader version is not supported by the upgrade program.
- Check if a new version of the upgrade program has been released, and 
try using that instead.

Bootloader... WARN <br>
&nbsp;&nbsp;Bad v2, low risk
- There is a low risk that the the so called bad bootloader v2 is
installed.
- This warning is displayed if bootloader v2 is installed and if
the current SMC version is older than 47.2.3, but not
version 45.1.0.

Bootloader... WARN <br>
&nbsp;&nbsp;Bad v2, high risk
- There is a high risk that the so called bad bootloader v2 is
installed.
- This warning is displayed if bootloader v2 is installed and
if the current SMC version is 45.1.0. If the board was
originally delivered with that firmware version, it is
almost certain that you have the bad bootloader.

Bootloader... WARN <br>
&nbsp;&nbsp;Bad v2, confirmed
- This warning is displayed if the upgrade program can
confirm definitely that the bad bootloader v2 is installed.
- This is possible to check if the current firmware
version is 47.2.3 or newer.

Bootloader... WARN <br>
&nbsp;&nbsp;Non-standard bootloader <br>
&nbsp;&nbsp;Updating the SMC not recommended <br>
&nbsp;&nbsp;Press any key (ESC Abort) <br>
- This warning is displayed if the bootloader identify
itself as one of the bootloader versions supported by
the upgrade program, but the checksum of the bootloader
code doesn't match any of the official releases.
- You may have a custom bootloader or the bootloader
code may have been corrupted.
- It is not recommended that you proceed unless you
know that there is a custom bootloader that you
trust.

Bootloader... WARN <br>
&nbsp;&nbsp;Bootloader version undetermined <br>
&nbsp;&nbsp;Updating the SMC not recommended <br>
&nbsp;&nbsp;Press any key (ESC Abort)
- Firmware versions before 47.2.3 cannot determine
if bootloader v3 is installed or if there is no
bootloader installed.
- Proceed if you know that bootloader v3 is
installed.

Momentarily press Power+Reset <br>
at the same time... 20
- This message is displayed before starting the SMC firmware
update.
- By pressing the Power and Reset buttons at the same time 
you confirm that you want to start the update procedure. This
is protection against unwanted updates.
- If you do not press Power and Reset within the 20 second
time limit, the program will try again and a new
20 second countdown is displayed.
- If you only press the Power button, the system will turn off
without update the SMC firmware. If you you only press the
Reset button, the system will reset without updating the
firmware.

Update failed. Last err: nn <br>
- This message is displayed if the SMC firmware update
failed. The last error number is displayed.
- The error numbers have the following meaning:
    - 2: Packet size not 9
    - 3: Checksum error
    - 5: Overwriting bootloader area

Bootloader v1 detected. Disconnect <br>
the computer from power for at least 20 <br>
seconds to finish the update.
- This message is displayed if bootloader v1 is installed.
- Bootloader v1 waits in an infinite loop after finishing
the update. You must disconnect the computer from
power to reset the SMC.

Corrupted (bad) bootloader v2 detected. <br>
DO NOT disconnect the computer from power. <br>
Standby for help...
- This message is displayed if the bad bootloader v2 is installed.
- After the countdown, a help screen is displayed with instructions on how
to finish the update.
- The official bootloader v2 resets the SMC and turns off the system
after finishing the update.
- The bad bootloader v2 hangs at the final stage of the update,
waiting for the SMC to be reset. After the reset, bootloader v2
sets up the first 64 bytes of the SMC flash memory.
- If you disconnect the computer from power, the SMC will most
likely be corrupted, and you need to install the firmware using
an external programmer.
- Instead you need to reset the SMC by connecting pin #10 to ground
using a piece of wire, as described in the help screen displayed
after the countdown.
- Instructions on how to ground SMC pin #10 are found 
[here](https://github.com/X16Community/x16-smc/blob/main/doc/update-with-bad-bootloader-v2.md).
