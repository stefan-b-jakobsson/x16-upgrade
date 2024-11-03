; Header
str_header: 
    .byt 147, "X16 UPGRADE", 13, 13, 0

str_ok:
    .byt "OK", 0

str_ok_cr:
    .byt "OK", 13, 0

str_pass:
    .byt "PASS", 0

str_pass_cr:
    .byt "PASS", 13, 0

str_fail:
    .byt "FAIL", 0

str_fail_cr:
    .byt "FAIL", 13, 0

str_warn:
    .byt "WARN", 0

str_warn_cr:
    .byt "WARN", 13, 0


; Screen 1: Welcome
str1_welcome: 
    .byt "Welcome to the X16 system upgrade tool!",13,13
    .byt "Before you proceed, take time to read the enclosed README file.",13,13,0

str1_file_prompt:
    .byt "Enter package file name:", 13, 0

str1_header_load_err:
    .byt 13, "FILE LOAD ERROR", 0

str1_header_magic_err:
    .byt 13,"UNRECOGNIZED FILE HEADER FORMAT ERROR", 0

str1_header_crc_err:
    .byt 13,"HEADER CRC-16 MISMATCH ERROR",0


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

str4_vera:
    .byt "VERA", 13, 0

str4_vera_nys:
    .byt "ERR:Not yet supported", 13,0

str4_smc:
    .byt "SMC", 13, 0

str4_smc_firmware_unsupported:
    .byt "ERR:Unsupported firmware", 13, 0

str4_smc_no_bootloader:
    .byt "ERR:No bootloader", 13,0

str4_smc_bootloader_unsupported:
    .byt "ERR:Unsupported bootloader", 13, 0

str4_smc_write_protected:
    .byt "ERR:Write-protected", 13,0

str4_smc_progress:
    .byt "Load... 0%",0

str4_smc_file_load_error:
    .byt 13, "ERR:File error", 13, 0

str4_smc_bad_bootloader1:
    .byt "WRN:Bad bootloader (low risk)",13,0

str4_smc_bad_bootloader2:
    .byt "WRN:Bad bootloader (high risk)",13, 0

str4_smc_bad_bootloader3:
    .byt "WRN:Bad bootloader (confirmed)",13, 0

str4_smc_unknown_bootloader:
    .byt "WRN:Unknown bootloader", 13, 0

str4_btn_back:
    .byt "<Back>"
str4_btn_back_end:

str4_btn_install:
    .byt "<Install>"
str4_btn_install_end:


; Screen5: Install

str5_header:
    .byt "Installing upgrade", 13, 13, 0

str5_rom_header:
    .byt "ROM",13, 0

str5_rom_erase:
    .byt "Erasing...", 0

str5_rom_write:
    .byt "Writing 0%", 0

str5_rom_verify:
    .byt "Verifying 0%", 0


