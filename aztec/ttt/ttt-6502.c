/*
   Tic-Tac-Toe proof you can't win for the Apple 1.
   This version builds with Aztec CG65 v3.2c 10-2-89 on MS-DOS targeting 6502.
   That compiler expects an Apple II, so there are hacks here for the Apple 1.
   There are dependencies on start.a, which initialzes for the Apple 1.
   Aztec C treats the char type as unsigned, and expands to 16 bits for any expression (slow!)
*/

#define LINT_ARGS

#include <stdio.h>

#define true 1
#define false 0

#define ABPrune true         /* alpha beta pruning */
#define WinLosePrune true    /* stop early on win/lose */
#define WinFunPointers true  /* use function pointers for each move position. slightly faster on Apple 1 */
#define ScoreWin 6
#define ScoreTie 5
#define ScoreLose  4
#define ScoreMax 9
#define ScoreMin 2
#define DefaultIterations 1

#define PieceX 1
#define PieceO 2
#define PieceBlank 0

int g_Iterations = DefaultIterations;
int g_Moves = 0;

char g_board[ 9 ];

#if WinFunPointers

char pos0func()
{
    register char x = g_board[0];
    
    if ( ( x == g_board[1] && x == g_board[2] ) ||
         ( x == g_board[3] && x == g_board[6] ) ||
         ( x == g_board[4] && x == g_board[8] ) )
        return x;
    return PieceBlank;
}

char pos1func()
{
    register char x = g_board[1];
    
    if ( ( x == g_board[0] && x == g_board[2] ) ||
         ( x == g_board[4] && x == g_board[7] ) )
        return x;
    return PieceBlank;
} 

char pos2func()
{
    register char x = g_board[2];
    
    if ( ( x == g_board[0] && x == g_board[1] ) ||
         ( x == g_board[5] && x == g_board[8] ) ||
         ( x == g_board[4] && x == g_board[6] ) )
        return x;
    return PieceBlank;
} 

char pos3func()
{
    register char x = g_board[3];
    
    if ( ( x == g_board[4] && x == g_board[5] ) ||
         ( x == g_board[0] && x == g_board[6] ) )
        return x;
    return PieceBlank;
} 

char pos4func()
{
    register char x = g_board[4];
    
    if ( ( x == g_board[0] && x == g_board[8] ) ||
         ( x == g_board[2] && x == g_board[6] ) ||
         ( x == g_board[1] && x == g_board[7] ) ||
         ( x == g_board[3] && x == g_board[5] ) )
        return x;
    return PieceBlank;
} 

char pos5func()
{
    register char x = g_board[5];
    
    if ( ( x == g_board[3] && x == g_board[4] ) ||
         ( x == g_board[2] && x == g_board[8] ) )
        return x;
    return PieceBlank;
} 

char pos6func()
{
    register char x = g_board[6];
    
    if ( ( x == g_board[7] && x == g_board[8] ) ||
         ( x == g_board[0] && x == g_board[3] ) ||
         ( x == g_board[4] && x == g_board[2] ) )
        return x;
    return PieceBlank;
} 

char pos7func()
{
    register char x = g_board[7];
    
    if ( ( x == g_board[6] && x == g_board[8] ) ||
         ( x == g_board[1] && x == g_board[4] ) )
        return x;
    return PieceBlank;
} 

char pos8func()
{
    register char x = g_board[8];
    
    if ( ( x == g_board[6] && x == g_board[7] ) ||
         ( x == g_board[2] && x == g_board[5] ) ||
         ( x == g_board[0] && x == g_board[4] ) )
        return x;
    return PieceBlank;
} 

typedef char pfunc_t();

pfunc_t * winner_functions[9] =
{
    pos0func,
    pos1func,
    pos2func,
    pos3func,
    pos4func,
    pos5func,
    pos6func,
    pos7func,
    pos8func,
};

#else /* WinFunPointers */

