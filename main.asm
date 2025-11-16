
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

.export file_cwd_buf

;******************************************************************************
; Global defines and macros
.include "common.inc"
.include "macro.inc"

;******************************************************************************
; Function name.......: main
; Purpose.............: Program main entry
; Input...............: Nothing
; Returns.............: Nothing
.proc main
    ; Clear exit request flag
    stz main_exit_request

    ; Backup current working directory
    jsr file_cwd_backup
    cpx #0 ; OK
    beq clrscr
    cpx #1 ; File read error
    beq cwd_readerr
    ; Else path too long

cwd_overflow:
    ; Current path doesn't fit in the buffer, abort
    ldx #str_cwd_overflow-str_cwd_readerr
    bra cwd_printerr

cwd_readerr:
    ; Current path read error, abort
    ldx #0

cwd_printerr:
    lda str_cwd_readerr,x
    beq cwd_exit
    jsr $ffd2
    inx
    bra cwd_printerr
cwd_exit:
    rts

clrscr:
    ; Clear screen
    lda #$93
    jsr $ffd2

    ; ISO off
    lda #$8f
    jsr $ffd2

    ; Select PETSCII upper case/graphics, and copy tile map to VERA address $00:$4000, to be used by the help screen
    lda #$8e
    jsr $ffd2

    lda #1              ; VERA destination address
    sta VERA_CTRL
    stz VERA_ADDR0
    lda #$40
    sta VERA_ADDR1
    lda #%00010000
    sta VERA_ADDR2

    stz VERA_CTRL       ; VERA source address
    stz VERA_ADDR0
    lda VERA_L1_TILEBASE
    and #%11111100
    asl
    sta VERA_ADDR1
    lda #%00001000
    rol
    sta VERA_ADDR2

    ldx #$ff
    ldy #$08

:   lda VERA_ADDR1
    lda VERA_D0
    inc VERA_CTRL
    sta VERA_D1
    dec VERA_CTRL
    dex
    bne :-
    dey
    bne :-

    ; Select PETSCII upper case/lower case
    lda #$0e
    jsr $ffd2
    
    ; Prevent charset changes from the keyboard
    lda #$08
    jsr $ffd2

    ; Select Kernal ROM bank
    lda ROM_SEL
    pha
    stz ROM_SEL

    ; Init
    jsr util_init
    jsr header_init
    jsr rom_init
    jsr vera_init
    jsr smc_init

    ; Show screen 1
    jsr screen1_show
    lda main_exit_request
    bne :+

    ; Check VERA JP1
    jsr screen7_show

    ; Check ROM write protect jumper (J1)
    jsr rom_is_write_enabled
    bcs :+ ; Jumper is not installed
    print 0, str_rom_remove_j1 ; Display recommendation to uninstall jumper

    ; Restore ROM bank
:   pla
    sta ROM_SEL

    ; Restore CWD
    jsr file_cwd_restore

    ; Position cursor before returning
    lda VERA_ADDR0
    lsr
    tay
    sec
    lda VERA_ADDR1
    sbc #$b0
    tax
    clc
    jsr $fff0   ; PLOT

    ; Quit program
    rts
.endproc

;******************************************************************************
; Include files

.include "version.inc"

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
.include "screen7.inc"
.include "screen8.inc"

.include "crc16.inc"
.include "progress.inc"
.include "widget.inc"
.include "screen.inc"
.include "strings.inc"

.include "file.inc"

.include "help.inc"

main_exit_request: .res 1
