// 6502 emulator.
// Written by David Lee
// Useful:   https://pastraiser.com/cpu/6502/6502_opcodes.html
//           http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
//           https://www.masswerk.at/6502/6502_instruction_set.html
//           https://www.masswerk.at/6502/
//           https://en.wikibooks.org/wiki/6502_Assembly#Absolute_Indexed_with_Y:_a.2Cy
//           http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
//           https://onlinedisassembler.com/odaweb/
// Tested with a handful of small apps and apps compiled with the BA BASIC compiler in my TTT repo.
// 100% success with 6502_functional_test.a65 from https://github.com/Klaus2m5/6502_65C02_functional_tests
// Success with that test means an infinite loop at address 0x347d (if you don't modify the sources).
// Tested with the Wozniak Apple 1 Monitor and BASIC interpreter.
// Tested with C apps compiled with Aztec and CC65 compilers.

#include <assert.h>
#include <djltrace.hxx>
#include "mos6502.hxx"

static uint8_t g_State = 0;

#define stateTraceInstructions 1
#define stateEndEmulation 2
#define stateSoftReset 4

void MOS_6502::trace_instructions( bool t ) { if ( t ) g_State |= stateTraceInstructions; else g_State &= ~stateTraceInstructions; }
void MOS_6502::end_emulation() { g_State |= stateEndEmulation; }
void MOS_6502::soft_reset() { g_State |= stateSoftReset; }

uint8_t memory[ 65536 ];
MOS_6502 cpu;

struct Instruction
{
    uint8_t length;            // # of bytes used by the instruction
    uint8_t cycles;            // base # of cpu cycles. conditional branches and page-crossing addressing use more
    const char assembly[ 14 ]; // 14 so indexing the array is a power of 2 since this struct is 16 bytes.
};

// instructions with zeroes are not documented and not supported. hook and halt are illegal, but used for emulation.

