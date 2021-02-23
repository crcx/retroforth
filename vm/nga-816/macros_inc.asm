; Set 8-bit accumulator
setaxs          .macro
                SEP #$30        ; set A&X short
                .as
                .xs 
                .endm
                
; Set 16-bit accumulator
setaxl          .macro
                REP #$30        ; set A&X long 
                .al
                .xl
                .endm

; Set 8-bit accumulator
setas           .macro
                SEP #$20        ; set A short 
                .as
                .endm
                
; Set 16-bit accumulator
setal           .macro
                REP #$20        ; set A long 
                .al
                .endm

; Set 8 bit index registers               
setxs           .macro
                SEP #$10        ; set X short 
                .xs
                .endm
                
; Set 16-bit index registers
setxl           .macro
                REP #$10        ; set X long 
                .xl
                .endm

sdb         .macro          ; Set the B (Data bank) register
            pha             ; begin setdbr macro
            php
            .setas
            lda #\1
            pha
            plb
            .databank \1
            plp
            pla             ; end setdbr macro
            .endm

