;===========================================================================
; DESCRIPTION:
;
; This file contains the STM8 option bytes definition.
; You can change their values below according to the device datasheet.
; Be careful to respect the complement bytes,
;
; Refer to the datasheet for more information on the meaning of each bit.
;
;---------------------------------------------------------------------------
; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
; ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
; OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;---------------------------------------------------------------------------
; Copyright (c) Raisonance S.A.S., 2004-2009
;==========================================================================*/

CSEG    AT      04800h

        DB      000h                    ;LOCKBYTE

        DB      000h                    ;OPT1
        DB      0FFh                    ;NOPT1
                                        
        DB      000h                    ;OPT2
        DB      0FFh                    ;NOPT2
                                        
        DB      000h                    ;OPT3
        DB      0FFh                    ;NOPT3
                                        
        DB      000h                    ;OPT4
        DB      0FFh                    ;NOPT4
                                        
        DB      000h                    ;OPT5
        DB      0FFh                    ;NOPT5
                                        
        DB      000h                    ;OPT6
        DB      0FFh                    ;NOPT6
                                        
        DB      000h                    ;OPT7 ; WAITSTATE, CPU > 16 MHz
        DB      0FFh                    ;NOPT7


END
