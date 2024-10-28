; Kernal functions (ROM bank 0)
GETIN = $ffe4
CHRIN = $ffcf
CHROUT = util_chrout    ; Replaced by this custom function
PLOT = util_plot        ; Replaced by this custom function
SCREEN = $ffed

I2C_READ_BYTE = $fec6
I2C_WRITE_BYTE = $fec9

SETNAM = $ffbd
SETLFS = $ffba
OPEN = $ffc0
CHKIN = $ffc6
MACPTR = $ff44  
READST = $ffb7
CLOSE = $ffc3
CLRCHN = $ffcc

; Math functions (ROM bank 4)
MOVFM = $fe63 
MOVMF = $fe66
FMULT = $fe1e
QINT = $fe8d

; VERA
VERA_ADDR0 = $9f20
VERA_ADDR1 = $9f21
VERA_ADDR2 = $9f22
VERA_D0 = $9f23

; Registers
ROM_SEL = $01
RAM_SEL = $00
r0 = $02
r1 = $04
r2 = $06
r3 = $08
r4 = $0a
r5 = $0c
r6 = $0e
