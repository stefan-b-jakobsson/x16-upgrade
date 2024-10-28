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
    stz issues

    ; Print title
    print 0, str4_smc

    jsr smc_load

t1:
    ; Error: Unsupported or unknown firmware version (255.xx.xx or <43.0.0)
    lda smc_cur_version
    cmp #$ff
    beq :+
    cmp #SMC_MIN_VERSION
    bcs t2
:   jsr add_error
    print 1, str4_smc_firmware_unsupported

t2: 
    ; Error: No bootloader
    lda smc_bootloader
    cmp #$ff
    bne t3
    jsr add_error
    print 1, str4_smc_no_bootloader
    bra t4

t3:
    ; Error: Unsupported bootloader
    lda smc_bootloader
    cmp #SMC_BOOTLOADER_MIN
    bcc :+
    cmp #SMC_BOOTLOADER_MAX+1
    bcc t4
:   jsr add_error
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
    jsr add_error
    print 1, str4_smc_write_protected

t5: 
    ; Warning: Bad bootloader (confirmed), CRC-16 = $7594; possible to check from SMC v 47.2.3
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
    cpx #$94
    bne exit
    cpy #$75
    bne exit
    
    jsr add_warning
    print 1, str4_smc_bad_bootloader3
    bra exit

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

    jsr add_warning
    print 1,str4_smc_bad_bootloader2
    bra exit

t7: 
    ; Warning: Bad bootloader (low risk)
    jsr add_warning
    print 1,str4_smc_bad_bootloader1
    
exit:
    ; Print PASS message if no issues
    lda issues
    bne :+
    print 0, str_pass
    lda #13
    jsr CHROUT

    ; Set status and return value
:   clc
    lda #SMC_STATUS_PRECHECK_PASS
    sta smc_status

    lda errors
    beq :+

    sec
    lda #SMC_STATUS_PRECHECK_FAIL
    sta smc_status

:   rts

add_error:
    lda errors
    bne :+
    print 0, str_fail
    lda #13
    jsr CHROUT
:   inc errors
    inc issues
    rts

add_warning:
    lda issues
    bne :+
    print 0, str_warn
    lda #13
    jsr CHROUT
:   inc issues
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
    ldx #<header_fname
    ldy #>header_fname
    lda #header_fname_end-header_fname
    jsr file_open_and_seek

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
    stz r4
    stz r4+1
    jsr file_read

    ; Close file
    jsr file_close

    rts
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

    ; Set return values
    ldx crc16_val
    ldy crc16_val+1

    rts
counter: .res 2
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


