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


; ROM update status values
ROM_STATUS_UNSELECTED = 0
ROM_STATUS_SELECTED = 1
ROM_STATUS_PRECHECK_FAIL = 2
ROM_STATUS_PRECHECK_PASS = 3
ROM_STATUS_LOAD_FAIL = 4
ROM_STATUS_LOAD_PASS = 5
ROM_STATUS_UPDATE_FAIL = 6
ROM_STATUS_UPDATE_PASS = 7
ROM_STATUS_VERIFY_FAIL = 8
ROM_STATUS_VERIFY_PASS = 9

; Chip ID (MSB, LSB)
ROM_CHIP_ID = $b7bf

; ROM buffer
ROM_BUF = $7f00

ROM_MAX_ATTEMPTS = 10

;******************************************************************************
; Function name.......: rom_init
; Purpose.............: Initialize ROM update functions
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_init
    lda #ROM_STATUS_SELECTED
    sta rom_status
    rts
.endproc

;******************************************************************************
; Function name.......: rom_run_prechecks
; Purpose.............: Runs the following pre-upgrade checks:
;                       1. ROM chip write enabled
;                       2. ROM chip ID
;                       3. CRC-16 check of SMC firmware loaded from package file
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc rom_run_prechecks
    ; Abort if ROM is unselected
    lda rom_status
    cmp #ROM_STATUS_UNSELECTED
    bne :+
    clc
    rts

    ; Print header
:   print 0, str4_rom

    ; Disable interrupts
    php
    sei

t1:
    ; Test 1: Chip write-enabled
    print 1, str4_rom_write_enabled
    
    ; Attempt to write $ff to $01:$ffff
    jsr rom_is_write_enabled
    bcs :+
    
    ; Test 1, OK
    print 1, str_ok_cr
    bra t2

:   ; Test 1, failed
    print 1, str_fail_cr
    print 1, str_f1_help
    jsr util_wait_keypress
    cmp #$70 ; f1
    bne fail
    jsr help_show
    jsr help_close_j1
    bra fail
    
t2:
    ; Test 2: Check chip ID
    print 1, str4_rom_chip_id
    jsr rom_read_chip_id
    cpx #<ROM_CHIP_ID
    bne :+
    cpy #>ROM_CHIP_ID
    bne :+

    ; Test 2, OK
    print 1, str_ok_cr
    bra t3

:   ; Test 2, failed
    print 1, str_fail_cr
    bra fail

t3: 
    ; Test 3: Try to load ROM from package file
    jsr rom_load
    bcc :+

    ; Test 3, failed
    print 1, str_fail_cr
    bra fail

    ; Test 3, OK
:   print 1, str_ok_cr

pass:
    ; Restore IRQ flag
    plp
    
    ; Set status
    lda #ROM_STATUS_LOAD_PASS
    sta rom_status

    ; Set return values
    clc
    rts

fail:
    ; Restore IRQ flag
    plp

    ; Set status
    lda #ROM_STATUS_PRECHECK_FAIL
    sta rom_status

    ; Set return values
    sec
    rts
.endproc

;******************************************************************************
; Function name.......: rom_update
; Purpose.............: Updates the ROM
; Input...............: Nothing
; Returns.............: Nothing
; Errors..............: C=1 on ROM write error
.proc rom_update
    ; Start update if status is ROM_STATUS_LOAD_PASS
    lda rom_status
    cmp #ROM_STATUS_LOAD_PASS
    beq :+
    cmp #ROM_STATUS_PRECHECK_PASS
    beq :+

    ; Otherwise abort
    clc
    rts

    ; Print header
:   print 0, str5_rom_header

    ; Load ROM from file
    lda rom_status
    cmp #ROM_STATUS_PRECHECK_PASS
    bne :+
    jsr rom_load
    bcc :+

    ; Abort, loading ROM failed
    lda #ROM_STATUS_LOAD_FAIL
    sta rom_status
    sec
    rts

    ; Disable interrupts, shouldn't be re-enabled after this point
:   sei

    ; Setup
    lda #<ROM_BUF           ; RAM address where new ROM image is stored
    sta rom_src_addr
    lda #>ROM_BUF
    sta rom_src_addr+1

    stz rom_target_bank     ; ROM address start, $c000
    stz rom_target_addr
    lda #$c0
    sta rom_target_addr+1

    stz count               ; Clear byte count
    stz count+1
    stz count+2

    lda #PROGRESS_ROM       ; Select progress meter
    jsr progress_select

    lda #ROM_MAX_ATTEMPTS   ; Set max write errors
    sta attempts

    ; Chip erase
    print 1, str5_rom_erase
    jsr rom_erase
    print 1, str_ok_cr

    ; Write loop
    print 1, str5_rom_write

loop:
    lda (rom_src_addr)
    jsr rom_write
    bcc :+
    dec attempts
    bne loop
    
    ; Write error
    lda #ROM_STATUS_UPDATE_FAIL
    sta rom_status
    sec
    rts

    ; Reset attempts counter
