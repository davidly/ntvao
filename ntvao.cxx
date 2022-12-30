// NT Virtual Apple One
// Written by David Lee
// Simulates the Apple 1 runtime environment enough to run simple apps.
// Uses the mos6502 emulator for the CPU
// Hooks are installed to emulate the Apple 1's keyboard and display functionality.
// If the monitor is loaded, it overwrites those hooks with the real code.
// Useful:  https://www.sbprojects.net/projects/apple1/wozmon.php
//          http://s3data.computerhistory.org/brochures/apple.applei.1976.102646518.pdf
//

#define UNICODE

#include <stdio.h>
#include <conio.h>
#include <vector>

#include <windows.h>
#include <djltrace.hxx>
#include <djl_con.hxx>
#include <djl_perf.hxx>
#include <djl_cycle.hxx>

#include "mos6502.hxx"

CDJLTrace tracer;
static bool g_haltExecuted = false;
static bool g_KbdPeekHappened = false;
static bool g_forceUppercase = true; // the apple 1 way
static uint16_t g_startAddress = 0x1000;
static bool g_fStartAddressSpecified = false;

void usage( char const * perr = 0 )
{
    if ( perr )
        printf( "error: %s\n", perr );

    printf( "NT Virtual Apple 1 Machine: emulates an Apple 1 on Windows\n" );
    printf( "usage: ntvao [-a] [-c] [-i] [-p] [-s:X] [-t] [-u] <.hex file>\n" );
    printf( "  arguments:\n" );
    printf( "     -a     address at which the run is started, e.g. /a:0x1000\n" );
    printf( "            this overrides the default, which is the first address in the input file.\n" ); 
    printf( "     -c     don't set the console to 40x24\n" );
    printf( "     -i     when tracing is enabled with -t, also show each instruction executed.\n" );
    printf( "     -p     show performance information at app exit.\n" ); 
    printf( "     -s:X   speed in Hz. Default is 0, which is as fast as possible.\n" );
    printf( "            for the Apple 1's speed use -s:1022727\n" );
    printf( "     -t     enable debug tracing to ntva1.log\n" );
    printf( "     -u     disable converting output chars to uppercase.\n" );
    printf( "  notes:\n" );
    printf( "     --     to assemble, load, and run test.s:\n" );
    printf( "                sbasm30306\\sbasm.py test.s\n" );
    printf( "                ntvao -c test.hex\n" );
    printf( "     --     .hex files can be in Apple 1 format or Intel hex format.\n" );
    printf( "     --     any 6502 app will run but with no OS support it can be hard to tell.\n" );
    printf( "     --     hooks are installed by default to emulate the Apple 1's output\n" );
    printf( "     --     installing the monitor overwrites those hooks with the real thing, e.g.:\n" );
    printf( "                to run the Apple 1 monitor: ntvao wozmon.hex\n" );
    printf( "                to run the monitor + basic: ntvao apple1.hex\n" );
    printf( "     --     control keys not passed to the Apple 1:\n" );
    printf( "                ^d        save a 64k memory dump in ntvao.dmp\n" );
    printf( "                ^l        load a file into the input stream\n" );
    printf( "                ^q        quit the app\n" );
    printf( "                ^r        soft reset via the 6502's 0xfffc reset vector\n" );
    printf( "                          likely an Apple 1 format .hex for monitor or .bas for BASIC\n" );
    printf( "                ^break    forcibly exit the app\n" );
    exit( -1 );
} //usage

void DumpBinaryData( byte * pData, DWORD length, DWORD indent )
{
    uint64_t offset = 0;
    uint64_t beyond = length;
    const uint64_t bytesPerRow = 32;
    byte buf[ bytesPerRow ];

    while ( offset < beyond )
    {
        for ( int i = 0; i < indent; i++ )
            tracer.TraceQuiet( " " );

        tracer.TraceQuiet( "%#10llx  ", offset );

        uint64_t cap = __min( offset + bytesPerRow, beyond );
        uint64_t toread = ( ( offset + bytesPerRow ) > beyond ) ? ( length % bytesPerRow ) : bytesPerRow;

        memcpy( buf, pData + offset, toread );

        for ( uint64_t o = offset; o < cap; o++ )
            tracer.TraceQuiet( "%02x ", buf[ o - offset ] );

        DWORD spaceNeeded = ( bytesPerRow - ( cap - offset ) ) * 3;

        for ( ULONG sp = 0; sp < ( 1 + spaceNeeded ); sp++ )
            tracer.TraceQuiet( " " );

        for ( uint64_t o = offset; o < cap; o++ )
        {
            char ch = buf[ o - offset ];

            if ( ch < ' ' || 127 == ch )
                ch = '.';
            tracer.TraceQuiet( "%c", ch );
        }

        offset += bytesPerRow;

        tracer.TraceQuiet( "\n" );
    }
} //DumpBinaryData

