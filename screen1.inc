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
; Function name.......: screen1_show
; Purpose.............: Shows screen 1 - Welcome message
; Input...............: Nothing
; Returns.............: Nothing
.proc screen1_show
    ; Clear and init widgets
    jsr widget_init
    
    ; Print header and welcome message
    jsr screen_header
    print 0, str1_welcome

    ; Prompt for file name
    print 0, str1_file_prompt

    ; Get file name
    sec
    jsr PLOT
    clc
    jsr $fff0   ; Real PLOT

    ldy #0
:   jsr CHRIN
    sta file_name,y
    iny
    cmp #13
    bne :-
    dey
    sty file_name_len

    jsr util_init
    sec
    jsr $fff0
    clc
    jsr PLOT
    
    ; Fetch file header and check integrity
    jsr header_fetch
    bcs exit
    jsr header_run_prechecks
    bcs exit

    ; Setup progress meters
    jsr progress_init

    ; Show screen 2
    jmp screen2_show

exit:
    rts
.endproc
