#include "CPUSharp.h"

#include "Memory.h"

FILE *LOG;
int lines = 0;

CPUSharp::CPUSharp() {
	// Boot ROM DMG
	AF.high = 0x01;
	AF.low = ZFlag;
	BC.value = 0x0013;
	DE.value = 0x00D8;
	HL.value = 0x014D;
	SP.value = 0xFFFE;
	PC.value = 0x100;

	//if (freopen_s(&LOG, "ZeldaOutOfBoundsLOG.txt", "a", stdout) == NULL) {
	//	return;
	//}
}

CPUSharp::~CPUSharp() {
	fclose(stdout);
}

void CPUSharp::InitMem() {
	// Checksum
	int x = 0;
	int i = 0x0134;
	while (i <= 0x014C) {
		x = x - Read(i++) - 1;
	}
	uint8_t checkSum = Read(0x014D);
	if (checkSum == (x & 0xFF)) {
		if (checkSum != 0) {
			AF.low |= HFlag;
			AF.low |= CFlag;
		}
	}
	else {
		std::cout << "Checksum Failed" << std::endl;
	}
}

int CPUSharp::Clock() {

	if (cycles == 0) {

		if (halted && interrupt) {
			PC.value++;
			halted = false;
			interrupt = false;
		}

		if (halted)
			cycles++;
		else {
			opcode = Read(PC.value++);

			Debug();

			if (stopped) {
				stopped = false;
			}

			opCycles = Execute();

			if (IMESch && opcode != 0xFB) {
				IMESch = false;
				IME = true;
			}

			cycles += opCycles;
		}		
	}
	else {
		opCycles = 0;
	}

	cycles--;
	totalCycles++;

	return opCycles;
}

void CPUSharp::Write(uint16_t add, uint8_t n) {
	if (debugMem)
		std::cout << "Writting in " << std::hex << (int)add << " Value: " << (int)n << std::endl;

	memory->Write(add, n);
}

uint8_t CPUSharp::Read(uint16_t add) {
	if (debugMem)
		std::cout << "Reading in " << std::hex << (int)add << std::endl;

	uint8_t value = memory->Read(add);

	return value;
}

void CPUSharp::Interrupt(uint16_t add) {
	if (IME && halted)
		halted = false;

	interrupt = true;
	Write(--SP.value, PC.high);
	Write(--SP.value, PC.low);
	PC.value = add;
	IME = false;
	cycles += 5;
}

void CPUSharp::Debug() {
	if (debug) {
		std::cout << std::setfill('0');
		std::cout << "A: " << std::hex << std::setw(2) << std::uppercase << (int)(AF.high);
		std::cout << " F: " << std::hex << std::setw(2) << std::uppercase << (int)(AF.low);
		std::cout << " B: " << std::hex << std::setw(2) << std::uppercase << (int)(BC.high);
		std::cout << " C: " << std::hex << std::setw(2) << std::uppercase << (int)(BC.low);
		std::cout << " D: " << std::hex << std::setw(2) << std::uppercase << (int)(DE.high);
		std::cout << " E: " << std::hex << std::setw(2) << std::uppercase << (int)(DE.low);
		std::cout << " H: " << std::hex << std::setw(2) << std::uppercase << (int)(HL.high);
		std::cout << " L: " << std::hex << std::setw(2) << std::uppercase << (int)(HL.low);
		std::cout << " SP: " << std::hex << std::setw(4) << std::uppercase << (int)(SP.value);
		std::cout << " PC: " << std::hex << "00:" << std::setw(4) << std::uppercase << (int)(PC.value - 1);
		std::cout << " (" << std::hex << std::setw(2) << std::uppercase << (int)(Read(PC.value - 1));
		std::cout << " " << std::hex << std::setw(2) << std::uppercase << (int)(Read(PC.value));
		std::cout << " " << std::hex << std::setw(2) << std::uppercase << (int)(Read(PC.value + 1));
		std::cout << " " << std::hex << std::setw(2) << std::uppercase << (int)(Read(PC.value + 2)) << ")" << std::endl;
		std::cout << "LCDC: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.LCDC);
		std::cout << " STAT: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.STAT);
		std::cout << " LY: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.LY);
		std::cout << " LYC: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.LYC);
		std::cout << " CNT: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.cnt);
		std::cout << " SCY: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.SCY);
		std::cout << " SCX: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->ppu.SCX);
		std::cout << " IE: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->IEReg);
		std::cout << " IF: " << std::hex << std::setw(2) << std::uppercase << (int)(memory->IFReg) << std::endl;
		//lines++;
		//std::cout << lines << std::endl;
		if (lines >= 161503) {
			fclose(stdout);
			std::cout << "Finished" << std::endl;
		}
	}
}

