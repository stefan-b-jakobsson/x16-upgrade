;******************************************************************************
; Function name.......: screen_header
; Purpose.............: Prints screen header
; Input...............: Nothing
; Returns.............: Nothing
.proc screen_header
    print 0,str_header
    ldx #0
    ldy #0
    lda #224
    sta r0
    lda screen_width
    jmp util_set_color
.endproc