char LookForWinner()
{
    char p = g_board[0];
    if ( PieceBlank != p )
    {
        if ( p == g_board[1] && p == g_board[2] )
            return p;

        if ( p == g_board[3] && p == g_board[6] )
            return p;
    }

    p = g_board[3];
    if ( PieceBlank != p && p == g_board[4] && p == g_board[5] )
        return p;

    p = g_board[6];
    if ( PieceBlank != p && p == g_board[7] && p == g_board[8] )
        return p;

    p = g_board[1];
    if ( PieceBlank != p && p == g_board[4] && p == g_board[7] )
        return p;

    p = g_board[2];
    if ( PieceBlank != p && p == g_board[5] && p == g_board[8] )
        return p;

    p = g_board[4];
    if ( PieceBlank != p )
    {
        if ( ( p == g_board[0] ) && ( p == g_board[8] ) )
            return p;

        if ( ( p == g_board[2] ) && ( p == g_board[6] ) )
            return p;
    }

    return PieceBlank;
} /*LookForWinner*/

#endif /* WinFunPointers */

int MinMax( alpha, beta, depth, move ) char alpha; char beta; char depth; char move;
{
    char value, pieceMove, score, p;
#if WinFunPointers
    pfunc_t * pf;
#endif

    g_Moves++;

    if ( depth >= 4 )
    {
#if WinFunPointers
        /* function pointers are faster on all platforms by 10-20% */

        pf = winner_functions[ move ];
        p = (*pf)();
#else
        p = LookForWinner();
#endif

        if ( PieceBlank != p )
        {
            if ( PieceX == p )
                return ScoreWin;

            return ScoreLose;
        }

        if ( 8 == depth )
            return ScoreTie;
    }

    if ( depth & 1 ) 
    {
        value = ScoreMin;
        pieceMove = PieceX;
    }
    else
    {
        value = ScoreMax;
        pieceMove = PieceO;
    }

    for ( p = 0; p < 9; p++ )
    {
        if ( PieceBlank == g_board[ p ] )
        {
            g_board[p] = pieceMove;
            score = MinMax( alpha, beta, depth + 1, p );
            g_board[p] = PieceBlank;

            if ( depth & 1 ) 
            {
#if WinLosePrune   /* #if statements must be in column 0 for MS C 1.0 */
                if ( ScoreWin == score )
                    return ScoreWin;
#endif

                if ( score > value )
                {
                    value = score;
#if ABPrune
                    if ( value >= beta )
                        return value;
                    if ( value > alpha )
                        alpha = value;
                }
#endif
            }
            else
            {
#if WinLosePrune
                if ( ScoreLose == score )
                    return ScoreLose;
#endif

                if ( score < value )
                {
                    value = score;
#if ABPrune
                    if ( value <= alpha )
                        return value;
                    if ( value < beta )
                        beta = value;
#endif
                }
            }
        }
    }

    return value;
}  /*MinMax*/

int FindSolution( position ) char position;
{
    g_board[ position ] = PieceX;
    MinMax( ScoreMin, ScoreMax, 0, position );
    g_board[ position ] = PieceBlank;

    return 0;
} /*FindSolution*/

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

void show_char( val ) char val;
{
    char * pc = (char *) 0xc0;
    *pc = val;

    #asm
        lda     $c0
        jsr     $ffef
    #endasm
} /*show_char*/

void show_string( str ) char * str;
{
    while ( *str )
    {
        show_char( *str );
        str++;
    }
} /*show_string*/

void bye()
{
    #asm
        jsr $ff1f
    #endasm
} /*bye*/

int main()
{
    int i;

    for ( i = 0; i < 9; i++ )
        g_board[ i ] = PieceBlank;

    for ( i = 0; i < g_Iterations; i++ )
    {
        g_Moves = 0;
        FindSolution( 0 );
        FindSolution( 1 );
        FindSolution( 4 );
    }

    show_int( g_Moves );
    show_char( '$' );

    /* The C runtime doesn't know how to exit or even return to the entry proc
       on an Apple 1, so exit with bye() */

    bye();

    return 0;
} /*main*/

