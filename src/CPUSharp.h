#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>

#define ZFlag 0x80
#define SFlag 0x40
#define HFlag 0x20
#define CFlag 0x10

class BoiBoy;

class CPUSharp
{
public:
	CPUSharp();
	~CPUSharp();

	void ConnectMem(BoiBoy* mem) { memory = mem; }

	void InitMem();

	int Clock();

	bool IME = false;
	bool interrupt = false;
	bool halted = false;

	bool stopped = false;

	void TimerInt();

private:
	void Debug();

	BoiBoy* memory = nullptr;

	uint8_t opcode = 0;

	int totalCycles = 0; // For Debug

	int opCycles = 0;
	int cycles = 0;

	bool ret = false;

	bool IMESch = false;

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

	int Execute();
	int ExecuteExt();

	union Register {
		struct {
			uint8_t low : 8;
			uint8_t high : 8;
		};

		uint16_t value;
	};

	Register AF, BC, DE, HL, SP, PC;

	int timerStage = 0;
	
#pragma region OP
	/*
	r1: Register to write
	r2: Register value to be written
	Cycles = 1
	*/
	int LDReg(Register& r1, uint8_t r2, bool isHigh);

	/*
	r1: Register to write
	add: Address of the value to be written
	Cycles = 2
	*/
	int LDFromMem(Register& r, uint16_t add, bool isHigh);

	/*
	add: Address to write
	r: Register value to be written
	Cycles = 2
	*/
	int LDToMem(uint16_t add, uint8_t r);

	/*
	add: Address to write next byte in memory
	Cycles = 3
	*/
	int LDFromMemToMem(uint16_t add);

	/*
	Save in address of the next 2 bytes the value in A
	Cycles = 4
	*/
	int LDToMemW();

	/*
	Save in A the value in address of the next 2 bytes
	Cycles = 4
	*/
	int LDFromMemW();

	/*
	Save in address 0xFFxx, where xx is C or next byte the value in A
	Cycles = 2 (or 3 if next byte)
	*/
	int LDHToMem(bool isC);

	/*
	Save in A the value in address 0xFFxx, where xx is C or next byte
	Cycles = 2 (or 3 if next byte)
	*/
	int LDHFromMem(bool isC);

	/*
	Save in Register the next word
	Cycles = 3
	*/
	int LDWToR(Register& r);

	/*
	Save SP in address of next word
	Cycles = 5
	*/
	int LDSP();

	/*
	Save HL in SP
	Cycles = 2
	*/
	int LDHL();

	/*
	Pops register in stack
	Cycles = 3
	*/
	int Pop(Register& r);

	/*
	Pushes register from stack
	Cycles = 4
	*/
	int Push(Register r);

	/*
	Unconditional jump to next word
	Cycles = 4
	*/
	int JP();

	/*
	Unconditional jump to HL
	Cycles = 1
	*/
	int JPHL();

	/*
	Conditional jump
	Cycles = 3 (or 4 if jump is true)
	*/
	int JPCond(uint8_t flag, bool isTrue);

	/*
	Unconditional jump to relative address
	Cycles = 3
	*/
	int JR();

	/*
	Conditional jump to relative address
	Cycles = 2 (or 3 if jump is true)
	*/
	int JRCond(uint8_t flag, bool isTrue);

	/*
	Unconditional function call
	Cycles = 6
	*/
	int Call();

	/*
	Conditional function call
	Cycles = 3 (or 6 if jump is true)
	*/
	int CallCond(uint8_t flag, bool isTrue);

	/*
	Unconditional return from function
	Cycles = 4
	*/
	int Ret();

	/*
	Conditional return from function
	Cycles = 2 (or 5 if jump is true)
	*/
	int RetCond(uint8_t flag, bool isTrue);

	/*
	Unconditional return from function
	And enables IME
	Cycles = 4
	*/
	int Reti();

	/*
	Unconditional function call to the address fixed by the opcode
	Cycles = 4
	*/
	int Rst();

	/*
	IME = 0
	Cycles = 1
	*/
	int DI();

	/*
	IME = 1 next cycle
	Cycles = 1 (and 1 later)
	*/
	int EI();

	/*
	Flips CFlag, and clears S and H Flags
	Cycles = 1
	*/
	int CCF();

	/*
	Sets CFlag, and clears S and H Flags
	Cycles = 1
	*/
	int SCF();

	int DAA();

	/*
	Flips A, and sets S and H Flags
	Cycles = 1
	*/
	int CPL();

