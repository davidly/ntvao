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
// 100% success (with ROM_vectors=0) with 6502_functional_test.a65 from https://github.com/Klaus2m5/6502_65C02_functional_tests
// Success with that test means an infinite loop at address 0x347d (if you don't modify the sources).
// Tested with the Wozniak Apple 1 Monitor and BASIC interpreter

#include <assert.h>
#include <djltrace.hxx>
#include "mos6502.hxx"

static uint32_t g_State = 0;

const uint32_t stateTraceInstructions = 1;
const uint32_t stateEndEmulation = 2;
const uint32_t stateSoftReset = 4;

void MOS_6502::trace_instructions( bool t ) { if ( t ) g_State |= stateTraceInstructions; else g_State &= ~stateTraceInstructions; }
void MOS_6502::end_emulation() { g_State |= stateEndEmulation; }
void MOS_6502::soft_reset() { g_State |= stateSoftReset; }

uint8_t memory[ 65536 ];
MOS_6502 cpu;

struct Instruction
{
    uint8_t length;            // # of bytes used by the instruction
    uint8_t cycles;            // base # of cpu cycles. conditional branches and page-crossing addressing use more
    const char assembly[ 14 ];
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
    /*e0*/ 2,2,"cpx #d8", 2,6,"sbc (a8,x)", 0,0,"",        0,0,"", 2,3,"cpx a8",    2,3,"sbc a8",    2,5,"inx a8",    0,0,"", 
    /*e8*/ 1,2,"inx",     2,2,"sbc #d8",    1,2,"nop",     0,0,"", 3,4,"cpx a16",   3,4,"sbc a16",   3,6,"inc a16",   0,0,"",
    /*f0*/ 2,3,"beq r8",  2,5,"sbc (a8),y", 0,0,"",        0,0,"", 0,0,"",          2,4,"sbc a8,x",  2,6,"inc a8,x",  0,0,"", 
    /*f8*/ 1,2,"sed",     3,4,"sbc a16,y",  0,0,"",        0,0,"", 0,0,"",          3,4,"sbc a16,x", 3,7,"inc a16,x", 1,1,"(halt)", 
};

uint16_t mword( uint16_t offset ) { return * ( (uint16_t * ) & memory[ offset ] ); }
void setmword( uint16_t offset, uint16_t value ) { * ( uint16_t * ) & memory[ offset ] = value; }
void push( uint8_t x ) { memory[ 0x0100 + cpu.sp ] = x; cpu.sp--; }
uint8_t pop() { cpu.sp++; return memory[ 0x0100 + cpu.sp ]; }

bool crosses_page( uint16_t address, uint8_t offset )
{
    return ( ( 0xff00 & address ) != ( 0xff00 & ( address + (uint16_t) offset ) ) );
} //crosses_page

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
        sprintf( actemp + ( pnum - ac - 1 ), "$%02x", (uint32_t) num );
        strcat( actemp, pnum + 1 );
        strcpy( ac, actemp );
    }
    else if ( ( pnum = strstr( ac, "16" ) ) )
    {
        uint16_t num = mword( address + 1 );
        strcpy( actemp, ac );
        sprintf( actemp + ( pnum - ac - 1 ), "$%04x", (uint32_t) num );
        strcat( actemp, pnum + 2 );
        strcpy( ac, actemp );
    }

    return ac;
} //render_operation

void MOS_6502::trace_state()
{
    if ( tracer.IsEnabled() ) // avoid the formatting if not actually tracing
    {
        //tracer.TraceBinaryData( & memory[ 0x3f ], 16, 2 );
        //tracer.TraceBinaryData( & memory[ 0x01f0 ], 16, 2 );
        tracer.Trace( "pc %04x, op %02x, op2 %02x, op3 %02x, a %02x, x %02x, y %02x, sp %02x, %s, %s\n",
                      pc, memory[ pc ], memory[ pc + 1 ], memory[ pc + 2 ],
                      a, x, y, sp, render_flags(), render_operation( pc ) );
    }
} //trace_state

uint8_t MOS_6502::op_rotate( uint8_t rotate, uint8_t val )
{
    assert( rotate < 4 );
    if ( 0 == rotate )         // asl
    {
        fCarry = ( 0 != ( 0x80 & val ) );
        val <<= 1;
        val &= 0xfe;
    }
    else if ( 1 == rotate )    // rol
    {
        bool oldCarry = fCarry;
        fCarry = ( 0 != ( 0x80 & val ) );
        val <<= 1;
        if ( oldCarry )
            val |= 1;
        else
            val &= 0xfe;
    }
    else if ( 2 == rotate )    // lsr
    {
        fCarry = ( 0 != ( 1 & val ) );
        val >>= 1;
        val &= 0x7f;
    }
    else                       // ror
    {
        bool oldCarry = fCarry;
        fCarry = ( 0 != ( 1 & val ) );
        val >>= 1;
        if ( oldCarry )
            val |= 0x80;
        else
            val &= 0x7f;
    }

    set_nz( val );
    return val;
} //op_rotate

