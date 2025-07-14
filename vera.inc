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
    print 0, str_vera
    
    ; Ensure JP1 is open
    sec ; abortable
    jsr vera_select_sdcard
    bcs err ; User aborted without opening JP1

    ; Print loading message
    print 1, str_loading

    ; Load VERA firmware
    jsr vera_load
    cmp #0
    beq load_ok
    cmp #2
    beq crc_err

    print 1, str_read_error
    bra err

crc_err:
    print 1, str_crc_error
    bra err

load_ok:
    print 1, str_ok_cr

    ; Ensure JP1 is closed
    sec ; make following prompt abortable
    jsr vera_select_flash
    bcs err ; User aborted without closing JP1
    
    ; Chip ID is OK
    print 1, str_vera_chip_ok

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
.endproc

;******************************************************************************
; Function name.......: vera_update
; Purpose.............: Updates the VERA
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_update
    ; Abort if VERA status is not VERA_LOAD_PASS
    lda vera_status
    cmp #VERA_STATUS_LOAD_PASS
    beq :+
    clc
    rts

:   ; Print header
    print 0, str_vera

    ; Validate CRC-16 of the buffered new firmware
    jsr vera_crc_validate
    bcc :+ ; OK
    
    ; Load VERA again
    clc ; Not user abortable
    jsr vera_select_sdcard
    print 1, str_loading
    jsr vera_load

    ; Ensure VERA JP1 is closed
:   clc ; Not user abortable
    jsr vera_select_flash

    ; Select progress counter
    lda #PROGRESS_VERA
    jsr progress_select

    ; Erase chip
    print 1, str_erasing
    jsr vera_chip_erase
    jsr vera_wait_busy
    print 0, str_ok_cr

    ; Setup target address
    stz vera_target_addr
    stz vera_target_addr+1
    stz vera_target_addr+2

    ; Setup source address
    lda vera_buf_bank
    sta RAM_SEL
    lda vera_buf_addr
    sta vera_src_addr
    lda vera_buf_addr+1
    sta vera_src_addr+1

    ; Print writing message
    print 1, str_writing

loop:
    ; Program page
    jsr vera_page_program
    bcs exit

    ; Update progress counter
    ldx vera_target_addr
    ldy vera_target_addr+1
    lda vera_target_addr+2
    jsr progress_update
    bra loop

exit:
    ; Wait for last operation to finish
    jsr vera_wait_busy

    ; Update progress counter
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
.endproc

;******************************************************************************
; Function name.......: vera_verify
; Purpose.............: Verifies VERA firmware
; Input...............: Nothing
; Returns.............: C=1 if verify failed
.proc vera_verify
    ; Abort if status is not VERA_STATUS_UPDATE_PASS
    lda vera_status
    cmp #VERA_STATUS_UPDATE_PASS
    beq :+
    clc
    rts

    ; Ensure VERA JP1 is closed
:   clc ; following prompt not abortable
    jsr vera_select_flash
    bcc :+
    print 1, str_aborted
    lda #VERA_STATUS_VERIFY_FAIL
    sta vera_status
    sec
    rts
    
    ; Print message
:   print 1, str_verifying
    
    ; Setup source pointer
    lda vera_buf_bank
    sta RAM_SEL
    lda vera_buf_addr
    sta vera_src_addr
    lda vera_buf_addr+1
    sta vera_src_addr+1

    ; Clear byte count
    stz vera_count
    stz vera_count+1
    stz vera_count+2

    ; Setup progress counter
    lda #PROGRESS_VERA
    jsr progress_select

    ; Select VERA and init reading from address 0
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

    ldx vera_count      ; Update progress count on every 256 bytes
    ldy vera_count+1
    lda vera_count+2
    jsr progress_update

    inc vera_count+1
    bne :+
    inc vera_count+2

    ; Check if all of firmware verified
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

    ; Progress, final update to 100%
    ldx vera_count
    ldy vera_count+1
    lda vera_count+2
    jsr progress_update

    ; Print OK messages
    print 1, str_ok_cr

    ; Ensure JP1 is open
    clc ; user not allowed to abort
    jsr vera_select_sdcard

    ; Set status
    lda #VERA_STATUS_VERIFY_PASS
    sta vera_status

    ; Return without error
    clc
    rts

err:
    ; Set status
    lda #VERA_STATUS_VERIFY_FAIL
    sta vera_status

    ; Print fail message
    print 1, str_fail_cr
    jsr vera_unselect

    ; Ensure JP1 is open
    clc ; user not allowed to abort
    jsr vera_select_sdcard

    ; Return with error
    sec
    rts
.endproc

