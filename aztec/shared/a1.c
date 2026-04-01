/* only enable what's needed before including this file to keep the binary size small 

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    0
#define A1_SUPPORT_KBD     0
#define A1_SUPPORT_PR_SXX  0

*/

#if A1_SUPPORT_SHOW

void show_int( val ) int val;
{
    int * pi = (int *) 0xc0;  
    *pi = val;

    #asm
        lda     $c1          /* print the high byte first */
        jsr     $ffdc
        lda     $c0
        jsr     $ffdc
    #endasm
} /*show_int*/

void show_long( val ) long val;
{
    long * pl = (long *) 0xc0; 
    *pl = val;

    #asm
        lda     $c3          /* print the high byte first */
        jsr     $ffdc
        lda     $c2          /* print the high byte first */
        jsr     $ffdc
        lda     $c1
        jsr     $ffdc
        lda     $c0
        jsr     $ffdc
    #endasm
} /*show_long*/

void show_string( str ) char * str;
{
    while ( *str )
    {
        pr_c( *str );
        str++;
    }
} /*show_string*/

#endif /* A1_SUPPORT_SHOW

void pr_c( val ) char val;
{
    char * pc = (char *) 0xc0;
    *pc = val;

    #asm
        lda     $c0
        jsr     $ffef
    #endasm
} /*pr_c*/

void pr_i( val ) int val;
{
    char acbuf[ 6 ];
    int digits, x;
    digits = 0;

    if ( val < 0 )
    {
        pr_c( '-' );
        val = -val;
    }

    if ( 0 == val )
    {
        pr_c( '0' );
        return;
    }

    while ( val )
    {
        x = val % 10;
        acbuf[ digits++ ] = x + '0';
        val /= 10;
    }

    for ( x = digits - 1; x >= 0; x-- )
        pr_c( acbuf[ x ] );
} /*pr_i*/

#if A1_SUPPORT_LONG

void pr_l( val ) long val;
{
    char acbuf[ 11 ];
    int digits, x;
    digits = 0;

    if ( val < 0 )
    {
        pr_c( '-' );
        val = -val;
    }

    if ( 0 == val )
    {
        pr_c( '0' );
        return;
    }

    while ( val )
    {
        x = (int) ( val % 10 );
        acbuf[ digits++ ] = x + '0';
        val /= (long) 10;
    }

    for ( x = digits - 1; x >= 0; x-- )
        pr_c( acbuf[ x ] );
} /*pr_l*/

#endif /* A1_SUPPORT_LONG */

int pr_s(s) char *s;
{
    char c;
    while ( *s )
    {
        c = *s;
        if ( '\n' == c )
            c = '\r';
        pr_c( c );
        s++;
    }
    return 0;
} /*pr_s*/

#if A1_SUPPORT_PR_SXX

int pr_sis( s, i, a ) char * s; int i; char * a;
{
    if ( s )
        pr_s( s );
    pr_i( i );
    if ( a )
        pr_s( a );
} /*pr_sis*/

#if A1_SUPPORT_LONG

int pr_sls( s, l, a ) char * s; long l; char * a;
{
    if ( s )
        pr_s( s );
    pr_l( l );
    if ( a )
        pr_s( a );
} /*pr_sls*/

#endif /* A1_SUPPORT_LONG */

#endif /* A1_SUPPORT_SXX */

void bye() /* exit to the apple 1 monitor */
{
    #asm
        jsr $ff1f
    #endasm
} /*bye*/

#if A1_SUPPORT_KBD

int get_input(buffer, max_len) char *buffer; int max_len;
{
    int i = 0;
    char c;
    char *temp = (char *) 0xc0;

    while (i < (max_len - 1))
    {
        #asm
WAITKBD lda     $d011    ; Keyboard control register
        bpl     WAITKBD  ; Wait for bit 7 (key press)
        lda     $d010    ; Keyboard data register
        sta     $c0      ; Save to temp for C
        jsr     $ffef    ; Echo to screen
        #endasm

        c = *temp;

        if ( 0x8d == c ) /* Apple 1 CR */
            break;

        buffer[i++] = c & 0x7f;
    }

    buffer[i] = '\0';
    return i;
} /*get_input*/

int kbhit()
{
    /* Keyboard control register is at 0xd011 */
    char *kbd_control = (char *) 0xd011;

    /* Bit 7 is set (1) if a key is available. 
       In 8-bit signed char, bit 7 set makes the value negative. */
    if ( 0 != ( ( *kbd_control ) & 0x80 ) )
        return 1;
    
    return 0;
} /*kbhit*/

char getch()
{
    char *kbd_status = (char *) 0xd011;
    char *kbd_data   = (char *) 0xd010;
    char key;

    /* Loop until bit 7 is set (value becomes negative). char is unsigned with this C implementation */
    while ( 0 == ( ( *kbd_status ) & 0x80 ) );

    /* Read the key (this also clears the status bit) */
    key = *kbd_data;

    /* Apple 1 uses high-ASCII (bit 7 set).  Strip it to get standard ASCII (0-127). */
    return (key & 0x7f);
} /*getch*/

char getche()
{
    char ch = getch();
    pr_c( ch );
    return ch;
} /*getche*/

#endif /* A1_SUPPORT_KBD */
