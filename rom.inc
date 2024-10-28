ROM_STATUS_UNSELECTED = 0
ROM_STATUS_SELECTED = 1
ROM_STATUS_PRECHECK_FAIL = 2
ROM_STATUS_PRECHECK_PASS = 3
ROM_STATUS_INSTALLED = 4

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
; Purpose.............: Runs the following precheck:
;                       - Write protect detection
;                       - Manufacturer and chip ID check
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

    ; Save ROM bank
    lda ROM_SEL
    pha

    ; Byte program entry
    lda #$01 
    sta ROM_SEL
    lda #$aa
    sta $d555

    dec ROM_SEL
    lda #$55
    sta $eaaa 

    inc ROM_SEL
    lda #$a0
    sta $d555

    ; Attempt to write $ff to 01:ffff
    lda #$ff
    sta $ffff

    ; Check if ROM chip entered byte programming mode,
    ; detected by bit 6 of the target address alternating
    ; between bytes
    lda $ffff
    cmp $ffff
    bne prg_wait

write_protected:
    bra fail
    
prg_wait:
    ; Wait for byte program operation to finish
    lda $ffff
    cmp $ffff
    bne prg_wait

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

    nop

    ; Read software ID
    dec ROM_SEL
    lda $c000
    sta chip_id
    lda $c001
    sta chip_id+1

    ; Software ID exit
    lda #$f0
    sta $c000
    nop

    ; Interpret result
    lda chip_id
    cmp #$bf
    bne fail
    lda chip_id+1
    cmp #$b5
    bcc fail

pass:
    ; Restore ROM bank
    pla
    sta ROM_SEL

    ; Restore IRQ flag
    plp

    ; Print PASS message
    print 0,str_pass
    lda #13
    jsr CHROUT

    ; Set return values
    ldx chip_id
    ldy chip_id+1
    clc
    rts

fail:
    ; Restore ROM bank
    pla
    sta ROM_SEL
    
    ; Restore IRQ flag
    plp

    ; Print FAIL message
    print 0, str_fail
    print 1, str4_rom_write_protected

    ; Set status
    lda #ROM_STATUS_PRECHECK_FAIL
    sta rom_status

    ; Set return values
    ldx #0
    ldy #0
    sec
    rts

chip_id: .res 2
.endproc

;******************************************************************************
; Function name.......: rom_update
; Purpose.............: Updates the ROM
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_update
    ; Print label
    print 0, rom_label

    ; Setup progress indicator
    ldx #0
    ldy #0
    lda #4
    jsr progress_kernal_init
    lda #0
    jsr progress_select

    ; Open file
    ldx #<fname
    ldy #>fname
    lda #fname_end-fname
    jsr file_open

    ; Loading
    print 1, rom_loadmsg

    stz r1
    lda #$50
    sta r1+1
    stz r2
    stz r2+1
    lda #4
    sta r2+2
    lda #<callback
    sta r4
    lda #>callback
    sta r4+1
    jsr file_read
    jsr file_close
    rts

callback:
    ; Calculate bytes sent
    sec
    lda #0
    sbc r2
    tax
    lda #0
    sbc r2+1
    tay
    lda #4
    sbc r2+2
    
    ; Update progress
    jmp progress_update

rom_label:
    .byt "ROM...",13,0

rom_loadmsg:
    .byt "Loading... 0%",0

fname:
    .byt "rom.bin"
fname_end:
.endproc

rom_status:
    .res 1

rom_version:
    .byt "R48",0
