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

HEADER_FIXED_PART_SIZE = 103
HEADER_MIN_VER = 1
HEADER_MAX_VER = 1

HEADER_BLOB_ROM = 1
HEADER_BLOB_VERA = 2
HEADER_BLOB_SMC = 3

;******************************************************************************
; Function name.......: header_init
; Purpose.............: Set header default values
; Input...............: Nothing
; Returns.............: Nothing
.proc header_init
    ; Clear BLOB info (ROM, VERA, SMC)
    ldx #header_smc_start-header_rom_ver+3
    lda #0
:   sta header_rom_ver-1,x
    dex
    bne :-

    rts
.endproc

;******************************************************************************
; Function name.......: header_fetch
; Purpose.............: Reads package header from file; also gets the total
;                       file size
; Input...............: Nothing
; Returns.............: Header data stored in the following variables:
;                       Fixed part:
;                       - header_magic          6 bytes
;                       - header_ver            1 byte
;                       - header_pkg_info       64 bytes, null-terminated
;                       - header_created_by     16 bytes, null-terminated
;                       - header_created_on     14 bytes
;                       - header_blob_count     2 bytes
;                       - Total                 103 bytes
;                       
;                       Blob metadata:
;                       - header_rom_ver        1 byte
;                       - header_rom_size       3 bytes
;                       - header_rom_crc        2 bytes
;                       - header_rom_start      3 bytes
;                       - header_vera_ver       3 bytes
;                       - header_vera_size      3 bytes
;                       - header_vera_crc       2 bytes
;                       - header_vera_start     3 bytes
;                       - header_smc_ver        3 bytes
;                       - header_smc_size       3 bytes
;                       - header_smc_crc        2 bytes
;                       - header_smc_start      3 bytes
;
;                       Header CRC-16:
;                       - header_crc1           1 byte
;                       - header_crc2           1 byte
;
;                       Package file size:
;                       - header_file_size      3 bytes
; Error...............: C=1 on read error
.proc header_fetch
    ; Reset CRC
    jsr crc16_init
    
    ; Open package file
    ldx #<file_name
    ldy #>file_name
    lda file_name_len
    jsr file_open
    bcc :+
    rts

    ; Read fixed part of header (103 first bytes from the package file) into header_buf
:   lda #<header_buf    ; Target pointer
    sta r1
    lda #>header_buf
    sta r1+1
    lda #103            ; Byte count
    sta r2
    stz r2+1
    stz r2+2
    stz r4              ; No callback
    stz r4+1
    jsr file_read       ; Read header
    bcc :+              ; Read error?
    jmp load_err

:   ; Store BLOB count
    lda header_blob_count
    sta blobs
    lda header_blob_count+1
    sta blobs+1

    ; Check BLOB count: If no BLOBs, continue to header CRC
    lda blobs
    ora blobs+1
    bne :+
    jmp crc

    ; Calculate file seek offset of first BLOB (= 103 + 16 x BLOB count + 2)
:   lda blobs               ; BLOB count x 16
    sta header_cur_addr
    lda blobs+1
    sta header_cur_addr+1
    stz header_cur_addr+2
    ldx #4
:   asl header_cur_addr
    rol header_cur_addr+1
    rol header_cur_addr+2
    dex
    bne :-
    
    clc                     ; Add 103 + 2 = 105 (header fixed size)
    lda header_cur_addr
    adc #105
    sta header_cur_addr
    ldx #0
:   lda header_cur_addr,x
    adc #0
    sta header_cur_addr,x
    inx
    cpx #2
    bne :-

loop:
    ; Read BLOB entry (16 bytes) from package file
    lda #<header_blob   ; Target address
    sta r1
    lda #>header_blob
    sta r1+1
    lda #16             ; Read 16 bytes
    sta r2
    stz r2+1
    stz r2+2
    jsr file_read
    bcc :+              ; Check read error
    jmp load_err

    ; Store data depending on BLOB type
:   lda header_blob_id
    cmp #HEADER_BLOB_ROM
    beq set_rom
    cmp #HEADER_BLOB_VERA
    beq set_vera
    cmp #HEADER_BLOB_SMC
    beq set_smc
    
    ; Unknown BLOB type, ignore and just increment package file seek offset
    bra inc_addr

next:
    ; Decrement counter for remaining BLOBs
    lda blobs
    bne :+
    dec blobs+1
:   dec blobs

    ; Check if BLOB counter is 0
    lda blobs
    ora blobs+1
    beq crc ; yes, now read the header CRC
    bra loop ; no, continue reading remaining BLOBs

set_rom:
    ldx #8
