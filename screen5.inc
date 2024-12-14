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
; Function name.......: screen5_show
; Purpose.............: Shows screen 5 - Run system upgrade
; Input...............: Nothing
; Returns.............: Nothing
.proc screen5_show
    ; Clear and init widgets
    jsr widget_init

    ; Print header
    jsr screen_header
    print 0, str5_header

    bra fail_vera

    ; Update VERA
install_vera:
    jsr vera_update
    bcs fail_vera
    jsr vera_verify
    bcs fail_vera

    ; Update Kernal ROM
install_rom:
    jsr rom_update
    bcs fail_rom
    jsr rom_verify
    bcs fail_rom

    ; Update SMC
install_smc:
    jsr smc_update
    bcs fail_smc
    jsr smc_verify
    bcs fail_smc
    jsr smc_reboot

    ; Reset computer after countdown
    print 0, str5_reset_msg

    ldx #0 ; 5 second countdown
    ldy #0
    lda #5
    jsr util_countdown

    ; Jump to reset vector
    stz ROM_SEL
    jmp ($fffc)

fail_vera:
    print 1, str5_vera_failed
    rts

fail_rom:
    print 1, str5_rom_failed
:   bra :-

fail_smc:
    print 1, str5_smc_failed
:   bra :-

.endproc