:   lda #ROM_MAX_ATTEMPTS
    sta attempts

    ; Increment byte count
    inc count
    bne :++
    inc count+1
    bne :+
    inc count+2

    ; Update progress meter
:   ldx count
    ldy count+1
    lda count+2
    jsr progress_update

    ; Check if done
:   lda count
    cmp header_rom_size
    bne src_inc
    lda count+1
    cmp header_rom_size+1
    bne src_inc
    lda count+2
    cmp header_rom_size+2
    beq exit

    ; Increment source address
src_inc:   
    inc rom_src_addr
    bne target_inc
    inc rom_src_addr+1
    
    lda rom_src_addr+1
    cmp #$c0 ; Overflow into ROM area, goto start of next RAM bank
    beq :+
    cmp #$9f ; Overflow into IO area, goto RAM bank 1
    bne target_inc
    stz RAM_SEL

:   inc RAM_SEL
    lda #$a0
    sta rom_src_addr+1

    ; Increment target address
target_inc:
    inc rom_target_addr
    bne loop
    inc rom_target_addr+1
    bne loop
    lda #$c0 ; Overflow, goto start of next ROM bank
    sta rom_target_addr+1
    inc rom_target_bank
    bra loop

exit:
    lda #13
    jsr CHROUT

    lda #ROM_STATUS_UPDATE_PASS
    sta rom_status

    stz ROM_SEL
    clc
    rts

count: .res 3
attempts: .res 1
.endproc

;******************************************************************************
; Function name.......: rom_verify
; Purpose.............: Verifies ROM content
; Input...............: Nothing
; Returns.............: C=1 if verify failed
.proc rom_verify
    ; Abort if status is not ROM_STATUS_UPDATE_PASS
    lda rom_status
    cmp #ROM_STATUS_UPDATE_PASS
    beq :+
    clc
    rts

    ; Print header
:   print 1, str5_rom_verify
    
    ; Setup
    stz ROM_SEL
    
    lda #<ROM_BUF
    sta rom_src_addr
    lda #>ROM_BUF
    sta rom_src_addr+1

    stz rom_target_addr
    lda #$c0
    sta rom_target_addr+1

    stz count
    stz count+1
    stz count+2

    lda #PROGRESS_ROM
    jsr progress_select

loop:    
    ; Verify
    lda (rom_src_addr)
    cmp (rom_target_addr)
    bne fail

    ; Increment byte count
    inc count
    bne :++
    inc count+1
    bne :+
    inc count+2

    ; Update progress meter
:   ldx count
    ldy count+1
    lda count+2
    jsr progress_update

    ; Check if done
:   lda count+2
    cmp header_rom_size+2
    bne next
    lda count+1
    cmp header_rom_size+1
    bne next
    lda count
    cmp header_rom_size
    beq ok

next:
    ; Increment source address
    inc rom_src_addr
    bne @2
    inc rom_src_addr+1
    
    lda rom_src_addr+1
    cmp #$c0 ; Overflow into ROM area, goto start of next RAM bank
    beq @1
    cmp #$9f ; Overflow into IO area, goto start of RAM bank 1
    bne @2
    stz RAM_SEL

@1: inc RAM_SEL
    lda #$a0
    sta rom_src_addr+1

    ; Increment target address
@2: inc rom_target_addr
    bne loop
    inc rom_target_addr+1
    bne loop
    lda #$c0 ; Overflow, goto start of next ROM bank
    sta rom_target_addr+1
    inc ROM_SEL
    bra loop

ok:
    ; Select Kernal ROM bank
    stz ROM_SEL
   
    ; Print message
    print 1,str_ok_cr
   
    ; Set status
    lda #ROM_STATUS_VERIFY_PASS
    sta rom_status
    
    ; Return with OK
    clc
    rts

fail:
    ; Select Kernal ROM bank
    stz ROM_SEL

    ; Print message
    print 1, str_fail_cr

    ; Set status
    lda #ROM_STATUS_VERIFY_FAIL
    sta rom_status
    
    ; Return with error
    sec
    rts

count: .res 3
.endproc

;******************************************************************************
; Function name.......: rom_load
; Purpose.............: Loads the ROM
; Input...............: Nothing
; Returns.............: Nothing
; Errors..............: C=1 on error
.proc rom_load
    ; Reset CRC
    jsr crc16_init

    ; Select progress meter
    lda #PROGRESS_ROM
    jsr progress_select

    ; Print header
    print 1, str4_rom_progress

    ; Open file and seek start of ROM firmware
    lda header_rom_start
    sta r1
    lda header_rom_start+1
    sta r1+1
    lda header_rom_start+2
    sta r1+2
    stz r1+3
    ldx #<file_name
    ldy #>file_name
    lda file_name_len
    jsr file_open_and_seek
    bcs err

    ; Load file
    lda #<ROM_BUF
    sta r1
    lda #>ROM_BUF
    sta r1+1
    lda header_rom_size
    sta r2
    lda header_rom_size+1
    sta r2+1
    lda header_rom_size+2
    sta r2+2
    lda #<callback
    sta r4
    lda #>callback
    sta r4+1
    jsr file_read
    bcs err
    
    ; Close file
    jsr file_close

    ; Return with error
    clc
    rts

