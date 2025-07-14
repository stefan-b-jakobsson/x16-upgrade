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


;******************************************************************************
; Function name.......: crc16_init
; Purpose.............: Resets the CRC-16 value to $ffff
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: None
.proc crc16_init
    lda #$ff
    sta crc16_val
    sta crc16_val+1
    rts
.endproc

;******************************************************************************
; Function name.......: crc16_update
; Purpose.............: Updates the CRC-16 value. The current value is stored
;						in variable crc16_val (16 bits)
; Input...............: A = byte value
; Returns.............: Nothing
; Error...............: None
.proc crc16_update
    ; Taken from Commander X16 ROM, that took it from
    ; http://www.6502.org/source/integers/crc-more.html
    ; (November 23rd, 2004, "alternate ending" version, preserving .Y)

	eor crc16_val+1 ; A contained the data
	sta crc16_val+1 ; XOR it into high byte
	lsr             ; right shift A 4 bits
	lsr             ; to make top of x^12 term
	lsr             ; ($1...)
	lsr
	tax             ; save it
	asl             ; then make top of x^5 term
	eor crc16_val         ; and XOR that with low byte
	sta crc16_val         ; and save
	txa             ; restore partial term
	eor crc16_val+1         ; and update high byte
	sta crc16_val+1         ; and save
	asl             ; left shift three
	asl             ; the rest of the terms
	asl             ; have feedback from x^12
	tax             ; save bottom of x^12
	asl             ; left shift two more
	asl             ; watch the carry flag
	eor crc16_val+1         ; bottom of x^5 ($..2.)
	sta crc16_val+1         ; save high byte
	txa             ; fetch temp value
	rol             ; bottom of x^12, middle of x^5!
	eor crc16_val   ; finally update low byte
	ldx crc16_val+1 ; then swap high and low bytes
	sta crc16_val+1
	stx crc16_val
	rts
.endproc

;******************************************************************************
; Function name.......: crc16_buffer
; Purpose.............: Calculates CRC16 for the specified RAM buffer
; Input...............: r1		Pointer to start of buffer
;						r2/r3L	Buffer len
;						r4		Callback pointer, or 0 if no callback
; Returns.............: X/Y		CRC16 (low/high)
; Error...............: None
.proc crc16_buffer
	jsr crc16_init

loop:
	; Check if done
	lda r2
	ora r2+1
	ora r2+2
	bne :+

	; Yes, exit
	jsr callback2			; Final callback
	ldx crc16_val
	ldy crc16_val+1
	rts

:	; Add next value
	lda (r1)
	jsr crc16_update

	; Decrement byte count
	lda r2
	bne :++
	lda r2+1
	bne :+
	dec r2+2
:	dec r2+1
:	dec r2

	; Increment pointer
	inc r1
	bne callback
	inc r1+1

	; On overflow into I/O area, go to $01:$a000
	lda r1+1
	cmp #$9f
	bne :+
	lda #1
	sta RAM_SEL
	lda #$a0
	sta r1+1
	bra callback

:	; On overflow into ROM, go to start of next RAM bank
	cmp #$c0
	bcc callback
	inc RAM_SEL
	lda #$a0
	sta r1+1

callback:
	; Callbacks on every 256 byte
	lda r2
	bne loop

	; Check if callback pointer is null
	lda r4
	ora r4+1
	beq loop

	; Jump to callback pointer
	jsr callback2
	bra loop
callback2:
	jmp (r4)
	rts
.endproc

crc16_val: .res 2
