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

ENTER_BASIC = $ff47

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
VERA_D1 = $9f24
VERA_CTRL = $9f25

VERA_DC_VIDEO = $9f29
VERA_DC_HSCALE = $9f2a
VERA_DC_VSCALE = $9f2b
VERA_DC_BORDER = $922c

VERA_DC_HSTART = $9f29
VERA_DC_HSTOP = $9f2a
VERA_DC_VSTART = $9f2b
VERA_DC_VSTOP = $9f2c

VERA_L1_CONFIG = $9f34
VERA_L1_MAPBASE = $9f35
VERA_L1_TILEBASE = $9f36
VERA_L1_HSCROLL_L = $9f37  
VERA_L1_HSCROLL_H = $9f38
VERA_L1_VSCROLL_L = $9f39
VERA_L1_VSCROLL_H = $9f3a

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
