;******************************************************************************
; Function name.......: screen4_show
; Purpose.............: Shows screen 4 - Pre-upgrade tests
; Input...............: Nothing
; Returns.............: Nothing
.proc screen4_show
    ; Init screen
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str4_header

    ; Run pre-upgrade checks
    jsr rom_run_prechecks
    jsr vera_run_prechecks
    jsr smc_run_prechecks

    ; Add Back button (index 0)
    lda #13
    jsr CHROUT
    ldx #<str4_btn_back
    ldy #>str4_btn_back
    lda #str4_btn_back_end-str4_btn_back
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Next button, if no failed pre-checks (index 1)
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_FAIL
    beq listen
    lda vera_status
    cmp #VERA_STATUS_PRECHECK_FAIL
    beq listen
    lda smc_status
    cmp #SMC_STATUS_PRECHECK_FAIL
    beq listen

    ldx #<str4_btn_next
    ldy #>str4_btn_next
    lda #str4_btn_next_end-str4_btn_next
    jsr widget_add_button
    
listen:
    jsr widget_listen

    ; Back button?
    cpx #0
    bne :+
    jmp screen2_show

    ; Next button?
:   cpx #1
    bne listen
    
    jmp screen6_show
.endproc
