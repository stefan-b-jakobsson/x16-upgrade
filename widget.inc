; Copyright (c) 2024-2025, Stefan Jakobsson

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


WIDGET_TYPE_BUTTON = 0
WIDGET_TYPE_CHKBOX = 1

WIDGET_STATE_INACTIVE = 0
WIDGET_STATE_ACTIVE = 1
WIDGET_STATE_SELECTED = 2

;******************************************************************************
; Function name.......: widget_init
; Purpose.............: Clears and inits widget functions
; Input...............: Nothing
; Returns.............: Nothing
.proc widget_init
    stz widget_count
    lda #$ff
    sta widget_sel_index
    rts
.endproc

;******************************************************************************
; Function name.......: widget_add_button
; Purpose.............: Adds a button at the cursor position
; Input...............: X = String, LSB
;                       Y = String, MSB
;                       A = String len
; Returns.............: X = Widget index
; Error...............: C=1 if could not add widget
.proc widget_add_button
    ; Store input
    stx tmp1
    sty tmp1+1
    pha

    ; Set widget position, size, type, and state
    sec
    jsr PLOT
    txa
    ldx widget_count
    sta widget_y,x
    tya
    sta widget_x,x
    pla
    sta widget_len,x
    lda #WIDGET_TYPE_BUTTON
    sta widget_type,x
    lda #WIDGET_STATE_ACTIVE
    sta widget_state,x

    ; Print widget
    ldy #0
:   lda (tmp1),y
    jsr CHROUT
    iny
    tya
    cmp widget_len,x
    bne :-

    ; Print space
    lda #32
    jsr CHROUT

    ; Increment index
    inc widget_count
    
    ; Return, X still holds widget index
    clc
    rts
.endproc

;******************************************************************************
; Function name.......: widget_add_chkbox
; Purpose.............: Adds a checkbox at the cursor position
; Input...............: A = value
;                           0: off
;                           1: on
; Returns.............: X = Widget index
; Error...............: C=1 if could not add widget
.proc widget_add_chkbox
    pha

    ldx #<on
    ldy #>on
    cmp #0
    
    bne :+
    ldx #<off
    ldy #>off

:   lda #3
    jsr widget_add_button
    lda #WIDGET_TYPE_CHKBOX
    sta widget_type,x
    pla
    sta widget_value,x

    rts

on:
    .byt "[x]"

off:
    .byt "[ ]"

.endproc

;******************************************************************************
; Function name.......: widget_chkbox_toggle
; Purpose.............: Toggles checkbox on/off
; Input...............: X = index of checkbox widget
; Returns.............: Nothing
.proc widget_chkbox_toggle
    lda widget_type,x
    cmp #WIDGET_TYPE_CHKBOX
    beq :+
    rts

:   lda widget_value,x
    bne off

on:
    lda #1
    sta widget_value,x
    lda #'x'
    bra show

off:
    lda #0
    sta widget_value,x
    lda #' '

show:
    pha
    lda widget_x,x
    inc
    tay
    lda widget_y,x
    tax
    clc
    jsr PLOT
    pla
    jmp CHROUT

.endproc

;******************************************************************************
; Function name.......: widget_set_state
; Purpose.............: Sets state of specified widget
; Input...............: X = Widget index
;                       Y = Widget state
;                           0: Inactive
;                           1: Active
;                           2: Selected
; Returns.............: Nothing
; Error...............: C=1 if could not add widget
.proc widget_set_state
    cpy #WIDGET_STATE_INACTIVE
    bne :+
    lda #111
    bra cont

:   cpy #WIDGET_STATE_ACTIVE
    bne :+
    lda #97
    bra cont

:   cpy #WIDGET_STATE_SELECTED
    bne exit
    stx widget_sel_index
    lda #160

cont:   
    ; Save color
    sta r0

    ; Set state
    tya
    sta widget_state,x

    ; Set color
    lda widget_y,x
    tay
    lda widget_len,x
    pha
    lda widget_x,x
    tax
    pla
    jmp util_set_color

exit:
    rts
.endproc

;******************************************************************************
; Function name.......: widget_next
; Purpose.............: Selects next widget
; Input...............: Nothing
; Returns.............: Nothing
.proc widget_next
    ldx widget_sel_index
    cpx widget_count
    bcc unselect
    ldx #0
    bra select

unselect:
    ldy #WIDGET_STATE_ACTIVE
    jsr widget_set_state

    inc widget_sel_index
    ldx widget_sel_index
    cpx widget_count
    bcc select
    ldx #0

select:
    stx widget_sel_index
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state
    rts
.endproc

;******************************************************************************
; Function name.......: widget_prev
; Purpose.............: Selects previous widget
; Input...............: Nothing
; Returns.............: Nothing
.proc widget_prev
    ldx widget_sel_index
    cpx widget_count
    bcc unselect
    ldx #0
    bra select

unselect:
    ldy #WIDGET_STATE_ACTIVE
    jsr widget_set_state

    ldx widget_sel_index
    bne :+
    ldx widget_count
:   dex
    stx widget_sel_index

select:
    stx widget_sel_index
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state
    rts
.endproc

;******************************************************************************
; Function name.......: widget_listen
; Purpose.............: Listens for keyboard action, returns when Enter key
;                       is pressed
; Input...............: Nothing
; Returns.............: X = Index of active widget on return
.proc widget_listen
loop:
    jsr GETIN
    beq loop

    cmp #9
    beq @1
    cmp #$1d
    bne @2
@1: jsr widget_next
    bra loop

@2: cmp #$18
    beq @3
    cmp #$9d
    bne @4 
@3: jsr widget_prev
    bra loop

@4: cmp #13
    bne loop

    ldx widget_sel_index
    
    lda widget_type,x
    cmp #WIDGET_TYPE_BUTTON
    beq exit
    cmp #WIDGET_TYPE_CHKBOX
    bne loop
    jsr widget_chkbox_toggle
    bra loop

exit:
    rts
.endproc

;******************************************************************************
; Global variables
widget_count: .res 1
widget_sel_index: .res 1
widget_x: .res 5
widget_y: .res 5
widget_len: .res 5
widget_type: .res 5
widget_state: .res 5
widget_value: .res 5
