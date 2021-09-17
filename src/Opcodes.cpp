#include "CPUSharp.h"
#include "Memory.h"

#pragma region LOAD
int CPUSharp::LDReg(Register& r1, uint8_t r2, bool isHigh) {
	if (isHigh)
		r1.high = r2;
	else
		r1.low = r2;
	return 1;
}

int CPUSharp::LDFromMem(Register& r, uint16_t add, bool isHigh) {
	if(isHigh)
		r.high = Read(add);
	else
		r.low = Read(add);
	return 2;
}

int CPUSharp::LDToMem(uint16_t add, uint8_t r) {
	Write(add, r);
	return 2;
}

int CPUSharp::LDFromMemToMem(uint16_t add) {
	Write(add, Read(PC.value++));
	return 3;
}

int CPUSharp::LDToMemW() {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	Write(add, AF.high);
	return 4;
}

int CPUSharp::LDFromMemW() {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	AF.high = Read(add);
	return 4;
}

int CPUSharp::LDHToMem(bool isC) {
	if (isC) {
		Write(0xFF00 + BC.low, AF.high);
		return 2;
	}
	else {
		Write(0xFF00 + Read(PC.value++), AF.high);
		return 3;
	}
}

int CPUSharp::LDHFromMem(bool isC) {
	if (isC) {
		AF.high = Read(0xFF00 + BC.low);
		return 2;
	}
	else {
		AF.high = Read(0xFF00 + Read(PC.value++));
		return 3;
	}
}

int CPUSharp::LDWToR(Register& r) {
	uint16_t n = Read(PC.value++);
	n |= Read(PC.value++) << 8;
	r.value = n;
	return 3;
}

int CPUSharp::LDSP() {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	Write(add, SP.low);
	Write(add + 1, SP.high);
	return 5;
}

int CPUSharp::LDHL() {
	SP.value = HL.value;
	return 2;
}

int CPUSharp::Pop(Register& r) {
	r.low = Read(SP.value++);
	r.high = Read(SP.value++);
	AF.low &= 0xF0;
	return 3;
}

int CPUSharp::Push(Register r) {
	Write(--SP.value, r.high);
	Write(--SP.value, r.low);
	return 4;
}

int CPUSharp::LDHLSP() {
	int8_t n = (int8_t)Read(PC.value++);

	HL.value = SP.value + n;

	if (SP.low > HL.low) {
		AF.low |= CFlag;
	}
	else {
		AF.low &= ~CFlag;
	}

	if ((((SP.low & 0x0F) + (n & 0x0F)) & 0x10) == 0x10) {
		AF.low |= HFlag;
	}
	else {
		AF.low &= ~HFlag;
	}

	AF.low &= ~SFlag;
	AF.low &= ~ZFlag;

	return 3;
}
#pragma endregion

#pragma region JUMPS
int CPUSharp::JP() {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	PC.value = add;
	return 4;
}

int CPUSharp::JPHL() {
	PC.value = HL.value;
	return 1;
}

int CPUSharp::JPCond(uint8_t flag, bool isTrue) {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	if (isTrue) {
		if ((AF.low & flag) != 0) {
			PC.value = add;
			return 4;
		}
	}
	else {
		if ((AF.low & flag) == 0) {
			PC.value = add;
			return 4;
		}
	}
	return 3;
}

int CPUSharp::JR() {
	PC.value += (int8_t)Read(PC.value++);
	return 3;
}

int CPUSharp::JRCond(uint8_t flag, bool isTrue) {
	int8_t e = (int8_t)Read(PC.value++);
	if (isTrue) {
		if ((AF.low & flag) != 0) {
			PC.value += e;
			return 3;
		}
	}
	else {
		if ((AF.low & flag) == 0) {
			PC.value += e;
			return 3;
		}
	}
	return 2;
}

int CPUSharp::Call() {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	Write(--SP.value, PC.high);
	Write(--SP.value, PC.low);
	PC.value = add;
	return 6;
}

