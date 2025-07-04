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


;******************************************************************************
; Function name.......: screen7_show
; Purpose.............: Shows screen 7 - Prompt user to uninstall JP1
; Input...............: Nothing
; Returns.............: Nothing
.proc screen7_show
    ; Check if JP is installed - Return if not
    jsr vera_jedec
    bcc :+
    rts

    ; Clear and init widgets
:   jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str7_header

    ; Print message
    print 0, str7_jp1

    ; Add OK button
    ldx #<str7_btn_ok
    ldy #>str7_btn_ok
    lda #str7_btn_ok_end-str7_btn_ok
    jsr widget_add_button
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add help button
    ldx #<str7_btn_help
    ldy #>str7_btn_help
    lda #str7_btn_help_end-str7_btn_help
    jsr widget_add_button

    ; Listen
listen:
    jsr widget_listen
    
    ; OK button?
    cpx #0
    bne :+

    jsr vera_jedec  ; Check JP1 status: exit if uninstalled (C=1)
    bcc listen
    rts

    ; Help button?
:   cpx #1
    bne listen

    jsr help_show
    clc
    jsr help_jp1
    ldx #0
    ldy #WIDGET_STATE_SELECTED 
    jsr widget_set_state
    ldx #1
    ldy #WIDGET_STATE_ACTIVE
    jsr widget_set_state
    bra listen
.endproc
    