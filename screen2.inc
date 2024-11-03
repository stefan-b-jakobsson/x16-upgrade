;******************************************************************************
; Function name.......: screen2_show
; Purpose.............: Shows screen 2 - Information and update summary
; Input...............: Nothing
; Returns.............: Nothing
.proc screen2_show
    ; Init screen
    jsr widget_init

    ; Print header
    jsr screen_header

    ; Print package information
    print 0, str2_summary
    print 0, header_pkg_info
    print 0, str2_pgk_by
    print 0, header_created_by
    lda #13
    jsr CHROUT
    ldx #0
@0: phx
    cpx #4
    beq @1 
    cpx #6
    beq @1
    cpx #8
    beq @2
    cpx #10
    beq @3
    cpx #12
    beq @5
    bra @4
@1: lda #'-'
    jsr CHROUT
    bra @4
@2: lda #' '
    jsr CHROUT
    bra @4
@3: lda #':'
    jsr CHROUT
@4: plx
    phx
    lda header_created_on,x
    jsr CHROUT
    plx
    inx
    bra @0
@5: plx
    print 0, str2_pkg_utc
    print 0, str2_pkg_components

    ; Print ROM version
    print 0, str2_rom
    lda rom_status
    bne :+
    print 0, str2_skip
    bra :++
:   lda #'R'
    jsr CHROUT
    lda header_rom_ver
    jsr util_print_num
:   lda #13
    jsr CHROUT

    ; Print VERA version
    print 0, str2_vera
    lda vera_status
    bne :+
    print 0, str2_skip
    bra :++
:   lda header_vera_ver
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_vera_ver+1 
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_vera_ver+2
    jsr util_print_num
    
:   lda #13
    jsr CHROUT

    ; Print SMC version
    print 0, str2_smc
    lda smc_status
    bne :+
    print 0, str2_skip
    bra :++

:   lda header_smc_ver
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_smc_ver+1
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_smc_ver+2
    jsr util_print_num

:   lda #13
    jsr CHROUT
    lda #13
    jsr CHROUT

    ; Add Exit button (index 0)
    ldx #<str2_btn_exit
    ldy #>str2_btn_exit
    lda #str2_btn_exit_end-str2_btn_exit
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Custom button (index 1)
    ldx #<str2_btn_custom
    ldy #>str2_btn_custom
    lda #str2_btn_custom_end-str2_btn_custom
    jsr widget_add_button

    ; Add Next button (index 2)
    ldx #<str2_btn_next
    ldy #>str2_btn_next
    lda #str2_btn_next_end-str2_btn_next
    jsr widget_add_button

    ; Listen for key events
loop:
    jsr widget_listen

    ; Exit button?
    cpx #0
    beq exit

    ; Custom button?
    cpx #1
    beq custom

    ; Next button?
    cpx #2
    bne loop

next:
    jmp screen4_show

custom:
    jmp screen3_show

exit:
    rts
.endproc
