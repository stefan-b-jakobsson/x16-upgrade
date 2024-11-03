SMC_ADDR = $42

SMC_EFUSE = $0c
SMC_VER_MAJOR = $30
SMC_VER_MINOR = $31
SMC_VER_PATCH = $32
SMC_BOOTLOADER_VER = $8e
SMC_SET_PAGE = $90
SMC_READ_FLASH = $91

SMC_MIN_VERSION = 43
SMC_BOOTLOADER_MIN = 1
SMC_BOOTLOADER_MAX = 3

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
;                       smc_status = SMC_STATUS_PRECHECK_PASS /
;                                    SMC_STATUS_PRECHECK_FAIL
.proc smc_run_prechecks
    ; Abort if not selected for update
    lda smc_status
    bne :+
    rts
    
:   stz errors

    ; Init and select SMC progress meter
    ldx header_smc_size
    ldy header_smc_size+1
    lda header_smc_size+2
    jsr progress_smc_init
    lda #2
    jsr progress_select

    ; Print title
    print 0, str4_smc

t1:
    ; Error: Unsupported or unknown firmware version (255.xx.xx or <43.0.0)
    lda smc_cur_version
    cmp #$ff
    beq :+
    cmp #SMC_MIN_VERSION
    bcs t2
:   inc errors
    print 1, str4_smc_firmware_unsupported

t2: 
    ; Error: No bootloader
    lda smc_bootloader
    cmp #$ff
    bne t3
    inc errors
    print 1, str4_smc_no_bootloader
    bra t4

t3:
    ; Error: Unsupported bootloader
    lda smc_bootloader
    cmp #SMC_BOOTLOADER_MIN
    bcc :+
    cmp #SMC_BOOTLOADER_MAX+1
    bcc t4
:   inc errors
    print 1, str4_smc_bootloader_unsupported

t4: 
    ; Error: Chip write-protected, possible to check from SMC v 47.2.4
    lda smc_cur_version
    cmp #$ff
    beq t5
    cmp #47
    bcc t5
    bne :+
    lda smc_cur_version+1
    cmp #2
    bcc t5
    bne :+
    lda smc_cur_version+2
    cmp #4
    bcc t5

:   ldx #SMC_ADDR
    ldy #SMC_EFUSE
    jsr I2C_READ_BYTE
    lsr
    bcc t5
    inc errors
    print 1, str4_smc_write_protected

t5: 
    ; Warning: Bad bootloader (confirmed)
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
    beq exit
    cmp #1
    bne :+
    print 1, str4_smc_bad_bootloader3
    bra t8
:   cmp #2
    bne t8
    print 1, str4_smc_unknown_bootloader
    bra t8

t6: 
    ; Warning: Bad bootloader (high risk)
    lda smc_bootloader
    cmp #2
    bne exit
    lda smc_cur_version
    cmp #45
    bne t7
    lda smc_cur_version+1
    cmp #1
    bne t7
    lda smc_cur_version+2
    cmp #0
    bne t7

    print 1,str4_smc_bad_bootloader2
    bra t8

t7: 
    ; Warning: Bad bootloader (low risk)
    print 1,str4_smc_bad_bootloader1

t8:
    ; Try to load firmware
    print 1,str4_smc_progress
    jsr smc_load
    bcs :+
    print 1, str_ok
    bra exit
:   print 1, str_fail
    inc errors
    
exit:
    ; Check errors
    lda errors
    beq :+

    ; Return with error
    sec
    lda #SMC_STATUS_PRECHECK_FAIL
    sta smc_status
    rts

    ; Return without error
:   print 1, str_pass
    clc
    lda #SMC_STATUS_PRECHECK_PASS
    sta smc_status
    rts

errors: .res 1
issues: .res 1
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
; Purpose.............: Returns firmware version
; Input...............: Nothing
; Returns.............: Firmware version
;                       A = major
;                       Y = minor
;                       X = patch
.proc smc_get_firmware_version
    lda #SMC_ADDR
    ldy #SMC_VER_MAJOR

:   jsr I2C_READ_BYTE
    pha
    iny
    cpy #SMC_VER_PATCH+1
    bcc :-

    pla
    ply
    plx

    rts
.endproc

;******************************************************************************
; Function name.......: smc_get_bootloader
; Purpose.............: Returns bootloader version
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
    lda #$ff
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

smc_new_version:
    .byt 47,2,4


