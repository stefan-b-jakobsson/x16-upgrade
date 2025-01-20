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

; Credits to MooingLemur who wrote the flashvera utility,
; https://github.com/mooinglemur/flashvera.
;
; The communication with the VERA's memory flash chip over
; SPI is inspired by MooingLemur's code.


; VERA update status values
VERA_STATUS_UNSELECTED = 0
VERA_STATUS_SELECTED = 1
VERA_STATUS_PRECHECK_FAIL = 2
VERA_STATUS_PRECHECK_PASS = 3
VERA_STATUS_LOAD_FAIL = 4
VERA_STATUS_LOAD_PASS = 5
VERA_STATUS_UPDATE_FAIL = 6
VERA_STATUS_UPDATE_PASS = 7
VERA_STATUS_VERIFY_FAIL = 8
VERA_STATUS_VERIFY_PASS = 9

; SPI registers
VERA_SPI_DATA = $9f3e
VERA_SPI_CTRL = $9f3f

; VERA SPI commands for the W25Q16JV flash memory chip
VERA_CMD_JEDEC = $9f
VERA_CMD_READ_DATA = $03
VERA_CMD_WRITE_ENABLE = $06
VERA_CMD_CHIP_ERASE = $c7
VERA_CMD_PAGE_PROGRAM = $02
VERA_CMD_WRITE_DISABLE = $04
VERA_CMD_READ_STATUS = $05

; Pointer to default RAM buffer
VERA_BUF = $7f00

;******************************************************************************
; Function name.......: vera_init
; Purpose.............: Initializes VERA update functions
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_init
    lda #VERA_STATUS_SELECTED
    sta vera_status
    rts
.endproc

;******************************************************************************
; Function name.......: vera_run_prechecks
; Purpose.............: Runs VERA pre-upgrade checks
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_run_prechecks
    ; Abort if not selected for update
    lda vera_status
    cmp #VERA_STATUS_UNSELECTED
    bne :+
    clc
    rts

:   ; Print header
    print 0, str4_vera

    ; Load VERA firmware
    jsr vera_load
    bcs err

    ; Request user to close JP1, fail after 10 attempts
    lda #10
    sta max_attempts
:   print 1, str4_vera_close_jp1
    print 1, str_f1_help
:   jsr util_wait_keypress
    cmp #$70 ; F1 key code
    bne :+

    jsr help_show
    sec
    jsr help_jp1
    bra :-

:   jsr vera_jedec
    bcc :+
    dec max_attempts
    bne :---
    bra err
    
    ; Chip ID is OK
:   print 1, str4_vera_chip_ok

    ; Return without error
    lda #VERA_STATUS_LOAD_PASS
    sta vera_status
    clc
    rts

err:
    ; Return with error
    lda #VERA_STATUS_PRECHECK_FAIL
    sta vera_status
    sec
    rts

max_attempts:
    .res 1
.endproc

;******************************************************************************
; Function name.......: vera_update
; Purpose.............: Updates the VERA
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_update
    ; Abort if VERA not selected for update
    lda vera_status
    cmp #VERA_STATUS_UNSELECTED
    bne :+
    clc
    rts

:   ; Print header
    print 0, str5_vera_header

    ; Abort with error if status is not LOAD_PASS
    lda vera_status
    cmp #VERA_STATUS_LOAD_PASS
    beq :+
    sec
    rts

    ; Ensure VERA JP1 is closed
:   lda #10
    sta max_attempts
:   jsr vera_jedec
    bcc :+++
    print 1, str5_vera_close_jp1
    print 1, str_f1_help
:   jsr util_wait_keypress
    cmp #$70 ; F1
    bne :+
    jsr help_show
    sec
    jsr help_jp1
    bra :-

:   dec max_attempts
    bne :---
    jmp err

    ; Select progress meter
:   lda #PROGRESS_VERA
    jsr progress_select

    ; Erase chip
    print 1, str5_vera_erase
    jsr vera_chip_erase
    jsr vera_wait_busy
    print 1, str_ok_cr

    ; Setup
    stz vera_target_addr
    stz vera_target_addr+1
    stz vera_target_addr+2

    lda vera_buf_bank
    sta RAM_SEL
    lda vera_buf_addr
    sta vera_src_addr
    lda vera_buf_addr+1
    sta vera_src_addr+1

    ; Print writing message
    print 1, str5_vera_write

loop:
    ; Program page
    jsr vera_page_program
    bcs exit

    ; Update progress meter
    ldx vera_target_addr
    ldy vera_target_addr+1
    lda vera_target_addr+2
    jsr progress_update
    bra loop

