;******************************************************************************
; Function name.......: screen6_show
; Purpose.............: Shows screen 6 - Warning
; Input...............: Nothing
; Returns.............: Nothing
.proc screen6_show
    ; Init screen
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str6_disclaimer

    ; Add Exit button
    ldx #<str6_btn_exit
    ldy #>str6_btn_exit
    lda #str6_btn_exit_end-str6_btn_exit
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Install button
    ldx #<str6_btn_install
    ldy #>str6_btn_install
    lda #str6_btn_install_end-str6_btn_install
    jsr widget_add_button

listen:
    jsr widget_listen

    ; Exit button?
    cpx #0
    bne :+
    rts

:   ; Install button
    cpx #1
    bne listen

    jmp screen5_show

.endproc
