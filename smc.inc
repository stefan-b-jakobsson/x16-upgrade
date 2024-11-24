; Supported SMC firmware and bootloader versions
SMC_MIN_VERSION = 43
SMC_BOOTLOADER_MIN = 1
SMC_BOOTLOADER_MAX = 3

; Status
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

; I2C interface
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

;******************************************************************************
; Function name.......: smc_init
; Purpose.............: Initializes SMC functions
; Input...............: Nothing
; Returns.............: smc_status = SMC_STATUS_SELECTED
;                       smc_curversion (3 bytes) = current version
;                       smc_bootloader (1 byte) = current version
.proc smc_init
    ; Mark SMC for update (default)
    lda #SMC_STATUS_SELECTED
    sta smc_status

    ; Get SMC firmware version
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
; Purpose.............: Runs SMC pre-upgrade checks
; Input...............: Nothing
; Returns.............: C=1 on error
.proc smc_run_prechecks
    ; Abort if not selected for update
    lda smc_status
    bne :+
    rts

    ; Init and select SMC progress meter
:   ldx header_smc_size
    ldy header_smc_size+1
    lda header_smc_size+2
    jsr progress_smc_init
    lda #2
    jsr progress_select

    ; Print title
    print 0, str4_smc

t1:
    ; Error: Unsupported firmware version (<43.0.0)
    print 1, str4_smc_firmware
    lda smc_cur_version
    cmp #SMC_MIN_VERSION
    bcc :+

    ; Test 1, OK
    print 1, str_ok_cr
    bra t2

:   ; Test 1, fail
    print 1, str_fail_cr
    print 2, str4_smc_firmware_unsupported
    jmp fail

t2:
    ; Error: Write-enabled, possible to check from SMC v 47.2.4
    lda smc_cur_version
    cmp #47
    bcc t3
    bne :+
    lda smc_cur_version+1
    cmp #2
    bcc t3
    bne :+
    lda smc_cur_version+2
    cmp #4
    bcc t3

:   ldx #SMC_ADDR
    ldy #SMC_EFUSE
    jsr I2C_READ_BYTE
    lsr
    bcs :+

    ; Test 2, OK
    print 1, str4_smc_write_enabled
    print 1, str_ok_cr
    bra t3

    ; Test 2, fail
:   print 1, str4_smc_write_enabled
    print 1, str_fail_cr
    jmp fail

t3: 
    ; Error: No bootloader
    print 1, str4_smc_bootloader

    lda smc_bootloader
    bne t4
    
    ; Test 3, fail
    print 1, str_fail_cr
    print 2, str4_smc_no_bootloader
    jmp fail

t4:
    ; Error: Unsupported bootloader
    lda smc_bootloader
    cmp #SMC_BOOTLOADER_MIN
    bcc :+
    cmp #SMC_BOOTLOADER_MAX+1
    bcc t5

    ; Test 4, fail
    print 1, str_fail_cr
    print 2, str4_smc_bootloader_unsupported
    jmp fail

t5: 
    ; Warning: Bad bootloader (confirmed), possible to check from firmware version 47.2.3
    lda smc_cur_version
    cmp #47
    bcc t6
    bne :+
    lda smc_cur_version+1
    cmp #2
    bcc t6
    bne :+
    lda smc_cur_version+2
    cmp #3
    bcc t6

:   jsr smc_get_bootloader_crc
    cmp #0  ; Known good bootloader
    bne :+
    print 1, str_ok_cr
    bra t7

:   cmp #1  ; Known bad bootloader
    bne :+
    print 1, str_warn_cr
    print 2, str4_smc_bad_bootloader3
    bra t7

:   cmp #2  ; Unknown bootloader
    bne t7
    print 1, str_warn_cr
    print 2, str4_smc_unknown_bootloader
    bra t7

t6: 
    ; Warning: Bad bootloader (high risk), bootloader v2 + firmware 45.1.0
    lda smc_bootloader
    cmp #2
    beq :+
    print 1, str_ok_cr
    bra t7

:   lda smc_cur_version
    cmp #45
    bne :+
    lda smc_cur_version+1
    cmp #1
    bne :+
    lda smc_cur_version+2
    cmp #0
    bne :+

    ; High risk
    print 1, str_warn_cr
    print 2, str4_smc_bad_bootloader2
    bra t7
 
    ; Low risk
    print 1, str_warn_cr
:   print 2,str4_smc_bad_bootloader1

t7:
    ; Try to load firmware
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
    sec
    lda #SMC_STATUS_PRECHECK_FAIL
    sta smc_status
    rts
.endproc

;******************************************************************************
; Function name.......: smc_update
; Purpose.............: Updates the SMC firmware
; Input...............: Nothing
; Returns.............: C=1 on error
.proc smc_update
    ; Disable interrupts
    sei

    ; Return if status != LOAD_PASS
    lda smc_status
    cmp #SMC_STATUS_LOAD_PASS
    beq :+
    sec
    rts

:   ; Print header
    print 0, str5_smc_header

wait_key:
    print 1, str5_smc_wait_key
    jsr util_wait_keypress

    ; Start bootloader
    print 1, str5_smc_start_bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_START
    lda #$31
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
    cmp #0
    bne wait_key

    ; Clear byte count
    stz count
    stz count+1

    ; Clear checksum
    stz checksum

    ; Set max attempts
    lda #10
    sta attempts

    ; Setup pointer to end of firmware, $5f00 + header_smc_size
    lda header_smc_size
    sta smc_src_addr
    lda header_smc_size+1
    adc #$5f
    sta smc_src_addr+1

    ; Pad end of firmware to page boundary
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

    ; Setup pointer to start of firmware, $5f00