int CPUSharp::Execute() {
	switch (opcode) {
	// NOP
	case 0x00:
		if(debugOp)
			std::cout << "NOP" << std::endl;

		if (halted) {
			PC.value--;
		}

		return 1;

	// LD BC, u16
	case 0x01:
		if (debugOp)
			std::cout << "LD BC, u16" << std::endl;

		return LDWToR(BC);

	// LD (BC), A
	case 0x02:
		if (debugOp)
			std::cout << "LD (BC), A" << std::endl;

		return LDToMem(BC.value, AF.high);

	// INC BC
	case 0x03:
		if (debugOp)
			std::cout << "INC BC" << std::endl;

		return IncReg(BC);

	// INC B
	case 0x04:
		if (debugOp)
			std::cout << "INC B" << std::endl;

		return INC(BC, true);

	// DEC B
	case 0x05:
		if (debugOp)
			std::cout << "DEC B" << std::endl;

		return DEC(BC, true);

	// LD B, u8
	case 0x06:
		if (debugOp)
			std::cout << "LD B, u8" << std::endl;

		return LDFromMem(BC, PC.value++, true);

	// RLCA
	case 0x07:
		if (debugOp)
			std::cout << "RLCA" << std::endl;

		return RotateA(true, true);

	// LD (u16), SP
	case 0x08:
		if (debugOp)
			std::cout << "LD (u16), SP" << std::endl;

		return LDSP();

	// ADD HL, BC
	case 0x09:
		if (debugOp)
			std::cout << "ADD HL, BC" << std::endl;

		return AddHL(BC.value);

	// LD A, (BC)
	case 0x0A:
		if (debugOp)
			std::cout << "LD A, (BC)" << std::endl;

		return LDFromMem(AF, BC.value, true);

	// DEC BC
	case 0x0B:
		if (debugOp)
			std::cout << "DEC BC" << std::endl;

		return DecReg(BC);

	// INC C
	case 0x0C:
		if (debugOp)
			std::cout << "INC C" << std::endl;

		return INC(BC, false);

	// DEC C
	case 0x0D:
		if (debugOp)
			std::cout << "DEC C" << std::endl;

		return DEC(BC, false);

	// LD C, u8
	case 0x0E:
		if (debugOp)
			std::cout << "LD C, u8" << std::endl;

		return LDFromMem(BC, PC.value++, false);

	// RRCA
	case 0x0F:
		if (debugOp)
			std::cout << "RRCA" << std::endl;

		return RotateA(false, true);

	// STOP
	case 0x10:
		if (debugOp)
			std::cout << "STOP" << std::endl;

		stopped = true;
		PC.value++;
		return 1;

	// LD DE, u16
	case 0x11:
		if (debugOp)
			std::cout << "LD DE, u16" << std::endl;

		return LDWToR(DE);

	// LD (DE), A
	case 0x12:
		if (debugOp)
			std::cout << "LD (DE), A" << std::endl;

		return LDToMem(DE.value, AF.high);

	// INC DE
	case 0x13:
		if (debugOp)
			std::cout << "INC DE" << std::endl;

		return IncReg(DE);

	// INC D
	case 0x14:
		if (debugOp)
			std::cout << "INC D" << std::endl;

		return INC(DE, true);

	// DEC D
	case 0x15:
		if (debugOp)
			std::cout << "DEC D" << std::endl;

		return DEC(DE, true);

	// LD D, u8
	case 0x16:
		if (debugOp)
			std::cout << "LD D, u8" << std::endl;

		return LDFromMem(DE, PC.value++, true);

	// RLA
	case 0x17:
		if (debugOp)
			std::cout << "RLA" << std::endl;

		return RotateA(true, false);

	// JR i8
	case 0x18:
		if (debugOp)
			std::cout << "JR i8" << std::endl;

		return JR();

	// ADD HL, DE
	case 0x19:
		if (debugOp)
			std::cout << "ADD HL, DE" << std::endl;

		return AddHL(DE.value);

	// LD A, (DE)
	case 0x1A:
		if (debugOp)
			std::cout << "LD A, (DE)" << std::endl;

		return LDFromMem(AF, DE.value, true);

	// DEC DE
	case 0x1B:
		if (debugOp)
			std::cout << "DEC DE" << std::endl;

		return DecReg(DE);

	// INC E
	case 0x1C:
		if (debugOp)
			std::cout << "INC E" << std::endl;

		return INC(DE, false);

	// DEC E
	case 0x1D:
		if (debugOp)
			std::cout << "DEC E" << std::endl;

		return DEC(DE, false);

	// LD E, u8
	case 0x1E:
		if (debugOp)
			std::cout << "LD E, u8" << std::endl;

		return LDFromMem(DE, PC.value++, false);

	// RRA
	case 0x1F:
		if (debugOp)
			std::cout << "RRA" << std::endl;

		return RotateA(false, false);

	// JR NZ, i8
	case 0x20:
		if (debugOp)
			std::cout << "JR NZ, i8" << std::endl;

		return JRCond(ZFlag, false);

	// LD HL, u16
	case 0x21:
		if (debugOp)
			std::cout << "LD HL, u16" << std::endl;

		return LDWToR(HL);

	// LD (HL+), A
	case 0x22:
		if (debugOp)
			std::cout << "LD (HL+), A" << std::endl;

		return LDToMem(HL.value++, AF.high);

	// INC HL
	case 0x23:
		if (debugOp)
			std::cout << "INC HL" << std::endl;

		return IncReg(HL);

	// INC H
	case 0x24:
		if (debugOp)
			std::cout << "INC H" << std::endl;

		return INC(HL, true);

	// DEC H
	case 0x25:
		if (debugOp)
			std::cout << "DEC H" << std::endl;

		return DEC(HL, true);

	// LD H, u8
	case 0x26:
		if (debugOp)
			std::cout << "LD H, u8" << std::endl;

		return LDFromMem(HL, PC.value++, true);

	// DAA
	case 0x27:
		if (debugOp)
			std::cout << "DAA" << std::endl;

		return DAA();

	// JR Z, i8
	case 0x28:
		if (debugOp)
			std::cout << "JR Z, i8" << std::endl;

		return JRCond(ZFlag, true);

	// ADD HL, HL
	case 0x29:
		if (debugOp)
			std::cout << "ADD HL, HL" << std::endl;

		return AddHL(HL.value);

	// LD A, (HL+)
	case 0x2A:
		if (debugOp)
			std::cout << "LD A, (HL+)" << std::endl;

		return LDFromMem(AF, HL.value++, true);

	// DEC HL
	case 0x2B:
		if (debugOp)
			std::cout << "DEC HL" << std::endl;

		return DecReg(HL);

	// INC L
	case 0x2C:
		if (debugOp)
			std::cout << "INC L" << std::endl;

		return INC(HL, false);

	// DEC L
	case 0x2D:
		if (debugOp)
			std::cout << "DEC L" << std::endl;

		return DEC(HL, false);

	// LD L, u8
	case 0x2E:
		if (debugOp)
			std::cout << "LD L, u8" << std::endl;

		return LDFromMem(HL, PC.value++, false);

	// CPL
	case 0x2F:
		if (debugOp)
			std::cout << "CPL" << std::endl;

		return CPL();

	// JR NC, i8
	case 0x30:
		if (debugOp)
			std::cout << "JR NC, i8" << std::endl;

		return JRCond(CFlag, false);

	// LD SP, u16
	case 0x31:
		if (debugOp)
			std::cout << "LD SP, u16" << std::endl;

		return LDWToR(SP);

	// LD (HL-), A
	case 0x32:
		if (debugOp)
			std::cout << "LD (HL-), A" << std::endl;

		return LDToMem(HL.value--, AF.high);

	// INC SP
	case 0x33:
		if (debugOp)
			std::cout << "INC SP" << std::endl;

		return IncReg(SP);

	// INC (HL)
	case 0x34:
		if (debugOp)
			std::cout << "INC (HL)" << std::endl;

		return INCHL();

	// DEC (HL)
	case 0x35:
		if (debugOp)
			std::cout << "DEC (HL)" << std::endl;

		return DECHL();

	// LD (HL), u8
	case 0x36:
		if (debugOp)
			std::cout << "LD (HL), u8" << std::endl;

		return LDFromMemToMem(HL.value);

	// SCF
	case 0x37:
		if (debugOp)
			std::cout << "SCF" << std::endl;

		return SCF();

	// JR C, i8
	case 0x38:
		if (debugOp)
			std::cout << "JR C, i8" << std::endl;

		return JRCond(CFlag, true);

	// ADD HL, SP
	case 0x39:
		if (debugOp)
			std::cout << "ADD HL, SP" << std::endl;

		return AddHL(SP.value);

	// LD A, (HL-)
	case 0x3A:
		if (debugOp)
			std::cout << "LD A, (HL-)" << std::endl;

		return LDFromMem(AF, HL.value--, true);

	// DEC SP
	case 0x3B:
		if (debugOp)
			std::cout << "DEC SP" << std::endl;

		return DecReg(SP);

	// INC A
	case 0x3C:
		if (debugOp)
			std::cout << "INC A" << std::endl;

		return INC(AF, true);

	// DEC A
	case 0x3D:
		if (debugOp)
			std::cout << "DEC A" << std::endl;

		return DEC(AF, true);

	// LD A, u8
	case 0x3E:
		if (debugOp)
			std::cout << "LD A, u8" << std::endl;

		return LDFromMem(AF, PC.value++, true);

	// CCF
	case 0x3F:
		if (debugOp)
			std::cout << "CCF" << std::endl;

		return CCF();

	// LD B,B
	case 0x40:
		if (debugOp)
			std::cout << "LD B, B" << std::endl;

		return LDReg(BC, BC.high, true);

	// LD B, C
	case 0x41:
		if (debugOp)
			std::cout << "LD B, C" << std::endl;

		return LDReg(BC, BC.low, true);

	// LD B, D
	case 0x42:
		if (debugOp)
			std::cout << "LD B, D" << std::endl;

		return LDReg(BC, DE.high, true);

	// LD B, E
	case 0x43:
		if (debugOp)
			std::cout << "LD B, E" << std::endl;

		return LDReg(BC, DE.low, true);

	// LD B, H
	case 0x44:
		if (debugOp)
			std::cout << "LD B, H" << std::endl;

		return LDReg(BC, HL.high, true);

	// LD B, L
	case 0x45:
		if (debugOp)
			std::cout << "LD B, L" << std::endl;

		return LDReg(BC, HL.low, true);

	// LD B, (HL)
	case 0x46:
		if (debugOp)
			std::cout << "LD B, (HL)" << std::endl;

		return LDFromMem(BC, HL.value, true);

	// LD B, A
	case 0x47:
		if (debugOp)
			std::cout << "LD B, A" << std::endl;

		return LDReg(BC, AF.high, true);

	// LD C, B
	case 0x48:
		if (debugOp)
			std::cout << "LD C, B" << std::endl;

		return LDReg(BC, BC.high, false);

	// LD C, C
	case 0x49:
		if (debugOp)
			std::cout << "LD C, C" << std::endl;

		return LDReg(BC, BC.low, false);

	// LD C, D
	case 0x4A:
		if (debugOp)
			std::cout << "LD C, D" << std::endl;

		return LDReg(BC, DE.high, false);

	// LD C, E
	case 0x4B:
		if (debugOp)
			std::cout << "LD C, E" << std::endl;

		return LDReg(BC, DE.low, false);

	// LD C, H
	case 0x4C:
		if (debugOp)
			std::cout << "LD C, H" << std::endl;

		return LDReg(BC, HL.high, false);

	// LD C, L
	case 0x4D:
		if (debugOp)
			std::cout << "LD C, L" << std::endl;

		return LDReg(BC, HL.low, false);

	// LD C, (HL)
	case 0x4E:
		if (debugOp)
			std::cout << "LD C, (HL)" << std::endl;

		return LDFromMem(BC, HL.value, false);

	// LD C, A
	case 0x4F:
		if (debugOp)
			std::cout << "LD C, A" << std::endl;

		return LDReg(BC, AF.high, false);

	// LD D, B
	case 0x50:
		if (debugOp)
			std::cout << "LD D, B" << std::endl;

		return LDReg(DE, BC.high, true);

	// LD D, C
	case 0x51:
		if (debugOp)
			std::cout << "LD D, C" << std::endl;

		return LDReg(DE, BC.low, true);

	// LD D, D
	case 0x52:
		if (debugOp)
			std::cout << "LD D, D" << std::endl;

		return LDReg(DE, DE.high, true);

	// LD D, E
	case 0x53:
		if (debugOp)
			std::cout << "LD D, E" << std::endl;

		return LDReg(DE, DE.low, true);

	// LD D, H
	case 0x54:
		if (debugOp)
			std::cout << "LD D, H" << std::endl;

		return LDReg(DE, HL.high, true);

	// LD D, L
	case 0x55:
		if (debugOp)
			std::cout << "LD D, L" << std::endl;

		return LDReg(DE, HL.low, true);

	// LD D, (HL)
	case 0x56:
		if (debugOp)
			std::cout << "LD D, (HL)" << std::endl;

		return LDFromMem(DE, HL.value, true);

	// LD D, A
	case 0x57:
		if (debugOp)
			std::cout << "LD D, A" << std::endl;

		return LDReg(DE, AF.high, true);

	// LD E, B
	case 0x58:
		if (debugOp)
			std::cout << "LD E, B" << std::endl;

		return LDReg(DE, BC.high, false);

	// LD E, C
	case 0x59:
		if (debugOp)
			std::cout << "LD E, C" << std::endl;

		return LDReg(DE, BC.low, false);

	// LD E, D
	case 0x5A:
		if (debugOp)
			std::cout << "LD E, D" << std::endl;

		return LDReg(DE, DE.high, false);

	// LD E, E
	case 0x5B:
		if (debugOp)
			std::cout << "LD E, E" << std::endl;

		return LDReg(DE, DE.low, false);

	// LD E, H
	case 0x5C:
		if (debugOp)
			std::cout << "LD E, H" << std::endl;

		return LDReg(DE, HL.high, false);

	// LD E, L
	case 0x5D:
		if (debugOp)
			std::cout << "LD E, L" << std::endl;

		return LDReg(DE, HL.low, false);

	// LD E, (HL)
	case 0x5E:
		if (debugOp)
			std::cout << "LD E, (HL)" << std::endl;

		return LDFromMem(DE, HL.value, false);

	// LD E, A
	case 0x5F:
		if (debugOp)
			std::cout << "LD E, A" << std::endl;

		return LDReg(DE, AF.high, false);

	// LD H, B
	case 0x60:
		if (debugOp)
			std::cout << "LD H, B" << std::endl;

		return LDReg(HL, BC.high, true);

	// LD H, C
	case 0x61:
		if (debugOp)
			std::cout << "LD H, C" << std::endl;

		return LDReg(HL, BC.low, true);

	// LD H, D
	case 0x62:
		if (debugOp)
			std::cout << "LD H, D" << std::endl;

		return LDReg(HL, DE.high, true);

	// LD H, E
	case 0x63:
		if (debugOp)
			std::cout << "LD H, E" << std::endl;

		return LDReg(HL, DE.low, true);

	// LD H, H
	case 0x64:
		if (debugOp)
			std::cout << "LD H, H" << std::endl;

		return LDReg(HL, HL.high, true);

	// LD H, L
	case 0x65:
		if (debugOp)
			std::cout << "LD H, L" << std::endl;

		return LDReg(HL, HL.low, true);

	// LD H, (HL)
	case 0x66:
		if (debugOp)
			std::cout << "LD H, (HL)" << std::endl;

		return LDFromMem(HL, HL.value, true);

	// LD H, A
	case 0x67:
		if (debugOp)
			std::cout << "LD H, A" << std::endl;

		return LDReg(HL, AF.high, true);

	// LD L, B
	case 0x68:
		if (debugOp)
			std::cout << "LD L, B" << std::endl;

		return LDReg(HL, BC.high, false);

	// LD L, C
	case 0x69:
		if (debugOp)
			std::cout << "LD L, C" << std::endl;

		return LDReg(HL, BC.low, false);

	// LD L, D
	case 0x6A:
		if (debugOp)
			std::cout << "LD L, D" << std::endl;

		return LDReg(HL, DE.high, false);

	// LD L, E
	case 0x6B:
		if (debugOp)
			std::cout << "LD L, E" << std::endl;

		return LDReg(HL, DE.low, false);

	// LD L, H
	case 0x6C:
		if (debugOp)
			std::cout << "LD L, H" << std::endl;

		return LDReg(HL, HL.high, false);

	// LD L, L
	case 0x6D:
		if (debugOp)
			std::cout << "LD L, L" << std::endl;

		return LDReg(HL, HL.low, false);

	// LD L, (HL)
	case 0x6E:
		if (debugOp)
			std::cout << "LD L, (HL)" << std::endl;

		return LDFromMem(HL, HL.value, false);

	// LD L, A
	case 0x6F:
		if (debugOp)
			std::cout << "LD L, A" << std::endl;

		return LDReg(HL, AF.high, false);

	// LD (HL), B
	case 0x70:
		if (debugOp)
			std::cout << "LD (HL), B" << std::endl;

		return LDToMem(HL.value, BC.high);

	// LD (HL), C
	case 0x71:
		if (debugOp)
			std::cout << "LD (HL), C" << std::endl;

		return LDToMem(HL.value, BC.low);

	// LD (HL), D
	case 0x72:
		if (debugOp)
			std::cout << "LD (HL), D" << std::endl;

		return LDToMem(HL.value, DE.high);

	// LD (HL), E
	case 0x73:
		if (debugOp)
			std::cout << "LD (HL), E" << std::endl;

		return LDToMem(HL.value, DE.low);

	// LD (HL), H
	case 0x74:
		if (debugOp)
			std::cout << "LD (HL), H" << std::endl;

		return LDToMem(HL.value, HL.high);

	// LD (HL), L
	case 0x75:
		if (debugOp)
			std::cout << "LD (HL), L" << std::endl;

		return LDToMem(HL.value, HL.low);

	// HALT
	case 0x76:
		if (debugOp)
			std::cout << "HALT" << std::endl;

		halted = true;
		return 1;

	// LD (HL), A
	case 0x77:
		if (debugOp)
			std::cout << "LD (HL), A" << std::endl;

		return LDToMem(HL.value, AF.high);

	// LD A, B
	case 0x78:
		if (debugOp)
			std::cout << "LD A, B" << std::endl;

		return LDReg(AF, BC.high, true);

	// LD A, C
	case 0x79:
		if (debugOp)
			std::cout << "LD A, C" << std::endl;

		return LDReg(AF, BC.low, true);

	// LD A, D
	case 0x7A:
		if (debugOp)
			std::cout << "LD A, D" << std::endl;

		return LDReg(AF, DE.high, true);

	// LD A, E
	case 0x7B:
		if (debugOp)
			std::cout << "LD A, E" << std::endl;

		return LDReg(AF, DE.low, true);

	// LD A, H
	case 0x7C:
		if (debugOp)
			std::cout << "LD A, H" << std::endl;

		return LDReg(AF, HL.high, true);

	// LD A, L
	case 0x7D:
		if (debugOp)
			std::cout << "LD A, L" << std::endl;

		return LDReg(AF, HL.low, true);

	// LD A, (HL)
	case 0x7E:
		if (debugOp)
			std::cout << "LD A, (HL)" << std::endl;

		return LDFromMem(AF, HL.value, true);

	// LD A, A
	case 0x7F:
		if (debugOp)
			std::cout << "LD A, A" << std::endl;

		return LDReg(AF, AF.high, true);

	// ADD A, B
	case 0x80:
		if (debugOp)
			std::cout << "ADD A, B" << std::endl;

		return AddA(BC.high, false);

	// ADD A, C
	case 0x81:
		if (debugOp)
			std::cout << "ADD A, C" << std::endl;

		return AddA(BC.low, false);

	// ADD A, D
	case 0x82:
		if (debugOp)
			std::cout << "ADD A, D" << std::endl;

		return AddA(DE.high, false);

	// ADD A, E
	case 0x83:
		if (debugOp)
			std::cout << "ADD A, E" << std::endl;

		return AddA(DE.low, false);

	// ADD A, H
	case 0x84:
		if (debugOp)
			std::cout << "ADD A, H" << std::endl;

		return AddA(HL.high, false);

	// ADD A, L
	case 0x85:
		if (debugOp)
			std::cout << "ADD A, L" << std::endl;

		return AddA(HL.low, false);

	// ADD A, (HL)
	case 0x86:
		if (debugOp)
			std::cout << "ADD A, (HL)" << std::endl;

		return AddAHL(false);

	// ADD A, A
	case 0x87:
		if (debugOp)
			std::cout << "ADD A, A" << std::endl;

		return AddA(AF.high, false);

	// ADC A, B
	case 0x88:
		if (debugOp)
			std::cout << "ADC A, B" << std::endl;

		return AddA(BC.high, true);

	// ADC A, C
	case 0x89:
		if (debugOp)
			std::cout << "ADC A, C" << std::endl;

		return AddA(BC.low, true);

	// ADC A, D
	case 0x8A:
		if (debugOp)
			std::cout << "ADC A, D" << std::endl;

		return AddA(DE.high, true);

	// ADC A, E
	case 0x8B:
		if (debugOp)
			std::cout << "ADC A, E" << std::endl;

		return AddA(DE.low, true);

	// ADC A, H
	case 0x8C:
		if (debugOp)
			std::cout << "ADC A, H" << std::endl;

		return AddA(HL.high, true);

	// ADC A, L
	case 0x8D:
		if (debugOp)
			std::cout << "ADC A, L" << std::endl;

		return AddA(HL.low, true);

	// ADC A, (HL)
	case 0x8E:
		if (debugOp)
			std::cout << "ADC A, (HL)" << std::endl;

		return AddAHL(true);

	// ADC A, A
	case 0x8F:
		if (debugOp)
			std::cout << "ADC A, A" << std::endl;

		return AddA(AF.high, true);

	// SUB A, B
	case 0x90:
		if (debugOp)
			std::cout << "SUB A, B" << std::endl;

		return SubA(BC.high, false);

	// SUB A, C
	case 0x91:
		if (debugOp)
			std::cout << "SUB A, C" << std::endl;

		return SubA(BC.low, false);

	// SUB A, D
	case 0x92:
		if (debugOp)
			std::cout << "SUB A, D" << std::endl;

		return SubA(DE.high, false);

	// SUB A, E
	case 0x93:
		if (debugOp)
			std::cout << "SUB A, E" << std::endl;

		return SubA(DE.low, false);

	// SUB A, H
	case 0x94:
		if (debugOp)
			std::cout << "SUB A, H" << std::endl;

		return SubA(HL.high, false);

	// SUB A, L
	case 0x95:
		if (debugOp)
			std::cout << "SUB A, L" << std::endl;

		return SubA(HL.low, false);

	// SUB A, (HL)
	case 0x96:
		if (debugOp)
			std::cout << "SUB A, (HL)" << std::endl;

		return SubAHL(false);

	// SUB A, A
	case 0x97:
		if (debugOp)
			std::cout << "SUB A, A" << std::endl;

		return SubA(AF.high, false);

	// SBC A, B
	case 0x98:
		if (debugOp)
			std::cout << "SBC A, B" << std::endl;

		return SubA(BC.high, true);

	// SBC A, C
	case 0x99:
		if (debugOp)
			std::cout << "SBC A, C" << std::endl;

		return SubA(BC.low, true);

	// SBC A, D
	case 0x9A:
		if (debugOp)
			std::cout << "SBC A, D" << std::endl;

		return SubA(DE.high, true);

	// SBC A, E
	case 0x9B:
		if (debugOp)
			std::cout << "SBC A, E" << std::endl;

		return SubA(DE.low, true);

	// SBC A, H
	case 0x9C:
		if (debugOp)
			std::cout << "SBC A, H" << std::endl;

		return SubA(HL.high, true);

	// SBC A, L
	case 0x9D:
		if (debugOp)
			std::cout << "SBC A, L" << std::endl;

		return SubA(HL.low, true);

	// SBC A, (HL)
	case 0x9E:
		if (debugOp)
			std::cout << "SBC A, (HL)" << std::endl;

		return SubAHL(true);

	// SBC A, A
	case 0x9F:
		if (debugOp)
			std::cout << "SBC A, A" << std::endl;

		return SubA(AF.high, true);

	// AND A, B
	case 0xA0:
		if (debugOp)
			std::cout << "AND A, B" << std::endl;

		return ANDA(BC.high);

	// AND A, C
	case 0xA1:
		if (debugOp)
			std::cout << "AND A, C" << std::endl;

		return ANDA(BC.low);

	// AND A, D
	case 0xA2:
		if (debugOp)
			std::cout << "AND A, D" << std::endl;

		return ANDA(DE.high);

	// AND A, E
	case 0xA3:
		if (debugOp)
			std::cout << "AND A, E" << std::endl;

		return ANDA(DE.low);

	// AND A, H
	case 0xA4:
		if (debugOp)
			std::cout << "AND A, H" << std::endl;

		return ANDA(HL.high);

	// AND A, L
	case 0xA5:
		if (debugOp)
			std::cout << "AND A, L" << std::endl;

		return ANDA(HL.low);

	// AND A, (HL)
	case 0xA6:
		if (debugOp)
			std::cout << "AND A, (HL)" << std::endl;

		return ANDAHL();

	// AND A, A
	case 0xA7:
		if (debugOp)
			std::cout << "AND A, A" << std::endl;

		return ANDA(AF.high);

	// XOR A, B
	case 0xA8:
		if (debugOp)
			std::cout << "XOR A, B" << std::endl;

		return XORA(BC.high);

	// XOR A, C
	case 0xA9:
		if (debugOp)
			std::cout << "XOR A, C" << std::endl;

		return XORA(BC.low);

	// XOR A, D
	case 0xAA:
		if (debugOp)
			std::cout << "XOR A, D" << std::endl;

		return XORA(DE.high);

	// XOR A, E
	case 0xAB:
		if (debugOp)
			std::cout << "XOR A, E" << std::endl;

		return XORA(DE.low);

	// XOR A, H
	case 0xAC:
		if (debugOp)
			std::cout << "XOR A, H" << std::endl;

		return XORA(HL.high);

	// XOR A, L
	case 0xAD:
		if (debugOp)
			std::cout << "XOR A, L" << std::endl;

		return XORA(HL.low);

	// XOR A, (HL)
	case 0xAE:
		if (debugOp)
			std::cout << "XOR A, (HL)" << std::endl;

		return XORAHL();

	// XOR A, A
	case 0xAF:
		if (debugOp)
			std::cout << "XOR A, A" << std::endl;

		return XORA(AF.high);

	// OR A, B
	case 0xB0:
		if (debugOp)
			std::cout << "OR A, B" << std::endl;

		return ORA(BC.high);

	// OR A, C
	case 0xB1:
		if (debugOp)
			std::cout << "OR A, C" << std::endl;

		return ORA(BC.low);

	// OR A, D
	case 0xB2:
		if (debugOp)
			std::cout << "OR A, D" << std::endl;

		return ORA(DE.high);

	// OR A, E
	case 0xB3:
		if (debugOp)
			std::cout << "OR A, E" << std::endl;

		return ORA(DE.low);

	// OR A, H
	case 0xB4:
		if (debugOp)
			std::cout << "OR A, H" << std::endl;

		return ORA(HL.high);

	// OR A, L
	case 0xB5:
		if (debugOp)
			std::cout << "OR A, L" << std::endl;

		return ORA(HL.low);

	// OR A, (HL)
	case 0xB6:
		if (debugOp)
			std::cout << "OR A, (HL)" << std::endl;

		return ORAHL();

	// OR A, A
	case 0xB7:
		if (debugOp)
			std::cout << "OR A, A" << std::endl;

		return ORA(AF.high);

	// CP A, B
	case 0xB8:
		if (debugOp)
			std::cout << "CP A, B" << std::endl;

		return CPA(BC.high);

	// CP A, C
	case 0xB9:
		if (debugOp)
			std::cout << "CP A, C" << std::endl;

		return CPA(BC.low);

	// CP A, D
	case 0xBA:
		if (debugOp)
			std::cout << "CP A, D" << std::endl;

		return CPA(DE.high);

	// CP A, E
	case 0xBB:
		if (debugOp)
			std::cout << "CP A, E" << std::endl;

		return CPA(DE.low);

	// CP A, H
	case 0xBC:
		if (debugOp)
			std::cout << "CP A, H" << std::endl;

		return CPA(HL.high);

	// CP A, L
	case 0xBD:
		if (debugOp)
			std::cout << "CP A, L" << std::endl;

		return CPA(HL.low);

	// CP A, (HL)
	case 0xBE:
		if (debugOp)
			std::cout << "CP A, (HL)" << std::endl;

		return CPAHL();

	// CP A, A
	case 0xBF:
		if (debugOp)
			std::cout << "CP A, A" << std::endl;

		return CPA(AF.high);

	// RET NZ
	case 0xC0:
		if (debugOp)
			std::cout << "RET NZ" << std::endl;

		return RetCond(ZFlag, false);

	// POP BC
	case 0xC1:
		if (debugOp)
			std::cout << "POP BC" << std::endl;

		return Pop(BC);

	// JP NZ, u16
	case 0xC2:
		if (debugOp)
			std::cout << "JP NZ, u16" << std::endl;

		return JPCond(ZFlag, false);

	// JP u16
	case 0xC3:
		if (debugOp)
			std::cout << "JP u16" << std::endl;

		return JP();

	// CALL NZ, u16
	case 0xC4:
		if (debugOp)
			std::cout << "CALL NZ, u16" << std::endl;

		return CallCond(ZFlag, false);

	// PUSH BC
	case 0xC5:
		if (debugOp)
			std::cout << "PUSH BC" << std::endl;

		return Push(BC);

	// ADD A, u8
	case 0xC6:
		if (debugOp)
			std::cout << "ADD A, u8" << std::endl;

		return AddAFromMem(false);

	// RST 00
	case 0xC7:
		if (debugOp)
			std::cout << "RST 00" << std::endl;

		return Rst();

	// RET Z
	case 0xC8:
		if (debugOp)
			std::cout << "RET Z" << std::endl;

		return RetCond(ZFlag, true);

	// RET
	case 0xC9:
		if (debugOp)
			std::cout << "RET" << std::endl;

		return Ret();

	// JP Z, u16
	case 0xCA:
		if (debugOp)
			std::cout << "JP Z, u16" << std::endl;

		return JPCond(ZFlag, true);

	// Extended Opcode
	case 0xCB:
		if (debugOp)
			std::cout << "Extended opcode" << std::endl;

		return ExecuteExt();

	// CALL Z, u16
	case 0xCC:
		if (debugOp)
			std::cout << "CALL Z, u16" << std::endl;

		return CallCond(ZFlag, true);

	// CALL u16
	case 0xCD:
		if (debugOp)
			std::cout << "CALL u16" << std::endl;

		return Call();

	// ADC A, u8
	case 0xCE:
		if (debugOp)
			std::cout << "ADC A, u8" << std::endl;

		return AddAFromMem(true);

	// RST 08
	case 0xCF:
		if (debugOp)
			std::cout << "RST 08" << std::endl;

		return Rst();

	// RET NC
	case 0xD0:
		if (debugOp)
			std::cout << "RET NC" << std::endl;

		return RetCond(CFlag, false);

	// POP DE
	case 0xD1:
		if (debugOp)
			std::cout << "POP DE" << std::endl;

		return Pop(DE);

	// JP NC, u16
	case 0xD2:
		if (debugOp)
			std::cout << "JP NC, u16" << std::endl;

		return JPCond(CFlag, false);

	// NOTHING
	case 0xD3:
		return 1;

	// CALL NC, u16
	case 0xD4:
		if (debugOp)
			std::cout << "CALL NC, u16" << std::endl;

		return CallCond(CFlag, false);

	// PUSH DE
	case 0xD5:
		if (debugOp)
			std::cout << "PUSH DE" << std::endl;

		return Push(DE);

	// SUB A, u8
	case 0xD6:
		if (debugOp)
			std::cout << "SUB A, u8" << std::endl;

		return SubAFromMem(false);

	// RST 10
	case 0xD7:
		if (debugOp)
			std::cout << "RST 10" << std::endl;

		return Rst();

	// RET C
	case 0xD8:
		if (debugOp)
			std::cout << "RET C" << std::endl;

		return RetCond(CFlag, true);

	// RETI
	case 0xD9:
		if (debugOp)
			std::cout << "RETI" << std::endl;

		return Reti();

	// JP C, u16
	case 0xDA:
		if (debugOp)
			std::cout << "JP C, u16" << std::endl;

		return JPCond(CFlag, true);

	// NOTHING
	case 0xDB:
		return 1;

	// CALL C, u16
	case 0xDC:
		if (debugOp)
			std::cout << "CALL C, u16" << std::endl;

		return CallCond(CFlag, true);

	// NOTHING
	case 0xDD:
		return 1;

	// SBC A, u8
	case 0xDE:
		if (debugOp)
			std::cout << "SBC A, u8" << std::endl;

		return SubAFromMem(true);

	// RST 18
	case 0xDF:
		if (debugOp)
			std::cout << "RST 18" << std::endl;

		return Rst();

	// LD (0xFF00 + u8), A
	case 0xE0:
		if (debugOp)
			std::cout << "LD (0xFF00 + u8), A" << std::endl;

		return LDHToMem(false);

	// POP HL
	case 0xE1:
		if (debugOp)
			std::cout << "POP HL" << std::endl;

		return Pop(HL);

	// LD (0xFF00 + C), A
	case 0xE2:
		if (debugOp)
			std::cout << "LD (0xFF00 + C), A" << std::endl;

		return LDHToMem(true);

	// NOTHING
	case 0xE3:
	case 0xE4:
		return 1;

	// PUSH HL
	case 0xE5:
		if (debugOp)
			std::cout << "PUSH HL" << std::endl;

		return Push(HL);

	// AND A, u8
	case 0xE6:
		if (debugOp)
			std::cout << "AND A, u8" << std::endl;

		return ANDAFromMem();

	// RST 20
	case 0xE7:
		if (debugOp)
			std::cout << "RST 20" << std::endl;

		return Rst();

	// ADD SP, i8
	case 0xE8:
		if (debugOp)
			std::cout << "ADD SP, i8" << std::endl;

		return AddSP();

	// JP HL
	case 0xE9:
		if (debugOp)
			std::cout << "JP HL" << std::endl;

		return JPHL();

	// LD (u16), A
	case 0xEA:
		if (debugOp)
			std::cout << "LD (u16), A" << std::endl;

		return LDToMemW();

	// NOTHING
	case 0xEB:
	case 0xEC:
	case 0xED:
		return 1;

	// XOR A, u8
	case 0xEE:
		if (debugOp)
			std::cout << "XOR A, u8" << std::endl;

		return XORAFromMem();

	// RST 28
	case 0xEF:
		if (debugOp)
			std::cout << "RST 28" << std::endl;

		return Rst();

	// LD A, (0xFF00 + u8)
	case 0xF0:
		if (debugOp)
			std::cout << "LD A, (0xFF00 + u8)" << std::endl;

		return LDHFromMem(false);

	// POP AF
	case 0xF1:
		if (debugOp)
			std::cout << "POP AF" << std::endl;

		return Pop(AF);

	// LD A, (0xFF00 + C)
	case 0xF2:
		if (debugOp)
			std::cout << "LD A, (0xFF00 + C)" << std::endl;

		return LDHFromMem(true);

	// DI
	case 0xF3:
		if (debugOp)
			std::cout << "DI" << std::endl;

		return DI();

	// NOTHING
	case 0xF4:
		return 1;

	// PUSH AF
	case 0xF5:
		if (debugOp)
			std::cout << "PUSH AF" << std::endl;

		return Push(AF);

	// OR A, u8
	case 0xF6:
		if (debugOp)
			std::cout << "OR A, u8" << std::endl;

		return ORAFromMem();

	// RST 30
	case 0xF7:
		if (debugOp)
			std::cout << "RST 30" << std::endl;

		return Rst();

	// LD HL, SP + i8
	case 0xF8:
		if (debugOp)
			std::cout << "LD HL, SP + i8" << std::endl;

		return LDHLSP();

	// LD SP, HL
	case 0xF9:
		if (debugOp)
			std::cout << "LD SP, HL" << std::endl;

		return LDHL();

	// LD A, (u16)
	case 0xFA:
		if (debugOp)
			std::cout << "LD A, (u16)" << std::endl;

		return LDFromMemW();

	// EI
	case 0xFB:
		if (debugOp)
			std::cout << "EI" << std::endl;

		return EI();

	// Nothing
	case 0xFC:
	case 0xFD:
		return 1;

	// CP A, u8
	case 0xFE:
		if (debugOp)
			std::cout << "CP A, u8" << std::endl;

		return CPAFromMem();

	// RST 38
	case 0xFF:
		if (debugOp)
			std::cout << "RST 38" << std::endl;

		return Rst();
	}
	return 0;
}