static const Instruction ins_6502[ 256 ] =
{
    /*00*/ 2,7,"brk",     2,6,"ora (a8,x)", 0,0,"",        0,0,"", 0,0,"",          2,3,"ora a8",    2,5,"asl a8",    0,0,"",
    /*08*/ 1,3,"php",     2,2,"ora #d8",    1,2,"asl a",   0,0,"", 0,0,"",          3,4,"ora a16",   3,6,"asl a16",   1,1,"(hook)",
    /*10*/ 2,3,"bpl r8",  2,5,"ora (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"ora a8,x",  2,6,"asl a8,x",  0,0,"",
    /*18*/ 1,2,"clc",     3,4,"ora a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"ora a16,x", 3,7,"asl a16,x", 0,0,"",
    /*20*/ 3,6,"jsr a16", 2,6,"and (a8,x)", 0,0,"",        0,0,"", 2,3,"bit a8",    2,3,"and a8",    2,5,"rol a8",    0,0,"",
    /*28*/ 1,4,"plp",     2,2,"and #d8",    1,2,"rol a",   0,0,"", 3,4,"bit a16",   3,4,"and a16",   3,6,"rol a16",   0,0,"",
    /*30*/ 2,3,"bmi r8",  2,5,"and (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"and a8,x",  2,6,"rol a8,x",  0,0,"",
    /*38*/ 1,2,"sec",     3,4,"and a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"and a16,x", 3,7,"rol a16,x", 0,0,"",
    /*40*/ 1,6,"rti",     2,6,"eor (a8,x)", 0,0,"",        0,0,"", 0,0,"",          2,3,"eor a8",    2,5,"lsr a8",    0,0,"",
    /*48*/ 1,3,"pha",     2,2,"eor #d8",    1,2,"lsr a",   0,0,"", 3,3,"jmp a16",   3,4,"eor a16",   3,6,"lsr a16",   0,0,"",
    /*50*/ 2,3,"bvc r8",  2,5,"eor (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"eor a8,x",  2,6,"lsr a8,x",  0,0,"",
    /*58*/ 1,2,"cli",     3,4,"eor a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"eor a16,x", 3,7,"lsr a16,x", 0,0,"",
    /*60*/ 1,6,"rts",     2,6,"adc (a8,x)", 0,0,"",        0,0,"", 0,0,"",          2,3,"adc a8",    2,5,"ror a8",    0,0,"",
    /*68*/ 1,4,"pla",     2,2,"adc #d8",    1,2,"ror a",   0,0,"", 3,5,"jmp (a16)", 3,4,"adc a16",   3,6,"ror a16",   0,0,"",
    /*70*/ 2,3,"bvs r8",  2,5,"adc (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"adc a8,x",  2,6,"ror a8,x",  0,0,"",
    /*78*/ 1,2,"sei",     3,4,"adc a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"adc a16,x", 3,7,"ror a16,x", 0,0,"",
    /*80*/ 0,0,"",        2,6,"sta (a8,x)", 0,0,"",        0,0,"", 2,3,"sty a8",    2,3,"sta a8",    2,3,"stx a8",    0,0,"",
    /*88*/ 1,2,"dey",     0,0,"",           1,2,"txa",     0,0,"", 3,4,"sty a16",   3,4,"sta a16",   3,4,"stx a16",   0,0,"",
    /*90*/ 2,3,"bcc r8",  2,6,"sta (a8),y", 0,0,"",        0,0,"", 2,4,"sty a8,x",  2,4,"sta a8,x",  2,4,"stx a8,y",  0,0,"",
    /*98*/ 1,2,"tya",     3,5,"sta a16,y",  1,2,"txs",     0,0,"", 0,0,"",          3,5,"sta a16,x", 0,0,"",          0,0,"",
    /*a0*/ 2,2,"ldy #d8", 2,6,"lda (a8,x)", 2,2,"ldx #d8", 0,0,"", 2,3,"ldy a8",    2,3,"lda a8",    2,3,"ldx a8",    0,0,"",
    /*a8*/ 1,2,"tay",     2,2,"lda #d8",    1,2,"tax",     0,0,"", 3,4,"ldy a16",   3,4,"lda a16",   3,4,"ldx a16",   0,0,"",
    /*b0*/ 2,3,"bcs r8",  2,5,"lda (a8),y", 0,0,"",        0,0,"", 2,4,"ldy a8,x",  2,4,"lda a8,x",  2,4,"ldx a8,y",  0,0,"",
    /*b8*/ 1,2,"clv",     3,4,"lda a16,y",  1,2,"tsx",     0,0,"", 3,4,"ldy a16,x", 3,4,"lda a16,x", 3,4,"ldx a16,y", 0,0,"",
    /*c0*/ 2,2,"cpy #d8", 2,6,"cmp (a8,x)", 0,0,"",        0,0,"", 2,3,"cpy a8",    2,3,"cmp a8",    2,5,"dec a8",    0,0,"",
    /*c8*/ 1,2,"iny",     2,2,"cmp #d8",    1,2,"dex",     0,0,"", 3,4,"cpy a16",   3,4,"cmp a16",   3,6,"dec a16",   0,0,"",
    /*d0*/ 2,3,"bne r8",  2,5,"cmp (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"cmp a8,x",  2,6,"dec a8,x",  0,0,"",
    /*d8*/ 1,2,"cld",     3,4,"cmp a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"cmp a16,x", 3,7,"dec a16,x", 0,0,"",
    /*e0*/ 2,2,"cpx #d8", 2,6,"sbc (a8,x)", 0,0,"",        0,0,"", 2,3,"cpx a8",    2,3,"sbc a8",    2,5,"inc a8",    0,0,"",
    /*e8*/ 1,2,"inx",     2,2,"sbc #d8",    1,2,"nop",     0,0,"", 3,4,"cpx a16",   3,4,"sbc a16",   3,6,"inc a16",   0,0,"",
    /*f0*/ 2,3,"beq r8",  2,5,"sbc (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"sbc a8,x",  2,6,"inc a8,x",  0,0,"",
    /*f8*/ 1,2,"sed",     3,4,"sbc a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"sbc a16,x", 3,7,"inc a16,x", 1,1,"(halt)",
};