:   stz smc_src_addr
    lda #$5f
    sta smc_src_addr+1

    ; Setup progress meter
    lda #2
    jsr progress_select

    ; Print write message
    print 1, str5_smc_write

loop:
    ; End of stream?
    lda count
    cmp size
    bne send
    lda count+1
    cmp size+1
    beq complete

send:
    ; Send byte
    ldx #SMC_ADDR
    ldy #$80
    lda (smc_src_addr)
    jsr I2C_WRITE_BYTE

    ; Update checksum
    clc
    lda checksum
    adc (smc_src_addr)
    sta checksum

commit:
    ; Check if at end of package
    lda count
    and #7
    cmp #7
    bne next

    ; Send checksum
    ldx #SMC_ADDR
    ldy #$80
    lda checksum
    eor #$ff
    inc
    jsr I2C_WRITE_BYTE

    ; Clear checksum
    stz checksum

    ; Commit
    ldx #SMC_ADDR
    ldy #$81
    jsr I2C_READ_BYTE
    cmp #1
    beq next

    ; Commit failed
    dec attempts
    beq fail
    
    ; Rewind source address to start of packet
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
    lda #10
    sta attempts

    ; Increment byte count
    inc count
    bne :+
    inc count+1

:   ; Increment source address
    inc smc_src_addr
    bne loop
    inc smc_src_addr+1
    
    ; Update progress meter
    ldx count
    ldy count+1
    lda #0
    jsr progress_update
    
    jmp loop

complete:
    ; Update progress meter
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
    pha
    print 1, str5_smc_update_failed
    pla
    jsr util_print_num
    lda #13
    jsr CHROUT
    lda #SMC_STATUS_UPDATE_FAIL
    sta smc_status
    sec
    rts

callback:
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_VER
    jsr I2C_READ_BYTE
    cmp #0
    bne :+
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
; Purpose.............: Verifies SMC update
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 if verify failed
.proc smc_verify
    ; Check bootloader version, verify available from v3
    lda smc_bootloader
    cmp #$ff
    bne :+
    clc
    rts

:   cmp #3
    bcs :+
    clc
    rts

:   ; Print verifying message
    print 1, str5_smc_verify

    ; Setup progress meter
    lda #2
    jsr progress_select

    ; Clear counter
    stz count
    stz count+1

    ; Set pointer to start of new SMC firmware in RAM
    lda #$00
    sta smc_src_addr
    lda #$5f
    sta smc_src_addr+1

    ; Rewind target address
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_REWIND
    lda #0
    jsr I2C_WRITE_BYTE

loop:
    ; Read byte from SMC flash
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_READ_FLASH
    jsr I2C_READ_BYTE

    ldx count+1
    bne compare
    ldx count

    ; Byte $0000? Should always be $ff
    bne :+
    cmp #$ff
    bne failed
    bra next

    ; Byte $0001? Should always be $ce
:   cpx #1
    bne :+
    cmp #$ce
    bne failed
    bra next

    ; Byte $0012? Should be Reset vector minus 9, low byte
:   cpx #$12
    bne :+
    tay

    sec
    lda $5f00
    sbc #9
    sta ee_rdy
    lda $5f01
    sbc #0
    and #%11001111
    ora #%11000000
    sta ee_rdy+1

    cpy ee_rdy
    bne failed
    bra next

    ; Byte $0013? Should be Reset vector minus 9, high byte
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
    lda count
    cmp header_smc_size
    bne loop
    lda count+1
    cmp header_smc_size+1
    beq exit
    jmp loop

exit:
    jsr progress
    
    lda #SMC_STATUS_VERIFY_PASS
    sta smc_status
    print 1, str_ok_cr
    clc
    rts

failed:
    jsr util_print_num
    lda #32
    jsr CHROUT
    lda (smc_src_addr)
    jsr util_print_num
    lda #32
    jsr CHROUT
    lda count
    jsr util_print_num
    lda #32
    jsr CHROUT
    lda count+1
    jsr util_print_num

    lda #SMC_STATUS_VERIFY_FAIL
    sta smc_status
    print 1, str_fail_cr
    sec
    rts

progress:
    ldx count
    ldy count+1
    lda #0
    jmp progress_update

count: .res 2
ee_rdy: .res 2
.endproc

.proc smc_reboot
    ; Check if SMC was selected for update, return if not
    lda smc_status
    cmp #SMC_STATUS_UNSELECTED
    bne :+
    rts

:   ; Wait 5 seconds
    print 1, str5_smc_reset
    lda #5
    ldx #0
    ldy #0  
    jsr util_countdown

    ; Send reboot command to bootloader
    ldx #SMC_ADDR
    ldy #SMC_BOOTLOADER_REBOOT
    jsr I2C_WRITE_BYTE

:   bra :-
.endproc

;******************************************************************************
; Function name.......: smc_load
; Purpose.............: Loads new SMC firmware from data file into low RAM
;                       address range $5F00-7EFF
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
    lda #$00
    sta r1
    lda #$5f
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
    ldx #SMC_ADDR
    ldy #SMC_VER_MAJOR

:   jsr I2C_READ_BYTE
    pha
    iny
    cpy #SMC_VER_PATCH+1
    bcc :-

    plx
    ply
    pla

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
    ldx #SMC_ADDR
    ldy #SMC_READ_FLASH
    jsr I2C_READ_BYTE
    jsr crc16_update

    lda counter
    bne :+
    dec counter+1
:   dec counter

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
