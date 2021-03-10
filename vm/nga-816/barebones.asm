; Copyright 2021 Piotr Meyer <aniou@smutek.pl>
;
; Permission to use, copy, modify, and/or distribute this
; software for any purpose with or without fee is hereby
; granted, provided that the above copyright notice and
; this permission notice appear in all copies.

; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
; ALL WARRANTIES WITH REGARD TO THIS SOFTWARE  INCLUDING ALL
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
; EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
; INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
; WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
; TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
; USE OR PERFORMANCE OF THIS SOFTWARE.

.cpu "65816"

.include "macros_inc.asm"

;----------------------------------------------------------
; # Description
;
; RETRO/816 - a port of RETRO Forth to C256 Foenix 
; RETRO Forth was created by Charles Childers (crc)
; see: http://retroforth.org/
;
; Program is created for C256 Foenix computer but should be
; able to run on almost any compatible system.
;
; At this moment we provide single NGA machine with fixed
; addresses at ZP and in main memory, but there is a room
; for multiple, independent VMs

; ## porting
;
; Current version is designed to be run on C256 Foenix
; computer with Foenix Kernel loaded
; We need two functions to be supported:
; C256_GETCHW - "wait for char and return it in A"
; C256_PUTC   - "print char from A to screen"
;
; ## memory layout in C256
;
; $0040 - begin of shared regions, used by various routines
; $00E0 - end   of shared regions
; $00F0 - 16 bytes of 'temporary user variables;
;
; $2000 - begin of free space (test code in Foenix)
; $7FFF - end   of free space
; $8000 - begin of CPU stack
; $FEFF - end   of CPU stack
;
; XXX - move it to ZP
; $01:0000 - beginning NGA (memory), single segment (64k)
;     0000 - begin of data stack
;     03ff - end of data stack
;     0400 - beign of return
;     07ff - end of return stack
;     .... - unused
; $02:0000 - start of main NGA memory
; $05:FFFF - end of main NGA memory
;
; $3a:0000 - beginning of NGA code (overwrites BASIC)
; ..
;
; ## implementation-specific notes
;
; There are few shortcuts and many inefficiences in code,
; it should be corrected or extend in future releases
;
; At this moment main pointers are somewhat inconsistent
; - IP counts CELLS when SP i RP count BYTES. It simplify
; code a lot
;
; IP    - 16bit instruction pointer, thats means that
;         system is able to use only FFFF cells
;         unit: CELLS
;
; IPPTR - 24bit in-memory pointer, allows to use
;         4*FFFF memory. It should be equal to IP << 2
;         unit: BYTES
;
; SP    - 16bit, stack pointer
;         unit: BYTES (inc/dec by 4 bytes)
;
; RP    - 16bit, return addres stack pointer,
;         unit: BYTES (inc/dec by 4 bytes)
;
; ## booting
;
; C256 boot process, as remainder:
;
; 1. after boot CPU PC gets addr from $FFFC,
; 2. that value points to $FF00 and following code
;    CLC
;    XCE
;    JML $1000 - BOOT vector of Foenix Kernel
; 3. JML IBOOT - internal boot routine
; 4. ... and, finally JML $03:A0000 to init BASIC

;----------------------------------------------------------
; # constants
;
; note - unlike in 65c816 stacks in original NGA grows up,
STACK_DEPTH =    $0400 ; bytes: depth of data stack
ADDRESSES   =    $0400 ; bytes: depth of address stack
IMAGE_ADDR  = $02_0000 ; bytes: base address in real memory
CELL_MAX    =    $FFFF ; max allowed cell, IP is word-sized
IMAGE_BANKS =        4 ; by 64k, max with word-sized IP
CELL_SIZE   =        4 ; bytes: single CELL size

; some sanitization checks
.cerror IMAGE_ADDR  & $00FFFF != $0000, "IMAGE_ADDR  should be bank-aligned!"

; TOS*, NOS*, TRS* and MEM* variant are accessed by indexed
; modes (,X and ,Y). Different base addresses (+0, +2) are
; used to access low and high words without extra inx/iny
DSTACK      = $0000      ; data   stack addr, grows up
NOSl        = DSTACK     ; second item  (,X)
NOSh        = DSTACK + 2 ; second item  (,X)
TOSl        = DSTACK + 4 ; current item, low  word
TOSh        = DSTACK + 6 ; current item, high word

RSTACK      = $0400      ; return stack addr, grows up
TRSl        = RSTACK     ; current stack item, low word
TRSh        = RSTACK + 2 ; current stack item. high word

; XXX - only used for stacks, rip it off
MEM_SEGMENT = $01        ; memory bank segment: $01:xxxx

; nymber of devices supported by system
NUM_DEVICES = 2

; ## debug variables (only for go65c816 emulator)
TRACE_ON    = $10
TRACE_OFF   = $11
KILL        = $20

; ## FMX kernel vectors
C256_GETCHW = $104c              ; get character (wait)
C256_PUTC   = $1018              ; put character

