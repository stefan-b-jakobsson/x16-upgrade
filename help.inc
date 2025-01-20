HELP_SCREEN_WIDTH = 64
HELP_SCREEN_HEIGHT = 50
HELP_BOARD_WIDTH = 30
HELP_BOARD_HEIGHT = 30

;******************************************************************************
; Function name.......: help_init
; Purpose.............: Initializes help screen buffer
; Input...............: Nothing
; Returns.............: Nothing
.proc help_init
    ; Clear help screen buffer, start address $0:0000
    stz VERA_ADDR0
    stz VERA_ADDR1
    lda #%00010000
    sta VERA_ADDR2
    ldx #HELP_SCREEN_WIDTH
:   lda #32
    ldy #1
:   sta VERA_D0
    sty VERA_D0
    dex
    bne :-
    ldx #HELP_SCREEN_WIDTH
    stz VERA_ADDR0
    inc VERA_ADDR1
    lda VERA_ADDR1
    cmp #HELP_SCREEN_HEIGHT
    bne :--

    ; Copy board image to screen buffer
    lda #<help_board_img
    sta tmp1
    lda #>help_board_img
    sta tmp1+1

    stz VERA_ADDR0
    stz VERA_ADDR1
    lda #%00010000
    sta VERA_ADDR2

    ldy #0
loop:
    lda (tmp1),y
    sta VERA_D0
    iny
    cpy #HELP_BOARD_WIDTH*2
    bne loop

    clc
    tya
    adc tmp1
    sta tmp1
    lda tmp1+1
    adc #0
    sta tmp1+1

    ldy #0

    stz VERA_ADDR0
    inc VERA_ADDR1
    lda VERA_ADDR1
    cmp #HELP_BOARD_HEIGHT
    bne loop

    ; Copy credits for the graphics to screen buffer
    lda #2
    sta VERA_ADDR0
    lda #HELP_BOARD_HEIGHT
    sta VERA_ADDR1
    lda #%00100000
    sta VERA_ADDR2

    ldx #0
:   lda help_board_credits, x
    beq :+
    sta VERA_D0
    inx
    bra :-

:   rts
.endproc

;******************************************************************************
; Function name.......: help_show
; Purpose.............: Displays help screen
; Input...............: Nothing
; Returns.............: Nothing
.proc help_show
    ; Backup current VERA settings
    lda VERA_ADDR0
    sta help_vera_backup+0
    lda VERA_ADDR1
    sta help_vera_backup+1
    lda VERA_ADDR2
    sta help_vera_backup+2
    lda VERA_CTRL
    sta help_vera_backup+3
    
    lda #2
    sta VERA_CTRL
    lda VERA_DC_HSTART
    sta help_vera_backup+7
    lda VERA_DC_HSTOP
    sta help_vera_backup+8
    lda VERA_DC_VSTART
    sta help_vera_backup+9
    lda VERA_DC_VSTOP
    sta help_vera_backup+10
    
    stz VERA_CTRL
    lda VERA_DC_HSCALE
    sta help_vera_backup+4
    lda VERA_DC_VSCALE
    sta help_vera_backup+5
    lda VERA_DC_BORDER
    sta help_vera_backup+6

    lda VERA_L1_MAPBASE
    sta help_vera_backup+11

    php
    pla
    sta help_vera_backup+12

    ; Init screen buffer
    jsr help_init

    ; Set help screen map base ($0:0000)
    stz VERA_L1_MAPBASE

    ; Config screen mode 8 (64x50)
    stz VERA_CTRL
    lda #$80
    sta VERA_DC_HSCALE
    lda #$80
    sta VERA_DC_VSCALE
    stz VERA_DC_BORDER

    lda #2
    sta VERA_CTRL
    lda #$10
    sta VERA_DC_HSTART
    lda #$90
    sta VERA_DC_HSTOP
    lda #$14
    sta VERA_DC_VSTART
    lda #$dc
    sta VERA_DC_VSTOP

    ; Select PETSCII uppercase/graphics
    lda #$8e
    jsr $ffd2

    ; Disable interrupts
    sei
    rts
.endproc

