;******************************************************************************
; Function name.......: screen1_show
; Purpose.............: Shows screen 1 - Welcome message
; Input...............: Nothing
; Returns.............: Nothing
.proc screen1_show
    ; Init screen
    jsr widget_init
    
    ; Print header and welcome message
    jsr screen_header
    print 0, str1_welcome

    ; Get file name
    print 0, str1_file_prompt
    
    lda VERA_ADDR0
    lsr
    tay
    sec
    lda VERA_ADDR1
    sbc #$b0
    tax
    clc
    jsr $fff0   ; Real PLOT

    ldy #0
:   jsr CHRIN
    cmp #13
    beq :+
    sta file_name,y
    iny
    bra :-
:   iny
    sta file_name_len
    
    jsr header_fetch
    bcs exit
    jsr header_run_prechecks
    bcs exit
    jsr progress_init
    jmp screen2_show

exit:
    rts
.endproc