int CPUSharp::CallCond(uint8_t flag, bool isTrue) {
	uint16_t add = Read(PC.value++);
	add |= Read(PC.value++) << 8;
	if (isTrue) {
		if ((AF.low & flag) != 0) {
			Write(--SP.value, PC.high);
			Write(--SP.value, PC.low);
			PC.value = add;
			return 6;
		}
	}
	else {
		if ((AF.low & flag) == 0) {
			Write(--SP.value, PC.high);
			Write(--SP.value, PC.low);
			PC.value = add;
			return 6;
		}
	}
	return 3;
}

int CPUSharp::Ret() {
	uint16_t add = Read(SP.value++);
	add |= Read(SP.value++) << 8;
	PC.value = add;
	ret = false;
	if (interrupt) {
		interrupt = false;
	}
	return 4;
}

int CPUSharp::RetCond(uint8_t flag, bool isTrue) {
	if (isTrue) {
		if ((AF.low & flag) != 0) {
			uint16_t add = Read(SP.value++);
			add |= Read(SP.value++) << 8;
			PC.value = add;
			return 5;
		}
	}
	else {
		if ((AF.low & flag) == 0) {
			uint16_t add = Read(SP.value++);
			add |= Read(SP.value++) << 8;
			PC.value = add;
			return 5;
		}
	}
	return 2;
}

int CPUSharp::Reti() {
	uint16_t add = Read(SP.value++);
	add |= Read(SP.value++) << 8;
	PC.value = add;
	IME = true;
	if (Vblank) {
		//std::cout << "finish vblank" << std::endl;
		//std::cout << memory->ppu.mode << std::endl << std::endl;
		Vblank = false;
	}
	if (stat) {
		//std::cout << "finish stat" << std::endl;
		//std::cout << memory->ppu.mode << std::endl << std::endl;
		stat = false;
	}
	return 4;
}

int CPUSharp::Rst() {
	uint8_t n = opcode - 0xC7;
	Write(--SP.value, PC.high);
	Write(--SP.value, PC.low);
	PC.value = n;
	ret = true;
	return 4;
}

int CPUSharp::DI() {
	IME = false;
	IMESch = false;
	return 1;
}

int CPUSharp::EI() {
	IMESch = true;
	return 1;
}

int CPUSharp::CCF() {
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low ^= CFlag;
	return 1;
}

int CPUSharp::SCF() {
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low |= CFlag;
	return 1;
}

int CPUSharp::DAA() {
	int16_t a = AF.high;

	AF.value &= ~(0xFF00 | (AF.low & ZFlag));

	if (AF.low & SFlag) {
		if (AF.low & HFlag) {
			a = (a - 0x06) & 0xFF;
		}

		if (AF.low & CFlag) {
			a -= 0x60;
		}
	}
	else {
		if ((AF.low & HFlag) || (a & 0x0F) > 0x09) {
			a += 0x06;
		}

		if ((AF.low & CFlag) || a > 0x9F) {
			a += 0x60;
		}
	}

	if ((a & 0xFF) == 0) {
		AF.low |= ZFlag;
	}

	if ((a & 0x100) == 0x100) {
		AF.low |= CFlag;
	}

	AF.low &= ~HFlag;

	AF.high = a;
	return 1;
}

int CPUSharp::CPL() {
	AF.high ^= 0xFF;
	AF.low |= SFlag;
	AF.low |= HFlag;
	return 1;
}
#pragma endregion

#pragma region ARIT
int CPUSharp::IncReg(Register& reg) {
	reg.value++;
	return 2;
}

int CPUSharp::DecReg(Register& reg) {
	reg.value--;
	return 2;
}

int CPUSharp::AddHL(uint16_t val) {
	uint16_t prevVal = HL.value;
	uint8_t prevLow = HL.low;

	HL.value += val;
	
	if (prevVal > HL.value) {
		AF.low |= CFlag;
	}
	else {
		AF.low &= ~CFlag;
	}

	if ((((prevVal & 0xFFF) + (val & 0xFFF)) & 0x1000) == 0x1000) {
		AF.low |= HFlag;
	}
	else {
		AF.low &= ~HFlag;
	}

	AF.low &= ~SFlag;

	return 2;
}