;******************************************************************************
; Function name.......: help_hide
; Purpose.............: Hides the help screen and restores orignal screen
;                       settings
; Input...............: Nothing
; Returns.............: Nothing
.proc help_hide
    ; Select PETSCII uppercase/lowercase
    lda #$0e
    jsr $ffd2
    
    ; Restore VERA settings from backup
    lda help_vera_backup+0
    sta VERA_ADDR0
    lda help_vera_backup+1
    sta VERA_ADDR1
    lda help_vera_backup+2
    sta VERA_ADDR2
    
    lda #2
    sta VERA_CTRL
    lda help_vera_backup+7
    sta VERA_DC_HSTART
    lda help_vera_backup+8
    sta VERA_DC_HSTOP
    lda help_vera_backup+9
    sta VERA_DC_VSTART
    lda help_vera_backup+10
    sta VERA_DC_VSTOP

    stz VERA_CTRL
    lda help_vera_backup+4
    sta VERA_DC_HSCALE
    lda help_vera_backup+5
    sta VERA_DC_VSCALE
    lda help_vera_backup+6
    sta VERA_DC_BORDER
    
    lda help_vera_backup+11
    sta VERA_L1_MAPBASE

    lda help_vera_backup+3
    sta VERA_CTRL

    lda help_vera_backup+12
    pha
    plp
    
    rts
.endproc

;******************************************************************************
; Function name.......: help_close_j1
; Purpose.............: Show J1 location
; Input...............: Nothing
; Returns.............: Nothing
.proc help_close_j1
    ; J1 = row 28, column 3

    ; Print message
    stz VERA_ADDR1
    ldx #<str_hlp_j1_close
    ldy #>str_hlp_j1_close
    jsr help_print

    ; Toggle colors
:   ldx #3
    ldy #28
    lda #3
    clc
    jsr help_set_color
    jsr help_delay
    jsr util_get_key_code
    cmp #0
    beq :-

    jmp help_hide
.endproc

;******************************************************************************
; Function name.......: help_jp1
; Purpose.............: Show JP1 location
; Input...............: C=0 => Instructions on opening JP1
;                       C=1 => Instructions on closing JP1
; Returns.............: Nothing
.proc help_jp1
    ; JP1 = row 5, column 17
    
    stz VERA_ADDR1
    bcc :+

    ; Close message
    ldx #<str_hlp_jp1_close
    ldy #>str_hlp_jp1_close
    bra :++
    
:   ; Open message
    ldx #<str_hlp_jp1_open
    ldy #>str_hlp_jp1_open

:   jsr help_print

    ; Toggle colors
:   ldx #17
    ldy #5
    lda #4
    clc
    jsr help_set_color
    jsr help_delay
    jsr util_get_key_code
    cmp #0
    beq :-

    jmp help_hide
.endproc

;******************************************************************************
; Function name.......: help_bad_bootloader
; Purpose.............: Show help for bad bootloader v2
; Input...............: A = 0 => Bad bootloader detected after update
;                       A = 1 => Pre-update warning: bad bootloader low risk
;                       A = 2 => Pre-update warning: bad bootloader high risk
;                       A = 3 => Pre-update warning: bad bootloader confirmed
; Returns.............: Nothing
.proc help_bad_bootloader
    ; Position to display enlarged SMC image
    xpos = 9
    ypos = 13

    ; Store input on stack
    pha

    ; Copy SMC image to screen buffer
    lda #xpos*2
    sta VERA_ADDR0
    lda #ypos
    sta VERA_ADDR1
    lda #%00010000
    sta VERA_ADDR2
    
    ldy #0
loop2:
    lda help_smc_img1,y
    sta VERA_D0
    iny
    lda VERA_ADDR0
    cmp #xpos*2+20
    bcc loop2
    lda #xpos*2
    sta VERA_ADDR0
    inc VERA_ADDR1
    lda VERA_ADDR1
    cmp #ypos+8
    bcc loop2
    
    ldy #0
loop3:
    lda help_smc_img2,y
    sta VERA_D0
    iny
    lda VERA_ADDR0
    cmp #xpos*2+20
    bcc loop3
    lda #xpos*2
    sta VERA_ADDR0
    inc VERA_ADDR1
    lda VERA_ADDR1
    cmp #ypos+16
    bcc loop3

    ; Show help text
    stz VERA_ADDR1

    pla
    beq post_update
    cmp #1
    beq pre_update_1
    cmp #2
    beq pre_update_2
    cmp #3
    bne hide ; Invalid input