void CreateMemoryDump()
{
    FILE * fp = fopen( "ntvao.dmp", "w" );
    if ( 0 == fp )
        tracer.Trace( "can't create memory dump file, error %d\n", errno );

    uint64_t offset = 0;
    uint64_t length = 65536;
    const uint64_t bytesPerRow = 32;

    while ( offset < length )
    {
        fprintf( fp, "%04X: ", (uint32_t) offset );

        for ( uint64_t o = offset; o < ( offset + bytesPerRow ); o++ )
            fprintf( fp, "%02X ", memory[ o ] );

        offset += bytesPerRow;
        fprintf( fp, "\n" );
    }

    fclose( fp );
} //CreateMemoryDump

// this hook is invoked to emulate an Apple 1 if the Apple 1 monitor isn't installed

uint8_t mos6502_invoke_hook( void )
{
    uint16_t address = cpu.pc;

    if ( 0xffe5 == address ) // apple 1
    {
        printf( "%X", 0xf & cpu.a );
        return 0x60; // rts
    }
    else if ( 0xffdc == address ) // apple 1
    {
        printf( "%02X", cpu.a );
        return 0x60; // rts
    }
    else if ( 0xffef == address ) // apple 1
    {
        char c = cpu.a;
        if ( 0x0d != c )
        {
            if ( g_forceUppercase )
                c = toupper( c );
            putchar( c );
        }

        return 0x60; // rts
    }
    else if ( 0xff1f == address )
    {
        // On the Apple 1 this emits a CR then returns to the monitor
        // Here the app is just terminated

        printf( "\n" );
        g_haltExecuted = true;
        return OPCODE_HALT;
    }

    return memory[ address ];
} //mos6502_invoke_hook

static vector<char> g_inputText;
static uint32_t g_inputOffset = 0;

void LoadInputFile()
{
    printf( "filename to read: " );
    char acfilename[ MAX_PATH ];
    char * result = gets_s( acfilename, _countof( acfilename ) );
    if ( result )
    {
        tracer.Trace( "reading file %s to input stream\n", result );
        FILE * fp = fopen( result, "r" );
        if ( fp )
        {
            uint32_t sizeSoFar = 0;
            do
            {
                int next = fgetc( fp );
                if ( EOF == next )
                    break;

                if ( 0x0a == next ) // the Apple 1 uses 0x0d; Windows uses 0x0a.
                    next = 0x0d;

                g_inputText.resize( sizeSoFar + 1 );
                g_inputText[ sizeSoFar ] = (char) next;
                sizeSoFar++;
            } while( true );
            
            fclose( fp );
        }
        else
            printf( "failed to open the file, error %d\n", errno );
    }
} //LoadInputFile

