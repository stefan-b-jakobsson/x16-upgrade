;******************************************************************************
; Function name.......: screen5_show
; Purpose.............: Shows screen 5 - System upgrade
; Input...............: Nothing
; Returns.............: Nothing
.proc screen5_show
    ; Init screen
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str5_header

    ; Install Kernal ROM
    jsr rom_update
    jsr rom_verify

    ; Install SMC
    jsr smc_update
    jsr smc_verify
    jsr smc_reboot

    ; Cold start
    print 0, rstmsg
    ldx #0
    ldy #0
    lda #5
    jsr util_countdown
    sec
    jmp $ff47 

rstmsg:
    .byt 13, "Reset... ", 0
.endproc

