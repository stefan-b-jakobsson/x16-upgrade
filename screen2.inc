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
; Function name.......: screen2_show
; Purpose.............: Shows screen 2 - Information and update summary
; Input...............: Nothing
; Returns.............: Nothing
.proc screen2_show
    ; Clear and init widgets
    jsr widget_init

    ; Print header
    jsr screen_header

    ; Print package information
    print 0, str2_summary           ; Summary
    print 0, header_pkg_info

    print 0, str2_pgk_by            ; Created by
    print 0, header_created_by
    lda #13
    jsr CHROUT
    
    ldx #0                          ; Created on UTC, formatted as "YYYY-mm-dd HH:SS UTC"
@0: phx
    cpx #4
    beq @1 
    cpx #6
    beq @1
    cpx #8
    beq @2
    cpx #10
    beq @3
    cpx #12
    beq @5
    bra @4
@1: lda #'-'
    jsr CHROUT
    bra @4
@2: lda #' '
    jsr CHROUT
    bra @4
@3: lda #':'
    jsr CHROUT
@4: plx
    phx
    lda header_created_on,x
    jsr CHROUT
    plx
    inx
    bra @0
@5: plx
    print 0, str2_pkg_utc
    
    ; Print header
    print 0, str2_pkg_components

    ; Print ROM version
    print 0, str2_rom
    lda rom_status
    cmp #ROM_STATUS_UNSELECTED
    bne :+
    print 0, str2_skip ; Not selected for upgrade
    bra :+++

:   lda header_rom_ver
    bmi :+

    lda #'R' ; Release
    jsr CHROUT
    lda header_rom_ver
    jsr util_print_num
    bra :++

:   lda #'P' ; Pre-release
    jsr CHROUT
    lda #'R'
    jsr CHROUT
    lda header_rom_ver
    ora #$ff
    dec
    jsr util_print_num

:   lda #13
    jsr CHROUT

    ; Print VERA version
    print 0, str2_vera
    lda vera_status
    bne :+
    print 0, str2_skip ; Not selected for upgrade
    bra :++

:   lda header_vera_ver ; Print VERA version number
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_vera_ver+1 
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_vera_ver+2
    jsr util_print_num
    
:   lda #13
    jsr CHROUT

    ; Print SMC version
    print 0, str2_smc
    lda smc_status
    bne :+
    print 0, str2_skip ; Not selected for upgrade
    bra :++

:   lda header_smc_ver ; Print SMC version number
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_smc_ver+1
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda header_smc_ver+2
    jsr util_print_num

:   lda #13
    jsr CHROUT
    lda #13
    jsr CHROUT

    ; Add Exit button (index 0)
    ldx #<str2_btn_exit
    ldy #>str2_btn_exit
    lda #str2_btn_exit_end-str2_btn_exit
    jsr widget_add_button
    
    ldx #0
    ldy #WIDGET_STATE_SELECTED
    jsr widget_set_state

    ; Add Custom button (index 1)
    ldx #<str2_btn_custom
    ldy #>str2_btn_custom
    lda #str2_btn_custom_end-str2_btn_custom
    jsr widget_add_button

    ; Add Next button (index 2)
    ldx #<str2_btn_next
    ldy #>str2_btn_next
    lda #str2_btn_next_end-str2_btn_next
    jsr widget_add_button

    ; Listen for key events
loop:
    jsr widget_listen

    ; Exit button?
    cpx #0
    beq exit

    ; Custom button?
    cpx #1
    beq custom

    ; Next button?
    cpx #2
    bne loop

next:
    jmp screen4_show

custom:
    jmp screen3_show

exit:
    rts
.endproc
