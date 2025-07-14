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
; Function name.......: screen4_show
; Purpose.............: Shows screen 4 - Pre-upgrade tests
; Input...............: Nothing
; Returns.............: Nothing
.proc screen4_show
    ; Clear and init widgets
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str4_header

    ; Run pre-upgrade checks
    jsr rom_run_prechecks
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_FAIL
    beq add_buttons ; abort

    jsr smc_run_prechecks
    lda smc_status
    cmp #SMC_STATUS_PRECHECK_FAIL
    beq add_buttons ; abort

    jsr vera_run_prechecks

add_buttons:
    ; Add Back button (index 0)
    lda #13
    jsr CHROUT
    ldx #<str4_btn_back
    ldy #>str4_btn_back
    lda #str4_btn_back_end-str4_btn_back
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Next button, if no failed pre-checks (index 1)
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_FAIL
    beq listen
    lda vera_status
    cmp #VERA_STATUS_PRECHECK_FAIL
    beq listen
    lda smc_status
    cmp #SMC_STATUS_PRECHECK_FAIL
    beq listen

    ldx #<str4_btn_next
    ldy #>str4_btn_next
    lda #str4_btn_next_end-str4_btn_next
    jsr widget_add_button
    
listen:
    jsr widget_listen

    ; Back button?
    cpx #0
    bne :+
    jmp screen2_show

    ; Next button?
:   cpx #1
    bne listen
    
    jmp screen6_show
.endproc