#ifdef TARGET_BIG_ENDIAN
static uint16_t get_word( uint16_t offset ) { return (uint16_t) memory[ offset ] | ( (uint16_t) memory[ offset + 1 ] << 8 ); } // do this in an endian-free way
#else
static uint16_t get_word( uint16_t offset ) { return * ( (uint16_t * ) & memory[ offset ] ); } // faster than big-endian way
#endif
#define get_byte( a ) memory[ a ]
#define set_byte( a, v ) memory[ a ] = v
static uint8_t * get_mem( uint16_t a ) { return & memory[ a ]; }
static void push( uint8_t x ) { memory[ 0x0100 + cpu.sp ] = x; cpu.sp--; }
static uint8_t pop() { cpu.sp++; return memory[ 0x0100 + cpu.sp ]; }

const char * MOS_6502::render_operation( uint16_t address )
{
    static char ac[ 60 ] = {0};
    char actemp[ 60 ];
    strcpy( ac, ins_6502[ memory[ address ] ].assembly );
    char * pnum = 0;
    if ( ( pnum = strstr( ac, "8" ) ) )
    {
        uint8_t num = memory[ address + 1 ];
        strcpy( actemp, ac );
        size_t offset = ( pnum - ac - 1 );
        snprintf( actemp + offset, _countof( actemp ) - offset, "$%02x", (uint32_t) num );
        strcat( actemp, pnum + 1 );
        strcpy( ac, actemp );
    }
    else if ( ( pnum = strstr( ac, "16" ) ) )
    {
        uint16_t num = get_word( address + 1 );
        strcpy( actemp, ac );
        size_t offset = ( pnum - ac - 1 );
        snprintf( actemp + offset, _countof( actemp ) - offset, "$%04x", (uint32_t) num );
        strcat( actemp, pnum + 2 );
        strcpy( ac, actemp );
    }

    return ac;
} //render_operation

void MOS_6502::trace_state()
{
    //tracer.TraceBinaryData( & memory[ 0x3f ], 16, 2 );
    tracer.Trace( "pc %04x, op %02x, op2 %02x, op3 %02x, a %02x, x %02x, y %02x, sp %02x, %s, %s\n",
                  pc, memory[ pc ], memory[ pc + 1 ], memory[ pc + 2 ],
                  a, x, y, sp, render_flags(), render_operation( pc ) );
} //trace_state

uint8_t MOS_6502::op_rotate( uint8_t op, uint8_t val )
{
    uint8_t rotate = op >> 5;
    assert( rotate < 4 );
    if ( 0 == rotate )         // asl
    {
        fCarry = ( 0 != ( 0x80 & val ) );
        val <<= 1;
    }
    else if ( 1 == rotate )    // rol
    {
        bool oldCarry = fCarry;
        fCarry = ( 0 != ( 0x80 & val ) );
        val <<= 1;
        if ( oldCarry )
            val |= 1;
    }
    else if ( 2 == rotate )    // lsr
    {
        fCarry = ( 0 != ( 1 & val ) );
        val >>= 1;
    }
    else                       // ror
    {
        bool oldCarry = fCarry;
        fCarry = ( 0 != ( 1 & val ) );
        val >>= 1;
        if ( oldCarry )
            val |= 0x80;
    }

    set_nz( val );
    return val;
} //op_rotate

void MOS_6502::op_cmp( uint8_t lhs, uint8_t rhs )
{
    set_nz( (uint8_t) ( (uint16_t) lhs - (uint16_t) rhs ) );
    fCarry = ( lhs >= rhs );
} //op_cmp

void MOS_6502::op_bit( uint8_t val )
{
    uint8_t result = a & val;
    fNegative = ( 0 != ( val & 0x80 ) ); // val, not result
    fOverflow = ( 0 != ( val & 0x40 ) ); // val, not result
    fZero = ( 0 == result );             // result, not val
} //op_bit