exit:
    ; Wait for last operation to finish
    jsr vera_wait_busy

    ; Update progress meter
    ldx vera_target_addr
    ldy vera_target_addr+1
    lda vera_target_addr+2
    jsr progress_update

    ; Print OK
    print 1, str_ok_cr

    ; Exit
    lda #VERA_STATUS_UPDATE_PASS
    sta vera_status
    clc
    rts

err:
    lda #VERA_STATUS_UPDATE_FAIL
    sta vera_status
    sec
    rts

max_attempts:
    .res 1
.endproc

;******************************************************************************
; Function name.......: vera_verify
; Purpose.............: Verifies VERA firmware
; Input...............: Nothing
; Returns.............: C=1 if verify failed
.proc vera_verify
    ; Abort if VERA not selected for update
    lda vera_status
    cmp #VERA_STATUS_UNSELECTED
    bne :+
    clc
    rts

    ; Ensure VERA JP1 is closed
:   lda #10
    sta max_attempts
:   jsr vera_jedec
    bcc :+
    print 1, str5_vera_close_jp1
    jsr util_wait_keypress
    dec max_attempts
    bne :-
    jmp err
    
    ; Print message
:   print 1, str5_vera_verify
    
    ; Setup
    lda vera_buf_bank
    sta RAM_SEL
    lda vera_buf_addr
    sta vera_src_addr
    lda vera_buf_addr+1
    sta vera_src_addr+1

    stz vera_count
    stz vera_count+1
    stz vera_count+2

    lda #PROGRESS_VERA
    jsr progress_select

    ; Set VERA addr
    jsr vera_select

    ldx #0
    ldy #0
    lda #0
    jsr vera_read_data
    
loop:    
    ; Verify
    jsr vera_read_byte
    cmp (vera_src_addr)
    beq :+
    jmp err

    ; Increment source address
:   inc vera_src_addr
    bne :++
    inc vera_src_addr+1
    lda vera_src_addr+1
    cmp #$9f ; Start of I/O area
    bne :+
    lda #$a0 ; Start of banked RAM
    sta vera_src_addr+1
    lda #$01
    sta RAM_SEL
    bra :++

:   cmp #$c0 ; Start of ROM
    bne :+
    lda #$a0 ; Start of banked RAM
    sta vera_src_addr+1
    inc RAM_SEL

    ; Increment byte count
:   inc vera_count
    bne :+

    ldx vera_count
    ldy vera_count+1
    lda vera_count+2
    jsr progress_update

    inc vera_count+1
    bne :+
    inc vera_count+2

    ; Check if done
:   lda vera_count
    cmp header_vera_size
    bne loop
    lda vera_count+1
    cmp header_vera_size+1
    bne loop
    lda vera_count+2
    cmp header_vera_size+2
    bne loop

    ; Unselect VERA
    jsr vera_unselect

    ; Progress, final update
    ldx vera_count
    ldy vera_count+1
    lda vera_count+2
    jsr progress_update

    ; Print OK messages
    print 1, str_ok_cr

    ; Request user to open VERA JP1
:   print 1, str5_vera_open_jp1
    print 1, str_f1_help
:   jsr util_wait_keypress
    cmp #$70 ; F1
    bne :+
    jsr help_show
    clc
    jsr help_jp1
    bra :-

:   jsr vera_jedec
    bcc :---

    ; Return without error
    clc
    rts

err:
    print 1, str_fail_cr
    jsr vera_unselect
    sec
    rts

max_attempts:
    .res 1
.endproc

;******************************************************************************
; Function name.......: vera_load
; Purpose.............: Loads new VERA firmware from data file into RAM buffer.
;                       If both the ROM image and the VERA firmware fits within
;                       512 kB, the RAM buffer starts at the end of the ROM
;                       buffer, otherwise at address VERA_BUF
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_load
    ; Print header
    print 1, str4_vera_loading

    ; Setup progress meter
    lda #PROGRESS_VERA
    jsr progress_select

    ; Check if VERA fits in RAM buffer (size $80000) after the ROM image
    clc
    lda header_rom_size
    adc header_vera_size
    sta vera_buf_addr
    lda header_rom_size+1
    adc header_vera_size+1
    sta vera_buf_addr+1
    lda header_rom_size+2
    adc header_vera_size+2
    cmp #$08
    bcc :+
    bne overflow
    lda vera_buf_addr
    ora vera_buf_addr
    bne overflow

    ; There's room in the RAM buffer after the ROM image, calculate VERA buffer start address