; ---------------------------------------------------------
; # local variables
;
            * = $60
IP          .word  0  ; instruction pointer - cells
IPPTR       .dword 0  ; instruction pointer - bytes
SP          .word  0  ; data   stack pointer - bytes
RP          .word  0  ; return stack pointer - bytes
CMD         .dword 0  ; temporary for OP unboundling
TMP         .dword 0  ; temporary
TMPa        = TMP     ; additional identifiers
TMPb        .dword 0  ; for various cases
TMPc        .word  0  ; at this moment inst_di
TMPd        .word  0  ; ...

; ---------------------------------------------------------
; # main routine
;
            * = $03A0000

main
            clc
            xce

main0       .setaxl
            .sdb `msg_banner
            ldx  #<>msg_banner
            jsr  prints

            jsr  prepare_vm
            jsr  execute

            .sdb `msg_end
            ldx  #<>msg_end
            jsr  prints

            jsl  C256_GETCHW
            jml  $1000           ; BOOT

; ## preparing environment
;
; 1. clear memory region
; 2. clear stacks
; 3. copy image to memory region
;
prepare_vm
            ; 1. clear memory
            .sdb `msg_mclean
            ldx  #<>msg_mclean
            jsr  prints

            .setas
            .setxl
            lda  #`IMAGE_ADDR
            sta  TMP
            ldy  #IMAGE_BANKS

mclean0     lda  TMP
            pha
            plb
            ldx  #$0000
mclean1     stz  $0,b,x
            inx
            bne  mclean1
            inc  TMP
            dey
            bne  mclean0

            ; 2. clear stacks
            .sdb `msg_sclean
            ldx  #<>msg_sclean
            jsr  prints

            .sdb MEM_SEGMENT
            .setal
            ldx  #STACK_DEPTH-2
prep0       stz  #DSTACK,b,x
            dex
            dex
            bpl  prep0

            ldx  #ADDRESSES-2
prep1       stz  #RSTACK,b,x
            dex
            dex
            bpl  prep1


            ; 4. copy image
            .sdb `msg_copy
            ldx  #<>msg_copy
            jsr  prints

            .setas
            ldy  #IMAGE_SIZE
            ldx  #0
            .databank ?
copy0       lda  IMAGE_SRC,x
            sta  IMAGE_ADDR,x
            inx
            dey
            bne  copy0

            ; 4. set DBR to stack area
            .sdb MEM_SEGMENT                    ; XXX fix it

            rts

; ## main execute loop
execute
            .setaxl
            lda  #CELL_SIZE
            sta  RP
            stz  SP
            stz  IP
            jsr  update_ipptr

execute0    jsr  process_bundle
            wdm  #4              ; debugging - op count
            lda  RP
            beq  quit

            jsr  next_ipptr
            inc  IP
            lda  IP
            cmp  #CELL_MAX       ; NGA exit condition
            bcc                  execute0
quit        rts

