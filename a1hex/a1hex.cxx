#include <stdio.h>
#include <stdint.h>
#include <string>
#include <cstring>

/*
   typical file:
1000: de 23 00 08 01 00 71 14
1008: 00 00 00 00 00 00 00 00
1010: 00 00 00 00 00 00 00 00
*/


void usage( const char * perr = 0 )
{
    if ( 0 != perr )
        printf( "error: %s\n", perr );

    printf( "a1hex -a:<address> <input>\n" );
    printf( "  converts a binary file to the hex format WozMon understands\n" );
    printf( "  creates an output file named <input>.hex\n" );
    printf( "  e.g.:  a1hex -a:0x1000 app.bin\n" );
    printf( "         ...produces app.hex\n" );
    exit( 1 );
} //usage

inline long portable_filelen( FILE * fp )
{
    long current = ftell( fp );
    fseek( fp, 0, SEEK_END );
    long len = ftell( fp );
    fseek( fp, current, SEEK_SET );
    return len;
} //portable_filelen

int main( int argc, char * argv[] )
{
    const char * pin = 0;
    char aout[ 255 ];
    uint16_t start = 0;

    for ( int i = 1; i < argc; i++ )
    {
        if ( !strncmp( argv[ i ], "-a:", 3 ) )
            start = (uint16_t) std::stol( argv[ i ] + 3, 0, 0 );
        else if ( 0 != pin )
            usage();
        else
            pin = argv[ i ];
    }

    if ( 0 == pin )
        usage( "no filename specified" );

    FILE * fpin = fopen( pin, "rb" );
    if ( !fpin )
        usage( "can't open input file" );

    if ( portable_filelen( fpin ) > 65536 )
        usage( "input file must be <= 65536 bytes\n" );

    strcpy( aout, pin );
    char * pdot = strchr( aout, '.' );
    if ( pdot )
        strcpy( pdot, ".hex" );
    else
        strcat( aout, ".hex" );

    FILE * fpout = fopen( aout, "w+b" );
    if ( !fpout )
        usage( "can't open output file" );

    int next;
    uint16_t address = start;
    uint16_t sofar = 0;

    while ( EOF != ( next = fgetc( fpin ) ) )
    {
        uint8_t b = next & 0xff;
        if ( 0 == ( sofar % 8 ) )
        {
            if ( 0 != sofar )
                fprintf( fpout, "\n" );
            fprintf( fpout, "%04x:", address );
        }
        fprintf( fpout, " %02x", b );
        sofar++;
        address++;
    }

    if ( sofar > 0 )
        fprintf( fpout, "\n" ); // so WozMon will consume the final line

    printf( "a1hex completed with great success\n" );
    fclose( fpout );
    fclose( fpin );
    return 0;
} //main


