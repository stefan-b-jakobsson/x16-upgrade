ROM_STATUS_UNSELECTED = 0
ROM_STATUS_SELECTED = 1
ROM_STATUS_PRECHECK_FAIL = 2
ROM_STATUS_PRECHECK_PASS = 3
ROM_STATUS_INSTALLED = 4

ROM_CHIP_ID = $b7bf

;******************************************************************************
; Function name.......: rom_init
; Purpose.............: Initialize ROM update functions
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_init
    lda #ROM_STATUS_SELECTED
    sta rom_status
    rts
.endproc

;******************************************************************************
; Function name.......: rom_run_prechecks
; Purpose.............: Runs the following prechecks:
;                       - Write protect detection
;                       - Manufacturer and chip ID check
;                       - Firmware load and CRC-16 check
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc rom_run_prechecks
    ; Abort if ROM is unselected
    lda rom_status
    bne :+
    clc
    rts

    ; Print header
:   print 0, str4_rom

    ; Disable interrupts
    php
    sei

    ; Save ROM bank in X
    ldx ROM_SEL

t1:
    ; Test 1: Chip write-protected
    print 1, str4_rom_write_enabled
    
    ; Attempt to write $ff to $01:$ffff
    lda #$01
    sta rom_target_bank
    lda #$ff
    sta rom_target_addr
    sta rom_target_addr+1
    jsr rom_write
    bcs :+
    
    ; Test 1, OK
    print 1, str_ok_cr
    bra t2

:   ; Test 1, failed
    print 1, str_fail_cr
    bra fail
    
t2:
    ; Test 2: Check chip ID
    print 1, str4_rom_chip_id
    jsr rom_read_chip_id
    cpx #<ROM_CHIP_ID
    bne :+
    cpy #>ROM_CHIP_ID
    bne :+

    ; Test 2, OK
    print 1, str_ok_cr
    bra t3

:   ; Test 2, failed
    print 1, str_fail_cr
    bra fail

t3: 
    ; Test 3: Try to load ROM
    jsr rom_load
    bcc :+

    ; Test 3, failed
    print 1, str_fail_cr
    bra fail

    ; Test 3, OK
:   print 1, str_ok_cr

pass:
    ; Set status
    lda #ROM_STATUS_PRECHECK_PASS
    sta rom_status

exit:
    ; Restore IRQ flag
    plp

    ; Set return values
    clc
    rts

fail:
    ; Set status
    lda #ROM_STATUS_PRECHECK_FAIL
    sta rom_status

    ; Set return values
    bra exit
.endproc

;******************************************************************************
; Function name.......: rom_update
; Purpose.............: Updates the ROM
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_update
    ; Abort if status != PRECHECK_PASS
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_PASS
    beq :+
    rts

    ; Disable interrupts
:   sei

    ; Print header
    print 0, str5_rom_header

    ; Setup
    stz rom_src_addr
    lda #$7f
    sta rom_src_addr+1

    stz rom_target_bank
    stz rom_target_addr
    lda #$c0
    sta rom_target_addr+1

    stz count
    stz count+1
    stz count+2

    lda #0
    jsr progress_select

    ; Chip erase
    print 1, str5_rom_erase
    jsr rom_erase
    print 1, str_ok
    lda #13
    jsr CHROUT

    ; Write loop
    print 1, str5_rom_write

loop:
    lda (rom_src_addr)
    jsr rom_write

    ; Increment byte count
    inc count
    bne :++
    inc count+1
    bne :+
    inc count+2

:   ldx count
    ldy count+1
    lda count+2
    jsr progress_update

    ; Check if done
:   lda count
    cmp header_rom_size
    bne src_inc
    lda count+1
    cmp header_rom_size+1
    bne src_inc
    lda count+2
    cmp header_rom_size+2
    beq exit

    ; Increment source address
src_inc:   
    inc rom_src_addr
    bne target_inc
    inc rom_src_addr+1
    
    lda rom_src_addr+1
    cmp #$c0
    beq :+
    cmp #$9f
    bne target_inc
    stz RAM_SEL

:   inc RAM_SEL
    lda #$a0
    sta rom_src_addr+1

    ; Increment target address
target_inc:
    inc rom_target_addr
    bne loop
    inc rom_target_addr+1
    bne loop
    lda #$c0
    sta rom_target_addr+1
    inc rom_target_bank
    bra loop

exit:
    lda #13
    jsr CHROUT

    ; Exit
    stz ROM_SEL
    rts

count: .res 3
.endproc

;******************************************************************************
; Function name.......: rom_verify
; Purpose.............: Verifies ROM content
; Input...............: Nothing
; Returns.............: C=1 if verify failed
.proc rom_verify
    ; Abort if status != PRECHECK_PASS
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_PASS
    beq :+
    sec
    rts

    ; Print header
:   print 1, str5_rom_verify
    
    ; Setup
    stz ROM_SEL
    
    stz rom_src_addr
    lda #$7f
    sta rom_src_addr+1

    stz rom_target_addr
    lda #$c0
    sta rom_target_addr+1

    stz count
    stz count+1
    stz count+2

    lda #0
    jsr progress_select

loop:    
    ; Verify
    lda (rom_src_addr)
    cmp (rom_target_addr)
    bne fail

    ; Increment
    inc count
    bne :++
    inc count+1
    bne :+
    inc count+2

:   ldx count
    ldy count+1
    lda count+2
    jsr progress_update

    ; Check if done