void MOS_6502::op_cmp( uint8_t lhs, uint8_t rhs )
{
    uint16_t result = (uint16_t) lhs - (uint16_t) rhs;
    set_nz( (uint8_t) result );
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

void MOS_6502::op_math( uint8_t math, uint8_t rhs )
{
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

    if ( 0x0 == math ) // or
        a |= rhs;
    else if ( 0x1 == math ) // and
        a &= rhs;
    else if ( 0x2 == math ) // eor
        a ^= rhs;
    else if ( 0x3 == math ) // adc
    {
        uint16_t result16 = (uint16_t) a + (uint16_t) rhs + (uint16_t) ( fCarry ? 1 : 0 );
        uint8_t result = result16 & 0xff;
        fCarry = ( 0 != ( result16 & 0xff00 ) );
        fOverflow = ( ! ( ( a ^ rhs ) & 0x80 ) ) && ( ( a ^ result ) & 0x80 );
        a = result;
    }
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
    fInterrupt = ( 0 != ( pf & 0x04 ) );
    fZero = ( 0 != ( pf & 0x02 ) );
    fCarry = ( 0 != ( pf & 0x01 ) ); 
} // op_pop_pf

uint64_t MOS_6502::emulate( uint64_t maxcycles )
{
    uint64_t cycles = 0;

    while ( cycles < maxcycles ) 
    {
        uint8_t op = memory[ pc ];

        if ( 0 != g_State )   // grouped into one check rather than 3 every loop
        {
            if ( g_State & stateEndEmulation )
            {
                g_State &= ~stateEndEmulation;
                break;
            }

            if ( g_State & stateSoftReset )
            {
                g_State &= ~stateSoftReset;
                pc = mword( 0xfffc );
                continue;
            }

            if ( g_State & stateTraceInstructions )
                trace_state();
        }

_restart_op:
        uint16_t nextpc = pc + ins_6502[ op ].length; // default, but will change for jump/branch/return
        cycles += ins_6502[ op ].cycles;

        bool handled = true;
        switch( op )  // switch on fixed opcodes to use a jump table
        {
            case 0x00: { fBreak = true; fInterrupt = true; break; } // brk
            case 0x08: // php
            {
                pf = 0x20; // unused bit always set
                if ( fNegative ) pf |= 0x80;
                if ( fOverflow ) pf |= 0x40;
                if ( true || fBreak ) pf |= 0x10;
                if ( fDecimal ) pf |= 0x08;
                if ( fInterrupt ) pf |= 0x04;
                if ( fZero ) pf |= 0x02;
                if ( fCarry ) pf |= 0x01;
                push( pf );
                break;
            }
            case 0x0f: { op = mos6502_invoke_hook(); goto _restart_op; } // hook
            case 0x18: { fCarry = false; break; } // clc
            case 0x20: // jsr a16
            {
                uint16_t address = mword( pc + 1 );
                uint16_t returnAddress = pc + 2;  // it's really +3, but this is how the 6502 works
                push( ( returnAddress >> 8 ) & 0xff );
                push( returnAddress & 0xff );
                nextpc = address;
                break;
            }
            case 0x24: { op_bit( memory[ memory[ pc + 1 ] ] ); break; } // bit a8   NZV
            case 0x28: { op_pop_pf(); break; } // plp NZCIDV
            case 0x2c: { op_bit( memory[ mword( pc + 1 ) ] ); break; } // bit a16   NZV
            case 0x38: { fCarry = true; break; } // sec
            case 0x40: // rti
            {
                op_pop_pf();
                nextpc = pop();
                nextpc |= ( ( ( (uint16_t) pop() ) << 8 ) & 0xff00 );
                break;
            }
            case 0x48: { push( a ); break; } // pha
            case 0x4c: { nextpc = mword( pc + 1 ); break; } // jmp a16
            case 0x58: { fInterrupt = false; break; } // cli
            case 0x60: { uint16_t lo = pop(); nextpc = 1 + ( ( (uint16_t) pop() << 8 ) | lo ); break; } // rts
            case 0x68: { a = pop(); set_nz( a ); break; } // pla    NZ
            case 0x6c: { nextpc = mword( mword( pc + 1 ) ); break; } // jmp (a16)
            case 0x78: { fInterrupt = true; break; } // sei
            case 0x88: { y--; set_nz( y ); break; } // dey
            case 0x8a: { a = x; set_nz( a ); break; } // txa
            case 0x98: { a = y; set_nz( a ); break; } // tya
            case 0x9a: { sp = x; break; } // txs no flags set
            case 0xa8: { y = a; set_nz( y ); break; } // tay
            case 0xaa: { x = a; set_nz( x ); break; } // tax
            case 0xb8: { fOverflow = false; break; } // clv
            case 0xba: { x = sp; set_nz( x ); break; } // tsx
            case 0xc0: { op_cmp( y, memory[ pc + 1 ] ); break; } //cpy #d8
            case 0xc4: { op_cmp( y, memory[ memory[ pc + 1 ] ] ); break; } // cpy a8
            case 0xc8: { y++; set_nz( y ); break; } // iny
            case 0xca: { x--; set_nz( x ); break; } // dex
            case 0xcc: { op_cmp( y, memory[ mword( pc + 1 ) ] ); break; } // cpy a16
            case 0xd8: { fDecimal = false; break; } // cld
            case 0xe0: { op_cmp( x, memory[ pc + 1 ] ); break; } //cpx #d8
            case 0xe4: { op_cmp( x, memory[ memory[ pc + 1 ] ] ); break; } // cpx a8
            case 0xe8: { x++; set_nz( x ); break; } // inx
            case 0xea: { break; } // nop
            case 0xec: { op_cmp( x, memory[ mword( pc + 1 ) ] ); break; } // cpx a16
            case 0xf8: { fDecimal = true; break; } // sed
            case 0xff: { mos6502_invoke_halt(); goto _all_done; } // halt
            default: { handled = false; break; }
        }

        if ( !handled )  // each of these cases handles multiple instructions
        {
            uint8_t hi = ( op & 0xf0 );
            uint8_t lo = ( op & 0x0f );
    
            if ( 0x10 == ( op & 0x1f ) ) // conditional branches
            {
                bool branch;
                if ( hi <= 0x30 )
                    branch = fNegative;
                else if ( hi <= 0x80 )
                    branch = fOverflow;
                else if ( hi <= 0xb0 )
                    branch = fCarry;
                else
                    branch = fZero;
    
                if ( 0 == ( hi & 0x20 ) )
                    branch = !branch;
    
                if ( branch )
                {
                    cycles++;
                    uint16_t oldpc = pc;
                    nextpc = pc + 2 + (int16_t) (int8_t) memory[ pc + 1 ];
                    if ( ( oldpc & 0xff00 ) != ( pc & 0xff00 ) )
                        cycles++;
                }
            }
            else if ( ( hi < 0x80 || hi >= 0xc0 ) && ( lo & 0x01 ) ) // math of various address modes
            {
                uint8_t math = ( hi >> 4 ) / 2;
                uint8_t val;
                if ( 0x01 == lo )
                {
                    if ( hi & 0x10 )                                    // (a8),y
                    {
                        uint16_t address = mword( memory[ pc + 1 ] );
                        if ( crosses_page( address, y ) )
                            cycles++;
                        val = memory[ (uint16_t) y + address ];
                    }
                    else                                                // (a8,x)
                        val = memory[ mword( 0xff & ( memory[ pc + 1 ] + x ) ) ];  // wrap
                }
                else if ( 0x05 == lo )
                {
                    uint16_t address = memory[ pc + 1 ];                // a8
                    if ( hi & 0x10 )                                    // a8,x
                        address = 0xff & ( address + x );               // wrap
                    val = memory[ address ];
                }
                else if ( 0x09 == lo )
                {
                    if ( hi & 0x10 )                                    // a16,y
                    {
                        uint16_t address = mword( pc + 1 );
                        if ( crosses_page( address, y ) )
                            cycles++;
                        val = memory[ address + y ];
                    }
                    else                                                // #d8
                        val = memory[ pc + 1 ];                         
                }
                else if ( 0x0d == lo )
                {
                    uint16_t address = mword( pc + 1 );                 // a16
                    if ( hi & 0x10 )                                    // a16,x
                    {
                        if ( crosses_page( address, x ) )
                            cycles++;
                        address += (uint16_t) x;
                    }
                    val = memory[ address ];
                }
                else
                    mos6502_hard_exit( "mos6502 unsupported comparison instruction %02x\n", op );
    
                op_math( math, val );
            }
            else if ( hi >=  0xc0 && ( 0x06 == lo || 0x0e == lo ) ) // dec and inc memory
            {
                uint16_t address;
                if ( 0x06 == lo )
                    address = memory[ pc + 1 ];
                else
                    address = mword( pc + 1 );
    
                if ( hi & 0x10 )
                    address += x;
    
                if ( hi >= 0xe0 )
                    memory[ address ]++;
                else
                    memory[ address ]--;
    
                set_nz( memory[ address ] );
            }
            else if ( op <= 0x6a && ( 0x0a == lo ) ) // rotate a
            {
                uint8_t rotate = ( ( hi >> 4 ) & 0xf ) / 2;
                a = op_rotate( rotate, a );
            }
            else if ( op < 0x80 && ( 0x06 == lo || 0x0e == lo ) ) // rotate memory at address
            {
                uint16_t address;
                if ( 0x06 == lo )
                {
                    address = memory[ pc + 1 ];                // a8
                    if ( hi & 0x10 )
                        address += (uint16_t) x;               // a8,x
                }
                else if ( 0x0e == lo )
                {
                    address = mword( pc + 1 );                 // a16
                    if ( hi & 0x10 )
                        address += (uint16_t) x;               // a16,x   crossing page boundry is free
                }
                else
                    mos6502_hard_exit("mos6502 unsupported rotate instruction %02x\n", op );
    
                uint8_t rotate = ( ( hi >> 4 ) & 0xf ) / 2;
                memory[ address ] = op_rotate( rotate, memory[ address ] );
            }
            else if ( op >= 0x81 && op <= 0x9d ) // sta, stx, sty. others in range handled above
            {
                uint8_t tostore = ( lo & 1 ) ? a : ( lo & 2 ) ? x : y;
                uint16_t address;
                if ( 0x81 == op )                              // (a8,x)
                    address = mword( 0xff & ( memory[ pc + 1 ] + x ) );   // wrap
                else if ( 0x91 == op )                         // (a8),y
                {
                    address = mword( memory[ pc + 1 ] );
                    if ( crosses_page( address, y ) )
                        cycles++;
                    address += y;
                }
                else if ( op >= 0x84 && op <= 0x86 )           // a8
                    address = memory[ pc + 1 ];                
                else if ( op == 0x94 || op == 0x95 )           // a8,x
                    address = 0xff & ( memory[ pc + 1 ] + x ); // wrap           
                else if ( 0x96 == op )                         // a8,y
                    address = 0xff & ( memory[ pc + 1 ] + y ); // wrap
                else if ( 0x99 == op )                         // a16,y 
                    address = mword( pc + 1 ) + y;             
                else if ( op >= 0x8c && op <= 0x8e )           // a16
                    address = mword( pc + 1 );                 
                else if ( 0x9d == op )                         // a16,x
                    address = mword( pc + 1 ) + x;             
                else
                    mos6502_hard_exit( "mos6502 unsupported store instruction %02x\n", op );

                memory[ address ] = tostore;

                // minimal Apple 1 emulation support
                if ( 0xd012 == address )
                    mos6502_apple1_store( address );
            }
            else if ( op >= 0xa0 && op <= 0xbe ) // lda, ldx, ldy. others in range handled above
            {
                uint16_t address;
                if ( ( 0xa0 == hi ) && ( 0 == lo || 2 == lo || 9 == lo ) ) // #d8
                    address = pc + 1;                          
                else if ( op == 0xa1 )                                     // (a8,x)
                    address = mword( 0xff & ( memory[ pc + 1 ] + x ) );    // wrap
                else if ( op == 0xb1 )                                     // (a8),y
                {
                    address = mword( memory[ pc + 1 ] );
                    if ( crosses_page( address, y ) )
                        cycles++;
                    address += y;
                }
                else if ( op >= 0xa4 && op <= 0xa6 )               // a8
                    address = memory[ pc + 1 ];                
                else if ( op >= 0xb4 && op <= 0xb5 )               // a8,x
                    address = 0xff & ( memory[ pc + 1 ] + x );     // wrap
                else if ( 0xb6 == op )                             // a8,y
                    address = 0xff & ( memory[ pc + 1 ] + y );     // wrap
                else if ( op >= 0xac && op <= 0xae )               // a16
                    address = mword( pc + 1 );                 
                else if ( 0xbc == op || 0xbd == op )               // a16,x
                    address = mword( pc + 1 ) + x;             
                else if ( 0xb9 == op || 0xbe == op )               // a16,y
                    address = mword( pc + 1 ) + y;             
                else
                    mos6502_hard_exit( "mos6502 unsupported load instruction opcode %02x\n", op );

                // minimal Apple 1 emulation support
                if ( address >= 0xd010 && address <= 0xd012 )
                    memory[ address ] = mos6502_apple1_load( address );

                uint8_t val = memory[ address ];
                set_nz( val );
    
                if ( op & 1 )
                    a = val;
                else if ( op & 2 )
                    x = val;
                else
                    y = val;
            }
            else
                mos6502_hard_exit( "mos6502 unimplemented instruction opcode %02x\n", op );
        }

        pc = nextpc;
    }

_all_done:
    return cycles;
} //emulate
