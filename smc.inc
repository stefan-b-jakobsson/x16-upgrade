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


; SMC update status values
SMC_STATUS_UNSELECTED = 0
SMC_STATUS_SELECTED = 1
SMC_STATUS_PRECHECK_FAIL = 2
SMC_STATUS_PRECHECK_PASS = 3
SMC_STATUS_LOAD_FAIL = 4
SMC_STATUS_LOAD_PASS = 5
SMC_STATUS_UPDATE_FAIL = 6
SMC_STATUS_UPDATE_PASS = 7
SMC_STATUS_VERIFY_FAIL = 8
SMC_STATUS_VERIFY_PASS = 9

; Supported SMC firmware and bootloader versions
SMC_MIN_VERSION = 43
SMC_BOOTLOADER_MIN = 1
SMC_BOOTLOADER_MAX = 3

; I2C defines
SMC_ADDR = $42

SMC_GET_KEY = $07
SMC_EFUSE = $0c
SMC_VER_MAJOR = $30
SMC_VER_MINOR = $31
SMC_VER_PATCH = $32
SMC_BOOTLOADER_VER = $8e
SMC_BOOTLOADER_START = $8f
SMC_SET_PAGE = $90
SMC_READ_FLASH = $91

SMC_BOOTLOADER_TRANSMIT = $80
SMC_BOOTLOADER_COMMIT = $81
SMC_BOOTLOADER_REBOOT = $82
SMC_BOOTLADER_GET_VER = $83
SMC_BOOTLOADER_REWIND = $84
SMC_BOOTLOADER_READ_FLASH = $85

SMC_MAX_ATTEMPTS = 10

; RAM buffer start address, buffer needs to hold at least $1e00 bytes of data
SMC_RAM_BUF = $6100

;******************************************************************************
; Function name.......: smc_init
; Purpose.............: Initializes SMC functions. 
;                       The SMC is selected for update by default. The function 
;                       fetches current SMC version and SMC bootloader version, 
;                       and stores these values to be used during the update.
; Input...............: Nothing
; Returns.............: Nothing
.proc smc_init
    ; Mark SMC for update (default)
    lda #SMC_STATUS_SELECTED
    sta smc_status

    ; Get current SMC firmware version
    jsr smc_get_firmware_version
    sta smc_cur_version
    sty smc_cur_version+1
    stx smc_cur_version+2

    ; Get SMC bootloader version
    jsr smc_get_bootloader_version
    sta smc_bootloader
    rts
.endproc

;******************************************************************************
; Function name.......: smc_run_prechecks
; Purpose.............: Runs the following SMC pre-upgrade checks:
;                       1. SMC firmware major version <= SMC_MIN_VERSION
;                       2. SMC write is enabled, if SMC firmware >= 47.2.4
;                       3. Bootloader is installed
;                       4. SMC_BOOTLOADER_MIN <= Bootloader version <= SMC_BOOTLOADER_MAX
;                       5. Bad v2 bootloader (CRC-16 confirmed), run if SMC firmware >= 47.2.3
;                       6. Bad v2 bootloader (high/low risk), run if SMC firmware < 47.2.3
;                       7. CRC-16 check of SMC firmware loaded from package file
; Input...............: Nothing
; Returns.............: C=1 on error
.proc smc_run_prechecks
    ; Abort if SMC not selected for update
    lda smc_status
    cmp #SMC_STATUS_UNSELECTED
    bne :+
    clc
    rts

    ; Select SMC progress counter
:   lda #PROGRESS_SMC
    jsr progress_select

    ; Print title
    print 0, str4_smc

t1:
    ; Test 1: Firmware version (>= SMC_MIN_VERSION)
    print 1, str4_smc_firmware
    lda smc_cur_version
    cmp #SMC_MIN_VERSION
    bcc :+

    ; t1: OK
    print 1, str_ok_cr
    bra t2

:   ; t1: Fail
    print 1, str_fail_cr
    print 2, str4_smc_firmware_unsupported
    jmp fail

