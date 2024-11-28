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

PROGRESS_ROM = 0
PROGRESS_VERA = 1
PROGRESS_SMC = 2

;******************************************************************************
; Function name.......: progress_init
; Purpose.............: Inits progress meters
; Prerequisite........: Call header_fetch
; Error...............: C=1 if end value is 0
.proc progress_init
    ; ROM
    ldx header_rom_size     ; Get size
    ldy header_rom_size+1
    lda header_rom_size+2

    stx progress_rom_end    ; Store size
    sty progress_rom_end+1
    sta progress_rom_end+2

    jsr progress_step_value ; Calculate 5 % step value
    bcs fail
    stx progress_rom_step
    sty progress_rom_step+1
    sta progress_rom_step+2

    ; VERA
    ldx header_vera_size
    ldy header_vera_size+1
    lda header_vera_size+2

    stx progress_vera_end
    sty progress_vera_end+1
    sta progress_vera_end+2

    jsr progress_step_value
    bcs fail
    stx progress_vera_step
    sty progress_vera_step+1
    sta progress_vera_step+2

    ; SMC
    ldx header_smc_size
    ldy header_smc_size+1
    lda #0

    stx progress_smc_end
    sty progress_smc_end+1
    sta progress_smc_end+2

    jsr progress_step_value
    bcs fail
    stx progress_smc_step
    sty progress_smc_step+1
    sta progress_smc_step+2

    clc
    rts

fail:
    rts

.endproc

;******************************************************************************
; Function name.......: progress_select
; Purpose.............: Selects one of the three progress meters
; Input...............: A = progress meter:
;                           0: ROM
;                           1: VERA
;                           2: SMC
; Returns.............: Nothing
.proc progress_select
    ldy #6

    cmp #PROGRESS_ROM
    bne :+
    ldx #6 ; Table index
    bra copy

:   cmp #PROGRESS_VERA
    bne :+
    ldx #12 ; Table index
    bra copy

:   cmp #PROGRESS_SMC
    bne exit
    ldx #18 ; Table index

copy:
    ; Copy end value
    lda progress_rom_end-1,x
    sta progress_cur_end-1,y
    dex
    dey
    bne copy

    ; Copy step value
    ldx #3
:   lda progress_cur_step-1,x
    sta progress_cur_next-1,x
    dex
    bne :-

    ; Setup, current progress 0 %
    stz progress_cur_pct

exit:
    rts
.endproc

;******************************************************************************
; Function name.......: progress_step_value
; Purpose.............: Calculates step 5% value, i.e. 5% of the specified
;                       end value
; Input...............: X = End value, bits 0..7
;                       Y = End value, bits 8..15
;                       A = End value, bits 16..23
; Returns.............: X = 5% step value, bits 0..7
;                       Y = 5% step value, bits 8..15
;                       A = 5% step value, bits 16..23
; Error...............: C=1 if end value is 0
.proc progress_step_value
    ; Convert end value to floating point
    stx fval+4
    sty fval+3
    sta fval+2
    stz fval+1
    stz fval

    ldx #32         ; FP bit count

    lda fval+4      ; Abort if end value is zero
    ora fval+3
    ora fval+2
    bne :+
    sec
    rts
    
:   lda fval+1      ; Shift left until leftmost bit is set
    bmi :+
    asl fval+4
    rol fval+3
    rol fval+2
    rol fval+1
    dex
    bne :-

:   lda fval+1      ; Set sign, always positive in this context
    and #$7f
    sta fval+1

    clc             ; Set exponent, X holds shift count
    txa
    adc #$80        ; $80 is base value
    sta fval

    lda ROM_SEL     ; Select BASIC bank
    pha
    lda #4
    sta ROM_SEL

calc_step:
    lda #<fval      ; Load end value into FACC
    ldy #>fval
    jsr MOVFM

    lda #<fivepct   ; Multiply by 5%
    ldy #>fivepct
    jsr FMULT

    ldx #<fval      ; Move result to fval
    ldy #>fval
    jsr MOVMF

    ; Convert fval to 32 bit integer
    lda fval+1
    ora #$80
    sta step+3
    lda fval+2
    sta step+2
    lda fval+3
    sta step+1
    lda fval+4
    sta step

    ldx fval
:   lsr step+3
    ror step+2
    ror step+1
    ror step
    inx
    cpx #$a0        ; Exponent base $80 + $20 = $a0 => done shifting
    bne :-

    ; Restore ROM bank
    pla
    sta ROM_SEL

    ; Set return value (24 bit integer)
    ldx step
    ldy step+1
    lda step+2
    clc
    rts

fivepct:
    .byt $7c, $4c, $cc, $cc, $cd    ; 0.05, floating point format

fval:
    .res 5

step:
    .res 4
.endproc

;******************************************************************************
; Function name.......: progress_update
; Purpose.............: Updates the current progess meter
; Input...............: X = Cur value, bits 0..7
;                       Y = Cur value, bits 8..15
;                       A = Cur value, bits 16..23
; Error...............: C=1 if end value is 0
.proc progress_update
    ; At end value?
    cpx progress_cur_end
    bne :+
    cpy progress_cur_end+1
    bne :+
    cmp progress_cur_end+2
    bne :+

    ; Yes, is at end, return 100 %
    lda #100
    cmp progress_cur_pct ; Avoid printing 100 % value more than once
    beq exit

    sta progress_cur_pct ; Print 100 % value
    bra pr

:   ; At next 5 % threshold?
    cmp progress_cur_next+2
    bcc exit
    bne :+ 
    cpy progress_cur_next+1
    bcc exit
    bne :+
    cpx progress_cur_next
    bcc exit

:   ; Yes, increment progress with 5 % units
    clc
    lda progress_cur_pct
    adc #5
    sta progress_cur_pct

    ; Calculate next 5 % threshold
    clc
    lda progress_cur_next
    adc progress_cur_step
    sta progress_cur_next

    lda progress_cur_next+1
    adc progress_cur_step+1
    sta progress_cur_next+1
    
    lda progress_cur_next+2
    adc progress_cur_step+2
    sta progress_cur_next+2

pr:
    ; Delete previosly printed progress % (2 or 3 chars)
    ldx #2
    lda progress_cur_pct
    cmp #11
    bcc :+
    inx
:   lda #20
    jsr CHROUT
    dex
    bne :-

    ; Print progress %
    lda progress_cur_pct
    jsr util_print_num
    lda #'%'
    jsr CHROUT

exit:
    rts

.endproc

progress_cur_end: .res 3
progress_cur_step: .res 3
progress_cur_next: .res 3
progress_cur_pct: .res 1

progress_rom_end: .res 3
progress_rom_step: .res 3

progress_vera_end: .res 3
progress_vera_step: .res 3

progress_smc_end: .res 3
progress_smc_step: .res 3
