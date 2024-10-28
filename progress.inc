;******************************************************************************
; Function name.......: progress_kernal_init
; Purpose.............: Inits progress meter for kernal upgrade
; Input...............: X = End value, bits 0..7
;                       Y = End value, bits 8..15
;                       A = End value, bits 16..23
; Error...............: C=1 if end value is 0
.proc progress_kernal_init
    stx progress_kernal_end
    sty progress_kernal_end+1
    sta progress_kernal_end+2

    jsr progress_step_value
    stx progress_kernal_step
    sty progress_kernal_step+1
    sta progress_kernal_step+2

    rts
.endproc

;******************************************************************************
; Function name.......: progress_vera_init
; Purpose.............: Inits progress meter for vera upgrade
; Input...............: X = End value, bits 0..7
;                       Y = End value, bits 8..15
;                       A = End value, bits 16..23
; Error...............: C=1 if end value is 0
.proc progress_vera_init
    stx progress_vera_end
    sty progress_vera_end+1
    sta progress_vera_end+2

    jsr progress_step_value
    stx progress_vera_step
    sty progress_vera_step+1
    sta progress_vera_step+2

    rts
.endproc

;******************************************************************************
; Function name.......: progress_smc_init
; Purpose.............: Inits progress meter for vera upgrade
; Input...............: X = End value, bits 0..7
;                       Y = End value, bits 8..15
;                       A = End value, bits 16..23
; Error...............: C=1 if end value is 0
.proc progress_smc_init
    stx progress_smc_end
    sty progress_smc_end+1
    sta progress_smc_end+2

    jsr progress_step_value
    stx progress_smc_step
    sty progress_smc_step+1
    sta progress_smc_step+2

    rts
.endproc

;******************************************************************************
; Function name.......: progress_select
; Purpose.............: Selects one of the three progress meters
; Input...............: A = progress meter:
;                           0: Kernal
;                           1: VERA
;                           2: SMC
.proc progress_select
    ldy #6

    cmp #0
    bne :+
    ldx #6
    bra copy

:   cmp #1
    bne :+
    ldx #12
    bra copy

:   cmp #2
    bne exit
    ldx #18

copy:
    lda progress_kernal_end-1,x
    sta progress_cur_end-1,y
    dex
    dey
    bne copy

    ldx #3
:   lda progress_cur_step-1,x
    sta progress_cur_next-1,x
    dex
    bne :-

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

    lda #100
    sta progress_cur_pct
    bra pr

:   cmp progress_cur_next+2
    bcc exit
    bne :+ 
    cpy progress_cur_next+1
    bcc exit
    bne :+
    cpx progress_cur_next
    bcc exit

:   clc
    lda progress_cur_pct
    adc #5
    sta progress_cur_pct

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
    ; Delete two chars
    lda #20
    jsr CHROUT
    jsr CHROUT

    ; If progress > 10%, delete another char
    lda progress_cur_pct
    cmp #11
    bcc :+
    lda #20
    jsr CHROUT
    lda progress_cur_pct

    ; Output progress value
:   jsr util_print_num
    lda #'%'
    jsr CHROUT

exit:
    rts

.endproc

progress_cur_end: .res 3
progress_cur_step: .res 3
progress_cur_next: .res 3
progress_cur_pct: .res 1

progress_kernal_end: .res 3
progress_kernal_step: .res 3

progress_vera_end: .res 3
progress_vera_step: .res 3

progress_smc_end: .res 3
progress_smc_step: .res 3
