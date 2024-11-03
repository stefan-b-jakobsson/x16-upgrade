FILE_NUM = 1
FILE_CMD1 = 15
FILE_CMD2 = 16
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
;                       r1-r2 = File seek offset
; Returns.............: Nothing
; Error...............: C=1 on error
.proc file_open_and_seek
    ; Open file
    jsr file_open
    bcs err

    ; Prepare command
    stz cmd+1   ; Channel

    ldx #4      ; p0-p3
:   lda r1-1,x
    sta cmd+1,x
    dex
    bne :-

    ; Send position command
    ldx #<cmd
    ldy #>cmd
    lda #6
    jsr SETNAM
    
    lda #FILE_CMD2
    ldx #8
    ldy #15
    jsr SETLFS
    jsr OPEN

    ldx #FILE_CMD2
    jsr CHKIN

    ; Get status
    ldx #$ff
@1: inx
    jsr CHRIN
    cpx #2
    bcs @2
    cmp #'0'
    bne err
@2: cmp #13
    bne @1
    
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
;                       The target address is not incremented if it is 
;                       within the I/O area ($9F00-$9FFF).
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
    bra exit

    ; Read byte
:   jsr CHRIN
    sta tmp
    
    jsr READST
    sta status
    and #%10000010
    bne err

    ; Save byte
    lda tmp
    sta (r1)

    ; Update CRC
    jsr crc16_update
    
    ; Increment target address (if applicable)
    lda r1+1            ; Don't increment if target within I/O area
    cmp #$9f
    beq dec_counter

    inc r1              ; Increment LSB
    bne dec_counter
    inc r1+1            ; Increment MSB
    
    lda r1+1            ; On overflow into I/O area, set address to 01:A000
    cmp #$9f
    bne :+
    lda #$a0
    sta r1+1
    lda #1
    sta RAM_SEL
    bra dec_counter

:   cmp #$c0            ; On overflow into ROM area, set address to start of next RAM bank
    bne dec_counter
    inc RAM_SEL
    lda #$a0
    sta r1+1

dec_counter:
    lda count
    bne :++
    lda count+1
    bne :+
    dec count+2
:   dec count+1
    jsr callback    ; Callback every time bits 8..15 are decremented
:   dec count

check_eof:
    lda status
    and #%01000001
    beq file_read

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
.endproc

;******************************************************************************
; Function name.......: file_close
; Purpose.............: Closes the file
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
.proc file_get_size
    ldx #<cmd
    ldy #>cmd
    lda #2
    jsr SETNAM
    
    lda #FILE_CMD1
    ldx #8
    ldy #15
    jsr SETLFS
    jsr OPEN

    ldx #FILE_CMD1
    jsr CHKIN

    stz index

loop:
    jsr CHRIN
    pha
    
    jsr READST
    bne eol
    
    pla

    ldx index
    cpx #2
    bcs :+
    sta code,x

:   cpx #12
    bcc next
    cpx #20
    bcs next
    sta size-12,x

next:
    inc index
    bra loop

eol:
    pla
    
    ldx #1
    jsr CHKIN 

    ldx #<size
    ldy #>size
    jsr util_hex2bin
    ldx r5
    ldy r5+1
    lda r5+2

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