:   clc                        ; RAM bank = header_rom_size / $2000, right shift 5 steps, and 8 steps more by discarding LSB, a total of 13 steps
    lda header_rom_size+2
    sta vera_buf_bank+1
    lda header_rom_size+1
    sta vera_buf_bank
    ldx #5
:   lsr vera_buf_bank+1
    ror vera_buf_bank
    dex
    bne :-

    lda vera_buf_bank
    bne :+

    clc                         ; Bank = 0 => Add start of VERA buffer ($7f00) to address bits 0..12
    lda header_rom_size
    adc #<VERA_BUF
    sta vera_buf_addr
    lda header_rom_size+1
    and #%00011111
    adc #>VERA_BUF
    sta vera_buf_addr+1
    bra load

:   clc                         ; Bank > 0 => Add start of banked RAM ($a000) to address bits 0..12
    lda header_rom_size
    sta vera_buf_addr
    lda header_rom_size+1
    and #%00011111
    adc #$a0
    sta vera_buf_addr+1

    bra load

overflow:
    ; VERA doesn't fit after ROM
    stz vera_buf_bank
    lda #<VERA_BUF
    sta vera_buf_addr
    lda #>VERA_BUF
    sta vera_buf_addr+1

    ; Downgrade ROM status from LOAD_PASS to PRECHECK_PASS, as we're overwriting the ROM image
    lda rom_status
    cmp #ROM_STATUS_LOAD_PASS
    bne load
    lda #ROM_STATUS_PRECHECK_PASS
    sta rom_status

load:
    ; Open package file and seek start of VERA firmware
    lda header_vera_start
    sta r1
    lda header_vera_start+1
    sta r1+1
    lda header_vera_start+2
    sta r1+2
    stz r1+3
    ldx #<file_name
    ldy #>file_name
    lda file_name_len
    jsr file_open_and_seek
    bcs err
    
    ; Read VERA firmware
    lda vera_buf_bank
    sta RAM_SEL
    lda vera_buf_addr
    sta r1
    lda vera_buf_addr+1
    sta r1+1

    lda header_vera_size
    sta r2
    lda header_vera_size+1
    sta r2+1
    lda header_vera_size+2
    sta r2+2
    
    lda #<callback
    sta r4
    lda #>callback
    sta r4+1

    jsr file_read
    bcs err

    ; Exit without errors
    jsr file_close
    print 1,str_ok_cr
    clc
    rts

err:
    jsr file_close
    print 1, str_fail_cr
    sec
    rts

callback:
    sec
    lda header_vera_size
    sbc r2
    tax
    lda header_vera_size+1
    sbc r2+1
    tay
    lda header_vera_size+2
    sbc r2+2
    jmp progress_update

.endproc

;******************************************************************************
; Function name.......: vera_jedec
; Purpose.............: Sends the JEDEC ID command (0x9f)
; Input...............: Nothing
; Returns.............: C = 0 if expected return values, else C = 1
.proc vera_jedec
    ; Wait if busy
    jsr vera_wait_busy

    ; Read JEDEC ID
    jsr vera_select
    lda #VERA_CMD_JEDEC
    jsr vera_write_byte
    jsr vera_read_byte
    tax
    jsr vera_read_byte
    tay
    jsr vera_read_byte

    ; Check if expected JEDEC ID
    cpx #$ef
    bne fail
    cpy #$40
    bne fail
    cmp #$15
    bne fail

    ; Unselect VERA and exit without error
    jsr vera_unselect
    clc
    rts

 fail:
    ; Unselect VERA and exit with error
    jsr vera_unselect
    sec
    rts
.endproc

;******************************************************************************
; Function name.......: vera_chip_erase
; Purpose.............: Erases the flash memory chip
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_chip_erase
    ; Wait busy
    jsr vera_wait_busy

    ; Write-enable
    jsr vera_select
    lda #VERA_CMD_WRITE_ENABLE
    jsr vera_write_byte

    ; Erase chip
    jsr vera_select
    lda #VERA_CMD_CHIP_ERASE
    jsr vera_write_byte

    ; Chip unselect
    jmp vera_unselect
.endproc

