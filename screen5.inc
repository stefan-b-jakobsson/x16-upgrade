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

    ; Install new ROM
    jsr rom_update
    rts
.endproc

