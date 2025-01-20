; Copyright (c) 2024, Stefan Jakobsson

; Redistribution and use in source and binary forms, with or without 
; modification, are permitted provided that the following conditions are met:

; (1) Redistributions of source code must retain the above copyright notice, 
;     this list of conditions and the following disclaimer. 
;
; (2) Redistributions in binary form must reproduce the above copyright notice,
;     this list of conditions and the following disclaimer in the documentation
;     and/or other materials provided with the distribution.

; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.


;******************************************************************************
; Function name.......: util_init
; Purpose.............: Inits util functions
; Input...............: Nothing
; Returns.............: Nothing
.proc util_init
    ; Get screen dimensions
    jsr SCREEN
    stx screen_width
    sty screen_height

    ; Set VERA address to top left corner
    stz VERA_ADDR0
    lda #$b0
    sta VERA_ADDR1
    lda #%00100001
    sta VERA_ADDR2
    rts
.endproc

;******************************************************************************
; Function name.......: util_hex2bin
; Purpose.............: Converts a null-terminated string containing a 
;                       HEX formatted number to a 32 bit binary number
; Input...............: X/Y = Pointer to string (LSB/MSB)
; Returns.............: r5/r6 = 32 bit binary number (LSB/MSB)
; Errors..............: C=1 on the following errors:
;                       - String contains illegal characters for a HEX number
;                       - String is too long (>8 chars)
.proc util_hex2bin
    ; Store input
    stx tmp1
    sty tmp1+1

    ; Clear output
    stz r5
    stz r5+1
    stz r6
    stz r6+1

    ; Find end of string
    ldy #0
    ldx #0
:   lda (tmp1),y
    beq loop
    iny
    bra :-
    cpy #9
    bcs err

loop:
    ; Check if done
    cpy #0
    beq exit
    dey

    ; Low nibble
    lda (tmp1),y
    jsr convert
    sta r5,x

    ; Check if done
    cpy #0
    beq exit
    dey

    ; High nibble
    lda (tmp1),y
    jsr convert
    asl
    asl
    asl
    asl
    ora r5,x
    sta r5,x

    ; Next
    inx
    bra loop

exit:
    clc
    rts

err:
    sec
    rts

convert:
    ; Converts one HEX char to a binary value in the range 0..15
    sec
    sbc #48
    cmp #10
    bcc :+

    sbc #7
    cmp #16
    bcc :+

    ; Illegal character: Restore stack, exit with error
    pla
    pla
    sec

:   rts

.endproc

;******************************************************************************
; Function name.......: util_print_str
; Purpose.............: Prints a null terminated string with word wrap
; Input...............: X = Pointer to string, LSB
;                       Y = Pointer to string, MSB
;                       A = indentation
; Returns.............: Nothing
.proc util_print_str
    ; Store input
    stx tmp1
    sty tmp1+1
    sta indent

    ; Prepare
    ldx indent ; X = column index
    ldy #0 ; Y = buffer index
    stz mark

loop:
    lda (tmp1),y
    beq pr ; End of input; print last line
    cmp #13 ; Is CR?
    beq cr
    cmp #32 ; Is blank space?
    bne next

setmark:
    stx mark ; Mark column where word wrap is possible

next:
    ; Increment indexes
    inx
    iny

    ; Continue loop, if X <= SCREEN_WIDTH
    cpx screen_width
    bcc loop
    beq loop

    ; Check if mark is within the line
    ldx mark
    cpx screen_width
    bcc :+

    ; Mark is not within line; insert no line break
    jsr pr
    bra loop
    
:   ; Mark is within line; insert line break
    jsr pr
    lda #13
    jsr CHROUT
    bra loop

cr:
    cpx indent
    bcc :+
    beq :+

    jsr pr
    lda #13
    jsr CHROUT
    bra loop

:   lda #13
    jsr CHROUT
    inc tmp1
    bne loop
    inc tmp1+1
    bra loop

pr:
    ; in: X = number of chars to print

    cpx indent
    beq print_done
    bcc print_done

    ; Indentation
    ldy indent
    beq :++
:   lda #32
    jsr CHROUT
    dex
    dey
    bne :-

    ; Print line
    ldy #0
:   lda (tmp1),y
    jsr CHROUT
    iny
    dex
    bne :- 

    ; Set buffer to start of next line
    clc
    tya
    inc
    adc tmp1
    sta tmp1
    lda tmp1+1
    adc #0
    sta tmp1+1

    ; Prepare for next line
    ldy #0
    ldx indent
    stz mark

print_done:
    rts

