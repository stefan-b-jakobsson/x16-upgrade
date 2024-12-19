# X16-UPGRADE

**WARNING**: This tool is under development, and is highly experimental.

## Introduction

The purpose of this program is to update the three major firmware components of the Commander X16:

- Kernal ROM
- VERA
- SMC

It is recommended that you have access to this README file during an update. The README file
provides more in-depth information about possible warnings and errors that are displayed
while updating.

## User Interface

The program's user interface consists of multiple screens, one for each step of the
upgrade process.

- Screen 1: Select package file
- Screen 2: Display information about the selected package file
    - Custom: Manually select which components you want to update
- Screen 3: Run pre-upgrade checks
- Screen 4: Show disclaimer & warnings
- Screen 5: Run updates

You interact with the program through widgets, such as buttons and checkboxes.

The active widget is color highlighted. You move between widgets with the following keys:

- Tab or right cursor key moves to next widget
- Shift+Tab or left cursor key moves to previous widget

The active widget's action is performed when you press the Return key:

- A checkbox is toggled on or off
- A button is clicked

[![Running X16UPDATE](https://i9.ytimg.com/vi/7_O305oB4j0/mq2.jpg?sqp=CKzm-roG-oaymwEmCMACELQB8quKqQMa8AEB-AH-CYAC0AWKAgwIABABGBMgEyh_MA8=&rs=AOn4CLDjf9O_3T6c-bbmiX9N2XqBAAidPg)](https://youtu.be/7_O305oB4j0)


## Choose Package File

At the first screen, the user is prompted to enter the name of a package file.

The package file format is made especially for this tool. A package file contains all 
three firmware components (ROM, VERA, and SMC).

Creators of package files are responsible to make sure that the firmware components in a package 
are compatible with each other.

You can create your own package file with tools included in this project:

- Type ```make latest``` in the project root folder. This command downloads the latest
releases for the ROM, the VERA and the SMC, and builds a package of them

- Type ```make package```in the project root folder. This opens up a GUI interface
that lets you select each of the three firmware files manually, and build a package
of them. The GUI inferface also supports downloading the latest releases from Github.

More information on the package file format is found [here](doc/package-format.md).

## Package and Update Summary

At the second screen, some general information about the selected package file is displayed:

- A short description of the package, for instance "R48 Official Relase"
- Name of the creator of the package
- Date and time when the package was created (UTC)

After that follows a list of firmware components to be installed including the
version numbers of these new components.

The program by default selects all three components to be installed. You may
press the Custom button to select only the components you want to install. 
Please note that a custom install requires that you verify yourself that the components 
to be installed are compatible with the components that are not updated.

The Kernal ROM version is prefixed with "R" if it's an official release, and
"PR" if it's a pre-release. If it's a custom build, the word "Custom" is displayed".

The respective version numbers of the VERA and SMC are displayed as such (major.minor.patch, for instance
47.0.0).

## Pre-upgrade Checks

The pre-upgrade checks described below are run before the update begins. No changes are
made to the installed components during this stage. The purpose of the pre-upgrade checks 
is to verify that it will actually be possible to install all the selected components.

It is not possible to proceed with the update if any of the pre-upgrade checks fail. In such
case you need to fix the issue, or unselect the component that fails the pre-upgrade
checks.

### Kernal ROM

| Issue               | Solution                                                                          | 
|---------------------|-----------------------------------------------------------------------------------| 
| Write-enabled error | Ensure JP1 on the main board is closed, and try again.                            |
| Chip ID error       | Ensure the ROM chip is an SST39SF040, and that it's properly seated in the socket.|
| Load error          | Try to restart the program.                                                       |


### SMC

| Issue                     | Solution                                                                      |
|---------------------------|-------------------------------------------------------------------------------|
| Firmware unsupported      | Use external programmer if the firmware is less than 43.0.0.                  |
| Write-enabled error       | Use external programmer to set the SMC fuses.                                 |
| Bootloader not found      | Use external programmer to install a bootloader.                              |
| Bootloader unsupported    | Check if there is a newer version of X16UPGRADE that suppports the bootloader.|
| Bad bootloader, confirmed | Prepare to follow the update procedure for the bad bootloader.                |
| Bad bootloader, high risk | Prepare to follow the update procedure for the bad bootloader.                |
| Bad bootloader, low risk  | Prepare to follow the update procedure for the bad bootloader.                |
| Bootloader, unknown       | An unknown version of the bootloader is installed, proceed at your own risk.  |
| Load error                | Try to restart the program.                                                   |

Updating the SMC with an external programmer requires that the SMC chip is removed from
the main board. Instructions on how to program the SMC are found [here](https://github.com/X16Community/x16-smc/blob/main/doc/update-guide.md#external-programmer).

The SMC fuses control, amongst other, the microcontroller speed, and whether or not the SMC
is write-enabled. The fuses can only be programmed using an external programmer. Information about 
programming the fuses is found [here](https://github.com/X16Community/x16-smc-bootloader) under the Fuse Settings header.

If there is a risk that you have the "bad" SMC bootloader you need to prepare to manually
reset the SMC at the end of the update process. If you just turn off the computer, the SMC
will most likely be corrupted. Instructions on the update procedure with the bad bootloader
are found [here](https://github.com/X16Community/x16-smc/blob/main/doc/update-with-bad-bootloader-v2.md).


### VERA

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Load error                | Try to restart the program.                                    |
| Close JP1                 | When prompted, JP1 on the VERA must be closed                  |


## Install Updates

### VERA

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Close JP1                 | JP1 on the VERA was closed during the pre-upgrade checks. Ensure it is still closed. |
| Verify error              | Follow procedure described below. |
| Open JP 1                 | When prompted, open JP1 again. |

The program exits if the VERA update fails. Next time you reset or power cycle the system, the
VERA will configure the FPGA from the likely corrupted firmware stored in flash memory. The
system will most likely not work.

Try to install the VERA firmware again without resetting or power cycling the system.

Instructions on how to recover a VERA board using an external programmer are found [here](https://github.com/X16Community/x16-docs/blob/master/X16%20Reference%20-%20Appendix%20B%20-%20VERA%20Recovery.md#appendix-b-vera-firmware-recovery).


### ROM

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Verify error              | Follow the procedure described below.                          |

If the Kernal ROM update fails you must recover it with an external programmer. To do that
you must first remove the ROM chip from its socket, and place it in the programmer. On top of the
ROM chip you find the model name "SST39SF040". Use the programmer's software to write the 
rom.bin file to the chip. The rom.bin file is found on the [Github release page](https://github.com/X16Community/x16-rom/releases)


### SMC

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Press Power+Reset         | To start the bootloader update procedure you must momentarily press Power+Reset at the same time before the 20 second countdown finishes. |
| Verify error              | Follow the procedure described below. |

If the SMC update fails, you can do a recovery update if you have bootloader version 3. Instructions
on that procedure are found [here](https://github.com/X16Community/x16-smc/blob/main/doc/update-guide.md#in-system-recovery-with-bootloader-v3).

If that is not an option, you need to recover the SMC using an external programmer, as described [here](https://github.com/X16Community/x16-smc/blob/main/doc/update-guide.md#external-programmer).

