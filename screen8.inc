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
; Function name.......: screen8_show
; Purpose.............: Shows screen 8 - Emulator presence
; Input...............: Nothing
; Returns.............: Nothing
.proc screen8_show
    ; Check if running in emulator
    lda EMULATOR_PRESENCE
    cmp #$36
    beq :+

    ; Return if emulator not detected
    rts

:   ; Print header
    jsr screen_header
    print 0, str8_header
    print 0, str8_emulator_detected

    ; Clear and init widgets
    jsr widget_init

    ldx #<str8_btn_exit
    ldy #>str8_btn_exit
    lda #str8_btn_exit_end-str8_btn_exit
    jsr widget_add_button
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ldx #<str8_btn_ignore
    ldy #>str8_btn_ignore
    lda #str8_btn_ignore_end-str8_btn_ignore
    jsr widget_add_button

listen:
    jsr widget_listen
    cpx #0
    beq exit
    cpx #1
    bne listen

ignore:
    rts

exit:
    lda #1
    sta main_exit_request
    rts
.endproc