;******************************************************************************
; Function name.......: vera_read_data
; Purpose.............: Sends the read command (0x03) to the VERA W25Q16JV
;                       flash memory, and sets the target address. After this
;                       procedure, call vera_read_byte one or more times to 
;                       read data from the flash memory target address.
;                       The target address is incremented after each read.
; Input...............: X Target address bits 0..7
;                       Y Target address bits 8..15
;                       A Target address bits 16..23
; Returns.............: Nothing
.proc vera_read_data
    ; Store target address on stack
    phx
    phy
    pha

    ; Send read data command
    lda #VERA_CMD_READ_DATA
    jsr vera_write_byte

    ; Send address
    ldx #3
:   pla
    jsr vera_write_byte
    dex
    bne :-

    rts
.endproc

;******************************************************************************
; Function name.......: vera_page_program
; Purpose.............: Updates max 256 bytes of VERA flash memory. Programming 
;                       is aborted if vera_target_addr LSB=0 or if it reaches 
;                       header_vera_size.
; Input...............: vera_target_addr = VERA flash memory target addr
;                       vera_src_addr = pointer to source data
; Returns.............: Nothing
.proc vera_page_program
    ; Wait if busy
    jsr vera_wait_busy

    ; Write-enable
    jsr vera_select
    lda #VERA_CMD_WRITE_ENABLE
    jsr vera_write_byte

    ; Page program command
    jsr vera_select
    lda #VERA_CMD_PAGE_PROGRAM
    jsr vera_write_byte
    
    ; Send target address
    lda vera_target_addr+2
    jsr vera_write_byte
    lda vera_target_addr+1
    jsr vera_write_byte
    lda vera_target_addr
    jsr vera_write_byte

loop:
    ; Check if done
    lda vera_target_addr
    cmp header_vera_size
    bne :+
    lda vera_target_addr+1
    cmp header_vera_size+1
    bne :+
    lda vera_target_addr+2
    cmp header_vera_size+2
    beq all_done

    ; Send next byte
:   lda (vera_src_addr)
    jsr vera_write_byte

    ; Increment source address
    inc vera_src_addr
    bne inc_targ
    inc vera_src_addr+1
    
    lda vera_src_addr+1
    cmp #$9f
    bne :+
    lda #1
    sta RAM_SEL
    lda #$a0
    sta vera_src_addr+1
    bra inc_targ

:   cmp #$c0
    bne inc_targ
    inc RAM_SEL
    lda #$a0
    sta vera_src_addr+1

inc_targ:
    ; Increment target address
    inc vera_target_addr
    bne :+
    inc vera_target_addr+1
    bne :+
    inc vera_target_addr+2

    ; Exit if target addr LSB = 0
:   lda vera_target_addr
    bne loop

page_done:
    ; Return
    jsr vera_unselect
    clc
    rts

all_done:
    jsr vera_unselect
    sec
    rts

bc:
    .res 2
.endproc


;******************************************************************************
; Function name.......: vera_write_byte
; Purpose.............: Writes one byte to the VERA flash memory
; Input...............: A = Byte value
; Returns.............: Nothing
.proc vera_write_byte
    sta VERA_SPI_DATA
:   bit VERA_SPI_CTRL
    bmi :-
    rts
.endproc

;******************************************************************************
; Function name.......: vera_read_byte
; Purpose.............: Reads one byte from the VERA flash memory
; Input...............: Nothing
; Returns.............: A = Byte value
.proc vera_read_byte
    stz VERA_SPI_DATA
:   bit VERA_SPI_CTRL
    bmi :-
    lda VERA_SPI_DATA
    rts
.endproc

;******************************************************************************
; Function name.......: vera_wait_busy
; Purpose.............: Waits until the VERA busy status flag is not active
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_wait_busy
:   jsr vera_select
    lda #VERA_CMD_READ_STATUS
    jsr vera_write_byte
    jsr vera_read_byte
    and #1
    beq exit
    jsr vera_unselect
    wai
    bra :-
exit:
    rts
.endproc

;******************************************************************************
; Function name.......: vera_select
; Purpose.............: Activates VERA chip select
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_select
    jsr vera_unselect
    lda #%00000001
    sta VERA_SPI_CTRL
    rts
.endproc

;******************************************************************************
; Function name.......: vera_unselect
; Purpose.............: Releases VERA chip select
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_unselect
    lda VERA_SPI_CTRL
    and #%11111110
    sta VERA_SPI_CTRL
    rts
.endproc

;******************************************************************************
; Global variables
vera_status: .res 1
vera_buf_bank: .res 2
vera_buf_addr: .res 2
vera_count: .res 3
vera_target_addr: .res 3

.ZEROPAGE
    vera_src_addr: .res 2
.CODE
