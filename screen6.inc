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
; Function name.......: screen6_show
; Purpose.............: Shows screen 6 - Warning/Disclaimer
; Input...............: Nothing
; Returns.............: Nothing
.proc screen6_show
    ; Clear and init widgets
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str6_disclaimer

    ; Add Exit button
    ldx #<str6_btn_exit
    ldy #>str6_btn_exit
    lda #str6_btn_exit_end-str6_btn_exit
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Install button
    ldx #<str6_btn_install
    ldy #>str6_btn_install
    lda #str6_btn_install_end-str6_btn_install
    jsr widget_add_button

listen:
    jsr widget_listen

    ; Exit button?
    cpx #0
    bne :+
    rts

:   ; Install button
    cpx #1
    bne listen

    jmp screen5_show

.endproc