:   lda count+2
    cmp header_rom_size+2
    bne next
    lda count+1
    cmp header_rom_size+1
    bne next
    lda count
    cmp header_rom_size
    beq ok

next:
    inc rom_src_addr
    bne @2

    inc rom_src_addr+1
    lda rom_src_addr+1
    cmp #$c0
    beq @1
    cmp #$9f
    bne @2
    stz RAM_SEL

@1: inc RAM_SEL
    lda #$a0
    sta rom_src_addr+1

@2: inc rom_target_addr
    bne loop
    inc rom_target_addr+1
    bne loop
    inc ROM_SEL
    lda #$c0
    sta rom_target_addr+1
    bra loop

ok:
    stz ROM_SEL
    print 1,str_ok
    lda #13
    jsr CHROUT
    rts

fail:    
    stz ROM_SEL
    print 1, str_fail
    lda #13
    jsr CHROUT
    rts

count: .res 3
.endproc

;******************************************************************************
; Function name.......: rom_load
; Purpose.............: Loads the ROM
; Input...............: Nothing
; Returns.............: Nothing
; Errors..............: C=1 on error
.proc rom_load
    ; Reset CRC
    jsr crc16_init

    ; Setup progress
    ldx header_rom_size
    ldy header_rom_size+1
    lda header_rom_size+2
    jsr progress_kernal_init
    lda #0
    jsr progress_select

    print 1, str4_smc_progress

    ; Open file and seek start of ROM firmware
    lda header_rom_start
    sta r1
    lda header_rom_start+1
    sta r1+1
    lda header_rom_start+2
    sta r1+2
    stz r1+3
    ldx #<file_name
    ldy #>file_name
    lda file_name_len
    jsr file_open_and_seek
    bcs err

    ; Load file
    lda #$00
    sta r1
    lda #$7f
    sta r1+1
    lda header_rom_size
    sta r2
    lda header_rom_size+1
    sta r2+1
    lda header_rom_size+2
    sta r2+2
    lda #<callback
    sta r4
    lda #>callback
    sta r4+1
    jsr file_read
    bcs err
    
    ; Close and exit
    jsr file_close
    clc
    rts

err:
    jsr file_close
    sec
    rts

callback:
    sec
    lda header_rom_size
    sbc r2
    tax
    lda header_rom_size+1
    sbc r2+1
    tay
    lda header_rom_size+2
    sbc r2+2
    jmp progress_update
.endproc

;******************************************************************************
; Function name.......: rom_read_chip_id
; Purpose.............: Gets ROM chip ID
; Input...............: Nothing
; Returns.............: X = LSB, Y = MSB
.proc rom_read_chip_id
    ; Save ROM bank on stack
    lda ROM_SEL
    pha

    ; Software ID entry
    lda #$01
    sta ROM_SEL
    lda #$aa
    sta $d555

    dec ROM_SEL
    lda #$55
    sta $eaaa 

    inc ROM_SEL
    lda #$90
    sta $d555

    ; Delay
    nop

    ; Read software ID
    dec ROM_SEL
    ldx $c000
    ldy $c001

    ; Software ID exit
    lda #$f0
    sta $c000
    
    ; Delay
    nop

    ; Restore ROM bank
    pla
    sta ROM_SEL
    rts
.endproc

;******************************************************************************
; Function name.......: rom_erase
; Purpose.............: Erases the ROM chip
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_erase
    ; Store current ROM bank in X
    ldx ROM_SEL

    ; Chip erase command
    lda #$01        ; 5555H=AAH 
    sta ROM_SEL
    lda #$aa
    sta $d555

    dec ROM_SEL     ; 2AAAH=55H
    lda #$55
    sta $eaaa 

    inc ROM_SEL     ; 5555H=80H
    lda #$80
    sta $d555

    lda #$aa        ; 5555H=AAH
    sta $d555

    dec ROM_SEL     ; 2AAAH=55H
    lda #$55
    sta $eaaa

    inc ROM_SEL     ; 5555H=10H
    lda #$10
    sta $d555

    stz ROM_SEL
wait:
    lda $c000
    cmp $c000
    bne wait

exit:
    stx ROM_SEL
    rts

.endproc

;******************************************************************************
; Function name.......: rom_write
; Purpose.............: Writes one byte to ROM at 
;                       rom_target_bank:rom_target_addr.
; Input...............: A = value
; Returns.............: Nothing
; Errors..............: C=1 if write operation failed
.proc rom_write
    ; Save current ROM bank
    ldx ROM_SEL
    phx

    ldx rom_target_bank

    ; Enter byte programming mode
    ldy #$01 
    sty ROM_SEL
    ldy #$aa
    sty $d555

    dec ROM_SEL
    ldy #$55
    sty $eaaa 

    inc ROM_SEL
    ldy #$a0
    sty $d555

    ; Write data
    stx ROM_SEL
    sta (rom_target_addr)

    lda (rom_target_addr)
    cmp (rom_target_addr)
    beq fail

wait:
    lda (rom_target_addr)
    cmp (rom_target_addr)
    bne wait

    ; Restore ROM bank
    plx
    stx ROM_SEL

    clc
    rts

fail:
    plx
    stx ROM_SEL
    
    sec
    rts
    
.endproc

;******************************************************************************
; Global variables

rom_status:
    .res 1

rom_version:
    .byt "R48",0

.ZEROPAGE
    rom_target_addr: .res 2
    rom_target_bank: .res 1
    rom_src_addr: .res 2
.CODE
