COPY    START   0
FIRST   STL     RETARD
        LDB     #LENGTH
        BASE    LENGTH
CLOOP   +JSUB   RDREC
        LDA     LENGTH
        COMP    #0
        JEQ     ENDFIL
        +JSUB   WRREC
        J       CLOOP
ENDFIL  LDA     EOF
        STA     BUFFER
        LDA     #3
        STA     LENGTH
        +JSUB   WRREC
        J       @RETARD
EOF     BYTE    C'EOF'
RETARD  RESW    1
LENGTH  RESW    1
BUFFER  RESB    4096