pre_update_3:
    ldx #<str_hlp_bad_bootloader_header
    ldy #>str_hlp_bad_bootloader_header
    jsr help_print
    ldx #<str_hlp_bad_bootloader_warning_3
    ldy #>str_hlp_bad_bootloader_warning_3
    jsr help_print
    bra pre_update

pre_update_2:
    ldx #<str_hlp_bad_bootloader_header
    ldy #>str_hlp_bad_bootloader_header
    jsr help_print
    ldx #<str_hlp_bad_bootloader_warning_2
    ldy #>str_hlp_bad_bootloader_warning_2
    jsr help_print
    bra pre_update

pre_update_1:
    ldx #<str_hlp_bad_bootloader_header
    ldy #>str_hlp_bad_bootloader_header
    jsr help_print
    ldx #<str_hlp_bad_bootloader_warning_1
    ldy #>str_hlp_bad_bootloader_warning_1
    jsr help_print

pre_update:
    ldx #<str_hlp_bad_bootloader_warning
    ldy #>str_hlp_bad_bootloader_warning
    jsr help_print

:   jsr set_colors
    jsr help_delay
    jsr util_get_key_code
    beq :-

hide:
    jmp help_hide

post_update:
    ldx #<str_hlp_bad_bootloader_header
    ldy #>str_hlp_bad_bootloader_header
    jsr help_print
    ldx #<str_hlp_bad_bootloader
    ldy #>str_hlp_bad_bootloader
    jsr help_print

:   jsr set_colors
    jsr help_delay
    bra :-

set_colors:
    ldx #10
    ldy #25
    lda #3
    sec
    jsr help_set_color

    ldx #18
    ldy #22
    lda #1
    clc
    jmp help_set_color
.endproc

;******************************************************************************
; Function name.......: help_set_color
; Purpose.............: Sets character color at specified location
; Input...............: X = start column
;                       Y = start row
;                       A = length
;                       C = 1 => Don't advance toggle color pointer
; Returns.............: Nothing
.proc help_set_color
    ; Store input
    php
    pha

    ; Set column (color setting)
    txa
    asl
    inc
    sta VERA_ADDR0

    ; Set row
    sty VERA_ADDR1

    ; Set high byte, autoincrement 2
    lda #%00000000
    sta VERA_ADDR2

    ; Prepare loop
    ply ; length
    ldx help_cur_col
    
:   lda VERA_D0
    and #$f0
    ora help_colors,x
    sta VERA_D0
    inc VERA_ADDR0
    inc VERA_ADDR0
    dey
    bne :-

    ; Increment color index
    plp
    bcs exit

    inx
    cpx #6 ; color count
    bcc :+
    ldx #0 ; wrap around to start
:   stx help_cur_col

exit:
    rts
.endproc

;******************************************************************************
; Function name.......: help_print
; Purpose.............: Prints text to help screen
; Input...............: X = Pointer to string, low byte
;                       Y = Pointer to string, high byte
; Returns.............: Nothing
.proc help_print
    lda #31*2
    sta VERA_ADDR0
    lda #%00100000
    sta VERA_ADDR2

    stx tmp1
    sty tmp1+1

    ldy #0
loop:
    lda (tmp1),y
    beq eoi
    cmp #13
    beq eol

    ldx #0
:   cmp pet2scr_in,x
    bcc put
    inx
    bra :-

put:
    adc pet2scr_adjust,x
    sta VERA_D0
    bra next

eol:
    lda #31*2
    sta VERA_ADDR0
    inc VERA_ADDR1

next:
    iny
    bne loop
    inc tmp1+1
    bra loop

eoi:
    rts
.endproc

;******************************************************************************
; Function name.......: help_delay
; Purpose.............: Delay
; Input...............: Nothing
; Returns.............: Nothing
.proc help_delay
    ; Store registers on stack
    php
    pha
    phx
    phy

    ; Delay loop
    lda #2
    ldx #0
    ldy #0
:   dex
    bne :-
    dey
    bne :-
    dec
    bne :-

    ; Restore registers
    ply
    plx
    pla
    plp

    rts
.endproc

;******************************************************************************
; Variables