void MOS_6502::op_bcd_math( uint8_t math, uint8_t rhs )
{
    // only set/use C flag. Ignore all other flags. Except clear Z since that indicates to some it's a plain old 6502 not 65C02
    fZero = false;

    uint8_t alo = a & 0xf;
    uint8_t ahi = ( a >> 4 ) & 0xf;
    uint8_t rlo = rhs & 0xf;
    uint8_t rhi = ( rhs >> 4 ) & 0xf;

    if ( alo > 9 || ahi > 9 || rlo > 9 || rhi > 9 ) // ignore if not already bcd
        return;

    uint8_t ad = ahi * 10 + alo;
    uint8_t rd = rhi * 10 + rlo;
    uint8_t result;

    if ( 7 == math ) // sbc
    {
        if ( !fCarry )
            rd += 1;

        if ( ad >= rd )
        {
            result = ad - rd;
            fCarry = true;
        }
        else
        {
            result = 100 + ad - rd;
            fCarry = false;
        }
    }
    else // adc
    {
        result = ad + rd + ( fCarry ? 1 : 0 );
        if ( result > 99 )
        {
            result -= 100;
            fCarry = true;
        }
        else
            fCarry = false;
    }

    a = ( ( result / 10 ) << 4 ) + ( result % 10 );
    assert( ( a & 0x0f ) <= 9 );
    assert( ( ( a & 0xf0 ) >> 4 ) <= 9 );
} //op_bcd_math

void MOS_6502::op_math( uint8_t op, uint8_t rhs )
{
    uint8_t math = op >> 5;
    if ( 6 == math ) // cmp
    {
        op_cmp( a, rhs );
        return;  // no need to fall through to set_nz below since it's already done
    }

    if ( fDecimal && ( 7 == math || 3 == math ) )
    {
        op_bcd_math( math, rhs );
        return;
    }

    if ( 7 == math ) // sbc -- invert and use adc
    {
        rhs = 255 - rhs;
        math = 3; // adc
    }

    if ( 3 == math ) // adc
    {
        uint16_t result16 = (uint16_t) a + (uint16_t) rhs + (uint16_t) ( fCarry ? 1 : 0 );
        uint8_t result = result16 & 0xff;
        fCarry = ( 0 != ( result16 & 0xff00 ) );
        fOverflow = ( ! ( ( a ^ rhs ) & 0x80 ) ) && ( ( a ^ result ) & 0x80 );
        a = result;
    }
    else if ( 0 == math ) // or
        a |= rhs;
    else if ( 1 == math ) // and
        a &= rhs;
    else if ( 2 == math ) // eor
        a ^= rhs;
    else
        assert( !"unexpected math" );

    set_nz( a );
} //op_math

void MOS_6502::op_pop_pf()
{
    pf = pop();
    fNegative = ( 0 != ( pf & 0x80 ) );
    fOverflow = ( 0 != ( pf & 0x40 ) );
    fDecimal = ( 0 != ( pf & 0x08 ) );
    fInterruptDisable = ( 0 != ( pf & 0x04 ) );
    fZero = ( 0 != ( pf & 0x02 ) );
    fCarry = ( 0 != ( pf & 0x01 ) );
} // op_pop_pf

void MOS_6502::op_php()
{
    pf = 0x20; // unused bit always set
    if ( fNegative ) pf |= 0x80;
    if ( fOverflow ) pf |= 0x40;
    pf |= 0x10; // Break is always set for software int BRK or PHP to distinguish from a hardware int
    if ( fDecimal ) pf |= 0x08;
    if ( fInterruptDisable ) pf |= 0x04;
    if ( fZero ) pf |= 0x02;
    if ( fCarry ) pf |= 0x01;
    push( pf );
} //op_php

