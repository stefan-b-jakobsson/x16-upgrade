; Copyright (c) 2024-2025, Stefan Jakobsson

; Redistribution and use in source and binary forms, with or without 
; modification, are permitted provided that the following conditions are met:

; (1) Redistributions of source code must retain the above copyright notice, 
;     this list of conditions and the following disclaimer. 
;
; (2) Redistributions in binary form must reproduce the above copyright notice,
;     this list of conditions and the following disclaimer in the documentation
;     and/or other materials provided with the distribution.

; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.

; Header
str_header: 
    .byt 147, "X16 UPGRADE ", 0

str_ok_cr:
    .byt "OK", 13, 0

str_fail_cr:
    .byt "FAIL", 13, 0

str_warn_cr:
    .byt "WARN", 13, 0

str_f1_help:
    .byt "Press any key, F1 for help", 13, 0

str_exit_j1:
    .byt 13, 13, "It is advised that you remove the jumper at position J1 on the main board after turning off the computer. That will protect the ROM from unintentional change.", 13, 13, 0

; Program Main: These strings must be stored in this order
str_cwd_readerr:
    .byt "error: could not read current path", 13, 0

str_cwd_overflow:
    .byt "error: current path too long", 13, 0

; Screen 1: Welcome
str1_welcome: 
    .byt "Welcome to the X16 system upgrade tool!",13,13
    .byt "Before you proceed, take time to read the enclosed README file.",13,13
    .byt "Enter name of package file that contains the new firmware.",13,13,0

str1_file_prompt:
    .byt "File name: ", 0

str1_header_load_err:
    .byt 13, "FILE LOAD ERROR", 0

str1_header_magic_err:
    .byt 13,"UNRECOGNIZED FILE HEADER FORMAT ERROR", 0

str1_header_crc_err:
    .byt 13,"HEADER CRC-16 MISMATCH ERROR",0

str1_header_unsupported:
    .byt 13, "UNSUPPORTED PACKAGE FILE VERSION", 0


; Screen2: Upgrade summary

str2_summary:
    .byt "PACKAGE INFORMATION", 13, 13, 0

str2_pgk_by: 
    .byt 13, "By ", 0

str2_pkg_utc:
    .byt " UTC",13,13,13, 0

str2_pkg_components:
    .byt "COMPONENTS TO BE INSTALLED", 13, 13, 0

str2_rom:
    .byt "ROM.: ", 0

str2_custom_rom:
    .byt "Custom build",0

str2_vera:
    .byt "VERA: ", 0

str2_smc:
    .byt "SMC.: ", 0

str2_skip:
    .byt "-", 0

str2_btn_exit:
    .byt "<Exit>"
str2_btn_exit_end:

str2_btn_custom:
    .byt "<Custom>"
str2_btn_custom_end:

str2_btn_next:
    .byt "<Next>"
str2_btn_next_end:


; Screen3: Custom options

str3_info:
    .byt "Select components to upgrade", 13, 13
    .byt "WARNING: Verify firmware compatibility manually.", 13, 13, 0

str3_rom:
    .byt "ROM", 13, 0

str3_vera:
    .byt "VERA", 13, 0

str3_smc:
    .byt "SMC", 13, 0

str3_btn_cancel:
    .byt "<Cancel>"
str3_btn_cancel_end:

str3_btn_ok:
    .byt "<OK>"
str3_btn_ok_end:


; Screen4: Pre-upgrade checks

str4_header:
    .byt "PRE-UPGRADE CHECKS",13, 13, 0

str4_rom:
    .byt "ROM", 13, 0

str4_rom_write_enabled:
    .byt "Write-enabled...", 0

str4_rom_chip_id:
    .byt "Chip ID...", 0

str4_rom_progress:
    .byt "Loading... 0%", 0

str4_smc:
    .byt "SMC", 13, 0

str4_smc_firmware:
    .byt "Firmware...", 0

str4_smc_firmware_unsupported:
    .byt "Unsupported version", 13, 0

str4_smc_write_enabled:
    .byt "Write-enabled...", 0

str4_smc_bootloader:
    .byt "Bootloader...",0

str4_smc_no_bootloader:
    .byt "No bootloader", 13,0