	/*
	Increments a full register
	Cycles = 2
	*/
	int IncReg(Register& reg);

	/*
	Decrements a full register
	Cycles = 2
	*/
	int DecReg(Register& reg);

	/*
	Add a register to HL
	Cycles = 2
	*/
	int AddHL(uint16_t val);

	/*
	Add signed byte to SP
	Cycles = 4
	*/
	int AddSP();

	/*
	Stores in HL SP + next signed byte
	Cycles = 3
	*/
	int LDHLSP();

	/*
	Rotates A using Carry or not
	Cycles = 1
	*/
	int RotateA(bool isLeft, bool withCarry);

	/*
	Increments a register
	Cycles = 1
	*/
	int INC(Register& reg, bool isHigh);

	/*
	Increments a value in (HL)
	Cycles = 3
	*/
	int INCHL();

	/*
	Decrements a register
	Cycles = 1
	*/
	int DEC(Register& reg, bool isHigh);

	/*
	Decrements a value in (HL)
	Cycles = 3
	*/
	int DECHL();

	/*
	Add a register to A
	Cycles = 1
	*/
	int AddA(uint8_t val, bool withC);

	/*
	Add the next byte to A
	Cycles = 2
	*/
	int AddAFromMem(bool withC);

	/*
	Add the byte in address HL to A
	Cycles = 2
	*/
	int AddAHL(bool withC);

	/*
	Substract a register to A
	Cycles = 1
	*/
	int SubA(uint8_t val, bool withC);

	/*
	Substract the next byte to A
	Cycles = 2
	*/
	int SubAFromMem(bool withC);

	/*
	Substract the byte in address HL to A
	Cycles = 2
	*/
	int SubAHL(bool withC);

	/*
	A <= A & Reg
	Cycles = 1
	*/
	int ANDA(uint8_t val);

	/*
	A <= A & (HL)
	Cycles = 2
	*/
	int ANDAHL();

	/*
	A <= A & next byte
	Cycles = 2
	*/
	int ANDAFromMem();

	/*
	A <= A ^ Reg
	Cycles = 1
	*/
	int XORA(uint8_t val);

	/*
	A <= A ^ (HL)
	Cycles = 2
	*/
	int XORAHL();

	/*
	A <= A ^ next byte
	Cycles = 2
	*/
	int XORAFromMem();

	/*
	A <= A | Reg
	Cycles = 1
	*/
	int ORA(uint8_t val);

	/*
	A <= A | (HL)
	Cycles = 2
	*/
	int ORAHL();

	/*
	A <= A | next byte
	Cycles = 2
	*/
	int ORAFromMem();

	/*
	A - Reg
	Cycles = 1
	*/
	int CPA(uint8_t val);

	/*
	A - (HL)
	Cycles = 2
	*/
	int CPAHL();

	/*
	A - next byte
	Cycles = 2
	*/
	int CPAFromMem();

	/*
	Rotates a register
	Cycles = 2
	*/
	int RotateReg(Register& reg, bool isHigh, bool isLeft, bool withCarry);

	/*
	Rotates a value in memory
	Cycles = 4
	*/
	int RotateMem(bool isLeft, bool withCarry);

	/*
	Shifts register
	Cycles = 2
	*/
	int Shift(Register& reg, bool isHigh, bool isLeft, bool retain);

	/*
	Shifts value in memory
	Cycles = 4
	*/
	int ShiftMem(bool isLeft, bool retain);

	/*
	Swaps nibbles of a byte
	Cycles = 2
	*/
	int Swap(Register& reg, bool isHigh);

	/*
	Swaps nibbles of value in memory
	Cycles = 4
	*/
	int SwapMem();

	/*
	Checks if bit is set or not in a register
	Cycles = 2
	*/
	int Bit(Register& reg, bool isHigh, int pos);

	/*
	Checks if bit is set or not in a value in memory
	Cycles = 3
	*/
	int BitMem(int pos);

	/*
	Resets bit of register
	Cycles = 2
	*/
	int Res(Register& reg, bool isHigh, int pos);

	/*
	Resets bit of value in memory
	Cycles = 4
	*/
	int ResMem(int pos);

	/*
	Sets bit of register
	Cycles = 2
	*/
	int Set(Register& reg, bool isHigh, int pos);

	/*
	Sets bit of value in memory
	Cycles = 4
	*/
	int SetMem(int pos);
#pragma endregion
};

