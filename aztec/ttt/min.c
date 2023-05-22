int main( argc, argv ) int argc; char * argv[];
{
#if 1
    int i, c;
#endif

#if 0
    outch( 'A' );
#endif

#if 1
    #asm
        lda     #$25      ; '%'
        jsr     $ffef
    #endasm
    c = 1;

    for ( i = 0; i < 5; i++ )
    {
        #asm
           lda   #$2a
           jsr   $ffef
        #endasm
        c += 1;
    }
#endif

    #asm
        lda     #$24      ; '$'
        jsr     $ffef
    #endasm

    #asm
        jsr     $ff1f
    #endasm

/* no sense in generating this code:   return 0; */
} /*main*/

