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
; Function name.......: screen3_show
; Purpose.............: Shows screen 3 - Custom options
; Input...............: Nothing
; Returns.............: Nothing
.proc screen3_show
    ; Clear and init widgets
    jsr widget_init

    ; Print header and summary
    jsr screen_header

    print 0, str3_info
    
    lda rom_status
    jsr widget_add_chkbox
    print 0, str3_rom

    lda vera_status
    jsr widget_add_chkbox
    print 0, str3_vera

    lda smc_status
    jsr widget_add_chkbox
    print 0, str3_smc

    ; Add line break
    lda #13
    jsr CHROUT

    ; Add Cancel button (index 3)
    ldx #<str3_btn_cancel
    ldy #>str3_btn_cancel
    lda #str3_btn_cancel_end-str3_btn_cancel
    jsr widget_add_button

    ; Add OK button (index 4)
    ldx #<str3_btn_ok
    ldy #>str3_btn_ok
    lda #str3_btn_ok_end-str3_btn_ok
    jsr widget_add_button

    ; Select Cancel button
    ldx #3
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

loop:
    ; Listen for key events
    jsr widget_listen

    ; Cancel button?
    cpx #3
    beq cancel

    ; OK button?
    cpx #4
    bne loop

ok:
    ldx #0
    lda widget_value,x
    sta rom_status

    inx
    lda widget_value,x
    sta vera_status

    inx
    lda widget_value,x
    sta smc_status

cancel:
    jmp screen2_show

.endproc
