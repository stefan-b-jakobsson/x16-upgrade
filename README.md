# X16-UPGRADE

**WARNING**: This tool is under development, and is highly experimental.

## Introduction

The purpose of this program is to update the three major firmware components of the Command X16:

- Kernal ROM
- VERA
- SMC

It is recommended that you have access to this README file while doing an update. The README
file is especially valuable, as it gives more in-depth information about possible warnings
and errors that occur when running the program.

## User Interface

The program's user interface consists of multiple screens, one for each step of the
upgrade process.

You interact with the program through widgets, such as buttons and checkboxes.

The active widget is highlighted. You move between widgets with Tab/right cursor and Shift+Tab/left cursor,
and Right Cursor. The active widget's action is performed when you press Return.

## Choose Package File

At the first screen, the user is prompted to enter the name of a package file.

A package file contains all three firmware components.

Creators of package files are responsible to make sure that the firmware components in a package 
are compatible with each other.

More information on the package file format and how you can make your own packages are found here.

## Package and Update Summary

Some general information about the selected package file is displayed at the top of the
second screen:

- A short description of the package, for instance "R48 Official Relase"
- Name of the creator of the package
- Date and time when the package was created (UTC)

Then follows a list of firmware components to be installed including the new
version numbers.

The program by default selects all three components to be installed. You may
press the Custom button to change that. A custom install requires that you
verify yourself that the components to be installed are compatible with the components that
are not updated.

The Kernal ROM version is prefixed with "R" if it's an official release, and
"PR" if it's a pre-release. If it's a custom build, the word "Custom" is displayed".

The VERA and the SMC use their version numbers (major.minor.patch).

## Pre-upgrade Checks

The pre-upgrade checks described below are run before beginning to install
any new firmware. The purpose of the pre-upgrade checks is to verify that it
will be possible to install all the selected components.

It is not possible to proceed with the update if any of the pre-upgrade checks fail.

### Kernal ROM

| Issue               | Solution                                                       | 
|---------------------|----------------------------------------------------------------|
| Write-enabled error | Ensure JP1 on the main board is closed, and try again.         |
| Chip ID error       | Ensure the ROM chip is an SST39SF040.                          |
| Load error          | Try to restart the program.                                    |


### SMC

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Firmware unsupported      | Use external programmer if the firmware is less than 43.0.0.   |
| Write-enabled error       | Use external programmer to set the SMC fuses                   |
| Bootloader not found      | Use external programmer to install a bootloader.               |
| Bootloader unsupported    | Check if there is a newer version of X16UPGRADE.               |
| Bad bootloader, confirmed | Prepare to follow the update procedure for the bad bootloader. |
| Bad bootloader, high risk | Prepare to follow the update procedure for the bad bootloader. |
| Bad bootloader, low risk  | Prepare to follow the update procedure for the bad bootloader. |
| Bootloader, unkown        | An unknown version of the bootloader is installed, proceed at your own risk. |
| Load error                | Try to restart the program.                                    |

Updating the SMC with an external programmer requires that the SMC chip is removed from
the main board. Instructions on how to program the SMC is found here.

The SMC fuses control, amongst other, the microcontroller speed, and whether or not the SMC
is write-enabled. The fuses can only be programmed using an external programmer.

If there is a risk that you have the "bad" SMC bootloader you need to prepare to manually
reset the SMC at the end of the update process. If you just turn off the computer, the SMC
will most likely be corrupted. Instructions on the update procedure with the bad bootloader
is found here.


### VERA

| Issue                     | Solution                                                       |
|---------------------------|----------------------------------------------------------------|
| Load error                | Try to restart the program.                                    |
| Close JP1                 | When prompted, JP1 on the VERA must be closed                  |