:   lda header_blob_ver-1,x
    sta header_rom_ver-1,x
    dex
    bne :-
    ldx #3
:   lda header_cur_addr-1,x
    sta header_rom_start-1,x
    dex
    bne :-
    bra inc_addr

set_vera:
    ldx #8
:   lda header_blob_ver-1,x
    sta header_vera_ver-1,x
    dex
    bne :-
    ldx #3
:   lda header_cur_addr-1,x
    sta header_vera_start-1,x
    dex
    bne :-
    bra inc_addr

set_smc:
    ldx #8
:   lda header_blob_ver-1,x
    sta header_smc_ver-1,x
    dex
    bne :-
    ldx #3
:   lda header_cur_addr-1,x
    sta header_smc_start-1,x
    dex
    bne :-

inc_addr:
    ; Calculate package file seek offset of the next BLOB
    clc
    lda header_cur_addr
    adc header_blob_size
    sta header_cur_addr
    
    lda header_cur_addr+1
    adc header_blob_size+1
    sta header_cur_addr+1
    
    lda header_cur_addr+2
    adc header_blob_size+2
    sta header_cur_addr+2
    
    bra next

crc:
    ; Store header CRC-16 value
    lda crc16_val
    sta header_crc1
    lda crc16_val+1
    sta header_crc1+1

    ; Read CRC-16 value from package file
    lda #<header_crc2   ; target address
    sta r1
    lda #>header_crc2
    sta r1+1
    lda #2              ; byte count
    sta r2
    stz r2+1
    stz r2+2
    jsr file_read
    bcs load_err        ; check file read error

    ; Unselect ROM if size is 0
    lda header_rom_size
    ora header_rom_size+1
    ora header_rom_size+2
    bne :+
    lda #ROM_STATUS_UNSELECTED
    sta rom_status

    ; Unselect VERA if size is 0
:   lda header_vera_size
    ora header_vera_size+1
    ora header_vera_size+2
    bne :+
    lda #VERA_STATUS_UNSELECTED
    sta vera_status

    ; Unselect SMC if size is 0
:   lda header_smc_size
    ora header_smc_size+1
    ora header_smc_size+2
    bne :+
    lda #SMC_STATUS_UNSELECTED
    sta smc_status

    ; Close package file
:   jmp file_close

load_err:
    jsr file_close
    print 0, str1_header_load_err
    sec
    rts

blobs:
    .res 2

cur_addr:
    .res 3
.endproc

;******************************************************************************
; Function name.......: header_run_prechecks
; Purpose.............: Run header pre-upgrade checks
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc header_run_prechecks
t1: 
    ; Check magic string
    ldx #0

    lda header_ver
    cmp #2
    bcs t1_m2

t1_m1:
    ; Magic string for package version 1
:   lda magic_1,x
    beq t2
    cmp header_magic,x
    bne t1_err
    inx
    bra :-

t1_m2:
    ; Magic string for package version 2 and above
:   lda magic_2,x
    beq t2
    cmp header_magic,x
    bne t1_err
    inx
    bra :-

t1_err:
    print 0,str1_header_magic_err
    sec
    rts

t2:
    ; Check package file version
    lda header_ver
    cmp #HEADER_MIN_VER
    bcc :+
    cmp #HEADER_MAX_VER+1
    bcc t3 ; ok
:   print 0, str1_header_unsupported
    sec
    rts

t3:
    ; Check header CRC-16
    lda header_crc1
    cmp header_crc2
    bne :+
    lda header_crc1+1
    cmp header_crc2+1
    beq pass

:   print 0,str1_header_crc_err
    sec
    rts

pass:
    clc
    rts

magic_1:
    .byt "X16PKG",0

magic_2:
    .byt "x16pkg",0

size:
    .res 3
.endproc

header_buf:
    header_magic:       .res 6
    header_ver:         .res 1
    header_pkg_info:    .res 64
    header_created_by:  .res 16
    header_created_on:  .res 14
    header_blob_count:  .res 2
    header_blob:
     header_blob_id:    .res 1
     header_blob_ver:   .res 3
     header_blob_size:  .res 3
     header_blob_crc:   .res 2
     header_blob_rsrv:  .res 7
    header_crc1:        .res 2
    header_crc2:        .res 2


header_rom_ver:         .res 3
header_rom_size:        .res 3
header_rom_crc:         .res 2
header_rom_start:       .res 3

header_vera_ver:        .res 3
header_vera_size:       .res 3
header_vera_crc:        .res 2
header_vera_start:      .res 3

header_smc_ver:         .res 3
header_smc_size:        .res 3
header_smc_crc:         .res 2
header_smc_start:       .res 3

header_cur_addr:        .res 3