err:
    ; Close file
    jsr file_close

    ; Return with error
    sec
    rts

callback:
    ; Calculate number of bytes updated
    sec
    lda header_rom_size
    sbc r2
    tax
    lda header_rom_size+1
    sbc r2+1
    tay
    lda header_rom_size+2
    sbc r2+2

    ; Update progress meter
    jmp progress_update
.endproc

;******************************************************************************
; Function name.......: rom_is_write_enabled
; Purpose.............: Checks if ROM chip is write-enabled
; Input...............: Nothing
; Returns.............: C = 0 => Write-enabled
;                       C = 1 => Write protected
.proc rom_is_write_enabled
    lda #$01
    sta rom_target_bank
    lda #$ff
    sta rom_target_addr
    sta rom_target_addr+1
    jmp rom_write
.endproc

;******************************************************************************
; Function name.......: rom_read_chip_id
; Purpose.............: Gets ROM chip ID
; Input...............: Nothing
; Returns.............: X = LSB, Y = MSB
.proc rom_read_chip_id
    ; Disable IRQ
    php
    sei

    ; Save ROM bank on stack
    lda ROM_SEL
    pha

    ; Software ID entry
    lda #$01
    sta ROM_SEL
    lda #$aa
    sta $d555

    dec ROM_SEL
    lda #$55
    sta $eaaa 

    inc ROM_SEL
    lda #$90
    sta $d555

    ; Delay
    nop

    ; Read software ID
    dec ROM_SEL
    ldx $c000
    ldy $c001

    ; Software ID exit
    lda #$f0
    sta $c000
    
    ; Delay
    nop

    ; Restore ROM bank
    pla
    sta ROM_SEL

    ; Restore IRQ flag
    plp

    ; Return
    rts
.endproc

;******************************************************************************
; Function name.......: rom_erase
; Purpose.............: Erases the ROM chip
; Input...............: Nothing
; Returns.............: Nothing
.proc rom_erase
    ; Disable IRQ
    php
    sei

    ; Store current ROM bank in X
    ldx ROM_SEL

    ; Chip erase command
    lda #$01        ; 5555H=AAH 
    sta ROM_SEL
    lda #$aa
    sta $d555

    dec ROM_SEL     ; 2AAAH=55H
    lda #$55
    sta $eaaa 

    inc ROM_SEL     ; 5555H=80H
    lda #$80
    sta $d555

    lda #$aa        ; 5555H=AAH
    sta $d555

    dec ROM_SEL     ; 2AAAH=55H
    lda #$55
    sta $eaaa

    inc ROM_SEL     ; 5555H=10H
    lda #$10
    sta $d555

    stz ROM_SEL
wait:
    lda $c000
    cmp $c000
    bne wait

exit:
    ; Restore ROM bank
    stx ROM_SEL

    ; Restore IRQ flag
    plp

    ; Return
    rts

.endproc

;******************************************************************************
; Function name.......: rom_write
; Purpose.............: Writes one byte to ROM at 
;                       rom_target_bank:rom_target_addr.
; Input...............: A = value
; Returns.............: Nothing
; Errors..............: C=1 if write operation failed
.proc rom_write
    ; Disable IRQ
    php
    sei

    ; Save current ROM bank on stack
    ldx ROM_SEL
    phx

    ; Get target ROM bank
    ldx rom_target_bank

    ; Enter byte programming mode
    ldy #$01 
    sty ROM_SEL
    ldy #$aa
    sty $d555

    dec ROM_SEL
    ldy #$55
    sty $eaaa 

    inc ROM_SEL
    ldy #$a0
    sty $d555

    ; Write data
    stx ROM_SEL
    sta (rom_target_addr)

    ; Test if the ROM chip entered byte programming mode
    ; Reading the same target address returns alternating values immediately after programming an ROM address
    lda (rom_target_addr)
    cmp (rom_target_addr)
    beq fail

wait:
    ; Wait until reading the target address no longer returns alternating values => byte programming done
    lda (rom_target_addr)
    cmp (rom_target_addr)
    bne wait

    ; Restore ROM bank
    plx
    stx ROM_SEL

    ; Retore IRQ flag
    plp

    ; Return without errors
    clc
    rts

fail:
    ; Restore ROM bank
    plx
    stx ROM_SEL

    ; Restore IRQ flag
    plp
    
    ; Return with error
    sec
    rts
    
.endproc

;******************************************************************************
; Global variables

rom_status:
    .res 1

rom_version:
    .byt "R48",0

.ZEROPAGE
    rom_target_addr: .res 2
    rom_target_bank: .res 1
    rom_src_addr: .res 2
.CODE