uint8_t mos6502_apple1_load( uint16_t address )
{
    // only called just before a load of these addresses

    if ( 0xd011 == address )
    {
        // KBDCR -- sets bit 7 when a key is available

        if ( g_inputText.size() > 0 )
            return 0x80; // a key is available

        g_KbdPeekHappened = true;

        if ( _kbhit() )
        {
            // if the input event is a control character, process it and don't pass it on
            // 1..26 are ^a through ^z. ^c isn't sent through _getch. pass through carriage returns and backspace

            char ch = _getch();
            if ( ch < 26 && 0x08 != ch && 0x0d != ch )
            {
                // ^ character processing...
                tracer.Trace( "control character: %d\n", ch );

                if ( 17 == ch ) // 'q' for quit
                {
                    g_haltExecuted = true;
                    cpu.end_emulation();
                }
                else if ( 4 == ch ) // 'd'
                    CreateMemoryDump();
                else if ( 12 == ch ) // 'l'
                {
                    LoadInputFile();
                    if ( g_inputText.size() > 0 )
                        return 0x80;
                }
                else if ( 18 == ch ) // 'r'
                    cpu.soft_reset();

                return 0; // no key is available for the emulated app -- control codes are for the host
            }

            // translate backspace to what the Apple 1 wants -- an undercore, which virtually backspaces

            if ( 0x08 == ch )
                ch = 0x5f;

            _ungetch( ch ); // put the character back in the buffer
            return 0x80;    // a key is available
        }
        else
        {
            SleepEx( 1, FALSE ); // prevent a tight busy loop
            return 0; // high bit off, no key available
        }
    }
    else if ( 0xd010 == address )
    {
        // KBD -- returns an uppercase char if a key is available. It's up to the caller
        // to be certain one is ready using KBDCR. This function won't block.

        if ( g_inputText.size() > 0 )
        {
            char ch = g_inputText[ g_inputOffset++ ];
            if ( g_inputOffset == g_inputText.size() )
            {
                g_inputOffset = 0;
                g_inputText.resize( 0 );
            }

            ch = toupper( ch );      // the Apple 1 expects only upppercase
            ch |= 0x80;              // the high bit should be set on the Apple 1
            memory[ 0xd011 ] = 0;    // this should alread be reset
            return ch;
        }

        g_KbdPeekHappened = true;

        if ( _kbhit() )
        {
            char ch = _getch();
            tracer.Trace( "_getch returned %02x\n", ch );
            ch = toupper( ch );      // the Apple 1 expects only upppercase
            ch |= 0x80;              // the high bit should be set on the Apple 1
            memory[ 0xd011 ] = 0;    // this should alread be reset
            return ch;
        }
    }

    return memory[ address ];
} //mos6502_apple1_load

void mos6502_apple1_store( uint16_t address )
{
    // only called just after a store of these addresses

    if ( 0xd012 == address )
    {
        char ch = 0x7f & memory[ 0xd012 ];
        if ( 0x7f != ch && 0x0a != ch ) // ignore 0a, and translate 0d to 0a, and 7f, which programs the hardware
        {
            if ( 0x0d == ch )
                ch = 0x0a;

            printf( "%c", ch );
        }
        memory[ 0xd012 ] = 0;  // Indicate that the character has been consumed
    }
} //mos6502_apple1_store

void mos6502_invoke_halt( void )
{
    g_haltExecuted = true;
} //mos6502_invoke_halt

static bool ishex( char c )
{
    return ( ( c >= 'A' && c <= 'F' ) || ( c >= 'a' && c <= 'f' ) || ( c >= ' ' && c <= '9' ) );
} //ishex

uint8_t read_byte( char * p )
{
    char ac[3];
    ac[0] = p[0];
    ac[1] = p[1];
    ac[2] = 0;

    return (uint8_t) strtoul( ac, 0, 16 );
} //read_byte

uint16_t read_word( char * p )
{
    char ac[5];
    ac[0] = p[0];
    ac[1] = p[1];
    ac[2] = p[2];
    ac[3] = p[3];
    ac[4] = 0;

    return (uint16_t) strtoul( ac, 0, 16 );
} //read_word

// http://www.piclist.com/techref/fileext/hex/intel.htm
// :, reclen(1), offset(2), rectype(1), data/info, chksum(1)

bool load_file_intel_format( FILE * fp )
{
    char acLine[ 120 ];

    do
    {
        char * buf = fgets( acLine, _countof( acLine), fp );

        if ( buf && strlen( buf ) >= 11 )
        {
            if ( ':' != buf[ 0 ] )
                usage( "error: input Intel HEX file is malformed" );

            uint8_t reclen = read_byte( buf + 1 );
            uint16_t offset = read_word( buf + 3 );
            uint8_t rectyp = read_byte( buf + 7 );

            if ( 1 == rectyp ) // eof
                break;

            if ( 0 != rectyp  )
                usage( "file format not recognized" );

            for ( uint8_t x = 0; x < reclen; x++ )
            {
                if ( feof( fp ) )
                    usage( "malformed input file" );

                uint8_t val = read_byte( buf + ( 2 * x ) + 9 );
                memory[ offset + x ] = val;
            }
        }
        else
            break;
    } while ( true );

    fclose( fp );

    //DumpBinaryData( memory + 0x400, 0x100, 2 );
    return true;
} //load_file_intel_format

