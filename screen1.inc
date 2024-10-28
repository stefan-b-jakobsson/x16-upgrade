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

    ; Add Exit button (index 0)
    ldx #<str1_btn_exit
    ldy #>str1_btn_exit
    lda #str1_btn_exit_end-str1_btn_exit
    jsr widget_add_button

    ; Add Continue button (index 1)
    ldx #<str1_btn_continue
    ldy #>str1_btn_continue
    lda #str1_btn_continue_end-str1_btn_continue
    jsr widget_add_button

    ; Select Exit button
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

listen:
    ; Listen for key events
    jsr widget_listen
    
    ; Exit button?
    cpx #0
    beq exit
    
    ; Next button?
    cpx #1
    bne listen
    
    jsr header_fetch
    jsr header_run_prechecks
    bcs exit
    jmp screen2_show

exit:
    rts
.endproc
