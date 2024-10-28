.proc crc16_init
    lda #$ff
    sta crc16_val
    sta crc16_val+1
    rts
.endproc

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

crc16_val: .res 2