int CPUSharp::AddSP() {
	int8_t n = Read(PC.value++);

	uint16_t prevVal = SP.value;
	uint8_t prevLow = SP.low;

	SP.value += n;

	if (prevLow > SP.low) {
		AF.low |= CFlag;
	}
	else {
		AF.low &= ~CFlag;
	}

	if ((((prevLow & 0x0F) + (n & 0x0F)) & 0x10) == 0x10) {
		AF.low |= HFlag;
	}
	else {
		AF.low &= ~HFlag;
	}

	AF.low &= ~SFlag;
	AF.low &= ~ZFlag;

	return 4;
}

int CPUSharp::AddA(uint8_t val, bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	AF.high += val + C;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA > AF.high || (val + C) > 0xFF) {
		AF.low |= CFlag;
	}

	if ((((prevA & 0xF) + (val & 0xF) + C) & 0x10) == 0x10) {
		AF.low |= HFlag;
	}

	return 1;
}

int CPUSharp::AddAFromMem(bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	uint8_t n = Read(PC.value++);
	AF.high += n + C;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA > AF.high || (n + C) > 0xFF) {
		AF.low |= CFlag;
	}

	if ((((prevA & 0xF) + (n & 0xF) + C) & 0x10) == 0x10) {
		AF.low |= HFlag;
	}

	return 2;
}

int CPUSharp::AddAHL(bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	uint8_t val = Read(HL.value);
	AF.high += val + C;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA > AF.high || (val + C) > 0xFF) {
		AF.low |= CFlag;
	}

	if ((((prevA & 0xF) + (val & 0xF) + C) & 0x10) == 0x10) {
		AF.low |= HFlag;
	}

	return 2;
}

int CPUSharp::SubA(uint8_t val, bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	AF.high -= val + C;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA < (val + C)) {
		AF.low |= CFlag;
	}

	if (((prevA & 0x0F) - (val & 0x0F) - C) < 0) {
		AF.low |= HFlag;
	}

	return 1;
}

int CPUSharp::SubAFromMem(bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	uint8_t n = Read(PC.value++);
	AF.high -= n + C;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA < (n + C)) {
		AF.low |= CFlag;
	}

	if (((prevA & 0x0F) - (n & 0x0F) - C) < 0) {
		AF.low |= HFlag;
	}

	return 2;
}

int CPUSharp::SubAHL(bool withC) {
	uint8_t prevA = AF.high;

	uint8_t C = 0;
	if (withC) {
		C = ((AF.low & CFlag) != 0) ? 1 : 0;
	}

	uint8_t val = Read(HL.value);
	AF.high -= val + C;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	if (prevA < (val + C)) {
		AF.low |= CFlag;
	}

	if (((prevA & 0x0F) - (val & 0x0F) - C) < 0) {
		AF.low |= HFlag;
	}

	return 2;
}

int CPUSharp::INC(Register& reg, bool isHigh) {
	uint8_t prev;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;

	if (isHigh) {
		prev = reg.high;
		reg.high++;

		if (reg.high == 0) {
			AF.low |= ZFlag;
		}

		if ((prev & 0xf) > (reg.high & 0xf)) {
			AF.low |= HFlag;
		}
	}
	else {
		prev = reg.low;
		reg.low++;

		if (reg.low == 0) {
			AF.low |= ZFlag;
		}

		if ((prev & 0xf) > (reg.low & 0xf)) {
			AF.low |= HFlag;
		}
	}

	return 1;
}

int CPUSharp::INCHL() {
	uint8_t prev = Read(HL.value);

	uint8_t val = prev + 1;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;

	if (val == 0) {
		AF.low |= ZFlag;
	}

	if ((prev & 0xf) > (val & 0xf)) {
		AF.low |= HFlag;
	}

	Write(HL.value, val);

	return 3;
}

int CPUSharp::DEC(Register& reg, bool isHigh) {
	uint8_t prev;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;

	if (isHigh) {
		prev = reg.high;
		reg.high--;

		if (reg.high == 0) {
			AF.low |= ZFlag;
		}

		if ((prev & 0xf) < (reg.high & 0xf)) {
			AF.low |= HFlag;
		}
	}
	else {
		prev = reg.low;
		reg.low--;

		if (reg.low == 0) {
			AF.low |= ZFlag;
		}

		if ((prev & 0xf) < (reg.low & 0xf)) {
			AF.low |= HFlag;
		}
	}

	return 1;
}