mark:
    .res 1
indent:
    .res 1
.endproc

;******************************************************************************
; Function name.......: util_print_num
; Purpose.............: Prints an 8 bit integer in decimal format
; Input...............: A = value
; Returns.............: Nothing
.proc util_print_num
    ; Special case, A = 0
    cmp #0
    bne :+
    lda #$30
    jmp CHROUT

    ; Store 8 bit input
:   sta input
 
    ; Clear 16 bit output
    stz output
    stz output+1
 
    ; Number of input bits
    ldx #8
 
    ; Set decimal mode
    sed
 
convert:
    ; Rotate input, leftmost bit -> C
    asl input
   
    ; Calculate output + C (16 bit operation)
    lda output
    adc output
    sta output
   
    lda output+1
    adc output+1
    sta output+1
 
    ; Decrease bit counter, continue if > 0
    dex
    bne convert
 
    ; Go back to binary mode
    cld
 
    ; Prepare to print digit
    ldx #3
    stz suppress

print_digit:
    ; Clear upper nibble
    lda output+1
    and #15
    
    ; Suppress output of leading zeros
    ldy suppress
    bne :+
    cmp #0
    beq next
    inc suppress

    ; Print
:   clc
    adc #48
    jsr CHROUT
    
next:
    ; Check if done
    dex
    beq exit
    
    ; Fetch digit into output+1
    ldy #4
:   asl output
    rol output+1
    dey
    bne :-
    bra print_digit
  
exit:
    rts
 
input: .res 1
output: .res 2
suppress: .res 1
.endproc

;******************************************************************************
; Function name.......: util_chrout
; Purpose.............: Prints a character to the screen
; Input...............: A = character PETSCII value
; Returns.............: Nothing
.proc util_chrout
    ; Save input
    pha
    phx
    phy

    ; Check if CR
    cmp #13
    beq CR

    ; Check if backspace
    cmp #20
    beq backspace

    ; Check if Clear Screen
    cmp #147
    bne :+
    jmp clrscr

    ; Convert PETSCII to screen code
:   ldy #0
:   cmp pet2scr_in,y
    bcc :+
    iny
    cpy #8
    bne :-
    jmp exit
:   clc
    adc pet2scr_adjust,y

store:
    sta VERA_D0
    lda VERA_ADDR0
    lsr
    cmp screen_width
    bcs CR
    jmp exit

CR: 
    stz VERA_ADDR0
    inc VERA_ADDR1
    
    clc
    lda screen_height
    dec
    adc #$b0
    cmp VERA_ADDR1
    beq scroll
    bcc scroll
    jmp exit

backspace:
    lda VERA_ADDR0
    cmp #2
    bcc :+
    dec VERA_ADDR0
    dec VERA_ADDR0
:   jmp exit

scroll:
    stz VERA_ADDR0
    lda #$b0+2
    sta VERA_ADDR1
    lda #$01
    sta VERA_ADDR2

    ldy screen_height
    dey
    dey
    dey
    lda screen_width
    asl
    tax

:   lda VERA_D0
    dec VERA_ADDR1
    sta VERA_D0
    inc VERA_ADDR0
    inc VERA_ADDR1
    dex
    bne :-

    lda screen_width
    asl
    tax
    stz VERA_ADDR0
    dey
    beq :+
    inc VERA_ADDR1
    bra :-

:   lda #%00100001
    sta VERA_ADDR2

    lda #32
    ldx screen_width
:   sta VERA_D0
    dex
    bne :-
    stz VERA_ADDR0
    bra exit

clrscr:
    stz VERA_ADDR0
    lda #$b0
    sta VERA_ADDR1
    lda #%00010001
    sta VERA_ADDR2
    ldy #97
:   lda #32
    ldx screen_width
:   sta VERA_D0
    sty VERA_D0
    dex
    bne :-
    clc
    lda screen_height
    adc #$b0
    cmp VERA_ADDR1
    bcs :--
    stz VERA_ADDR0
    lda #$b0
    sta VERA_ADDR1
    lda #%00100001
    sta VERA_ADDR2
    
exit:
    ; Restore input
    ply
    plx
    pla

    rts
.endproc

;******************************************************************************
; Function name.......: util_plot
; Purpose.............: Gets or sets cursor position
; Input...............: C=0 set position, or C=1 get position
;                       Y = column
;                       X = row
; Returns.............: Y = column
;                       X = row
.proc util_plot
    bcc setpos

getpos:
    lda VERA_ADDR0
    lsr
    tay
    
    sec
    lda VERA_ADDR1
    sbc #$b0
    tax
    rts