; ### process 4 commands in bundle
process_bundle
            ldy  #2
            lda  [IPPTR],y       ; 7 cycles
            sta  CMD+2
            lda  [IPPTR]         ; also 7 cycles
            sta  CMD

            and   #$ff
            beq   +              ; skip .. (nop)
            asl   a
            tax
            jsr   (#<>op_table,k,x)

+           lda   CMD+1
            and   #$ff
            beq   +              ; skip .. (nop)
            asl   a
            tax
            jsr   (#<>op_table,k,x)

+           lda   CMD+2
            and   #$ff
            beq   +              ; skip .. (nop)
            asl   a
            tax
            jsr   (#<>op_table,k,x)

+           lda   CMD+3
            and   #$ff
            beq   +              ; bne/rts for -1 cycle
            asl   a
            tax
            jsr   (#<>op_table,k,x)
+           rts

op_table
            .addr inst_no
            .addr inst_li
            .addr inst_du
            .addr inst_dr
            .addr inst_sw
            .addr inst_pu
            .addr inst_po
            .addr inst_ju
            .addr inst_ca
            .addr inst_cc
            .addr inst_re
            .addr inst_eq
            .addr inst_ne
            .addr inst_lt
            .addr inst_gt
            .addr inst_fe
            .addr inst_st
            .addr inst_ad
            .addr inst_su
            .addr inst_mu
            .addr inst_di
            .addr inst_an
            .addr inst_or
            .addr inst_xo
            .addr inst_sh
            .addr inst_zr
            .addr inst_ha
            .addr inst_ie
            .addr inst_iq
            .addr inst_ii

;----------------------------------------------------------
; ## tooling routines

; ### updates IPPTR (in bytes) from IP field (in cells)
update_ipptr
            lda  IP
            sta  IPPTR
            stz  IPPTR+2

            asl  IPPTR        ; IPPTR = IP*4
            rol  IPPTR+2

            asl  IPPTR
            rol  IPPTR+2

            clc               ; add base
            lda  IPPTR+2
            adc  #`IMAGE_ADDR
            sta  IPPTR+2

            rts

; ### increases in-memory IPPTR pointer by CELL_SIZE
next_ipptr
            lda  IPPTR
            clc
            adc  #CELL_SIZE
            sta  IPPTR
            bcs  +
            rts

+           inc  IPPTR+2
            rts

; ### print 0-terminated strings
; DBR - string segment
;   X - string address
prints      .proc
            php
            .setas
            .setxl
prints0     lda  $0,b,x
            beq  prints_done
            jsl  C256_PUTC
            inx
            bra  prints0

prints_done plp
            rts
            .pend



;----------------------------------------------------------
; # NGA VM
;
; Implementation of nga VM, based on `vm/nga-c/nga.c` code.
; Current version may be suboptimal, but the goal is in most
; accurate implementation.
;

            .al
            .xl

; ---------------------------------------------------------
; ## .. ( 0) stack:   -   |   -    nop

inst_no
            rts

; ---------------------------------------------------------
; ## li ( 1) stack:   -n  |   -    lit
;
; void inst_li() {
;   sp++;
;   ip++;
;   TOS = memory[ip];
; }

inst_li
            lda   SP              ; 4 cycles
            clc                   ; 1 cycle
            adc   #CELL_SIZE      ; 3 cycles
            sta   SP              ; 4 cycles
            tax                   ; 2 cycles

            inc  IP
            jsr  next_ipptr
            lda  [IPPTR]
            sta  #TOSl,b,x
            ldy  #2
            lda  [IPPTR],y
            sta  #TOSh,b,x

;            lda   IP              ; 4 cycles
;            clc                   ; 1 cycle
;            adc   #4              ; 3 cycles
;            sta   IP              ; 4 cycles
;            tay                   ; 2 cycles

;            lda   #MEMl,b,y
;            sta   #TOSl,b,x
;            lda   #MEMh,b,y
;            sta   #TOSh,b,x

            rts

; ---------------------------------------------------------
; ## du ( 2) stack:  n-nn |   -    dup
;
; void inst_du() {
;   sp++;
;   data[sp] = NOS;               // it means TOS = NOS?
; }

inst_du
            lda   SP              ; 4 cycles
            clc                   ; 1 cycle
            adc   #CELL_SIZE      ; 3 cycles
            sta   SP              ; 4 cycles
            tax                   ; 2 cycles

            lda   #NOSl,b,x
            sta   #TOSl,b,x
            lda   #NOSh,b,x
            sta   #TOSh,b,x

            rts

; ---------------------------------------------------------
; ## dr ( 3) stack:  n-   |   -    drop
;
; void inst_dr() {
;   data[sp] = 0;                 // it means TOS=0?
;    if (--sp < 0)
;      ip = CELL_MAX;
; }

inst_dr
            ldx   SP
            stz   #TOSl,b,x
            stz   #TOSh,b,x

            txa
            sec
            sbc   #4
            sta   SP
            bmi   inst_dr0
            rts

            ; IP+1 in exec loop == LIMIT == EXIT
inst_dr0    lda   #CELL_MAX-1
            sta   IP
            rts


; ---------------------------------------------------------
; ## sw ( 4) stack: xy-xy |   -    swap
;
; void inst_dr() {
;   data[sp] = 0;                 // it means TOS=0?
;    if (--sp < 0)
;      ip = CELL_MAX;
; }

inst_sw
            ldx   SP

            ldy   #TOSl,b,x        ; TOS -> TMP
            lda   #NOSl,b,x
            sta   #TOSl,b,x        ; NOS -> TOS
            tya
            sta   #NOSl,b,x        ; TMP -> NOS

            ldy   #TOSh,b,x        ; TOS -> TMP
            lda   #NOSh,b,x
            sta   #TOSh,b,x        ; NOS -> TOS
            tya
            sta   #NOSh,b,x        ; TMP -> NOS

            rts

; ---------------------------------------------------------
; ## pu ( 5) stack:  n-   |   -n   push
;
; void inst_pu() {
;  rp++;
;  TORS = TOS;
;  inst_dr();
; }

inst_pu
            lda   RP              ; 4 cycles
            clc                   ; 1 cycle
            adc   #CELL_SIZE      ; 3 cycles
            sta   RP              ; 4 cycles
            tay                   ; 2 cycles

            ldx   SP
            lda   #TOSl,b,x
            sta   #TRSl,b,y
            lda   #TOSh,b,x
            sta   #TRSh,b,y

            jmp   inst_dr

; ---------------------------------------------------------
; ## po ( 6) stack:   -n  |  n-    pop
;
; void inst_po() {
;   sp++;
;   TOS = TORS;
;   rp--;
; }

inst_po
            lda   SP
            clc
            adc   #CELL_SIZE
            sta   SP
            tax

            ldy   RP

            lda   #TRSl,b,y
            sta   #TOSl,b,x
            lda   #TRSh,b,y
            sta   #TOSh,b,x

            tya
            sec
            sbc   #4
            sta   RP
            rts

; ---------------------------------------------------------
; ## ju ( 7) stack:  a-   |   -    jump
;
; void inst_ju() {
;   ip = TOS - 1;         // I'm not sure about that '-1'
;   inst_dr();
; }

; PROBLEM THERE - SP is 16-bit and argument to JUMP may
; be 32bit  XXX - check it in already created image
; BUT - current image < 64k, so there shouldn't be problems

inst_ju
            ldx  SP
            lda  #TOSl,b,x

            dec  a
            sta  IP
            jsr  update_ipptr
            jmp  inst_dr

; ---------------------------------------------------------
; ## ca ( 8) stack:  a-   |   -A   call
;
; void inst_ca() {
;   rp++;
;   TORS = ip;
;   ip = TOS - 1;
;   inst_dr();
; }

inst_ca
            lda  RP
            clc
            adc  #CELL_SIZE
            sta  RP
            tay

            lda  IP
            sta  #TRSl,b,y

            ; for completness sake
            ldx  SP
            lda  #TOSl,b,x

            dec  a
            sta  IP
            jsr  update_ipptr
            jmp  inst_dr


; ---------------------------------------------------------
; ## cc ( 9) stack: af-   |   -A   conditional call
;
; void inst_cc() {
;   CELL a, b;
;   a = TOS; inst_dr();  /* Target */
;   b = TOS; inst_dr();  /* Flag   */
;   if (b != 0) {
;     rp++;
;     TORS = ip;
;     ip = a - 1;
;   }
; }

inst_cc
            ldx   SP              ; a
            lda   #TOSl,b,x
            sta   TMP
            jsr   inst_dr

            ldx   SP
            lda   #TOSl,b,x
            bne   inst_cc_jmp
            lda   #TOSh,b,x
            bne   inst_cc_jmp
            jmp   inst_dr

inst_cc_jmp jsr   inst_dr         ; for compatibility
            lda   RP
            clc
            adc   #CELL_SIZE
            sta   RP
            tay

            lda   IP
            sta   #TRSl,b,y
            lda   #$0
            sta   #TRSh,b,y       ; only lower..

            lda   TMP
            dec   a
            sta   IP
            jsr  update_ipptr

            rts
            ;jmp   inst_dr

; ---------------------------------------------------------
; ## re (10) stack:   -   |  A-    return
;
; void inst_re() {
;   ip = TORS;
;   rp--;
; }

inst_re
            ldy   RP
            lda   #TRSl,b,y
            sta   IP
            jsr  update_ipptr

            tya
            sec
            sbc   #4
            sta   RP

            rts

; ---------------------------------------------------------
; ## eq (11) stack: xy-f  |   -    equality
;
; void inst_eq() {
;   NOS = (NOS == TOS) ? -1 : 0;
;   inst_dr();
; }

inst_eq
            ldx   SP
            lda   #NOSl,b,x
            cmp   #TOSl,b,x
            bne   inst_eq_no

            lda   #NOSh,b,x
            cmp   #TOSh,b,x
            bne   inst_eq_no

            lda   #<>-1
            sta   #NOSl,b,x
            lda   #>`-1
            sta   #NOSh,b,x
            jmp   inst_dr

inst_eq_no  stz   #NOSl,b,x
            stz   #NOSh,b,x
            jmp   inst_dr


; ---------------------------------------------------------
; ## ne (12) stack: xy-f  |   -    inequality
;
; void inst_eq() {
;   NOS = (NOS != TOS) ? -1 : 0;
;   inst_dr();
; }

inst_ne
            ldx   SP
            lda   #NOSl,b,x
            cmp   #TOSl,b,x
            bne   inst_ne_no

            lda   #NOSh,b,x
            cmp   #TOSh,b,x
            bne   inst_ne_no

            stz   #NOSl,b,x
            stz   #NOSh,b,x
            jmp   inst_dr

inst_ne_no  lda   #<>-1
            sta   #NOSl,b,x
            lda   #>`-1
            sta   #NOSh,b,x
            jmp   inst_dr


; ---------------------------------------------------------
; ## lt (13) stack: xy-f  |   -    less than
;
; void inst_eq() {
;   NOS = (NOS < TOS) ? -1 : 0;
;   inst_dr();
; }

; it should be a signed comparison then
; http://www.6502.org/tutorials/compare_beyond.html#5

inst_lt
            ldx   SP
            lda   #NOSl,b,x
            cmp   #TOSl,b,x
            lda   #NOSh,b,x
            sbc   #TOSh,b,x
            bvc   inst_lt0        ; N eor V
            eor   #$80
inst_lt0    bmi   inst_lt_lt

            stz   #NOSl,b,x
            stz   #NOSh,b,x
            jmp   inst_dr

inst_lt_lt  lda   #<>-1
            sta   #NOSl,b,x
            lda   #>`-1
            sta   #NOSh,b,x
            jmp   inst_dr

; ---------------------------------------------------------
; ## gt (14) stack: xy-f  |   -    greater than
;
; void inst_eq() {
;   NOS = (NOS > TOS) ? -1 : 0;
;   inst_dr();
; }

; it should be a signed comparison then
; http://www.6502.org/tutorials/compare_beyond.html#5

inst_gt
            ldx   SP
            lda   #TOSl,b,x
            cmp   #NOSl,b,x
            lda   #TOSh,b,x
            sbc   #NOSh,b,x
            bvc   inst_gt0        ; N eor V
            eor   #$80
inst_gt0    bmi   inst_gt_gt

            stz   #NOSl,b,x
            stz   #NOSh,b,x
            jmp   inst_dr

inst_gt_gt  lda   #<>-1
            sta   #NOSl,b,x
            lda   #>`-1
            sta   #NOSh,b,x
            jmp   inst_dr

; ---------------------------------------------------------
; ## fe (15) stack:  a-n  |   -    fetch
;
; void inst_fe() {
; #ifndef NOCHECKS
;   if (TOS >= CELL_MAX || TOS < -5) {
;     ip = CELL_MAX;
;     printf("\nERROR (nga/inst_fe): Fetch beyond valid memory range\n");
;     exit(1);
;   } else {
; #endif
;     switch (TOS) {
;       case -1: TOS = sp - 1; break;
;       case -2: TOS = rp; break;
;       case -3: TOS = CELL_MAX; break;
;       case -4: TOS = CELL_MIN_VAL; break;
;       case -5: TOS = CELL_MAX_VAL; break;
;       default: TOS = memory[TOS]; break;
;     }
; #ifndef NOCHECKS
;   }
; #endif
; }

; XXX - there no checks now, as we don't have a way
;       to report them
;

inst_fe
            ldx   SP
            lda   #TOSh,b,x
            bmi   inst_fe0       ; special values

            lda   #TOSl,b,x      ; only 16 bit
            sta  TMP
            stz  TMP+2

            asl  TMP             ; IPPTR = IP*4
            rol  TMP+2
            asl  TMP
            rol  TMP+2

            clc                  ; add base
            lda  TMP+2
            adc  #`IMAGE_ADDR
            sta  TMP+2

            lda  TMP

            lda  [TMP]
            sta  #TOSl,b,x
            ldy  #2
            lda  [TMP],y
            sta  #TOSh,b,x
            rts

inst_fe0    lda   #TOSl,b,x
            inc   a               ; it was -1?
            bne   inst_fe1        ; no
            lda   SP              ; "TOS = sp-1"
            dec   a
            lsr   a               ; SP in bytes
            lsr   a               ; stack uses cells
            sta   #TOSl,b,x
            stz   #TOSh,b,x
            rts

inst_fe1    inc   a               ; it was -2?
            bne   inst_fe2
            lda   RP              ; "TOS = rp"
            lsr   a
            lsr   a
            sta   #TOSl,b,x
            stz   #TOSh,b,x
            rts

inst_fe2    inc   a               ; it was -3?
            bne   inst_fe3
            lda   #CELL_MAX       ; "TOS = CELL_MAX"
            sta   #TOSl,b,x
            stz   #TOSh,b,x       ; XXX - we uses max 64k
            rts

inst_fe3    inc   a               ; it was -4?
            bne   inst_fe4
            lda   #$0000          ; "TOS = CELL_MIN_VAL"
            sta   #TOSl,b,x
            lda   #$8000          ; XXX - check it
            sta   #TOSh,b,x
            rts

inst_fe4    inc   a               ; it was -5?
            bne   inst_bad
            lda   #$ffff          ; "TOS = CELL_MAX_VAL"
            sta   #TOSl,b,x
            lda   #$7fff          ; XXX - check it
            sta   #TOSh,b,x
            rts

inst_bad                          ; XXX - message to interpreter
            .sdb `err_memuf
            ldx  #<>err_memuf
            jsr  prints
            .setaxl
            lda  #CELL_MAX-1
            sta  IP
            rts


; ---------------------------------------------------------
; ## st (16) stack: na-   |   -    store
;
; void inst_st() {
; #ifndef NOCHECKS
;   if (TOS <= CELL_MAX && TOS >= 0) {
; #endif
;     memory[TOS] = NOS;
;     inst_dr();
;     inst_dr();
; #ifndef NOCHECKS
;   } else {
;     ip = CELL_MAX;
;     printf("\nERROR (nga/inst_st): Store beyond valid memory range\n");
;     exit(1);
;   }
; #endif
; }

; XXX - no checks now

inst_st
            ldx  SP
            lda  #TOSl,b,x       ; XXX only low word in use

            sta  TMP
            stz  TMP+2

            asl  TMP             ; IPPTR = IP*4
            rol  TMP+2
            asl  TMP
            rol  TMP+2

            clc                  ; add base
            lda  TMP+2
            adc  #`IMAGE_ADDR
            sta  TMP+2

            lda  #NOSl,b,x
            sta  [TMP]
            ldy  #2
            lda  #NOSh,b,x
            sta  [TMP],y

            jsr  inst_dr
            jmp  inst_dr

; ---------------------------------------------------------
; ## ad (17) stack: xy-n  |   -    addition
;
; void inst_ad() {
;   NOS += TOS;
;   inst_dr();
; }

inst_ad
            ldx  SP
            clc
            lda   #NOSl,b,x
            adc  #TOSl,b,x
            sta  #NOSl,b,x
            lda  #NOSh,b,x
            adc  #TOSh,b,x
            sta  #NOSh,b,x
            jmp  inst_dr

; ---------------------------------------------------------
; ## su (18) stack: xy-n  |   -    subtraction
;
; void inst_su() {
;   NOS -= TOS;
;   inst_dr();
; }

inst_su
            ldx  SP
            sec
            lda   #NOSl,b,x
            sbc  #TOSl,b,x
            sta  #NOSl,b,x
            lda  #NOSh,b,x
            sbc  #TOSh,b,x
            sta  #NOSh,b,x
            jmp  inst_dr

; ---------------------------------------------------------
; ## mu (19) stack: xy-n  |   -    multiplication
;
; void inst_mu() {
;   NOS *= TOS;
;   inst_dr();
; }

; taken almost verbatim from of816 forth:
; 32-bit unsigned multiplication with 64-bit result
; right-shifting version by dclxvi

N = TMP

inst_mu
            ldx   SP
            ; only for 1:1 with original nga
            lda   #TOSh,b,x
            pha
            lda   #TOSl,b,x
            pha
            ; end

            lda   N+2
            pha
            lda   N
            pha
            lda   #$00
            sta   N
            ldy   #32
            lsr   #TOSh,b,x      ; STACKBASE+6,x
            ror   #TOSl,b,x      ; STACKBASE+4,x
l1:         bcc   l2
            clc
            sta   N+2
            lda   N
            adc   #NOSl,b,x      ; STACKBASE+0,x
            sta   N
            lda   N+2
            adc   #NOSh,b,x      ; STACKBASE+2,x
l2:         ror   a
            ror   N
            ror   #TOSh,b,x      ; STACKBASE+6,x
            ror   #TOSl,b,x      ; STACKBASE+4,x
            dey
            bne   l1
            sta   #NOSh,b,x      ; STACKBASE+2,x
            lda   N
            sta   #NOSl,b,x      ; STACKBASE+0,x
            pla
            sta   N
            pla
            sta   N+2

            ; only for 1:1 with original nga - XXX - fix it
            lda   #TOSl,b,x
            sta   #NOSl,b,x
            lda   #TOSh,b,x
            sta   #NOSh,b,x
            ;
            pla
            sta   #TOSl,b,x
            pla
            sta   #TOSh,b,x
            ; end

            jmp   inst_dr

; ---------------------------------------------------------
; ## di (20) stack: xy-rq |   -    divide & remainder
;
; void inst_di() {
;   CELL a, b;
;   a = TOS;
;   b = NOS;
; #ifndef NOCHECKS
;   if (a == 0) {
;     printf("\nERROR (nga/inst_di): Division by zero\n");
;     exit(1);
;   }
; #endif
;   TOS = b / a;
;   NOS = b % a;
; }

; XXX - now all operation are unsigned, change this!
            .warn "inst_di is unsigned, change it"

inst_di
            ldx  SP

            lda  #TOSl,b,x
            bne  di4
            lda  #TOSh,b,x
            bne  di4

            .sdb `err_0div
            ldx  #<>err_0div
            jsr  prints
            .sdb MEM_SEGMENT
            .setaxl
            lda  #CELL_MAX-1
            sta  IP
            rts

di4         lda  #NOSl,b,x       ; NOS to TMPb via TMP(a)
            sta  TMP
            lda  #NOSh,b,x
            sta  TMP+2
            bit  TMP+2
            bpl  di3

            inc  TMPd
            jsr  negate_tmp
di3         lda  TMP             ; NOS from TMP(a) to TMPb
            sta  TMPb
            lda  TMP+2
            sta  TMPb+2

            lda  #TOSl,b,x       ; TOS to TMP(a)
            sta  TMP
            lda  #TOSh,b,x
            sta  TMP+2
            bit  TMP+2
            bpl  di2

            inc  TMPd
            jsr  negate_tmp
di2         stz  #TOSl,b,x       ; prepare result space
            stz  #TOSh,b,x
            stz  #NOSl,b,x
            stz  #NOSh,b,x

di0         lda  TMPb            ; is NOS<TOS?
            cmp  TMPa
            lda  TMPb+2
            sbc  TMPa+2
            bvc  di1
            eor  #$80
di1         bmi  finish          ; yes, NOS<TOS

            sec
            lda  TMPb
            sbc  TMPa
            sta  TMPb
            lda  TMPb+2
            sbc  TMPa+2
            sta  TMPb+2

            ; increase result
            inc  #TOSl,b,x
            bne  di0             ; overflow means high+=1
            inc  #TOSh,b,x
            bra  di0

finish      lda  TMPb            ; remainder
            sta  #NOSl,b,x
            lda  TMPb+2
            sta  #NOSh,b,x
            rts

            ; additional routines for shifting ------------
negate_tmp
            jsr   invert_tmp
            inc   TMP
            bne   +
            inc   TMP+2
+           rts

invert_tmp
            lda   TMP
            eor   #$FFFF
            sta   TMP
            lda   TMP+2
            eor   #$FFFF
            sta   TMP+2
            rts



; ---------------------------------------------------------
; ## an (21) stack: xy-n  |   -    bitwise and
;
; void inst_an() {
;   NOS = TOS & NOS;
;   inst_dr();
; }

inst_an
            ldx  SP

            lda   #NOSl,b,x
            and   #TOSl,b,x
            sta  #NOSl,b,x

            lda   #NOSh,b,x
            and   #TOSh,b,x
            sta  #NOSh,b,x

            jmp  inst_dr

; ---------------------------------------------------------
; ## or (22) stack: xy-n  |   -    bitwise or
;
; void inst_an() {
;   NOS = TOS | NOS;
;   inst_dr();
; }

inst_or
            ldx  SP

            lda   #NOSl,b,x
            ora   #TOSl,b,x
            sta  #NOSl,b,x

            lda   #NOSh,b,x
            ora   #TOSh,b,x
            sta  #NOSh,b,x

            jmp  inst_dr

; ---------------------------------------------------------
; ## xo (23) stack: xy-n  |   -    bitwise xor
;
; void inst_an() {
;   NOS = TOS ^ NOS;
;   inst_dr();
; }

inst_xo
            ldx  SP

            lda   #NOSl,b,x
            eor   #TOSl,b,x
            sta  #NOSl,b,x

            lda   #NOSh,b,x
            eor   #TOSh,b,x
            sta  #NOSh,b,x

            jmp  inst_dr

; ---------------------------------------------------------
; ## sh (24) stack: xy-n  |   -    shift
;
; void inst_sh() {
;   CELL y = TOS;
;   CELL x = NOS;
;   if (TOS < 0)
;     NOS = NOS << (TOS * -1);
;   else {
;     if (x < 0 && y > 0)
;       NOS = x >> y | ~(~0U >> y);
;     else
;       NOS = x >> y;
;   }
;   inst_dr();
; }

; 1. because effective shift for a 32bit value is... 32
;    there is no need in using high word in shift count,
;    low word is sufficient for 65535 shifts
; 2. because there is no need in shifting more than 32
;    times, then value of low word is masked to six lower
;    bits

; NOTE: code would be simpler if in case of separate shift
;       operations (i.e. shift right/shift left) in muri
;

inst_sh
            ldx  SP

            ; check if shift count is positive or negative
            bit  #TOSh,b,x
            bpl  shr_main        ; shift to right

            ; we shifting left, so we need to negate arg
            jsr  negate_tos

            lda   #TOSl,b,x
            and   #63            ; we need only low 6 bits
            bne   +              ; do something if > 0
            jmp  inst_dr
+           tay

            ; shifting left is the same for neg and pos vals
shl_main    asl  #NOSl,b,x
            rol   #NOSh,b,x
            dey
            bne   shl_main
            jmp  inst_dr


            ; shift right ---------------------------------
shr_main    lda   #TOSl,b,x
            and   #63            ; we need only low 6 bits
            bne   +              ; do something if > 0
            jmp  inst_dr
+           tay

            ; did we shifting negative or positive value?
            bit  #NOSh,b,x
            bmi  shr_neg

shr_pos     lsr   #NOSh,b,x
            ror  #NOSl,b,x
            dey
            bne   shr_pos
            jmp  inst_dr

shr_neg     clc
            ror   #NOSh,b,x
            ror  #NOSl,b,x
            dey
            bne   shr_neg
            jmp  inst_dr

            ; additional routines for shifting ------------
negate_tos nop
            jsr   invert_tos
            inc   #TOSl,b,x       ; STACKBASE+0,x
            bne   +
            inc   #TOSh,b,x       ; STACKBASE+2,x
+           rts

invert_tos
            lda   #TOSl,b,x      ; STACKBASE+0,x
            eor   #$FFFF
            sta   #TOSl,b,x      ; STACKBASE+0,x
            lda   #TOSh,b,x       ; STACKBASE+2,x
            eor   #$FFFF
            sta   #TOSh,b,x       ; STACKBASE+2,x
            rts

; ---------------------------------------------------------
; ## zr (25) stack:  n-?  |   -    zero return
;
; returns from a subroutine if the top item on the stack is zero.
; If not, it acts like a NOP instead.
;
; void inst_zr() {
;   if (TOS == 0) {
;     inst_dr();
;     ip = TORS;
;     rp--;
;   }
; }

inst_zr
            ldx  SP
            lda  #TOSl,b,x
            bne  zr_quit
            lda  #TOSh,b,x
            bne  zr_quit

do_zr       jsr  inst_dr
            ldy  RP
            lda  #TRSl,b,y
            sta  IP
            jsr  update_ipptr

            tya
            sec
            sbc   #CELL_SIZE
            sta   RP
zr_quit     rts

; ---------------------------------------------------------
; ## ha (26) stack:   -   |   -    halt

inst_ha
            lda  #CELL_MAX-1     ; XXX - change it
            sta  IP
            rts

; ---------------------------------------------------------
; ## ie (27) stack:   -n  |   -    i/o enumerate

inst_ie
            lda  SP
            clc
            adc  #CELL_SIZE      ; 4 bytes
            sta  SP
            tax

            lda  #NUM_DEVICES
            sta  #TOSl,b,x
            stz  #TOSh,b,x
            rts

; ---------------------------------------------------------
; ## iq (28) stack:  n-xy |   -    i/o query
;
; void inst_iq() {
;   CELL Device = TOS;
;   inst_dr();
;   IO_queryHandlers[Device]();
; }

inst_iq
            ldx  SP
            lda  #TOSl,b,x
            pha
            jsr  inst_dr
            jmp  io_query


; ## ii (29) stack: ...n- |   -    i/o invoke
;
; void inst_ii() {
;   CELL Device = TOS;
;   inst_dr();
;   IO_deviceHandlers[Device]();
; }
;
; void generic_output() {
;  putc(stack_pop(), stdout);
;  fflush(stdout);
;}


inst_ii
            ldx  SP
            lda  #TOSl,b,x
            pha
            jsr  inst_dr
            jmp  io_handle

; ---------------------------------------------------------
; device support
; ---------------------------------------------------------

; number of device on stack
io_query
            ; numbers are counted from 0, so val should be
            ; lower than number of devices
            ply
            cpy  #NUM_DEVICES
            bcc  query

            lda  #CELL_MAX-1
            sta  IP
            ;wdm  #KILL                          ; effective error

query       lda  SP
            clc
            adc  #4
            sta  SP
            tax

            tya
            sta  #TOSl,b,x       ; device number
            stz  #TOSh,b,x

            bne  is_key
is_output   jmp  version0        ; output ver0

is_key      cmp   #1
            bne  unknown
            jmp  version0        ; keyboard ver0

unknown     jsr  inst_dr         ; drop already put dev no
            rts                  ; never reachable

version0    stz  #NOSl,b,x
            stz  #NOSh,b,x
            rts


; ---------------------------------------------------------
; number of device on stack
io_handle
            ; numbers are counted from 0, so val should be
            ; lower than number of devices
            ply
            cpy  #NUM_DEVICES
            bcc  interact
            lda  #CELL_MAX-1
            sta  IP
            ;wdm  #KILL            ; stop - error

interact    cpy  #0
            beq  screen          ; crude, but should work

            ; keyboard input
            lda  SP
            clc
            adc  #4
            sta  SP
            tax

            ;wdm   #TRACE_OFF
            jsl   C256_GETCHW    ; all regs preserved here
            ;wdm   #TRACE_ON
            .setaxl              ; redundant
            and   #$00ff          ; only byte lower is needed
            cmp   #$0d            ; change 0d to 0a
            bne   +
            lda   #$0a
+           nop
            sta  #TOSl,b,x
            stz  #TOSh,b,x
            rts

            ; screen output
screen      ldx  SP
            txa
            sec
            sbc  #4
            sta  SP

            ; SP is new but X point to previous element
            lda  #TOSl,b,x
            and  #$00ff
            cmp  #$0a
            bne  +
            lda  #$0d
+           nop
            ;wdm  #TRACE_OFF
            jsl  C256_PUTC
            ;wdm  #TRACE_ON
            .setaxl              ; redundant
            rts


            .warn "Code size: ", repr(* - main)

; ---------------------------------------------------------
; # messages

; a counted-string experiment
pstring     .macro txt
            .word(len(\txt))
            .text \txt
            .endm

; zero-terminated strings
msg_banner  .text $d, "RETRO/816 - NGA/816-32 2021-02-21", $d, $0
msg_mclean  .text "cleaning memory...", $d, $0
msg_sclean  .text "cleaning stack...", $d, $0
msg_copy    .text "copying image...", $d, $0
msg_end     .text "NGA finished, press any key to restart", $d, $0

err_uf      .text "ERROR: stack underflow, re-starting system!", $d, $0
err_0div    .text "ERROR: division-by-zero, re-starting system!", $d, $0
err_halt    .text "INFO: halt op called! Going to infinite loop.", $d, $0
err_memuf   .text "ERROR: read from unknown bad, negative mem addr!", $d, $0

; ---------------------------------------------------------
; # forth image

; image create by standard RETRO tools
; cp ngaImage barebones.image
; ./bin/retro-extend barebones.image interface/barebones.forth
;

IMAGE_SRC   .binary "barebones.image"
IMAGE_END   = *
IMAGE_SIZE  = IMAGE_END - IMAGE_SRC

; eof
