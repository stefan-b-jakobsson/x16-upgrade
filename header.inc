HEADER_FIXED_PART_SIZE = 103
HEADER_MIN_VER = 1
HEADER_MAX_VER = 1

;******************************************************************************
; Function name.......: header_fetch
; Purpose.............: Reads package header from file; also gets the total
;                       file size
; Input...............: Nothing
; Returns.............: Fetched data stored in the following variables:
;                       - header_magic          6 bytes
;                       - header_ver            1 byte
;                       - header_pkg_info       64 bytes, null-terminated
;                       - header_created_by     16 bytes, null-terminated
;                       - header_created_on     14 bytes
;                       - header_blob_count     2 bytes
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

    ; Fetch fixed part of header (103 bytes)
:   lda #<header_buf
    sta r1
    lda #>header_buf
    sta r1+1
    lda #103
    sta r2
    stz r2+1
    stz r2+2
    stz r4
    stz r4+1
    jsr file_read
    bcc :+
    jmp load_err

:   jsr file_get_size
    stx header_file_size
    sty header_file_size+1
    sta header_file_size+2

    ; Set blob counter
    lda header_blob_count
    sta blobs
    lda header_blob_count+1
    sta blobs+1

    ; Go to header CRC if no blobs
    lda blobs
    ora blobs+1
    bne :+
    jmp crc

    ; Calculate file seek offset of first BLOB (103 + 16 x BLOB count + 2)
:   lda blobs
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
    
    clc
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
    ; Read BLOB entry (16 bytes)
    lda #<header_blob
    sta r1
    lda #>header_blob
    sta r1+1
    lda #16
    sta r2
    stz r2+1
    stz r2+2
    jsr file_read
    bcc :+
    jmp load_err

    ; Check BLOB type
:   lda header_blob_id
    cmp #1
    beq set_rom
    cmp #2
    beq set_vera
    cmp #3
    beq set_smc

next:
    lda blobs
    bne :+
    dec blobs+1
:   dec blobs
    lda blobs
    ora blobs+1
    beq crc
    bra loop

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
    ; Store calculated CRC-16 value
    lda crc16_val
    sta header_crc1
    lda crc16_val+1
    sta header_crc1+1

    ; Read CRC-16 value from file
    lda #<header_crc2
    sta r1
    lda #>header_crc2
    sta r1+1
    lda #2
    sta r2
    stz r2+1
    stz r2+2
    jsr file_read
    bcs load_err

exit:
    lda #<header_rom_ver
    lda #>header_rom_ver
    jmp file_close

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
    ; Check magic string
t1: 
    ldx #0
:   lda magic,x
    beq t2
    cmp header_magic,x
    bne :+
    inx
    bra :-

:   print 0,str1_header_magic_err
    sec
    rts

t2:
    lda header_ver
    cmp #HEADER_MIN_VER
    bcc t3
    cmp #HEADER_MAX_VER+1
    bcc t3

t3:
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

magic:
    .byt "X16PKG",0

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

header_file_size:       .res 3

header_cur_addr:        .res 3