t2:
    ; Test 2: SMC is write-enabled, available from SMC firmware 47.2.4
    lda #47
    ldy #2
    ldx #4
    jsr smc_compare_version_number
    bcc t3

    ; SMC firmware is >= 47.2.4, continue
    print 1, str4_smc_write_enabled

    ; Read SMC extended fuse (efuse)
    ldx #SMC_ADDR
    ldy #SMC_EFUSE
    jsr I2C_READ_BYTE
    lsr
    bcs :+ ; efuse bit 0 is 1 => SMC not write-enabled

    ; t2: OK
    print 1, str_ok_cr
    bra t3

    ; t2: Fail
:   print 1, str_fail_cr
    jmp fail

t3: 
    ; Test 3: Bootloader installed
    print 1, str4_smc_bootloader

    lda smc_bootloader
    bne t4
    
    ; Test 3, fail
    print 1, str_fail_cr
    print 2, str4_smc_no_bootloader
    jmp fail

t4:
    ; Test 4: Bootloader supported, >= SMC_BOOTLOADER_MIN and <= SMC_BOOTLOADER_MAX
    lda smc_bootloader
    cmp #SMC_BOOTLOADER_MIN
    bcc :+
    cmp #SMC_BOOTLOADER_MAX+1
    bcc t5

    ; t4: Fail
    print 1, str_fail_cr
    print 2, str4_smc_bootloader_unsupported
    jmp fail

t5: 
    ; Test 5: Bad bootloader (confirmed), possible to check from SMC firmware version 47.2.3
    lda #47
    ldy #2
    ldx #3
    jsr smc_compare_version_number
    bcc t6

:   ; SMC firware is >= 47.2.3, continue
    jsr smc_get_bootloader_crc
    cmp #0  ; Known good bootloader
    bne :+
    print 1, str_ok_cr
    jmp t7

:   cmp #1  ; Known bad bootloader
    bne :+
    print 1, str_warn_cr
    print 2, str4_smc_bad_bootloader3
    jsr util_wait_keypress
    jsr help_show
    lda #3
    jsr help_bad_bootloader
    jmp t7

:   cmp #2  ; Unknown bootloader
    beq :+
    jmp t7

:   print 1, str_warn_cr
    print 2, str4_smc_unknown_bootloader
    bra t7

t6: 
    ; Test 6: Bad bootloader v2 (high risk or low risk)
    lda smc_bootloader
    cmp #2
    beq :+

    ; Not bootloader v2
    print 1, str_ok_cr
    bra t7

:   ; Bootloader v2 + SMC firmware version 45.1.0 => High risk of bad bootloader, otherwise low risk
    lda #45
    ldy #1
    ldx #0
    jsr smc_compare_version_number
    bne t6l

    ; High risk
t6h:
    print 1, str_warn_cr
    print 2, str4_smc_bad_bootloader2
:   jsr util_wait_keypress
    
    cmp #$70 ; f1
    bne :+
    jsr help_show
    lda #2
    jsr help_bad_bootloader
    bra :-

:   cmp #$6e ; ESC key code
    beq fail
    bra t7
 
    ; Low risk
t6l:
    print 1, str_warn_cr
    print 2,str4_smc_bad_bootloader1
:   jsr util_wait_keypress
    
    cmp #$70 ; f1
    bne :+
    jsr help_show
    lda #1
    jsr help_bad_bootloader
    bra :-

:   cmp #$6e ; ESC key code
    beq fail

t7:
    ; Test 7: Load SMC firmware into RAM buffer
    print 1, str4_smc_loading
    jsr smc_load
    bcc :+

    print 1, str_fail_cr
    bra fail

:   print 1, str_ok_cr
    
exit:
    ; Return without error
    clc
    lda #SMC_STATUS_LOAD_PASS
    sta smc_status
    rts

fail:
    ; Return with error code
    sec
    lda #SMC_STATUS_PRECHECK_FAIL
    sta smc_status
    rts
.endproc

