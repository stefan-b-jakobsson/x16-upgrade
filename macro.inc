.macro print indent, strptr
    .scope
        lda #indent
        ldx #<strptr
        ldy #>strptr
        jsr util_print_str
    .endscope
.endmacro
