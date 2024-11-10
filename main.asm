
; cl65 -o TEST.PRG -u __EXEHDR__ -t cx16 -C cx16-asm.cfg main.asm

.include "kernal.inc"
.include "macro.inc"

;******************************************************************************
; Function name.......: main
; Purpose.............: Program main function
; Input...............: Nothing
; Returns.............: Nothing
.proc main
    ; Set PETSCII upper/lower case
    lda #$8f
    jsr $ffd2
    lda #$0e
    jsr $ffd2
    lda #$08
    jsr $ffd2

    ; Select Kernal ROM bank
    lda ROM_SEL
    pha
    stz ROM_SEL
    jsr util_init

    ; Init
    jsr rom_init
    jsr vera_init
    jsr smc_init

    ; Show screen 1
    jsr screen1_show

    ; Restore ROM bank
    pla
    sta ROM_SEL

    ; Position cursor before returning
    lda VERA_ADDR0
    lsr
    tay
    sec
    lda VERA_ADDR1
    sbc #$b0
    tax
    clc
    jsr $fff0   ; Real PLOT

    ; Quit program
    rts
.endproc

.include "util.inc"

.include "header.inc"
.include "rom.inc"
.include "vera.inc"
.include "smc.inc"

.include "screen1.inc"
.include "screen2.inc"
.include "screen3.inc"
.include "screen4.inc"
.include "screen5.inc"
.include "screen6.inc"

.include "crc16.inc"
.include "progress.inc"
.include "widget.inc"
.include "screen.inc"
.include "strings.inc"

.include "file.inc"
