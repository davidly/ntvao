#pragma once

extern uint8_t memory[ 65536 ];

#define OPCODE_HOOK 0x0f  // this unused 6502 instruction will call the hook callback
#define OPCODE_HALT 0xff  // this unused 6502 instruction will terminate the emulator
#define OPCODE_RTS 0x60   // return from function call

struct MOS_6502
{
    // emulator API

    uint64_t emulate( uint64_t maxcycles );             // execute up to about maxcycles
    void trace_instructions( bool trace );              // enable/disable tracing each instruction
    void trace_state( void );                           // trace the registers
    const char * render_operation( uint16_t address );  // return a string with the disassembled instruction at address
    void end_emulation( void );                         // the emulator returns at the start of the next instruction
    void soft_reset( void );                            // sets pc to contents of 0xfffc, the 6502 reset vector

    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;   // 0x01 is the implicit high byte
    uint16_t pc;
    uint8_t pf;   // NV-BDIZC. State is tracked in bools below and only updated for pf and php

    bool fNegative, fOverflow, fUnused, fDecimal, fInterruptDisable, fZero, fCarry;

    void power_on()
    {
        pc = memory[ 0xfffc ] | ( memory[ 0xfffd ] << 8 );
        fInterruptDisable = true; // the only flag with a defined value on reset
    } //powerOn

    const char * render_flags()
    {
        static char ac[ 10 ] = {0};
        size_t next = 0;
        ac[ next++ ] = fNegative ? 'N' : 'n';
        ac[ next++ ] = fOverflow ? 'V' : 'v';
        ac[ next++ ] = fDecimal ? 'D' : 'd';
        ac[ next++ ] = fInterruptDisable ? 'I' : 'i';
        ac[ next++ ] = fZero ? 'Z' : 'z';
        ac[ next++ ] = fCarry ? 'C' : 'c';
        ac[ next ] = 0;

        return ac;
    } //render_flags

  private:
    void op_pop_pf( void );
    void op_bit( uint8_t val );
    void op_math( uint8_t math, uint8_t rhs );
    void op_bcd_math( uint8_t math, uint8_t rhs );
    uint8_t op_rotate( uint8_t rotate, uint8_t val );
    void op_cmp( uint8_t lhs, uint8_t rhs );
    void op_php();
    void op_rts();

    void set_nz( uint8_t val )
    {
         fNegative = ( 0 != ( val & 0x80 ) );
         fZero = ( 0 == val );
    } //set_nz
};

extern MOS_6502 cpu;

// callbacks into hosting app

extern void mos6502_invoke_halt( void );                 // called when the OPCODE_HALT instruction is executed
extern uint8_t mos6502_invoke_hook( void );              // called when the OPCODE_HOOK instruction is executed
extern uint8_t mos6502_apple1_load( uint16_t address );  // when 0xd010-0xd013 are used with a load instruction
extern void mos6502_apple1_store( uint16_t address );    // when 0xd010-0xd013 are used with a store instruction
extern void mos6502_hard_exit( const char * perror, uint8_t val ); // called when the emulator executes invalid instructions