;******************************************************************************
; Function name.......: smc_update
; Purpose.............: Updates the SMC firmware
; Prerequisite........: Successfully call smc_run_prechecks
; Input...............: Nothing
; Returns.............: C=1 on error
.proc smc_update
    ; Abort if status is not SMC_STATUS_LOAD_PASS
    lda smc_status
    cmp #SMC_STATUS_LOAD_PASS
    beq :+
    clc
    rts

:   ; Disable interrupts; interrupts can't be enabled after this point
    sei
    
    ; Print header
    print 0, str5_smc_header

wait_key:
    ; Wait for key
    print 1, str5_smc_wait_key
    jsr util_wait_keypress

    ; Start bootloader
    print 1, str5_smc_start_bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_START
    lda #$31 ; Magic value required by this SMC command
    jsr I2C_WRITE_BYTE

    ; Wait max 20 seconds
    lda #20
    ldx #<callback
    ldy #>callback
    jsr util_countdown

    ; Check if bootloader was started, or go again
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_VER
    jsr I2C_READ_BYTE
    cmp #0 ; When bootloader is started, reading this command offset returns 0
    bne wait_key

    ; Setup variables
    stz count               ; Clear byte count
    stz count+1

    stz checksum            ; Clear checksum

    lda #SMC_MAX_ATTEMPTS   ; Set max attempts
    sta attempts

    ; Pointer to end of firmware, i.e. SMC_RAM_BUF + header_smc_size
    clc
    lda header_smc_size
    adc #<SMC_RAM_BUF
    sta smc_src_addr
    lda header_smc_size+1
    adc #>SMC_RAM_BUF
    sta smc_src_addr+1

    ; Pad end of firmware to page boundary (page size 64 bytes)
    lda header_smc_size
    eor #$ff
    and #$3f
    inc
    tay

    clc
    adc header_smc_size
    sta size
    lda header_smc_size+1
    adc #0
    sta size+1

    lda #$ff
:   dey
    cpy #$ff
    beq :+
    sta (smc_src_addr),y
    bra :-

    ; Set pointer to start of firmware
:   lda #<SMC_RAM_BUF
    sta smc_src_addr
    lda #>SMC_RAM_BUF
    sta smc_src_addr+1

    ; Select progress counter
    lda #PROGRESS_SMC
    jsr progress_select

    ; Print write message
    print 1, str5_smc_write

loop:
    ; At end of stream?
    lda count
    cmp size
    bne send
    lda count+1
    cmp size+1
    beq complete

send:
    ; Transmit one byte to bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_TRANSMIT
    lda (smc_src_addr)
    jsr I2C_WRITE_BYTE

    ; Update checksum
    clc
    lda checksum
    adc (smc_src_addr)
    sta checksum

commit:
    ; At end of package (8 data bytes)?
    lda count
    and #7
    cmp #7
    bne next

    ; Calculate and send checksum; checksum is the 2's complement of the sum of the previous 8 bytes in the packet
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_TRANSMIT
    lda checksum
    eor #$ff
    inc
    jsr I2C_WRITE_BYTE

    ; Clear checksum
    stz checksum

    ; Send commit packet command
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_COMMIT
    jsr I2C_READ_BYTE
    cmp #1 ; Return value 1 = OK, other return values indicate an error
    beq next

    ; Commit failed, abort update if reaching max attempts
    dec attempts
    beq fail
    
    ; Reset byte count
    sec
    lda count
    sbc #7
    sta count
    lda count+1
    sbc #0
    sta count+1
    bra loop

next:
    ; Reset attempts
    lda #SMC_MAX_ATTEMPTS
    sta attempts

    ; Increment byte count
    inc count
    bne :+
    inc count+1

:   ; Increment source address
    inc smc_src_addr
    bne loop
    inc smc_src_addr+1
    
    ; Update progress counter
    ldx count
    ldy count+1
    lda #0
    jsr progress_update
    
    jmp loop

complete:
    ; Update progress counter
    ldx count
    ldy count+1
    lda #0
    jsr progress_update

    ; Print OK
    print 1, str_ok_cr

    ; Set status
    lda #SMC_STATUS_UPDATE_PASS
    sta smc_status
    clc
    rts