help_vera_backup:
    .res 3      ; 0: address
    .res 1      ; 3: ctrl
    .res 1      ; 4: hscale
    .res 1      ; 5: vscale
    .res 1      ; 6: border
    .res 1      ; 7: hstart
    .res 1      ; 8: hstop
    .res 1      ; 9: vstart
    .res 1      ; 10: vstop
    .res 1      ; 11: l1_mapbase
    .res 1      ; 12: interrupt flag

help_cur_col:
    .byt $01

help_colors:
    .byt $01    ; White
    .byt $0f    ; Light grey
    .byt $0c    ; Grey
    .byt $0b    ; Dark grey 
    .byt $0c    ; Grey
    .byt $0f    ; Light grey

;******************************************************************************
; X16 board PETSCII graphics (30x30), designed by Xiphod/voidstar
help_board_img:
    .byt $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $09, $0c, $05, $0c, $03, $0c, $62, $01, $62, $01, $62, $01, $7b, $01, $cf, $0c, $d0, $0c, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01
    .byt $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $20, $01, $70, $01, $43, $01, $6e, $01, $a0, $61, $93, $01, $84, $01, $61, $61, $cc, $6c, $fa, $6c, $f4, $6b, $ea, $6b, $f5, $6f, $43, $bf, $43, $bf, $43, $bf, $f6, $0f, $20, $01, $20, $01, $10, $0c, $13, $0c, $32, $0c
    .byt $20, $01, $20, $01, $55, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $42, $01, $20, $01, $42, $01, $a0, $61, $a0, $61, $a0, $61, $61, $61, $20, $61, $20, $61, $cc, $6b, $fa, $6b, $f5, $6f, $16, $bf, $07, $bf, $01, $bf, $f6, $6f, $f4, $6b, $ea, $6b, $70, $cf, $43, $cf, $6e, $cf
    .byt $20, $01, $20, $01, $42, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6d, $61, $43, $61, $7d, $61, $f4, $61, $a0, $61, $a0, $61, $61, $61, $16, $61, $05, $61, $12, $61, $01, $61, $f5, $6f, $f9, $6b, $f9, $6b, $f9, $6b, $f6, $6f, $cc, $6b, $fa, $6b, $42, $cf, $20, $cf, $42, $cf
    .byt $05, $0c, $31, $0c, $42, $61, $05, $6c, $32, $6c, $05, $6c, $33, $6c, $05, $6c, $34, $6c, $20, $61, $20, $61, $20, $61, $f4, $61, $a0, $61, $a0, $61, $61, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61, $20, $61, $20, $61, $6d, $cf, $43, $cf, $73, $cf
    .byt $20, $01, $70, $01, $73, $61, $70, $01, $6e, $01, $70, $01, $6e, $01, $70, $01, $6e, $01, $20, $61, $20, $61, $20, $61, $42, $61, $20, $61, $20, $61, $20, $61, $3a, $61, $3c, $63, $0a, $63, $10, $63, $31, $63, $20, $61, $20, $61, $20, $61, $42, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $20, $61, $20, $61, $6d, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $7d, $61, $20, $61, $01, $6f, $14, $6f, $18, $6f, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $70, $01, $43, $01, $6e, $01, $20, $61, $70, $01, $43, $01, $6e, $01, $20, $61, $70, $01, $43, $01, $6e, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $70, $1f, $6e, $1f, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $36, $01, $42, $01, $20, $61, $42, $01, $16, $01, $42, $01, $20, $61, $42, $01, $16, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6b, $1f, $73, $1f, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $35, $01, $42, $01, $20, $61, $42, $01, $09, $01, $42, $01, $20, $61, $42, $01, $09, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6b, $1f, $73, $1f, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $30, $01, $42, $01, $20, $61, $42, $01, $01, $01, $42, $01, $20, $61, $42, $01, $01, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6b, $1f, $73, $1f, $42, $61
    .byt $20, $01, $42, $01, $42, $61, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $32, $01, $42, $01, $20, $61, $42, $01, $31, $01, $42, $01, $20, $61, $42, $01, $32, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6b, $1f, $73, $1f, $42, $61
    .byt $20, $01, $6d, $01, $73, $61, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $20, $61, $6d, $01, $43, $01, $7d, $01, $20, $61, $6d, $01, $43, $01, $7d, $01, $20, $61, $6d, $01, $43, $01, $7d, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6d, $1f, $7d, $1f, $42, $61
    .byt $20, $01, $55, $61, $4b, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $df, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $e9, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $f4, $61, $df, $61, $20, $61, $20, $61, $20, $61, $e9, $61, $e7, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $e1, $61, $a0, $61, $df, $61, $20, $61, $e9, $61, $a0, $61, $61, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $77, $61, $fb, $61, $20, $61, $ec, $61, $77, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6f, $61, $fe, $61, $20, $61, $fc, $61, $6f, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $6a, $61, $a0, $61, $69, $61, $20, $61, $5f, $61, $a0, $61, $74, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $e1, $61, $69, $61, $20, $61, $20, $61, $20, $61, $5f, $61, $75, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $70, $01, $43, $01, $6e, $01, $70, $01, $6e, $01, $70, $01, $6e, $01, $70, $01, $6e, $01, $70, $01, $6e, $01, $70, $01, $6e, $01, $20, $61, $70, $01, $43, $01, $6e, $01, $20, $60, $13, $6f, $0d, $6f, $03, $6f, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $42, $01, $12, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $19, $01, $42, $01, $70, $01, $6e, $01, $20, $6f, $20, $6f, $20, $6f, $20, $6f, $20, $6f, $20, $6f, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $42, $01, $0f, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $42, $01, $0d, $01, $42, $01, $42, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $42, $01, $0d, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $6d, $01, $43, $01, $7d, $01, $6d, $01, $7d, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $47, $61, $42, $01, $20, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $42, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $10, $14, $12, $14, $30, $14, $30, $14, $31, $14, $f6, $61, $55, $01, $49, $01, $42, $61
    .byt $20, $01, $47, $61, $6d, $01, $43, $01, $7d, $01, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $6d, $01, $7d, $01, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $06, $6f, $10, $6f, $20, $61, $09, $6f, $32, $6f, $03, $6f, $4a, $01, $4b, $01, $42, $61
    .byt $20, $01, $47, $61, $20, $61, $0a, $63, $31, $63, $3e, $63, $3a, $61, $20, $61, $20, $61, $20, $61, $51, $12, $20, $61, $51, $12, $20, $61, $51, $12, $20, $61, $20, $61, $51, $67, $20, $61, $51, $62, $20, $61, $20, $61, $66, $01, $20, $61, $20, $61, $66, $01, $20, $61, $20, $61, $20, $61, $42, $61
    .byt $20, $01, $4a, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $4b, $61
    