str4_smc_bootloader_unsupported:
    .byt "Unsupported version", 13, 0

str4_smc_bad_bootloader1:
    .byt "Bad v2, low risk",13
    .byt "Press F1 for help, ESC to abort, any other key to continue", 13, 0

str4_smc_bad_bootloader2:
    .byt "Bad v2, high risk",13
    .byt "Press F1 for help, ESC to abort, any other key to continue", 13, 0

str4_smc_bad_bootloader3:
    .byt "Bad v2, confirmed",13
    .byt "Press F1 for help, ESC to abort, any other key to continue", 13, 0

str4_smc_unknown_bootloader:
    .byt "Unknown version", 13, 0

str4_smc_loading:
    .byt "Loading... 0%",0

str4_vera:
    .byt "VERA", 13, 0

str4_vera_loading:
    .byt "Loading... 0%", 0

str4_vera_close_jp1:
    .byt "Close JP1 on the VERA board", 13
    .byt "Press any key, F1 for help, ESC to abort", 13, 0

str4_vera_chip_ok:
    .byt "Chip ID... OK", 13, 0

str4_btn_back:
    .byt "<Back>"
str4_btn_back_end:

str4_btn_next:
    .byt "<Next>"
str4_btn_next_end:


; Screen5: Install

str5_header:
    .byt "INSTALLING UPDATES", 13, 13, 0

str5_vera_header:
    .byt "VERA", 13, 0

str5_vera_close_jp1:
    .byt "Close JP1 on the VERA board", 13
    .byt "Press any key, F1 for help, ESC to abort", 13, 0

str5_vera_erase:
    .byt "Erasing...", 0

str5_vera_write:
    .byt "Writing... 0%", 0

str5_vera_verify:
    .byt "Verifying... 0%", 0

str5_vera_open_jp1:
    .byt "Open JP1 on the VERA board", 13, 0

str5_rom_header:
    .byt "ROM", 13, 0

str5_rom_write_protected:
    .byt "Chip is write protected", 13
    .byt "Reinstall jumper at position J1", 13
    .byt "Press any key to continue", 13, 0

str5_rom_erase:
    .byt "Erasing...", 0

str5_rom_write:
    .byt "Writing... 0%", 0

str5_rom_verify:
    .byt "Verifying... 0%", 0

str5_smc_header:
    .byt "SMC",13,0

str5_smc_wait_key:
    .byt "Press any key...",13,0

str5_smc_start_bootloader:
    .byt "Momentarily press Power+Reset", 13
    .byt "at the same time... ", 0

str5_smc_write:
    .byt "Writing... 0%", 0

str5_smc_verify:
    .byt "Verifying... 0%", 0

str5_smc_update_failed:
    .byt "Update failed. Last err no: ", 0

str5_smc_reset:
    .byt "System shutdown... ", 0

str5_smc_bootloader_v1:
    .byt "Bootloader v1 detected. Disconnect "
    .byt "the computer from mains power for at least 20 "
    .byt "seconds to finish the update.", 0

str5_smc_bad_bootloader:
    .byt 13, 13, "Corrupted (bad) bootloader v2 detected. ", 13
    .byt "DO NOT disconnect the computer from power. ", 13
    .byt "Stand by for help... ", 0


str5_reset_msg:
    .byt "System reset... ", 0

str5_vera_failed:
    .byt "The VERA update failed. The video output will probably not work if you reset or power cycle the computer. Try to update the VERA again.", 13, 0

str5_rom_failed:
    .byt "The ROM update failed. The chip must probably be restored with an external programmer.", 0

str5_smc_failed:
    .byt "The SMC update failed. If you have bootloader v3, it might be possible to do a recovery update of the SMC. Otherwise it must probably be restored with an external programmer.", 0


; Screen 6: Warning

str6_disclaimer:
    .byt "DISCLAIMER", 13, 13
    .byt "Interrupting or failing to complete the upgrade may render your X16 computer inoperable, "
    .byt "requiring external programming tools to recover it. The developers of this software are not liable " 
    .byt "for any damage that may result from its use.", 13, 13
    .byt "For additional information, please refer to the enclosed README file.", 13, 13
    .byt "Press <Exit> to quit the program or <Install> to begin the upgrade.", 13, 13, 0