fail:
    ; Print fail message
    pha
    print 1, str5_smc_update_failed
    pla

    ; Print last error code
    jsr util_print_num
    lda #13
    jsr CHROUT

    ; Set status
    lda #SMC_STATUS_UPDATE_FAIL
    sta smc_status
    sec
    rts

callback:
    ; Check if bootloader is active
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_VER
    jsr I2C_READ_BYTE
    cmp #0 ; Reading this command offset returns 0 if bootloader is active
    bne :+

    ; End countdown
    print 1, str_ok_cr
    stz util_countdown::counter

:   rts

count:
    .res 2
size:
    .res 2
checksum:
    .res 1
attempts:
    .res 1
.endproc

;******************************************************************************
; Function name.......: smc_verify
; Purpose.............: Verifies the SMC firmware to the RAM buffer
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 if verify failed
.proc smc_verify
    ; Abort if state is not SMC_STATUS_UPDATE_PASS
    lda smc_status
    cmp #SMC_STATUS_UPDATE_PASS
    beq :+
    clc
    rts

:   ; Abort if bootloader version < 3, when support for verifying an update was added
    lda smc_bootloader
    cmp #3
    bcs :+
    clc
    rts

:   ; Print verifying message
    print 1, str5_smc_verify

    ; Select progress counter
    lda #PROGRESS_SMC
    jsr progress_select

    ; Clear byte counter
    stz count
    stz count+1

    ; Set pointer to start of new SMC firmware in RAM
    lda #<SMC_RAM_BUF
    sta smc_src_addr
    lda #>SMC_RAM_BUF
    sta smc_src_addr+1

    ; Rewind target address
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_REWIND
    jsr I2C_WRITE_BYTE

loop:
    ; Read byte from SMC flash
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_READ_FLASH
    jsr I2C_READ_BYTE

    ; Check if first page, special action needed.
    ; Bootloader v3 moves the firmware's Reset vector to EE_RDY, 
    ; and points the Reset vector to start of the bootloader.
    ; Byte addresses $0001-0002 and $0012-0013 will not match
    ; the content of the RAM buffer.
    ldx count+1
    bne compare
    ldx count

    ; Byte address $0000? Should always be $ff
    bne :+
    cmp #$ff
    bne failed
    bra next

    ; Byte address $0001? Should always be $ce
:   cpx #1
    bne :+
    cmp #$ce
    bne failed
    bra next

    ; Byte address $0012? Should be Reset vector minus 9, low byte
:   cpx #$12
    bne :+
    tay

    sec
    lda SMC_RAM_BUF
    sbc #9
    sta ee_rdy
    lda SMC_RAM_BUF+1
    sbc #0
    and #%11001111
    ora #%11000000
    sta ee_rdy+1

    cpy ee_rdy
    bne failed
    bra next

    ; Byte address $0013? Should be Reset vector minus 9, high byte
:   cpx #$13
    bne compare
    cmp ee_rdy+1
    bne failed
    bra next

compare:
    ; Compare against RAM buffer
    cmp (smc_src_addr)
    bne failed

    ; Next byte
next:
    inc smc_src_addr
    bne :+
    inc smc_src_addr+1

:   ; Increment counter
    inc count
    bne done
    inc count+1
    jsr progress

done:
    ; Check if all bytes have been verified
    lda count
    cmp header_smc_size
    bne loop
    lda count+1
    cmp header_smc_size+1
    beq exit
    jmp loop

exit:
    ; Update progress counter
    jsr progress
    
    ; Return without errors
    lda #SMC_STATUS_VERIFY_PASS
    sta smc_status
    print 1, str_ok_cr
    clc
    rts

failed:
    ; Return with error
    lda #SMC_STATUS_VERIFY_FAIL
    sta smc_status
    print 1, str_fail_cr
    sec
    rts

progress:
    ; Updates progress counter
    ldx count
    ldy count+1
    lda #0 ; MSB always 0 for the SMC
    jmp progress_update

count: .res 2
ee_rdy: .res 2
.endproc