int CPUSharp::ExecuteExt() {
	opcode = Read(PC.value++);
	switch (opcode) {
	// RLC B
	case 0x00:
		if (debugOp)
			std::cout << "RLC B" << std::endl;

		return RotateReg(BC, true, true, true);

	// RLC C
	case 0x01:
		if (debugOp)
			std::cout << "RLC C" << std::endl;

		return RotateReg(BC, false, true, true);

	// RLC D
	case 0x02:
		if (debugOp)
			std::cout << "RLC D" << std::endl;

		return RotateReg(DE, true, true, true);

	// RLC E
	case 0x03:
		if (debugOp)
			std::cout << "RLC E" << std::endl;

		return RotateReg(DE, false, true, true);

	// RLC H
	case 0x04:
		if (debugOp)
			std::cout << "RLC H" << std::endl;

		return RotateReg(HL, true, true, true);

	// RLC L
	case 0x05:
		if (debugOp)
			std::cout << "RLC L" << std::endl;

		return RotateReg(HL, false, true, true);

	// RLC (HL)
	case 0x06:
		if (debugOp)
			std::cout << "RLC (HL)" << std::endl;

		return RotateMem(true, true);

	// RLC A
	case 0x07:
		if (debugOp)
			std::cout << "RLC A" << std::endl;

		return RotateReg(AF, true, true, true);

	// RRC B
	case 0x08:
		if (debugOp)
			std::cout << "RRC B" << std::endl;

		return RotateReg(BC, true, false, true);

	// RRC C
	case 0x09:
		if (debugOp)
			std::cout << "RRC C" << std::endl;

		return RotateReg(BC, false, false, true);

	// RRC D
	case 0x0A:
		if (debugOp)
			std::cout << "RRC D" << std::endl;

		return RotateReg(DE, true, false, true);

	// RRC E
	case 0x0B:
		if (debugOp)
			std::cout << "RRC E" << std::endl;

		return RotateReg(DE, false, false, true);

	// RRC H
	case 0x0C:
		if (debugOp)
			std::cout << "RRC H" << std::endl;

		return RotateReg(HL, true, false, true);

	// RRC L
	case 0x0D:
		if (debugOp)
			std::cout << "RRC L" << std::endl;

		return RotateReg(HL, false, false, true);

	// RRC (HL)
	case 0x0E:
		if (debugOp)
			std::cout << "RRC (HL)" << std::endl;

		return RotateMem(false, true);

	// RRC A
	case 0x0F:
		if (debugOp)
			std::cout << "RRC A" << std::endl;

		return RotateReg(AF, true, false, true);

	// RL B
	case 0x10:
		if (debugOp)
			std::cout << "RL B" << std::endl;

		return RotateReg(BC, true, true, false);

	// RL C
	case 0x11:
		if (debugOp)
			std::cout << "RL C" << std::endl;

		return RotateReg(BC, false, true, false);

	// RL D
	case 0x12:
		if (debugOp)
			std::cout << "RL D" << std::endl;

		return RotateReg(DE, true, true, false);

	// RL E
	case 0x13:
		if (debugOp)
			std::cout << "RL E" << std::endl;

		return RotateReg(DE, false, true, false);

	// RL H
	case 0x14:
		if (debugOp)
			std::cout << "RL H" << std::endl;

		return RotateReg(HL, true, true, false);

	// RL L
	case 0x15:
		if (debugOp)
			std::cout << "RL L" << std::endl;

		return RotateReg(HL, false, true, false);

	// RL (HL)
	case 0x16:
		if (debugOp)
			std::cout << "RL (HL)" << std::endl;

		return RotateMem(true, false);

	// RL A
	case 0x17:
		if (debugOp)
			std::cout << "RL A" << std::endl;

		return RotateReg(AF, true, true, false);

	// RR B
	case 0x18:
		if (debugOp)
			std::cout << "RR B" << std::endl;

		return RotateReg(BC, true, false, false);

	// RR C
	case 0x19:
		if (debugOp)
			std::cout << "RR C" << std::endl;

		return RotateReg(BC, false, false, false);

	// RR D
	case 0x1A:
		if (debugOp)
			std::cout << "RR D" << std::endl;

		return RotateReg(DE, true, false, false);

	// RR E
	case 0x1B:
		if (debugOp)
			std::cout << "RR E" << std::endl;

		return RotateReg(DE, false, false, false);

	// RR H
	case 0x1C:
		if (debugOp)
			std::cout << "RR H" << std::endl;

		return RotateReg(HL, true, false, false);

	// RR L
	case 0x1D:
		if (debugOp)
			std::cout << "RR L" << std::endl;

		return RotateReg(HL, false, false, false);

	// RR (HL)
	case 0x1E:
		if (debugOp)
			std::cout << "RR (HL)" << std::endl;

		return RotateMem(false, false);

	// RR A
	case 0x1F:
		if (debugOp)
			std::cout << "RR A" << std::endl;

		return RotateReg(AF, true, false, false);

	// SLA B
	case 0x20:
		if (debugOp)
			std::cout << "SLA B" << std::endl;

		return Shift(BC, true, true, false);

	// SLA C
	case 0x21:
		if (debugOp)
			std::cout << "SLA C" << std::endl;

		return Shift(BC, false, true, false);

	// SLA D
	case 0x22:
		if (debugOp)
			std::cout << "SLA D" << std::endl;

		return Shift(DE, true, true, false);

	// SLA E
	case 0x23:
		if (debugOp)
			std::cout << "SLA E" << std::endl;

		return Shift(DE, false, true, false);

	// SLA H
	case 0x24:
		if (debugOp)
			std::cout << "SLA H" << std::endl;

		return Shift(HL, true, true, false);

	// SLA L
	case 0x25:
		if (debugOp)
			std::cout << "SLA L" << std::endl;

		return Shift(HL, false, true, false);

	// SLA (HL)
	case 0x26:
		if (debugOp)
			std::cout << "SLA (HL)" << std::endl;

		return ShiftMem(true, false);

	// SLA A
	case 0x27:
		if (debugOp)
			std::cout << "SLA A" << std::endl;

		return Shift(AF, true, true, false);

	// SRA B
	case 0x28:
		if (debugOp)
			std::cout << "SRA B" << std::endl;

		return Shift(BC, true, false, true);

	// SRA C
	case 0x29:
		if (debugOp)
			std::cout << "SRA C" << std::endl;

		return Shift(BC, false, false, true);

	// SRA D
	case 0x2A:
		if (debugOp)
			std::cout << "SRA D" << std::endl;

		return Shift(DE, true, false, true);

	// SRA E
	case 0x2B:
		if (debugOp)
			std::cout << "SRA E" << std::endl;

		return Shift(DE, false, false, true);

	// SRA H
	case 0x2C:
		if (debugOp)
			std::cout << "SRA H" << std::endl;

		return Shift(HL, true, false, true);

	// SRA L
	case 0x2D:
		if (debugOp)
			std::cout << "SRA L" << std::endl;

		return Shift(HL, false, false, true);

	// SRA (HL)
	case 0x2E:
		if (debugOp)
			std::cout << "SRA (HL)" << std::endl;

		return ShiftMem(false, true);

	// SRA A
	case 0x2F:
		if (debugOp)
			std::cout << "SRA A" << std::endl;

		return Shift(AF, true, false, true);

	// SWAP B
	case 0x30:
		if (debugOp)
			std::cout << "SWAP B" << std::endl;

		return Swap(BC, true);

	// SWAP C
	case 0x31:
		if (debugOp)
			std::cout << "SWAP C" << std::endl;

		return Swap(BC, false);

	// SWAP D
	case 0x32:
		if (debugOp)
			std::cout << "SWAP D" << std::endl;

		return Swap(DE, true);

	// SWAP E
	case 0x33:
		if (debugOp)
			std::cout << "SWAP E" << std::endl;

		return Swap(DE, false);

	// SWAP H
	case 0x34:
		if (debugOp)
			std::cout << "SWAP H" << std::endl;

		return Swap(HL, true);

	// SWAP L
	case 0x35:
		if (debugOp)
			std::cout << "SWAP L" << std::endl;

		return Swap(HL, false);

	// SWAP (HL)
	case 0x36:
		if (debugOp)
			std::cout << "SWAP (HL)" << std::endl;

		return SwapMem();

	// SWAP A
	case 0x37:
		if (debugOp)
			std::cout << "SWAP A" << std::endl;

		return Swap(AF, true);

	// SRL B
	case 0x38:
		if (debugOp)
			std::cout << "SRL B" << std::endl;

		return Shift(BC, true, false, false);

	// SRL C
	case 0x39:
		if (debugOp)
			std::cout << "SRL C" << std::endl;

		return Shift(BC, false, false, false);

	// SRL D
	case 0x3A:
		if (debugOp)
			std::cout << "SRL D" << std::endl;

		return Shift(DE, true, false, false);

	// SRL E
	case 0x3B:
		if (debugOp)
			std::cout << "SRL E" << std::endl;

		return Shift(DE, false, false, false);

	// SRL H
	case 0x3C:
		if (debugOp)
			std::cout << "SRL H" << std::endl;

		return Shift(HL, true, false, false);

	// SRL L
	case 0x3D:
		if (debugOp)
			std::cout << "SRL L" << std::endl;

		return Shift(HL, false, false, false);

	// SRL (HL)
	case 0x3E:
		if (debugOp)
			std::cout << "SRL (HL)" << std::endl;

		return ShiftMem(false, false);

	// SRL A
	case 0x3F:
		if (debugOp)
			std::cout << "SRL A" << std::endl;

		return Shift(AF, true, false, false);

	// BIT 0, B
	case 0x40:
		if (debugOp)
			std::cout << "BIT 0, B" << std::endl;

		return Bit(BC, true, 0);

	// BIT 0, C
	case 0x41:
		if (debugOp)
			std::cout << "BIT 0, C" << std::endl;

		return Bit(BC, false, 0);

	// BIT 0, D
	case 0x42:
		if (debugOp)
			std::cout << "BIT 0, D" << std::endl;

		return Bit(DE, true, 0);

	// BIT 0, E
	case 0x43:
		if (debugOp)
			std::cout << "BIT 0, E" << std::endl;

		return Bit(DE, false, 0);

	// BIT 0, H
	case 0x44:
		if (debugOp)
			std::cout << "BIT 0, H" << std::endl;

		return Bit(HL, true, 0);

	// BIT 0, L
	case 0x45:
		if (debugOp)
			std::cout << "BIT 0, L" << std::endl;

		return Bit(HL, false, 0);

	// BIT 0, (HL)
	case 0x46:
		if (debugOp)
			std::cout << "BIT 0, (HL)" << std::endl;

		return BitMem(0);

	// BIT 0, A
	case 0x47:
		if (debugOp)
			std::cout << "BIT 0, A" << std::endl;

		return Bit(AF, true, 0);

	// BIT 1, B
	case 0x48:
		if (debugOp)
			std::cout << "BIT 1, B" << std::endl;

		return Bit(BC, true, 1);

	// BIT 1, C
	case 0x49:
		if (debugOp)
			std::cout << "BIT 1, C" << std::endl;

		return Bit(BC, false, 1);

	// BIT 1, D
	case 0x4A:
		if (debugOp)
			std::cout << "BIT 1, D" << std::endl;

		return Bit(DE, true, 1);

	// BIT 1, E
	case 0x4B:
		if (debugOp)
			std::cout << "BIT 1, E" << std::endl;

		return Bit(DE, false, 1);

	// BIT 1, H
	case 0x4C:
		if (debugOp)
			std::cout << "BIT 1, H" << std::endl;

		return Bit(HL, true, 1);

	// BIT 1, L
	case 0x4D:
		if (debugOp)
			std::cout << "BIT 1, L" << std::endl;

		return Bit(HL, false, 1);

	// BIT 1, (HL)
	case 0x4E:
		if (debugOp)
			std::cout << "BIT 1, (HL)" << std::endl;

		return BitMem(1);

	// BIT 1, A
	case 0x4F:
		if (debugOp)
			std::cout << "BIT 1, A" << std::endl;

		return Bit(AF, true, 1);

	// BIT 2, B
	case 0x50:
		if (debugOp)
			std::cout << "BIT 2, B" << std::endl;

		return Bit(BC, true, 2);

	// BIT 2, C
	case 0x51:
		if (debugOp)
			std::cout << "BIT 2, C" << std::endl;

		return Bit(BC, false, 2);

	// BIT 2, D
	case 0x52:
		if (debugOp)
			std::cout << "BIT 2, D" << std::endl;

		return Bit(DE, true, 2);

	// BIT 2, E
	case 0x53:
		if (debugOp)
			std::cout << "BIT 2, E" << std::endl;

		return Bit(DE, false, 2);

	// BIT 2, H
	case 0x54:
		if (debugOp)
			std::cout << "BIT 2, H" << std::endl;

		return Bit(HL, true, 2);

	// BIT 2, L
	case 0x55:
		if (debugOp)
			std::cout << "BIT 2, L" << std::endl;

		return Bit(HL, false, 2);

	// BIT 2, (HL)
	case 0x56:
		if (debugOp)
			std::cout << "BIT 2, (HL)" << std::endl;

		return BitMem(2);

	// BIT 2, A
	case 0x57:
		if (debugOp)
			std::cout << "BIT 2, A" << std::endl;

		return Bit(AF, true, 2);

	// BIT 3, B
	case 0x58:
		if (debugOp)
			std::cout << "BIT 3, B" << std::endl;

		return Bit(BC, true, 3);

	// BIT 3, C
	case 0x59:
		if (debugOp)
			std::cout << "BIT 3, C" << std::endl;

		return Bit(BC, false, 3);

	// BIT 3, D
	case 0x5A:
		if (debugOp)
			std::cout << "BIT 3, D" << std::endl;

		return Bit(DE, true, 3);

	// BIT 3, E
	case 0x5B:
		if (debugOp)
			std::cout << "BIT 3, E" << std::endl;

		return Bit(DE, false, 3);

	// BIT 3, H
	case 0x5C:
		if (debugOp)
			std::cout << "BIT 3, H" << std::endl;

		return Bit(HL, true, 3);

	// BIT 3, L
	case 0x5D:
		if (debugOp)
			std::cout << "BIT 3, L" << std::endl;

		return Bit(HL, false, 3);

	// BIT 3, (HL)
	case 0x5E:
		if (debugOp)
			std::cout << "BIT 3, (HL)" << std::endl;

		return BitMem(3);

	// BIT 3, A
	case 0x5F:
		if (debugOp)
			std::cout << "BIT 3, A" << std::endl;

		return Bit(AF, true, 3);

	// BIT 4, B
	case 0x60:
		if (debugOp)
			std::cout << "BIT 4, B" << std::endl;

		return Bit(BC, true, 4);

	// BIT 4, C
	case 0x61:
		if (debugOp)
			std::cout << "BIT 4, C" << std::endl;

		return Bit(BC, false, 4);

	// BIT 4, D
	case 0x62:
		if (debugOp)
			std::cout << "BIT 4, D" << std::endl;

		return Bit(DE, true, 4);

	// BIT 4, E
	case 0x63:
		if (debugOp)
			std::cout << "BIT 4, E" << std::endl;

		return Bit(DE, false, 4);

	// BIT 4, H
	case 0x64:
		if (debugOp)
			std::cout << "BIT 4, H" << std::endl;

		return Bit(HL, true, 4);

	// BIT 4, L
	case 0x65:
		if (debugOp)
			std::cout << "BIT 4, L" << std::endl;

		return Bit(HL, false, 4);

	// BIT 4, (HL)
	case 0x66:
		if (debugOp)
			std::cout << "BIT 4, (HL)" << std::endl;

		return BitMem(4);

	// BIT 4, A
	case 0x67:
		if (debugOp)
			std::cout << "BIT 4, A" << std::endl;

		return Bit(AF, true, 4);

	// BIT 5, B
	case 0x68:
		if (debugOp)
			std::cout << "BIT 5, B" << std::endl;

		return Bit(BC, true, 5);

	// BIT 5, C
	case 0x69:
		if (debugOp)
			std::cout << "BIT 5, C" << std::endl;

		return Bit(BC, false, 5);

	// BIT 5, D
	case 0x6A:
		if (debugOp)
			std::cout << "BIT 5, D" << std::endl;

		return Bit(DE, true, 5);

	// BIT 5, E
	case 0x6B:
		if (debugOp)
			std::cout << "BIT 5, E" << std::endl;

		return Bit(DE, false, 5);

	// BIT 5, H
	case 0x6C:
		if (debugOp)
			std::cout << "BIT 5, H" << std::endl;

		return Bit(HL, true, 5);

	// BIT 5, L
	case 0x6D:
		if (debugOp)
			std::cout << "BIT 5, L" << std::endl;

		return Bit(HL, false, 5);

	// BIT 5, (HL)
	case 0x6E:
		if (debugOp)
			std::cout << "BIT 5, (HL)" << std::endl;

		return BitMem(5);

	// BIT 5, A
	case 0x6F:
		if (debugOp)
			std::cout << "BIT 5, A" << std::endl;

		return Bit(AF, true, 5);

	// BIT 6, B
	case 0x70:
		if (debugOp)
			std::cout << "BIT 6, B" << std::endl;

		return Bit(BC, true, 6);

	// BIT 6, C
	case 0x71:
		if (debugOp)
			std::cout << "BIT 6, C" << std::endl;

		return Bit(BC, false, 6);

	// BIT 6, D
	case 0x72:
		if (debugOp)
			std::cout << "BIT 6, D" << std::endl;

		return Bit(DE, true, 6);

	// BIT 6, E
	case 0x73:
		if (debugOp)
			std::cout << "BIT 6, E" << std::endl;

		return Bit(DE, false, 6);

	// BIT 6, H
	case 0x74:
		if (debugOp)
			std::cout << "BIT 6, H" << std::endl;

		return Bit(HL, true, 6);

	// BIT 6, L
	case 0x75:
		if (debugOp)
			std::cout << "BIT 6, L" << std::endl;

		return Bit(HL, false, 6);

	// BIT 6, (HL)
	case 0x76:
		if (debugOp)
			std::cout << "BIT 6, (HL)" << std::endl;

		return BitMem(6);

	// BIT 6, A
	case 0x77:
		if (debugOp)
			std::cout << "BIT 6, A" << std::endl;

		return Bit(AF, true, 6);

	// BIT 7, B
	case 0x78:
		if (debugOp)
			std::cout << "BIT 7, B" << std::endl;

		return Bit(BC, true, 7);

	// BIT 7, C
	case 0x79:
		if (debugOp)
			std::cout << "BIT 7, C" << std::endl;

		return Bit(BC, false, 7);

	// BIT 7, D
	case 0x7A:
		if (debugOp)
			std::cout << "BIT 7, D" << std::endl;

		return Bit(DE, true, 7);

	// BIT 7, E
	case 0x7B:
		if (debugOp)
			std::cout << "BIT 7, E" << std::endl;

		return Bit(DE, false, 7);

	// BIT 7, H
	case 0x7C:
		if (debugOp)
			std::cout << "BIT 7, H" << std::endl;

		return Bit(HL, true, 7);

	// BIT 7, L
	case 0x7D:
		if (debugOp)
			std::cout << "BIT 7, L" << std::endl;

		return Bit(HL, false, 7);

	// BIT 7, (HL)
	case 0x7E:
		if (debugOp)
			std::cout << "BIT 7, (HL)" << std::endl;

		return BitMem(7);

	// BIT 7, A
	case 0x7F:
		if (debugOp)
			std::cout << "BIT 7, A" << std::endl;

		return Bit(AF, true, 7);

	// RES 0, B
	case 0x80:
		if (debugOp)
			std::cout << "RES 0, B" << std::endl;

		return Res(BC, true, 0);

	// RES 0, C
	case 0x81:
		if (debugOp)
			std::cout << "RES 0, C" << std::endl;

		return Res(BC, false, 0);

	// RES 0, D
	case 0x82:
		if (debugOp)
			std::cout << "RES 0, D" << std::endl;

		return Res(DE, true, 0);

	// RES 0, E
	case 0x83:
		if (debugOp)
			std::cout << "RES 0, E" << std::endl;

		return Res(DE, false, 0);

	// RES 0, H
	case 0x84:
		if (debugOp)
			std::cout << "RES 0, H" << std::endl;

		return Res(HL, true, 0);

	// RES 0, L
	case 0x85:
		if (debugOp)
			std::cout << "RES 0, L" << std::endl;

		return Res(HL, false, 0);

	// RES 0, (HL)
	case 0x86:
		if (debugOp)
			std::cout << "RES 0, (HL)" << std::endl;

		return ResMem(0);

	// RES 0, A
	case 0x87:
		if (debugOp)
			std::cout << "RES 0, A" << std::endl;

		return Res(AF, true, 0);

	// RES 1, B
	case 0x88:
		if (debugOp)
			std::cout << "RES 1, B" << std::endl;

		return Res(BC, true, 1);

	// RES 1, C
	case 0x89:
		if (debugOp)
			std::cout << "RES 1, C" << std::endl;

		return Res(BC, false, 1);

	// RES 1, D
	case 0x8A:
		if (debugOp)
			std::cout << "RES 1, D" << std::endl;

		return Res(DE, true, 1);

	// RES 1, E
	case 0x8B:
		if (debugOp)
			std::cout << "RES 1, E" << std::endl;

		return Res(DE, false, 1);

	// RES 1, H
	case 0x8C:
		if (debugOp)
			std::cout << "RES 1, H" << std::endl;

		return Res(HL, true, 1);

	// RES 1, L
	case 0x8D:
		if (debugOp)
			std::cout << "RES 1, L" << std::endl;

		return Res(HL, false, 1);

	// RES 1, (HL)
	case 0x8E:
		if (debugOp)
			std::cout << "RES 1, (HL)" << std::endl;

		return ResMem(1);

	// RES 1, A
	case 0x8F:
		if (debugOp)
			std::cout << "RES 1, A" << std::endl;

		return Res(AF, true, 1);

	// RES 2, B
	case 0x90:
		if (debugOp)
			std::cout << "RES 2, B" << std::endl;

		return Res(BC, true, 2);

	// RES 2, C
	case 0x91:
		if (debugOp)
			std::cout << "RES 2, C" << std::endl;

		return Res(BC, false, 2);

	// RES 2, D
	case 0x92:
		if (debugOp)
			std::cout << "RES 2, D" << std::endl;

		return Res(DE, true, 2);

	// RES 2, E
	case 0x93:
		if (debugOp)
			std::cout << "RES 2, E" << std::endl;

		return Res(DE, false, 2);

	// RES 2, H
	case 0x94:
		if (debugOp)
			std::cout << "RES 2, H" << std::endl;

		return Res(HL, true, 2);

	// RES 2, L
	case 0x95:
		if (debugOp)
			std::cout << "RES 2, L" << std::endl;

		return Res(HL, false, 2);

	// RES 2, (HL)
	case 0x96:
		if (debugOp)
			std::cout << "RES 2, (HL)" << std::endl;

		return ResMem(2);

	// RES 2, A
	case 0x97:
		if (debugOp)
			std::cout << "RES 2, A" << std::endl;

		return Res(AF, true, 2);

	// RES 3, B
	case 0x98:
		if (debugOp)
			std::cout << "RES 3, B" << std::endl;

		return Res(BC, true, 3);

	// RES 3, C
	case 0x99:
		if (debugOp)
			std::cout << "RES 3, C" << std::endl;

		return Res(BC, false, 3);

	// RES 3, D
	case 0x9A:
		if (debugOp)
			std::cout << "RES 3, D" << std::endl;

		return Res(DE, true, 3);

	// RES 3, E
	case 0x9B:
		if (debugOp)
			std::cout << "RES 3, E" << std::endl;

		return Res(DE, false, 3);

	// RES 3, H
	case 0x9C:
		if (debugOp)
			std::cout << "RES 3, H" << std::endl;

		return Res(HL, true, 3);

	// RES 3, L
	case 0x9D:
		if (debugOp)
			std::cout << "RES 3, L" << std::endl;

		return Res(HL, false, 3);

	// RES 3, (HL)
	case 0x9E:
		if (debugOp)
			std::cout << "RES 3, (HL)" << std::endl;

		return ResMem(3);

	// RES 3, A
	case 0x9F:
		if (debugOp)
			std::cout << "RES 3, A" << std::endl;

		return Res(AF, true, 3);

	// RES 4, B
	case 0xA0:
		if (debugOp)
			std::cout << "RES 4, B" << std::endl;

		return Res(BC, true, 4);

	// RES 4, C
	case 0xA1:
		if (debugOp)
			std::cout << "RES 4, C" << std::endl;

		return Res(BC, false, 4);

	// RES 4, D
	case 0xA2:
		if (debugOp)
			std::cout << "RES 4, D" << std::endl;

		return Res(DE, true, 4);

	// RES 4, E
	case 0xA3:
		if (debugOp)
			std::cout << "RES 4, E" << std::endl;

		return Res(DE, false, 4);

	// RES 4, H
	case 0xA4:
		if (debugOp)
			std::cout << "RES 4, H" << std::endl;

		return Res(HL, true, 4);

	// RES 4, L
	case 0xA5:
		if (debugOp)
			std::cout << "RES 4, L" << std::endl;

		return Res(HL, false, 4);

	// RES 4, (HL)
	case 0xA6:
		if (debugOp)
			std::cout << "RES 4, (HL)" << std::endl;

		return ResMem(4);

	// RES 4, A
	case 0xA7:
		if (debugOp)
			std::cout << "RES 4, A" << std::endl;

		return Res(AF, true, 4);

	// RES 5, B
	case 0xA8:
		if (debugOp)
			std::cout << "RES 5, B" << std::endl;

		return Res(BC, true, 5);

	// RES 5, C
	case 0xA9:
		if (debugOp)
			std::cout << "RES 5, C" << std::endl;

		return Res(BC, false, 5);

	// RES 5, D
	case 0xAA:
		if (debugOp)
			std::cout << "RES 5, D" << std::endl;

		return Res(DE, true, 5);

	// RES 5, E
	case 0xAB:
		if (debugOp)
			std::cout << "RES 5, E" << std::endl;

		return Res(DE, false, 5);

	// RES 5, H
	case 0xAC:
		if (debugOp)
			std::cout << "RES 5, H" << std::endl;

		return Res(HL, true, 5);

	// RES 5, L
	case 0xAD:
		if (debugOp)
			std::cout << "RES 5, L" << std::endl;

		return Res(HL, false, 5);

	// RES 5, (HL)
	case 0xAE:
		if (debugOp)
			std::cout << "RES 5, (HL)" << std::endl;

		return ResMem(5);

	// RES 5, A
	case 0xAF:
		if (debugOp)
			std::cout << "RES 5, A" << std::endl;

		return Res(AF, true, 5);

	// RES 6, B
	case 0xB0:
		if (debugOp)
			std::cout << "RES 6, B" << std::endl;

		return Res(BC, true, 6);

	// RES 6, C
	case 0xB1:
		if (debugOp)
			std::cout << "RES 6, C" << std::endl;

		return Res(BC, false, 6);

	// RES 6, D
	case 0xB2:
		if (debugOp)
			std::cout << "RES 6, D" << std::endl;

		return Res(DE, true, 6);

	// RES 6, E
	case 0xB3:
		if (debugOp)
			std::cout << "RES 6, E" << std::endl;

		return Res(DE, false, 6);

	// RES 6, H
	case 0xB4:
		if (debugOp)
			std::cout << "RES 6, H" << std::endl;

		return Res(HL, true, 6);

	// RES 6, L
	case 0xB5:
		if (debugOp)
			std::cout << "RES 6, L" << std::endl;

		return Res(HL, false, 6);

	// RES 6, (HL)
	case 0xB6:
		if (debugOp)
			std::cout << "RES 6, (HL)" << std::endl;

		return ResMem(6);

	// RES 6, A
	case 0xB7:
		if (debugOp)
			std::cout << "RES 6, A" << std::endl;

		return Res(AF, true, 6);

	// RES 7, B
	case 0xB8:
		if (debugOp)
			std::cout << "RES 7, B" << std::endl;

		return Res(BC, true, 7);

	// RES 7, C
	case 0xB9:
		if (debugOp)
			std::cout << "RES 7, C" << std::endl;

		return Res(BC, false, 7);

	// RES 7, D
	case 0xBA:
		if (debugOp)
			std::cout << "RES 7, D" << std::endl;

		return Res(DE, true, 7);

	// RES 7, E
	case 0xBB:
		if (debugOp)
			std::cout << "RES 7, E" << std::endl;

		return Res(DE, false, 7);

	// RES 7, H
	case 0xBC:
		if (debugOp)
			std::cout << "RES 7, H" << std::endl;

		return Res(HL, true, 7);

	// RES 7, L
	case 0xBD:
		if (debugOp)
			std::cout << "RES 7, L" << std::endl;

		return Res(HL, false, 7);

	// RES 7, (HL)
	case 0xBE:
		if (debugOp)
			std::cout << "RES 7, (HL)" << std::endl;

		return ResMem(7);

	// RES 7, A
	case 0xBF:
		if (debugOp)
			std::cout << "RES 7, A" << std::endl;

		return Res(AF, true, 7);

	// SET 0, B
	case 0xC0:
		if (debugOp)
			std::cout << "SET 0, B" << std::endl;

		return Set(BC, true, 0);

	// SET 0, C
	case 0xC1:
		if (debugOp)
			std::cout << "SET 0, C" << std::endl;

		return Set(BC, false, 0);

	// SET 0, D
	case 0xC2:
		if (debugOp)
			std::cout << "SET 0, D" << std::endl;

		return Set(DE, true, 0);

	// SET 0, E
	case 0xC3:
		if (debugOp)
			std::cout << "SET 0, E" << std::endl;

		return Set(DE, false, 0);

	// SET 0, H
	case 0xC4:
		if (debugOp)
			std::cout << "SET 0, H" << std::endl;

		return Set(HL, true, 0);

	// SET 0, L
	case 0xC5:
		if (debugOp)
			std::cout << "SET 0, L" << std::endl;

		return Set(HL, false, 0);

	// SET 0, (HL)
	case 0xC6:
		if (debugOp)
			std::cout << "SET 0, (HL)" << std::endl;

		return SetMem(0);

	// SET 0, A
	case 0xC7:
		if (debugOp)
			std::cout << "SET 0, A" << std::endl;

		return Set(AF, true, 0);

	// SET 1, B
	case 0xC8:
		if (debugOp)
			std::cout << "SET 1, B" << std::endl;

		return Set(BC, true, 1);

	// SET 1, C
	case 0xC9:
		if (debugOp)
			std::cout << "SET 1, C" << std::endl;

		return Set(BC, false, 1);

	// SET 1, D
	case 0xCA:
		if (debugOp)
			std::cout << "SET 1, D" << std::endl;

		return Set(DE, true, 1);

	// SET 1, E
	case 0xCB:
		if (debugOp)
			std::cout << "SET 1, E" << std::endl;

		return Set(DE, false, 1);

	// SET 1, H
	case 0xCC:
		if (debugOp)
			std::cout << "SET 1, H" << std::endl;

		return Set(HL, true, 1);

	// SET 1, L
	case 0xCD:
		if (debugOp)
			std::cout << "SET 1, L" << std::endl;

		return Set(HL, false, 1);

	// SET 1, (HL)
	case 0xCE:
		if (debugOp)
			std::cout << "SET 1, (HL)" << std::endl;

		return SetMem(1);

	// SET 1, A
	case 0xCF:
		if (debugOp)
			std::cout << "SET 1, A" << std::endl;

		return Set(AF, true, 1);

	// SET 2, B
	case 0xD0:
		if (debugOp)
			std::cout << "SET 2, B" << std::endl;

		return Set(BC, true, 2);

	// SET 2, C
	case 0xD1:
		if (debugOp)
			std::cout << "SET 2, C" << std::endl;

		return Set(BC, false, 2);

	// SET 2, D
	case 0xD2:
		if (debugOp)
			std::cout << "SET 2, D" << std::endl;

		return Set(DE, true, 2);

	// SET 2, E
	case 0xD3:
		if (debugOp)
			std::cout << "SET 2, E" << std::endl;

		return Set(DE, false, 2);

	// SET 2, H
	case 0xD4:
		if (debugOp)
			std::cout << "SET 2, H" << std::endl;

		return Set(HL, true, 2);

	// SET 2, L
	case 0xD5:
		if (debugOp)
			std::cout << "SET 2, L" << std::endl;

		return Set(HL, false, 2);

	// SET 2, (HL)
	case 0xD6:
		if (debugOp)
			std::cout << "SET 2, (HL)" << std::endl;

		return SetMem(2);

	// SET 2, A
	case 0xD7:
		if (debugOp)
			std::cout << "SET 2, A" << std::endl;

		return Set(AF, true, 2);

	// SET 3, B
	case 0xD8:
		if (debugOp)
			std::cout << "SET 3, B" << std::endl;

		return Set(BC, true, 3);

	// SET 3, C
	case 0xD9:
		if (debugOp)
			std::cout << "SET 3, C" << std::endl;

		return Set(BC, false, 3);

	// SET 3, D
	case 0xDA:
		if (debugOp)
			std::cout << "SET 3, D" << std::endl;

		return Set(DE, true, 3);

	// SET 3, E
	case 0xDB:
		if (debugOp)
			std::cout << "SET 3, E" << std::endl;

		return Set(DE, false, 3);

	// SET 3, H
	case 0xDC:
		if (debugOp)
			std::cout << "SET 3, H" << std::endl;

		return Set(HL, true, 3);

	// SET 3, L
	case 0xDD:
		if (debugOp)
			std::cout << "SET 3, L" << std::endl;

		return Set(HL, false, 3);

	// SET 3, (HL)
	case 0xDE:
		if (debugOp)
			std::cout << "SET 3, (HL)" << std::endl;

		return SetMem(3);

	// SET 3, A
	case 0xDF:
		if (debugOp)
			std::cout << "SET 3, A" << std::endl;

		return Set(AF, true, 3);

	// SET 4, B
	case 0xE0:
		if (debugOp)
			std::cout << "SET 4, B" << std::endl;

		return Set(BC, true, 4);

	// SET 4, C
	case 0xE1:
		if (debugOp)
			std::cout << "SET 4, C" << std::endl;

		return Set(BC, false, 4);

	// SET 4, D
	case 0xE2:
		if (debugOp)
			std::cout << "SET 4, D" << std::endl;

		return Set(DE, true, 4);

	// SET 4, E
	case 0xE3:
		if (debugOp)
			std::cout << "SET 4, E" << std::endl;

		return Set(DE, false, 4);

	// SET 4, H
	case 0xE4:
		if (debugOp)
			std::cout << "SET 4, H" << std::endl;

		return Set(HL, true, 4);

	// SET 4, L
	case 0xE5:
		if (debugOp)
			std::cout << "SET 4, L" << std::endl;

		return Set(HL, false, 4);

	// SET 4, (HL)
	case 0xE6:
		if (debugOp)
			std::cout << "SET 4, (HL)" << std::endl;

		return SetMem(4);

	// SET 4, A
	case 0xE7:
		if (debugOp)
			std::cout << "SET 4, A" << std::endl;

		return Set(AF, true, 4);

	// SET 5, B
	case 0xE8:
		if (debugOp)
			std::cout << "SET 5, B" << std::endl;

		return Set(BC, true, 5);

	// SET 5, C
	case 0xE9:
		if (debugOp)
			std::cout << "SET 5, C" << std::endl;

		return Set(BC, false, 5);

	// SET 5, D
	case 0xEA:
		if (debugOp)
			std::cout << "SET 5, D" << std::endl;

		return Set(DE, true, 5);

	// SET 5, E
	case 0xEB:
		if (debugOp)
			std::cout << "SET 5, E" << std::endl;

		return Set(DE, false, 5);

	// SET 5, H
	case 0xEC:
		if (debugOp)
			std::cout << "SET 5, H" << std::endl;

		return Set(HL, true, 5);

	// SET 5, L
	case 0xED:
		if (debugOp)
			std::cout << "SET 5, L" << std::endl;

		return Set(HL, false, 5);

	// SET 5, (HL)
	case 0xEE:
		if (debugOp)
			std::cout << "SET 5, (HL)" << std::endl;

		return SetMem(5);

	// SET 5, A
	case 0xEF:
		if (debugOp)
			std::cout << "SET 5, A" << std::endl;

		return Set(AF, true, 5);

	// SET 6, B
	case 0xF0:
		if (debugOp)
			std::cout << "SET 6, B" << std::endl;

		return Set(BC, true, 6);

	// SET 6, C
	case 0xF1:
		if (debugOp)
			std::cout << "SET 6, C" << std::endl;

		return Set(BC, false, 6);

	// SET 6, D
	case 0xF2:
		if (debugOp)
			std::cout << "SET 6, D" << std::endl;

		return Set(DE, true, 6);

	// SET 6, E
	case 0xF3:
		if (debugOp)
			std::cout << "SET 6, E" << std::endl;

		return Set(DE, false, 6);

	// SET 6, H
	case 0xF4:
		if (debugOp)
			std::cout << "SET 6, H" << std::endl;

		return Set(HL, true, 6);

	// SET 6, L
	case 0xF5:
		if (debugOp)
			std::cout << "SET 6, L" << std::endl;

		return Set(HL, false, 6);

	// SET 6, (HL)
	case 0xF6:
		if (debugOp)
			std::cout << "SET 6, (HL)" << std::endl;

		return SetMem(6);

	// SET 6, A
	case 0xF7:
		if (debugOp)
			std::cout << "SET 6, A" << std::endl;

		return Set(AF, true, 6);

	// SET 7, B
	case 0xF8:
		if (debugOp)
			std::cout << "SET 7, B" << std::endl;

		return Set(BC, true, 7);

	// SET 7, C
	case 0xF9:
		if (debugOp)
			std::cout << "SET 7, C" << std::endl;

		return Set(BC, false, 7);

	// SET 7, D
	case 0xFA:
		if (debugOp)
			std::cout << "SET 7, D" << std::endl;

		return Set(DE, true, 7);

	// SET 7, E
	case 0xFB:
		if (debugOp)
			std::cout << "SET 7, E" << std::endl;

		return Set(DE, false, 7);

	// SET 7, H
	case 0xFC:
		if (debugOp)
			std::cout << "SET 7, E" << std::endl;

		return Set(HL, true, 7);

	// SET 7, L
	case 0xFD:
		if (debugOp)
			std::cout << "SET 7, L" << std::endl;

		return Set(HL, false, 7);

	// SET 7, (HL)
	case 0xFE:
		if (debugOp)
			std::cout << "SET 7, (HL)" << std::endl;

		return SetMem(7);

	// SET 7, A
	case 0xFF:
		if (debugOp)
			std::cout << "SET 7, A" << std::endl;

		return Set(AF, true, 7);
	}
	return 0;
}