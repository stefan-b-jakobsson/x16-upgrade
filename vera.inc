VERA_STATUS_UNSELECTED = 0
VERA_STATUS_SELECTED = 1
VERA_STATUS_PRECHECK_FAIL = 2
VERA_STATUS_PRECHECK_PASS = 3
VERA_STATUS_LOAD_FAIL = 4
VERA_STATUS_LOAD_PASS = 5
VERA_STATUS_UPDATE_FAIL = 6
VERA_STATUS_UPDATE_PASS = 7
VERA_STATUS_VERIFY_FAIL = 8
VERA_STATUS_VERIFY_PASS = 9

;******************************************************************************
; Function name.......: vera_init
; Purpose.............: Initialize VERA update functions
; Input...............: Nothing
; Returns.............: Nothing
.proc vera_init
    lda #VERA_STATUS_UNSELECTED
    sta vera_status
    rts
.endproc

;******************************************************************************
; Function name.......: vera_run_prechecks
; Purpose.............: Runs VERA pre-upgrade checks
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_run_prechecks
    lda vera_status
    bne :+
    rts

:   print 0, str4_vera
    print 0, str_fail
    lda #13
    jsr CHROUT

    print 1, str4_vera_nys

    lda #VERA_STATUS_PRECHECK_FAIL
    sta vera_status
    sec
    rts
.endproc

;******************************************************************************
; Function name.......: vera_update
; Purpose.............: Updates the VERA
; Input...............: Nothing
; Returns.............: Nothing
; Error...............: C=1 on error
.proc vera_update
    print 1, ns
    rts

ns:
    .byt "Not yet supported", 13, 0
.endproc

;******************************************************************************
; Global variables
vera_status:
    .res 1