;******************************************************************************
; Function name.......: smc_reboot
; Purpose.............: Resets the SMC, which will reboot the system
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 if verify failed
.proc smc_reboot
    ; Abort if SMC was not selected for update
    lda smc_status
    cmp #SMC_STATUS_UNSELECTED
    bne :+
    clc
    rts

:   ; Check bootloader version
    lda smc_bootloader
    cmp #1
    beq bootloader_v1

    ; Advise user to remove J1 jumper
    jsr rom_is_write_enabled
    bcs :+
    print 1, str_exit_j1

    ; Wait 20 seconds
:   print 1, str5_smc_reset
    lda #20
    ldx #0
    ldy #0  
    jsr util_countdown

    ; Send reboot command to bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_REBOOT
    jsr I2C_WRITE_BYTE

    ; Wait 2 seconds, the computer should turn off before the end of the delay
    jsr util_delay_1s
    jsr util_delay_1s

    ; We're still alive, consequently the bad bootloader v2
    print 1, str5_smc_bad_bootloader
    lda #9
    ldx #0
    ldy #0
    jsr util_countdown
    jsr help_show
    lda #0
    jmp help_bad_bootloader ; Does not return

bootloader_v1:
    ; Send reboot command to bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_REBOOT
    jsr I2C_WRITE_BYTE

    ; Print message and wait until computer is power cycled
    print 1, str5_smc_bootloader_v1
:   bra :-
.endproc

;******************************************************************************
; Function name.......: smc_load
; Purpose.............: Loads new SMC firmware from data file into RAM buffer
;                       at address SMC_RAM_BUF
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 if load failed
.proc smc_load
    ; Open file and seek start of SMC firmware
    lda header_smc_start
    sta r1
    lda header_smc_start+1
    sta r1+1
    lda header_smc_start+2
    sta r1+2
    stz r1+3
    ldx #<file_name
    ldy #>file_name
    lda file_name_len
    jsr file_open_and_seek
    bcs err

    ; Reset CRC
    jsr crc16_init

    ; Read firmware from file
    lda #<SMC_RAM_BUF
    sta r1
    lda #>SMC_RAM_BUF
    sta r1+1
    lda header_smc_size
    sta r2
    lda header_smc_size+1
    sta r2+1
    lda header_smc_size+2
    sta r2+2
    lda #<progress
    sta r4
    lda #>progress
    sta r4+1
    jsr file_read
    bcs err

    ; Close file
    jsr file_close

    ; Compare CRCs
    lda crc16_val
    cmp header_smc_crc
    bne err
    lda crc16_val+1
    cmp header_smc_crc+1
    bne err

    ; Return without errors
    clc
    rts

    ; CRC mismatch
err:
    jsr file_close
    sec
    rts

progress:
    sec
    lda header_smc_size
    sbc r2
    tax
    lda header_smc_size+1
    sbc r2+1
    tay
    lda header_smc_size+2
    sbc r2+2
    jmp progress_update
.endproc

;******************************************************************************
; Function name.......: smc_get_firmware_version
; Purpose.............: Returns current firmware version, or 0.0.0 if 
;                       firmware version not available
; Input...............: Nothing
; Returns.............: Firmware version
;                       A = major
;                       Y = minor
;                       X = patch
.proc smc_get_firmware_version
    ; Read version
    ldx #SMC_ADDR
    ldy #SMC_VER_MAJOR
:   jsr I2C_READ_BYTE
    pha
    iny
    cpy #SMC_VER_PATCH+1
    bcc :-

    ; Pull version, patch-minor-major, from stack
    plx
    ply
    pla

    ; If major version is $ff, return v 0.0.0
    cmp #$ff
    bne :+

    lda #0
    ldx #0
    lda #0

:   rts
.endproc

;******************************************************************************
; Function name.......: smc_get_bootloader
; Purpose.............: Returns bootloader version or 0 if not found
; Input...............: Nothing
; Returns.............: A = Bootloader version
.proc smc_get_bootloader_version
    ; Read bootloader version
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_VER
    jsr I2C_READ_BYTE

    ; Return if bootloader version found (!=$ff)
    cmp #$ff
    beq :+
    rts

    ; Try to read bootloader version from end of flash memory (supported in firmware >= 47.2.0)
