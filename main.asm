
; Copyright (c) 2024, Stefan Jakobsson

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
    jsr header_init
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

;******************************************************************************
; Include files

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