;******************************************************************************
; Function name.......: vera_load
; Purpose.............: Loads new VERA firmware from data file into RAM buffer.
;                       If both the ROM image and the VERA firmware fits within
;                       512 kB, the RAM buffer starts at the end of the ROM
;                       buffer, otherwise at address VERA_BUF
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: A = 0   => OK
;                           1   => Read error
;                           2   => Checksum error
.proc vera_load
    ; Reset CRC value
    jsr crc16_init

    ; Setup progress counter
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

    ; Downgrade ROM status from LOAD_PASS to PRECHECK_PASS, as we're overwriting the ROM image buffer
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

    jsr file_close
    
    ; Compare CRCs
    lda crc16_val
    cmp header_vera_crc
    bne err2
    lda crc16_val+1
    cmp header_vera_crc+1
    bne err2

    ; Exit without errors
    lda #0
    rts

err:
    jsr file_close
    lda #1
    rts

err2:
    lda #2
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
; Function name.......: vera_crc_validate
; Purpose.............: Calculates VERA buffer CRC-16 checksum and verifies that
;                       it matches the checksum stored in the header
; Input...............: Nothing
; Returns.............: C=0 OK
;                       C=1 Checksum error
.proc vera_crc_validate
    ; Print message
    print 1, str_validating

    ; Setup progress counter
    lda #PROGRESS_VERA
    jsr progress_select

    ; Calculate CRC for ROM buffer
    lda vera_buf_addr
    sta r1
    lda vera_buf_addr+1
    sta r1+1
    lda vera_buf_bank
    sta RAM_SEL
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
    jsr crc16_buffer

    ; Compare with header data
    cpx header_vera_crc
    bne err
    cpy header_vera_crc+1
    bne err
    
    ; Exit without error
    print 1, str_ok_cr
    clc
    rts

err:
    print 1, str_crc_error
    sec
    rts

callback:
    ; Calculate number of bytes processed
    sec
    lda header_vera_size
    sbc r2
    tax
    lda header_vera_size+1
    sbc r2+1
    tay
    lda header_vera_size+2
    sbc r2+2

    ; Update progress counter
    jmp progress_update
.endproc


;******************************************************************************
; Function name.......: vera_select_sdcard
; Purpose.............: Prompts user to remove JP1 to select the SD card
; Input...............: C = 1   Allow user to abort
; Returns.............: C = 0   OK, JP1 is closed
;                       C = 1   User aborted, and JP1 is open
.proc vera_select_sdcard
    stz flags
    bcc :+
    inc flags

    ; Print message
:   print 1, str_vera_select_sdcard

    jsr vera_jedec
    bcc :+
    
    ; JP1 not installed, return with OK
    jsr file_cwd_restore
    print 0, str_ok_cr
    clc
    rts

:   lda #13 ; Line break
    jsr CHROUT

loop:
    ; Get JP1 status
    jsr vera_jedec
    bcs success
    
    ; Prompt user to open JP1
    print 2, str_vera_open_jp1
    lda flags
    bne :+
    print 2, str_f1_help
    bra keypress
:   print 2, str_f1_help_esc_abort

keypress:
    jsr util_wait_keypress
    cmp #$70 ; F1
    beq help
    cmp #$6e ; ESC
    bne loop

abort:
    lda flags
    beq loop ; Not allowed to abort
    print 2, str_aborted
    sec
    rts

success:
    jsr file_cwd_restore
    print 2, str_ok_cr
    clc
    rts

help:
    jsr help_show
    clc ; Help on opening JP1
    jsr help_jp1
    bra keypress

flags:
    .res 1
.endproc

;******************************************************************************
; Function name.......: vera_select_flash
; Purpose.............: Prompts user to install JP1 to select VERA flash
; Input...............: C = 1   Allow user to abort
; Returns.............: C = 0   OK, JP1 is open
;                       C = 1   User aborted, and JP1 is closed
.proc vera_select_flash
    stz flags
    bcc :+
    inc flags

    ; Print message
:   print 1, str_vera_select_flash

    jsr vera_jedec
    bcs :+
    
    ; JP1 is installed, return with OK
    print 0, str_ok_cr
    clc
    rts

:   lda #13 ; Line break
    jsr CHROUT

loop:
    ; Get JP1 status
    jsr vera_jedec
    bcc success
    
    ; Prompt user to close JP1
    print 2, str_vera_close_jp1
    lda flags
    bne :+
    print 2, str_f1_help
    bra keypress
:   print 2, str_f1_help_esc_abort

keypress:
    jsr util_wait_keypress
    cmp #$70 ; F1
    beq help
    cmp #$6e ; ESC
    bne loop

abort:
    lda flags
    beq loop ; Not allowed to abort
    print 2, str_aborted
    sec
    rts

success:
    print 2, str_ok_cr
    clc
    rts

help:
    jsr help_show
    sec ; Help on closing JP1
    jsr help_jp1
    bra keypress

flags:
    .res 1
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
; Purpose.............: Updates max 256 bytes of VERA flash memory. Returns
;                       on vera_target_addr LSB=0 or if source address reaches
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