:   ldx #SMC_ADDR           ; Set page $7f = byte address $1fc0
    ldy #SMC_SET_PAGE
    lda #$7f
    jsr I2C_WRITE_BYTE

    lda #62                 ; Skip 62 first bytes
    sta temp
    ldx #SMC_ADDR
    ldy #SMC_READ_FLASH
 :  jsr I2C_READ_BYTE
    dec temp
    bne :-

    jsr I2C_READ_BYTE       ; Read byte address $1ffe
    cmp #$8a                ; Is magic number?
    bne not_found
    
    jsr I2C_READ_BYTE       ; Read byte address $1fff, the bootloader version
    rts

not_found:
    lda #0
    rts

temp: .res 1
.endproc

;******************************************************************************
; Function name.......: smc_get_bootloader_crc
; Purpose.............: Calculates CRC-16 of the bootloader
; Input...............: Nothing
; Returns.............: X/Y = CRC-16 (LSB/MSB)
;                       A = 0: Known good bootloader
;                           1: Known bad bootloader
;                           2: Unknown bootloader
;                           3: No bootloader
.proc smc_get_bootloader_crc
    ; Clear CRC value
    jsr crc16_init
    
    ; Set counter to read $200 bytes
    lda #2
    sta counter+1
    stz counter

    ; Go to start of bootloader (page = $1e00 / $40 = $78)
    ldx #SMC_ADDR
    ldy #SMC_SET_PAGE
    lda #$78
    jsr I2C_WRITE_BYTE

loop:
    ; Read byte from SMC flash
    ldx #SMC_ADDR
    ldy #SMC_READ_FLASH
    jsr I2C_READ_BYTE
    jsr crc16_update

    ; Decrement counter
    lda counter
    bne :+
    dec counter+1
:   dec counter

    ; Go back to start of loop if not done
    lda counter
    ora counter+1
    bne loop

    ; Compare against list of known CRCs
    ldx #0
loop2:
    lda known_good,x
    cmp crc16_val
    bne next
    lda known_good+1,x
    cmp crc16_val+1
    beq match
next:
    inx
    inx
    cpx #known_bad_end-known_good
    bcc loop2

unknown:
    ; Check if no bootloader, CRC=$6995
    lda #$95
    cmp crc16_val
    bne :+
    lda #$69
    cmp crc16_val+1
    bne :+
    lda #3
    bra exit

:   ; Otherwise it's an unknown bootloader
    lda #2
    bra exit

match:
    cpx #known_bad-known_good
    bcs bad

good:
    lda #0
    bra exit

bad:
    lda #1

exit:
    ; Set return values
    ldx crc16_val
    ldy crc16_val+1

    rts
counter: .res 2

known_good:
    .word $19B5 ;v1
    .word $15C7 ;v2
    .word $BF63 ;v3
known_bad:
    .word $7594 ;v2 (bad)
known_bad_end:
.endproc

;******************************************************************************
; Function name.......: smc_compare_version_number
; Purpose.............: Compares specified version number to the current
;                       SMC firmware version stored in variable smc_cur_version
; Input...............: A = major version
;                       Y = minor version
;                       X = patch
; Returns.............: C = 0 if specified version is less than current version
;                       C = 1 if specified version is equal to or greater than current version
;                       Z = 1 if specified version is equal to the current version
.proc smc_compare_version_number
    cmp smc_cur_version
    beq :+
    bcs lt
    bra gt

:   cpy smc_cur_version+1
    beq :+
    bcs lt
    bra gt

:   cpx smc_cur_version+2
    beq eq
    bcs lt
    bra gt

eq:
    lda #0
    sec
    rts

gt:
    lda #1
    sec
    rts

lt:
    lda #1
    clc
    rts
.endproc 

;******************************************************************************
; Global variables
;******************************************************************************

smc_status:
    .res 1

smc_cur_version:
    .res 3

smc_bootloader:
    .res 1

.ZEROPAGE
    smc_src_addr: .res 2
.CODE
