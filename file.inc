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


FILE_NUM = 1        ; For reading file content
FILE_CMD1 = 15      ; For get file size command
FILE_CMD2 = 16      ; For file seek command
FILE_DEVICE = 8

;******************************************************************************
; Function name.......: file_open
; Purpose.............: Opens a file for reading
; Input...............: X/Y = Pointer to file name (LSB/MSB)
;                       A = File name length
; Returns.............: Nothing
; Error...............: C=1 on error
.proc file_open
    ; Set name
    jsr SETNAM

    ; Ensure files are closed
    lda #FILE_NUM
    jsr CLOSE
    lda #FILE_CMD1
    jsr CLOSE
    lda #FILE_CMD2
    jsr CLOSE

    ; Set params
    lda #FILE_NUM
    ldx #FILE_DEVICE
    ldy #0
    jsr SETLFS

    ; Open
    jsr OPEN
    bcs err

    ; Set file as input
    ldx #FILE_NUM
    jsr CHKIN
    bcs err

    ; Return without errors
    stz file_err
    rts

err:
    ; Store error number and return
    sta file_err
    rts
.endproc

;******************************************************************************
; Function name.......: file_open_and_seek
; Purpose.............: Opens a file for reading, and seeks the specified
;                       file offset
; Input...............: X/Y = Pointer to file name (LSB/MSB)
;                       A = File name length
;                       r1-r2 = File seek offset (32 bits)
; Returns.............: Nothing
; Error...............: C=1 on error
.proc file_open_and_seek
    ; Open file
    jsr file_open
    bcs err

    ; Prepare DOS position "P" command
    stz cmd+1   ; Channel

    ldx #4      ; p0-p3
:   lda r1-1,x
    sta cmd+1,x
    dex
    bne :-

    ; Send DOS position command
    ldx #<cmd
    ldy #>cmd
    lda #6
    jsr SETNAM
    
    lda #FILE_CMD2
    ldx #FILE_DEVICE
    ldy #15 ; Command channel
    jsr SETLFS
    jsr OPEN

    ldx #FILE_CMD2
    jsr CHKIN

    ; Read drive status
    ldx #$ff
@1: inx
    jsr CHRIN
    cpx #2
    bcs @2
    cmp #'0' ; An error occured if first two chars are not "00"
    bne err
@2: jsr READST
    beq @1
    
    ; Set main file as input
    ldx #FILE_NUM
    jsr CHKIN
    
    ; Return without error
    clc
    rts

err:
    sec
    rts

cmd:
    .byt "p"
    .res 5
.endproc

;******************************************************************************
; Function name.......: file_read
; Purpose.............: Reads the specified number of bytes into memory from 
;                       the currently open file.
;                       If the target address overflows into the I/O area, it
;                       automatically continues from $01:$A000.
;                       If the target address overflows into the ROM area, it
;                       automatically continues from the start of the next
;                       RAM bank.
; Input...............: r1 = Target address
;                       r2-r3L = Byte count
;                       r4 = Callback function every 256 bytes, 0 = null
; Returns.............: r1 = Last target address + 1
;                       r2-r3L = Number of bytes not read
; Error...............: C = 1 on error
.proc file_read
    count = r2

    ; Check remaining byte count
    lda count
    ora count+1
    ora count+2
    bne :+
    jsr callback
    jmp exit

    ; Special case: Target address within range of I/O area, ensure we don't overflow into that
:   lda r1+1
    cmp #$9e
    bne :+
    sec
    lda #0
    sbc r1
    cmp #128
    bcc :+++

    ; Read max 128 bytes
:   lda count+1
    ora count+2
    bne :+
    lda count
    cmp #129
    bcc :++
:   lda #128
:   ldx r1
    ldy r1+1
    clc
    jsr MACPTR
    bcs err

    ; Check errors
    jsr READST
    sta status
    and #%10000010
    bne err

    ; Store number of bytes actually read
    stx batch_read

    ; Update CRC
    ldy #0
:   lda (r1),y
    jsr crc16_update
    iny
    cpy batch_read
    bne :-

    ; Decrement byte count
    sec
    lda count
    sbc batch_read
    sta count
    lda count+1
    sbc #0
    sta count+1
    lda count+2
    sbc #0
    sta count+2

    ; Increment target address
    clc
    lda batch_read
    adc r1
    sta r1
    lda r1+1
    adc #0
    sta r1+1

    ; Check target address overflow
    lda r1+1
    cmp #$9f
    bne :+
    lda #$a0
    sta r1+1
    lda #1
    sta RAM_SEL
    bra :++

:   cmp #$c0
    bne :+
    lda #$a0
    sta r1+1

:   jsr callback

check_eof:
    lda status
    and #%01000001
    bne exit
    jmp file_read

exit:
    ; Return with error if not all requested bytes available
    lda count
    ora count+1
    ora count+2
    bne err

    ; Return without error
    clc
    rts

callback:
    ; Invoke callback function, pointer in r4, ignore if address is 0
    lda r4
    ora r4+1
    beq :+
    jmp (r4)
:   rts

err:
    sec
    rts

tmp: .res 1
status: .res 1
batch_read: .res 1
.endproc

;******************************************************************************
; Function name.......: file_close
; Purpose.............: Closes all file handles
; Input...............: Nothing
; Returns.............: Nothing
.proc file_close
    lda #FILE_NUM
    jsr CLOSE
    lda #FILE_CMD1
    jsr CLOSE
    lda #FILE_CMD2
    jsr CLOSE
    jsr CLRCHN
    rts
.endproc

;******************************************************************************
; Function name.......: file_get_size
; Purpose.............: Returns the size of the currently open file
; Input...............: Nothing
; Returns.............: X/Y/A = File size (24 bits, low, mid, high)
; Error...............: C=1 on error
.proc file_get_size
    ; Prepare DOS tell "T" command
    ldx #<cmd
    ldy #>cmd
    lda #2
    jsr SETNAM
    
    ; Send DOS tell command
    lda #FILE_CMD1
    ldx #FILE_DEVICE
    ldy #15 ; Command channel
    jsr SETLFS
    jsr OPEN

    ; Set as input
    ldx #FILE_CMD1
    jsr CHKIN

    stz index
loop:
    jsr CHRIN ; Read char
    pha
    
    jsr READST ; Check if done
    bne eol
    
    pla ; Get char from stack

    ldx index
    
    ; Store response code, 2 chars
    cpx #2
    bcs :+
    sta code,x

:   ; Store file size hex string value at byte index 12..19
    cpx #12 
    bcc next
    cpx #20
    bcs next
    sta size-12,x

next:
    inc index
    bra loop

eol:
    ; Get last char from stack
    pla
    
    ; Set main file as input
    ldx #FILE_NUM
    jsr CHKIN 

    ; Check response code
    lda code
    cmp #'0'
    bne err
    lda code+1
    cmp #'7'
    bne err

    ; Set return values
    ldx #<size
    ldy #>size
    jsr util_hex2bin
    ldx r5
    ldy r5+1
    lda r5+2

    clc
    rts

err:
    sec
    rts

cmd:
    .byt 't'
    .byt 0

index:
    .res 1

code:
    .res 2

size:
    .res 8
    .byt 0
.endproc

;******************************************************************************
; Global variables

file_err: .res 1
file_name: .res 256
file_name_len: .res 1