// sample Apple 1 input file:
// 1000: A9 05 C5 04 F0 09 10 0F
// 1008: 30 15 B0 1B 4C 2F 10 A9
// 1010: 3D 20 EF FF 4C 2F 10 A9
// 1018: 2B 20 EF FF 4C 2F 10 A9
// 1020: 2D 20 EF FF 4C 2F 10 A9
// 1028: 43 20 EF FF 4C 2F 10 A9
// 1030: 24 20 EF FF 4C 1F FF

static bool load_file( char const * file_path )
{               
    bool ok = FALSE;
    FILE *fp = fopen( file_path, "r" );
    char acLine[ 120 ];
    bool first_address = true;
    uint16_t initial_address = 0;

    if ( 0 != fp )
    {
        char c = fgetc( fp );
        fseek( fp, 0, SEEK_SET );

        if ( ':' == c )
            return load_file_intel_format( fp );

        // read the file and write the data into memory

        do
        {
            char * buf = fgets( acLine, _countof( acLine), fp );

            if ( buf && strlen( buf ) >= 7 )
            {
                if ( ':' != buf[ 4 ] || ' ' != buf[ 5 ] )
                    usage( "error: input HEX file is malformed" );

                uint16_t a = (uint16_t) strtoul( buf, 0, 16 );
                if ( first_address )
                {
                    first_address = false;
                    initial_address = a;

                    if ( !g_fStartAddressSpecified )
                        g_startAddress = a;
                }

                char * pbyte = acLine + 5; // at the space
                ok = true;

                while ( ' ' == *pbyte && ishex( * ( pbyte + 1 ) ) )
                {
                    pbyte++;

                    uint8_t b = (uint8_t) strtoul( pbyte, 0, 16 );
                    pbyte += 2;
                    memory[ a++ ] = b;
                }
            }
            else
                break;
        } while ( true );

        fclose( fp );

        DumpBinaryData( memory + initial_address, 0x100, 2 );
    }

    return ok;
} //load_file

uint64_t invoke_command( char const * pc, uint64_t clockrate )
{
    cpu.power_on();

    // loading the file may overwrite the hooks, and that's OK

    bool ok = load_file( pc );
    if ( !ok )
    {
        printf( "unable to load command %s\n", pc );
        exit( 1 );
    }

    // Apple 1 apps expect these

    cpu.pc = g_startAddress;
    cpu.sp = 0xff;

    g_haltExecuted = FALSE;
    ULONGLONG cycles_executed = 0;
    CPUCycleDelay delay( clockrate );
    ULONGLONG cycles_since_last_kbd_peek = 0;

    do
    {
        ULONGLONG cycles_this_run = cpu.emulate( 1000 );
        cycles_executed += cycles_this_run;
        cycles_since_last_kbd_peek += cycles_this_run;

        if ( g_haltExecuted )
            break;

        if ( g_KbdPeekHappened )
        {
            // peeking the keyboard sleeps, thowing off execution times. Start again.

            delay.Reset();
            cycles_since_last_kbd_peek = 0;
            g_KbdPeekHappened = false;
            continue;
        }

        delay.Delay( cycles_since_last_kbd_peek );
    } while ( true );

    return cycles_executed;
} //invoke_command

