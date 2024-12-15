version_major = 0
version_minor = 0
version_patch = 1

.proc version_print
    lda #version_major
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda #version_minor
    jsr util_print_num
    lda #'.'
    jsr CHROUT
    lda #version_patch
    jmp util_print_num   
.endproc