int CPUSharp::DECHL() {
	uint8_t prev = Read(HL.value);

	uint8_t val = prev - 1;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;

	if (val == 0) {
		AF.low |= ZFlag;
	}

	if ((prev & 0xf) < (val & 0xf)) {
		AF.low |= HFlag;
	}

	Write(HL.value, val);

	return 3;
}
#pragma endregion

#pragma region LOGI
int CPUSharp::ANDA(uint8_t val) {
	AF.high &= val;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low |= HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 1;
}

int CPUSharp::ANDAHL() {
	AF.high &= Read(HL.value);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low |= HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::ANDAFromMem() {
	AF.high &= Read(PC.value++);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low |= HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::XORA(uint8_t val) {
	AF.high ^= val;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 1;
}

int CPUSharp::XORAHL() {
	AF.high ^= Read(HL.value);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::XORAFromMem() {
	AF.high ^= Read(PC.value++);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::ORA(uint8_t val) {
	AF.high |= val;

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 1;
}

int CPUSharp::ORAHL() {
	AF.high |= Read(HL.value);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::ORAFromMem() {
	AF.high |= Read(PC.value++);

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (AF.high == 0) {
		AF.low |= ZFlag;
	}

	return 2;
}

int CPUSharp::CPA(uint8_t val) {
	uint8_t cp = AF.high - val;

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (cp == 0) {
		AF.low |= ZFlag;
	}

	if (cp > AF.high) {
		AF.low |= CFlag;
	}

	if ((cp & 0xf) > (AF.high & 0xf)) {
		AF.low |= HFlag;
	}

	return 1;
}

int CPUSharp::CPAHL() {
	uint8_t cp = AF.high - Read(HL.value);

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (cp == 0) {
		AF.low |= ZFlag;
	}

	if (cp > AF.high) {
		AF.low |= CFlag;
	}

	if ((cp & 0xf) > (AF.high & 0xf)) {
		AF.low |= HFlag;
	}

	return 2;
}

int CPUSharp::CPAFromMem() {
	uint8_t cp = AF.high - Read(PC.value++);

	AF.low &= ~ZFlag;
	AF.low |= SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	if (cp == 0) {
		AF.low |= ZFlag;
	}

	if (cp > AF.high) {
		AF.low |= CFlag;
	}

	if ((cp & 0xf) > (AF.high & 0xf)) {
		AF.low |= HFlag;
	}

	return 2;
}
#pragma endregion

#pragma region BIT
int CPUSharp::RotateA(bool isLeft, bool withCarry) {
	if (isLeft) {
		bool HB = (AF.high & 0x80) == 0x80;
		AF.high = AF.high << 1;
		if (withCarry) {
			if (HB) {
				AF.low |= CFlag;
				AF.high |= 0x1;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
		else {
			AF.high |= (((AF.low & CFlag) != 0) ? 0x1 : 0);
			if (HB) {
				AF.low |= CFlag;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
	}
	else {
		bool HB = (AF.high & 0x1) == 0x1;
		AF.high = AF.high >> 1;
		if (withCarry) {
			if (HB) {
				AF.low |= CFlag;
				AF.high |= 0x80;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
		else {
			AF.high |= (((AF.low & CFlag) != 0) ? 0x80 : 0);
			if (HB) {
				AF.low |= CFlag;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
	}

	AF.low &= ~ZFlag;
	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	return 1;
}

int CPUSharp::RotateReg(Register& reg, bool isHigh, bool isLeft, bool withCarry) {
	if (isLeft) {
		if (isHigh) {
			bool HB = (reg.high & 0x80) == 0x80;
			reg.high = reg.high << 1;
			if (withCarry) {
				if (HB) {
					AF.low |= CFlag;
					reg.high |= 0x1;
				}
				else {
					AF.low &= ~CFlag;
				}
			}
			else {
				reg.high |= (((AF.low & CFlag) != 0) ? 0x1 : 0);
				if (HB) {
					AF.low |= CFlag;
				}
				else {
					AF.low &= ~CFlag;
				}
			}

			AF.low &= ~ZFlag;
			if (reg.high == 0)
				AF.low |= ZFlag;

		}
		else {
			bool HB = (reg.low & 0x80) == 0x80;
			reg.low = reg.low << 1;
			if (withCarry) {
				if (HB) {
					AF.low |= CFlag;
					reg.low |= 0x1;
				}
				else {
					AF.low &= ~CFlag;
				}
			}
			else {
				reg.low |= (((AF.low & CFlag) != 0) ? 0x1 : 0);
				if (HB) {
					AF.low |= CFlag;
				}
				else {
					AF.low &= ~CFlag;
				}
			}

			AF.low &= ~ZFlag;
			if (reg.low == 0)
				AF.low |= ZFlag;
		}
	}
	else {
		if (isHigh) {
			bool HB = (reg.high & 0x1) == 0x1;
			reg.high = reg.high >> 1;
			if (withCarry) {
				if (HB) {
					AF.low |= CFlag;
					reg.high |= 0x80;
				}
				else {
					AF.low &= ~CFlag;
				}
			}
			else {
				reg.high |= (((AF.low & CFlag) != 0) ? 0x80 : 0);
				if (HB) {
					AF.low |= CFlag;
				}
				else {
					AF.low &= ~CFlag;
				}
			}

			AF.low &= ~ZFlag;
			if (reg.high == 0)
				AF.low |= ZFlag;

		}
		else {
			bool HB = (reg.low & 0x1) == 0x1;
			reg.low = reg.low >> 1;
			if (withCarry) {
				if (HB) {
					AF.low |= CFlag;
					reg.low |= 0x80;
				}
				else {
					AF.low &= ~CFlag;
				}
			}
			else {
				reg.low |= (((AF.low & CFlag) != 0) ? 0x80 : 0);
				if (HB) {
					AF.low |= CFlag;
				}
				else {
					AF.low &= ~CFlag;
				}
			}

			AF.low &= ~ZFlag;
			if (reg.low == 0)
				AF.low |= ZFlag;

		}
	}

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	return 2;
}

int CPUSharp::RotateMem(bool isLeft, bool withCarry) {
	uint8_t value = Read(HL.value);
	if (isLeft) {
		bool HB = (value & 0x80) == 0x80;
		value = value << 1;
		if (withCarry) {
			if (HB) {
				AF.low |= CFlag;
				value |= 0x1;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
		else {
			value |= (((AF.low & CFlag) != 0) ? 0x1 : 0);
			if (HB) {
				AF.low |= CFlag;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
	}
	else {
		bool HB = (value & 0x1) == 0x1;
		value = value >> 1;
		if (withCarry) {
			if (HB) {
				AF.low |= CFlag;
				value |= 0x80;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
		else {
			value |= (((AF.low & CFlag) != 0) ? 0x80 : 0);
			if (HB) {
				AF.low |= CFlag;
			}
			else {
				AF.low &= ~CFlag;
			}
		}
	}

	AF.low &= ~ZFlag;
	if (value == 0)
		AF.low |= ZFlag;

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;

	Write(HL.value, value);

	return 4;
}

int CPUSharp::Shift(Register& reg, bool isHigh, bool isLeft, bool retain) {
	if (isLeft) {
		if (isHigh) {
			if((reg.high & 0x80) != 0)
				AF.low |= CFlag;
			else
				AF.low &= ~CFlag;

			reg.high = reg.high << 1;

			AF.low &= ~ZFlag;
			if(reg.high == 0)
				AF.low |= ZFlag;
		}
		else {
			if ((reg.low & 0x80) != 0)
				AF.low |= CFlag;
			else
				AF.low &= ~CFlag;

			reg.low = reg.low << 1;

			AF.low &= ~ZFlag;
			if (reg.low == 0)
				AF.low |= ZFlag;
		}
	}
	else {
		if (isHigh) {
			if ((reg.high & 0x01) != 0)
				AF.low |= CFlag;
			else
				AF.low &= ~CFlag;
			if (retain) {
				reg.high = (reg.high >> 1) | (reg.high & 0x80);
			}
			else {
				reg.high = reg.high >> 1;
			}

			AF.low &= ~ZFlag;
			if (reg.high == 0)
				AF.low |= ZFlag;
		}
		else {
			if ((reg.low & 0x01) != 0)
				AF.low |= CFlag;
			else
				AF.low &= ~CFlag;
			if (retain) {
				reg.low = (reg.low >> 1) | (reg.low & 0x80);
			}
			else {
				reg.low = reg.low >> 1;
			}

			AF.low &= ~ZFlag;
			if (reg.low == 0)
				AF.low |= ZFlag;
		}
	}

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;

	return 2;
}

int CPUSharp::ShiftMem(bool isLeft, bool retain) {
	uint8_t value = Read(HL.value);
	if (isLeft) {
		if ((value & 0x80) != 0)
			AF.low |= CFlag;
		else
			AF.low &= ~CFlag;

		value = value << 1;

		AF.low &= ~ZFlag;
		if (value == 0)
			AF.low |= ZFlag;
	}
	else {
		if ((value & 0x01) != 0)
			AF.low |= CFlag;
		else
			AF.low &= ~CFlag;
		if (retain) {
			value = (value >> 1) | (value & 0x80);
		}
		else {
			value = value >> 1;
		}

		AF.low &= ~ZFlag;
		if (value == 0)
			AF.low |= ZFlag;
	}

	Write(HL.value, value);

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;

	return 4;
}

int CPUSharp::Swap(Register& reg, bool isHigh) {
	if (isHigh) {
		reg.high = (reg.high << 4) | (reg.high >> 4);

		AF.low &= ~ZFlag;
		if(reg.high == 0)
			AF.low |= ZFlag;
	}
	else {
		reg.low = (reg.low << 4) | (reg.low >> 4);

		AF.low &= ~ZFlag;
		if (reg.low == 0)
			AF.low |= ZFlag;
	}

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	return 2;
}

int CPUSharp::SwapMem() {
	uint8_t value = Read(HL.value);
	value = (value << 4) | (value >> 4);

	AF.low &= ~ZFlag;
	if (value == 0)
		AF.low |= ZFlag;

	AF.low &= ~SFlag;
	AF.low &= ~HFlag;
	AF.low &= ~CFlag;

	Write(HL.value, value);

	return 4;
}

int CPUSharp::Bit(Register& reg, bool isHigh, int pos) {
	if (isHigh) {
		if ((reg.high & (0x1 << pos)) != (0x1 << pos))
			AF.low |= ZFlag;
		else
			AF.low &= ~ZFlag;
	}
	else {
		if ((reg.low & (0x1 << pos)) != (0x1 << pos))
			AF.low |= ZFlag;
		else
			AF.low &= ~ZFlag;
	}

	AF.low &= ~SFlag;
	AF.low |= HFlag;

	return 2;
}

int CPUSharp::BitMem(int pos) {
	uint8_t value = Read(HL.value);

	if ((value & (0x1 << pos)) != (0x1 << pos))
		AF.low |= ZFlag;
	else
		AF.low &= ~ZFlag;

	AF.low &= ~SFlag;
	AF.low |= HFlag;

	return 3;
}

int CPUSharp::Res(Register& reg, bool isHigh, int pos) {
	if (isHigh) {
		reg.high &= ~(0x1 << pos);
	}
	else {
		reg.low &= ~(0x1 << pos);
	}

	return 2;
}

int CPUSharp::ResMem(int pos) {
	uint8_t value = Read(HL.value);

	value &= ~(0x1 << pos);

	Write(HL.value, value);

	return 4;
}

int CPUSharp::Set(Register& reg, bool isHigh, int pos) {
	if (isHigh) {
		reg.high |= (0x1 << pos);
	}
	else {
		reg.low |= (0x1 << pos);
	}

	return 2;
}

int CPUSharp::SetMem(int pos) {
	uint8_t value = Read(HL.value);

	value |= (0x1 << pos);

	Write(HL.value, value);

	return 4;
}
#pragma endregion