int main( int argc, char * argv[] )
{
    char * pcHEX = 0;
    bool trace = false;
    bool traceInstructions = false;
    bool use40x24 = true;
    uint64_t clockrate = 0;
    bool showPerformance = false;

    for ( int i = 1; i < argc; i++ )
    {
        char *parg = argv[i];
        char c = *parg;

        if ( '-' == c || '/' == c )
        {
            char ca = tolower( parg[1] );

            if ( 'a' == ca )
            {
                if ( ':' == parg[2] )
                {
                    g_startAddress = (uint16_t) strtoul( parg + 3 , 0, 16 );
                    g_fStartAddressSpecified = true;
                }
                else
                    usage( "colon required after a argument" );
            }
            else if ( 's' == ca )
            {
                if ( ':' == parg[2] )
                    clockrate = (uint64_t) _strtoui64( parg + 3 , 0, 10 );
                else
                    usage( "colon required after c argument" );
            }
            else if ( 'i' == ca )
                traceInstructions = true;
            else if ( 't' == ca )
                trace = true;
            else if ( 'u' == ca )
                g_forceUppercase = false;
            else if ( 'p' == ca )
                showPerformance = true;
            else if ( 'c' == ca )
                use40x24 = false;
            else
                usage( "invalid argument specified" );
        }
        else
        {
            if ( 0 == pcHEX )
                pcHEX = parg;
            else
                usage( "too many arguments" );
        }
    }

    tracer.Enable( trace, L"ntvao.log", true );
    tracer.SetQuiet( true );
    tracer.SetFlushEachTrace( false );
    cpu.trace_instructions( traceInstructions );

    if ( 0 == pcHEX )
        usage( "no command specified" );

    char acHEX[ MAX_PATH ];
    strcpy( acHEX, pcHEX );
    strupr( acHEX );
    DWORD attr = GetFileAttributesA( acHEX );
    if ( INVALID_FILE_ATTRIBUTES == attr )
    {
        if ( strstr( acHEX, ".HEX" ) )
            usage( "can't find command file" );
        else
        {
            strcat( acHEX, ".HEX" );
            attr = GetFileAttributesA( acHEX );
            if ( INVALID_FILE_ATTRIBUTES == attr )
                usage( "can't find command file" );
        }
    }

    memset( &cpu, 0, sizeof( cpu ) );
    memset( memory, 0, sizeof( memory ) );

    // Set hooks for Apple 1 API compatibility. Installing the monitor will overwrite these.
    
    memory[ 0xff1f ] = OPCODE_HOOK;
    memory[ 0xffe5 ] = OPCODE_HOOK;
    memory[ 0xffdc ] = OPCODE_HOOK;
    memory[ 0xffef ] = OPCODE_HOOK;

    ConsoleConfiguration consoleConfig;
    if ( use40x24 )
        consoleConfig.EstablishConsole( 40, 24 );

    CPerfTime perfApp;

    uint64_t total_cycles = invoke_command( acHEX, clockrate );

    LONGLONG elapsed = 0;
    perfApp.CumulateSince( elapsed );
    FILETIME creationFT, exitFT, kernelFT, userFT;
    GetProcessTimes( GetCurrentProcess(), &creationFT, &exitFT, &kernelFT, &userFT );

    if ( use40x24 )
        consoleConfig.RestoreConsole();

    if ( showPerformance )
    {
        printf( "6502 cycles:   %18ws\n", perfApp.RenderLL( (LONGLONG) total_cycles ) );
        printf( "clock rate: " );
        if ( 0 == clockrate )
        {
            printf( "      %15s\n", "unbounded" );
            printf( "ms at 1.022727 Mhz: %13ws\n", perfApp.RenderLL( total_cycles * 1000 / 1022727 ) );
        }
        else
            printf( "      %15ws Hz\n", perfApp.RenderLL( (LONGLONG ) clockrate ) );

        ULARGE_INTEGER ullK, ullU;
        ullK.HighPart = kernelFT.dwHighDateTime;
        ullK.LowPart = kernelFT.dwLowDateTime;
    
        ullU.HighPart = userFT.dwHighDateTime;
        ullU.LowPart = userFT.dwLowDateTime;
    
        printf( "kernel CPU ms:    %15ws\n", perfApp.RenderDurationInMS( ullK.QuadPart ) );
        printf( "user CPU ms:      %15ws\n", perfApp.RenderDurationInMS( ullU.QuadPart ) );
        printf( "total CPU ms:     %15ws\n", perfApp.RenderDurationInMS( ullU.QuadPart + ullK.QuadPart ) );
        printf( "elapsed ms:       %15ws\n", perfApp.RenderDurationInMS( elapsed ) );
    }

    tracer.Shutdown();
} //main