setpos:
    tya
    asl
    sta VERA_ADDR0
    txa
    clc
    adc #$b0
    sta VERA_ADDR1
    rts

.endproc


;******************************************************************************
; Function name.......: util_set_color
; Purpose.............: Sets color at specified screen location
; Input...............: X = Start column
;                       Y = Row
;                       A = Len
;                       r0 = Color
; Returns.............: Nothing
.proc util_set_color
    ; Store input
    sta counter

    ; Preserve VERA address
    lda VERA_ADDR0
    pha
    lda VERA_ADDR1
    pha
    lda VERA_ADDR2
    pha

    ; Set VERA address
    txa
    asl
    inc
    sta VERA_ADDR0
    tya
    clc
    adc #$b0
    sta VERA_ADDR1
    lda #%00100001
    sta VERA_ADDR2

    lda r0
loop:
    sta VERA_D0
    dec counter
    bne loop

    ; Restore VERA address
    pla
    sta VERA_ADDR2
    pla
    sta VERA_ADDR1
    pla
    sta VERA_ADDR0

    ; Exit
    rts

counter: .res 1
.endproc

;******************************************************************************
; Function name.......: util_countdown
; Purpose.............: Executes and displays a countdown.     
;                       After each step, a possible callback function is run.
;                       X is set to the counter value before jumping to the
;                       callback function. The callback is expected to return
;                       with a RTS instruction,
; Input...............: A = Start value
;                       X = Pointer to callback after each step, LSB
;                       Y = Pointer to callback after each step, MSB
;                       X/Y pointing to $0000 = no callback
; Returns.............: Nothing
.proc util_countdown
    sta counter
    stx callback
    sty callback+1

    ; Print
    lda counter
    jsr util_print_num

loop:
    ; Exit if done
    lda counter
    bne :+
    rts
    
    ; Delay
:   jsr util_delay_1s

    ; Decrement counter
    dec counter

    ; Restore cursor
    ldx #1
    lda counter
    cmp #9
    bcc :+
    inx
    cmp #99
    bcc :+
    inx

 :  lda #20
    jsr CHROUT
    dex
    bne :-

    ; Print
    lda counter
    jsr util_print_num

    ; Callback is set?
    lda callback
    ora callback+1
    beq loop ; no callback

    ; Push return address on stack
    lda #>(loop-1)
    pha
    lda #<(loop-1)
    pha

    ; Jump to callback, expected to return with RTS
    ldx counter
    jmp (callback)

counter:
    .res 1

callback:
    .res 2
.endproc

;******************************************************************************
; Function name.......: util_delay_1s
; Purpose.............: Delays approx 1 second
; Input...............: Nothing
; Returns.............: Nothing
.proc util_delay_1s
    lda #168
    sta counter
    lda #70
    sta counter+1
    lda #14
    sta counter+2

loop:
    dec counter         ; 6
    bne loop            ; 3     9x
    dec counter+1       ; 6
    bne loop            ; 3     9x / 256
    dec counter+2       ; 6
    bne loop            ; 3     9x / 65535
    rts                 ; =>    9x + 9x/256 + 9x/65536 = 8000000 => 65536x + 256x + x = 8000000/9*65536 => x = 885416
    
    counter: .res 3
.endproc

;******************************************************************************
; Function name.......: util_wait_keypress
; Purpose.............: Waits for keypress; reads keycodes directly over I2C
; Input...............: Nothing
; Returns.............: A = keycode
.proc util_wait_keypress
    ; Read keycodes until buffer is empty
:   ldx #SMC_ADDR
    ldy #SMC_GET_KEY
    jsr I2C_READ_BYTE
    cmp #0
    bne :-

    ; Wait for key down event
:   jsr util_get_key_code
    beq :-

    rts
.endproc

;******************************************************************************
; Function name.......: util_get_key_code
; Purpose.............: Get key code directly from SMC; key up events ignored
; Input...............: Nothing
; Returns.............: A = key code
.proc util_get_key_code
    ldx #SMC_ADDR
    ldy #SMC_GET_KEY
    jsr I2C_READ_BYTE
    ora #0
    bmi keyup
    rts
keyup:
    lda #0
    rts
.endproc


screen_height: .res 1
screen_width: .res 1

pet2scr_in:
    .byt $20, $40, $60, $80, $a0, $c0, $e0, $ff

pet2scr_adjust:
    .byt $80, $00, $c0, $e0, $40, $c0, $80, $80

.ZEROPAGE
    tmp1: .res 2
.CODE