help_board_credits:
    .byt $07, $06, $18, $20, $02, $19, $20, $18, $09, $10, $08, $0f, $04, $00       

;******************************************************************************
; SMC chip PETSCII graphics
help_smc_img1:
    .byt $55, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $49, $61, $20, $61
    .byt $42, $61, $20, $61, $20, $61, $13, $6f, $0d, $6f, $03, $6f, $20, $61, $20, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $20, $61, $70, $01, $43, $01, $6e, $01, $20, $61, $20, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $31, $61, $73, $01, $01, $01, $6b, $01, $32, $61, $30, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $32, $61, $73, $01, $14, $01, $6b, $01, $31, $61, $39, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $33, $61, $73, $01, $14, $01, $6b, $01, $31, $61, $38, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $34, $61, $73, $01, $09, $01, $6b, $01, $31, $61, $37, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $35, $61, $73, $01, $0e, $01, $6b, $01, $31, $61, $36, $61, $42, $61, $20, $61

help_smc_img2:
    .byt $42, $61, $20, $61, $36, $61, $73, $01, $19, $01, $6b, $01, $31, $61, $35, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $37, $61, $73, $01, $20, $01, $6b, $01, $31, $61, $34, $61, $42, $61, $1f, $61
    .byt $42, $61, $20, $61, $38, $61, $73, $01, $38, $01, $6b, $01, $31, $61, $33, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $39, $61, $73, $01, $36, $01, $6b, $01, $31, $61, $32, $61, $42, $61, $20, $61
    .byt $42, $61, $31, $61, $30, $61, $73, $01, $31, $01, $6b, $01, $31, $61, $31, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $20, $61, $6d, $01, $43, $01, $7d, $01, $20, $61, $20, $61, $42, $61, $20, $61
    .byt $42, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $20, $61, $42, $61, $20, $61
    .byt $4a, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $43, $61, $4b, $61, $20, $61