uint64_t MOS_6502::emulate( uint16_t maxcycles )
{
    uint8_t val, *pb;
    uint16_t address;
    uint16_t cycles = 0;

    while ( cycles < maxcycles )
    {
        if ( 0 != g_State )   // grouped into one check rather than 3 every loop
        {
            if ( g_State & stateEndEmulation )
            {
                g_State &= ~stateEndEmulation;
                break;
            }
            else if ( g_State & stateSoftReset )
            {
                g_State &= ~stateSoftReset;
                pc = get_word( 0xfffc );
                continue;
            }
            else if ( g_State & stateTraceInstructions )
                trace_state();
        }

        uint8_t op = memory[ pc ];
        cycles += ins_6502[ op ].cycles;

        switch( op )  // switch on fixed opcodes to use a jump table
        {
            case 0x00: // brk
            {
                address = pc + 2; // brk is a 00 + one padding/intent byte; return past the padding
                push( address >> 8 );
                push( address & 0xff );
                op_php(); // push with old state of interrupt flag
                fInterruptDisable = true;
                pc = get_word( 0xfffe ); // jump to the IRQ software interrupt vector
                continue;
            }
            case 0x01: case 0x21: case 0x41: case 0x61: case 0xc1: case 0xe1: /* ora/and/eor/adc/cmp/sbc (a8, x) */
            {
                val = get_byte( cpu.pc + 1 ) + cpu.x; /* reduce expression complexity for hisoft C */
                op_math( op, get_byte( get_word( val ) ) );
                break;
            }
            case 0x11: case 0x31: case 0x51: case 0x71: case 0xd1: case 0xf1: /* ora/and/eor/adc/cmp/sbc (a8), y */
            {
                val = get_byte( cpu.pc + 1 ); /* reduce expression complexity for hisoft C */
                op_math( op, get_byte( cpu.y + get_word( val ) ) );
                break;
            }
            case 0x05: case 0x25: case 0x45: case 0x65: case 0xc5: case 0xe5: /* ora/and/eor/adc/cmp/sbc a8 */
            {
                op_math( op, get_byte( get_byte( cpu.pc + 1 ) ) );
                break;
            }
            case 0x15: case 0x35: case 0x55: case 0x75: case 0xd5: case 0xf5: /* ora/and/eor/adc/cmp/sbc a8, x */
            {
                op_math( op, get_byte( cpu.x + get_byte( cpu.pc + 1 ) ) );
                break;
            }
            case 0x09: case 0x29: case 0x49: case 0x69: case 0xc9: case 0xe9:  /* ora/and/eor/adc/cmp/sbc #d8 */
            {
                op_math( op, get_byte( cpu.pc + 1 ) );
                break;
            }
            case 0x19: case 0x39: case 0x59: case 0x79: case 0xd9: case 0xf9: /* ora/and/eor/adc/cmp/sbc a16, y */
            {
                op_math( op, get_byte( get_word( cpu.pc + 1 ) + cpu.y ) );
                break;
            }
            case 0x0d: case 0x2d: case 0x4d: case 0x6d: case 0xcd: case 0xed:  /* ora/and/eor/adc/cmp/sbc a16 */
            {
                op_math( op, get_byte( get_word( cpu.pc + 1 ) ) );
                break;
            }
            case 0x1d: case 0x3d: case 0x5d: case 0x7d: case 0xdd: case 0xfd: /* ora/and/eor/adc/cmp/sbc a16, x */
            {
                op_math( op, get_byte( cpu.x + get_word( cpu.pc + 1 ) ) );
                break;
            }
            case 0x06: case 0x26: case 0x46: case 0x66: { address = get_byte( cpu.pc + 1 ); goto _rot_complete; }             /* asl/rol/lsr/ror a8 */
            case 0x16: case 0x36: case 0x56: case 0x76: { address = ( cpu.x + get_byte( cpu.pc + 1 ) ); goto _rot_complete; } /* asl/rol/lsr/ror a8, x*/
            case 0x0e: case 0x2e: case 0x4e: case 0x6e: { address = get_word( cpu.pc + 1 ); goto _rot_complete; }             /* asl/rol/lsr/ror a16 */
            case 0x1e: case 0x3e: case 0x5e: case 0x7e:                                                                       /* asl/rol/lsr/ror a16, x */
            {
                address = cpu.x + get_word( cpu.pc + 1 );
_rot_complete:
                pb = (uint8_t *) get_mem( address ); /* avoid two calls to get_mem */
                *pb = op_rotate( op, *pb );
                break;
            }
            case 0x08: { op_php(); break; } // php
            case 0x0f: { op = mos6502_invoke_hook(); assert( OPCODE_RTS == op ); goto _op_rts; } // hook
            case 0x10: { if ( !cpu.fNegative ) goto _branch; break; }                  /* bpl */
            case 0x30: { if ( cpu.fNegative ) goto _branch; break; }                   /* bmi */
            case 0x50: { if ( !cpu.fOverflow ) goto _branch; break; }                  /* bvc */
            case 0x70: { if ( cpu.fOverflow ) goto _branch; break; }                   /* bvs */
            case 0x90: { if ( !cpu.fCarry ) goto _branch; break; }                     /* bcc */
            case 0xb0: { if ( cpu.fCarry ) goto _branch; break; }                      /* bcs */
            case 0xd0: { if ( !cpu.fZero ) goto _branch; break; }                      /* bne */
            case 0xf0:                                                                 /* beq */
            {
                if ( !cpu.fZero )
                    break;
_branch:
                cpu.pc += ( 2 + (int16_t) (int8_t) get_byte( cpu.pc + 1 ) );
                continue;
            }
            case 0x18: { fCarry = false; break; } // clc
            case 0x20: // jsr a16
            {
                address = pc + 2;  // it's really +3, but this is how the 6502 works
                push( address >> 8 );
                push( address & 0xff );
                pc = get_word( pc + 1 );
                continue;
            }
            case 0x24: { op_bit( memory[ memory[ pc + 1 ] ] ); break; } // bit a8   NZV
            case 0x28: { op_pop_pf(); break; } // plp NZCIDV
            case 0x2c: { op_bit( memory[ get_word( pc + 1 ) ] ); break; } // bit a16   NZV
            case 0x38: { fCarry = true; break; } // sec
            case 0x40: // rti
            {
                op_pop_pf();
                pc = pop();
                pc |= ( ( (uint16_t) pop() ) << 8 );
                continue;
            }
            case 0x48: { push( a ); break; } // pha
            case 0x4c: { pc = get_word( pc + 1 ); continue; } // jmp a16
            case 0x58: { fInterruptDisable = false; break; } // cli
            case 0x60: // rts
            {
_op_rts:
                uint16_t lo = pop();
                pc = 1 + ( ( (uint16_t) pop() << 8 ) | lo );
                continue;
            }
            case 0x68: { a = pop(); set_nz( a ); break; } // pla    NZ
            case 0x6a: case 0x4a: case 0x2a: case 0x0a: { a = op_rotate( op, a ); break; } // asl, rol, lsr, ror
            case 0x6c: { pc = get_word( get_word( pc + 1 ) ); continue; } // jmp (a16)
            case 0x78: { fInterruptDisable = true; break; } // sei
            case 0x81: { address = get_word( (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.x ) ); goto _st_complete; } /* stx (a8, x) */
            case 0x84: case 0x85: case 0x86: { address = get_byte( cpu.pc + 1 ); goto _st_complete; }             /* sty/sta/stx a8 */
            case 0x8c: case 0x8d: case 0x8e: { address = get_word( cpu.pc + 1 ); goto _st_complete; }             /* sty/sta/stx a16 */
            case 0x91: { address = cpu.y + get_word( get_byte( cpu.pc + 1 ) ); goto _st_complete; }               /* sta (a8), y */
            case 0x94: case 0x95: { address = (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.x ); goto _st_complete; }  /* sta/sty a8, x */
            case 0x96: { address = (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.y ); goto _st_complete; }             /* stx a8, y */
            case 0x99: { address = get_word( cpu.pc + 1 ) + cpu.y; goto _st_complete; }                           /* sta a16, y */
            case 0x9d:                                                                                            /* sta a16, x */
            {
                address = get_word( cpu.pc + 1 ) + cpu.x;
_st_complete:
                set_byte( address, ( op & 1 ) ? cpu.a : ( op & 2 ) ? cpu.x : cpu.y );
                if ( 0xd012 == address )                                               /* apple 1 memory-mapped I/O */
                    mos6502_apple1_store( address );
                break;
            }
            case 0x88: { y--; set_nz( y ); break; } // dey
            case 0x8a: { a = x; set_nz( a ); break; } // txa
            case 0x98: { a = y; set_nz( a ); break; } // tya
            case 0x9a: { sp = x; break; } // txs no flags set
            case 0xa0: case 0xa2: case 0xa9: { address = cpu.pc + 1; goto _ld_complete; }                         /* ldy/ldx/lda #d8 */
            case 0xa1: { address = get_word( (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.x ) ); goto _ld_complete; } /* lda (a8, x ) */
            case 0xa4 : case 0xa5: case  0xa6: { address = get_byte( cpu.pc + 1 ); goto _ld0_complete; }          /* ldy/lda/ldx a8 */
            case 0xac: case 0xad: case 0xae:{ address = get_word( cpu.pc + 1 ); goto _ld_complete; }              /* ldy/lda/ldx a16 */
            case 0xb1: { address = cpu.y + get_word( (uint16_t) get_byte( cpu.pc + 1 ) ); goto _ld_complete; }    /* lda (a8), y */
            case 0xb4: case 0xb5: { address = (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.x ); goto _ld0_complete; } /* ldy/lda a8, x */
            case 0xb6: { address = (uint8_t) ( get_byte( cpu.pc + 1 ) + cpu.y ); goto _ld0_complete; }            /* ldx a8, y */
            case 0xb9 : case 0xbe: { address = get_word( cpu.pc + 1 ) + cpu.y; goto _ld_complete; }               /* lda/ldx a16, y */
            case 0xbc: case 0xbd:                                                                                 /* ldy/lda a16, x */
            {
                address = get_word( cpu.pc + 1 ) + cpu.x;
_ld_complete:   /* load */
                if ( address >= 0xd010 && address <= 0xd012 )                          /* apple 1 memory-mapped I/O */
                    set_byte( address, mos6502_apple1_load( address ) );
_ld0_complete:  /* load from page 0 */
                val = get_byte( address );
                set_nz( val );

                if ( op & 1 )
                    cpu.a = val;
                else if ( op & 2 )
                    cpu.x = val;
                else
                    cpu.y = val;
                break;
            }
            case 0xa8: { y = a; set_nz( y ); break; } // tay
            case 0xaa: { x = a; set_nz( x ); break; } // tax
            case 0xb8: { fOverflow = false; break; } // clv
            case 0xba: { x = sp; set_nz( x ); break; } // tsx
            case 0xc0: { op_cmp( y, memory[ pc + 1 ] ); break; } // cpy #d8
            case 0xc4: { op_cmp( y, memory[ memory[ pc + 1 ] ] ); break; } // cpy a8
            case 0xc6 : case 0xe6: { address = get_byte( cpu.pc + 1 ); goto _crement_complete; }         /* inc/dec a8 */
            case 0xce : case 0xee: { address = get_word( cpu.pc + 1 ); goto _crement_complete; }         /* inc/dec a16 */
            case 0xd6 : case 0xf6: { address = (uint8_t) ( cpu.x + get_byte( cpu.pc + 1 ) ); goto _crement_complete; } /* inc/dec a8, x */
            case 0xde : case 0xfe:                                                                       /* inc/dec a16, x */
            {
                address = cpu.x + get_word( cpu.pc + 1 );
_crement_complete:
                pb = (uint8_t *) get_mem( address );
                if ( op >= 0xe6 )
                    (*pb)++;
                else
                    (*pb)--;
                set_nz( *pb );
                break;
            }
            case 0xc8: { y++; set_nz( y ); break; } // iny
            case 0xca: { x--; set_nz( x ); break; } // dex
            case 0xcc: { op_cmp( y, memory[ get_word( pc + 1 ) ] ); break; } // cpy a16
            case 0xd8: { fDecimal = false; break; } // cld
            case 0xe0: { op_cmp( x, memory[ pc + 1 ] ); break; } // cpx #d8
            case 0xe4: { op_cmp( x, memory[ memory[ pc + 1 ] ] ); break; } // cpx a8
            case 0xe8: { x++; set_nz( x ); break; } // inx
            case 0xea: { break; } // nop
            case 0xec: { op_cmp( x, memory[ get_word( pc + 1 ) ] ); break; } // cpx a16
            case 0xf8: { fDecimal = true; break; } // sed
            case 0xff: { mos6502_invoke_halt(); goto _all_done; } // halt
            default: mos6502_hard_exit( "mos6502 unimplemented instruction opcode %02x\n", op );
        } //switch

        pc += ins_6502[ op ].length;
    } //while

_all_done:
    return cycles;
} //emulate