str6_btn_exit:
    .byt "<Exit>"
str6_btn_exit_end:

str6_btn_install:
    .byt "<Install>"
str6_btn_install_end:


; Screen 7: Remove JP1

str7_header:
    .byt "UNINSTALL VERA JP1", 13, 13, 0

str7_jp1:
    .byt "Remove jumper JP1 on the VERA board to restore normal operation of the VERA and the SD card.", 13, 13, 0

str7_btn_ok:
    .byt "<OK>"
str7_btn_ok_end:

str7_btn_help:
    .byt "<Help>"
str7_btn_help_end:

; Help screen

str_hlp_j1_close:
    .byt "rom chip write-protected", 13
    .byt $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, 13, 13
    .byt "the rom chip is currently write-", 13
    .byt "protected.", 13, 13
    .byt "to write-enable the rom chip,", 13
    .byt "place a jumper over the pins at", 13
    .byt "position ", 34, "j1", 34, ".", 13, 13
    .byt "it is advised that you unplug the", 13
    .byt "computer from mains power before", 13
    .byt "installing the jumper.", 13, 13, 13
    .byt "press any key", 0

str_hlp_jp1_close:
    .byt "vera write-protected", 13
    .byt $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, 13, 13
    .byt "connect the pins at position", 13
    .byt 34, "jp1", 34, " to write-enable the vera.", 13
    .byt "this must be done without turning", 13
    .byt "off the computer or this program.", 13, 13
    .byt "avoid touching any of the", 13
    .byt "components on the vera board.", 13, 13, 13
    .byt "press any key", 0

str_hlp_jp1_open:
    .byt "restore vera to normal operation", 13
    .byt $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, 13, 13
    .byt "the vera and sd card do not work", 13
    .byt "when the pins at position ", 34, "jp1", 34, 13
    .byt "are connected.", 13, 13
    .byt "remove the jumper placed there",13
    .byt "without turning off the computer",13
    .byt "or this program.", 13, 13
    .byt "avoid touching any of the", 13
    .byt "components on the vera board.", 13, 13, 13
    .byt "press any key", 0

str_hlp_bad_bootloader_header:
    .byt "bad bootloader", 13
    .byt $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, $63, 0

str_hlp_bad_bootloader_warning_1:
    .byt "there is a low risk that the", 13
    .byt "system microcontroller (smc) has", 13
    .byt "a corrupted bootloader.", 13, 13, 0

str_hlp_bad_bootloader_warning_2:
    .byt "it is very likely that the system", 13
    .byt "microcontroller (smc) has a", 13
    .byt "corrupted bootloader.", 13, 13, 0

str_hlp_bad_bootloader_warning_3:
    .byt "the system microcontroller (smc)" ,13
    .byt "has a corrupted bootloader.", 13, 13, 0

str_hlp_bad_bootloader_warning:
    .byt "you must be prepared to reset the", 13
    .byt "smc after the update by", 13
    .byt "connecting smc pin #10 to ground.", 13
    .byt "that requires good access to the", 13
    .byt "board and a suitable piece of", 13
    .byt "wire.", 13, 13
    
    .byt "if you are not sure how to", 13
    .byt "proceed, it is recommended that", 13
    .byt "you do not update the smc.", 13, 13, 13
    
    .byt "press any key", 0
    
str_hlp_bad_bootloader:    
    .byt "the system microcontroller (smc)", 13
    .byt "has a corrupted bootloader.", 13, 13
    
    .byt "**do not** unplug the computer", 13
    .byt "from power, as it will brick the", 13
    .byt "system.", 13, 13
    
    .byt "to finish the update you need to", 13
    .byt "connect smc pin #10 to ground", 13
    .byt "using a piece of wire.", 13, 13

    .byt "use the metal plating around", 13
    .byt "a mounting hole as ground point.", 13, 13
    
    .byt "avoid touching any other part of", 13
    .byt "the board.", 13, 13
    
    .byt "the connection should be just a", 13
    .byt "short tap. the computer turns", 13
    .byt "off immediately, and can then be", 13
    .byt "restarted by pressing the power", 13
    .byt "button.", 0
