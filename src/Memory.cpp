#include "Memory.h"

#include <fstream>
#include <iostream>

BoiBoy::BoiBoy(Cartridge* game) {
	cart = game;
	cpu.ConnectMem(this);
	cpu.InitMem();
	ppu.ConnectMem(this);

	cycles = 0;
}
void BoiBoy::InitializeIO() {
	// Memory range 0xFF00 - 0xFF70
	IORegisters[0x00] = 0xCF;
	IORegisters[0x01] = 0x00;
	IORegisters[0x02] = 0x7E;
	//IORegisters[0x03] = 0x00;	// Not init (?)
	dividerReg = 0xAB;
	IORegisters[0x04] = 0xAB;
	TIMA = 0x00;
	IORegisters[0x05] = 0x00;
	TMA = 0x00;
	IORegisters[0x06] = 0x00;
	TAC = 0xF8;
	IORegisters[0x07] = 0xF8;
	//IORegisters[0x08] = 0x00;	// Not init (?)
	//IORegisters[0x09] = 0x00;	// Not init (?)
	//IORegisters[0x0A] = 0x00;	// Not init (?)
	//IORegisters[0x0B] = 0x00;	// Not init (?)
	//IORegisters[0x0C] = 0x00;	// Not init (?)
	//IORegisters[0x0D] = 0x00;	// Not init (?)
	//IORegisters[0x0E] = 0x00;	// Not init (?)
	IFReg = 0xE1;
	IORegisters[0x0F] = 0xE1;	// Should be E1
	IORegisters[0x10] = 0x80;
	IORegisters[0x11] = 0xBF;
	IORegisters[0x12] = 0xF3;
	IORegisters[0x13] = 0xFF;
	IORegisters[0x14] = 0xBF;
	//IORegisters[0x15] = 0x00;	// Not init (?)
	IORegisters[0x16] = 0x3F;
	IORegisters[0x17] = 0x00;
	IORegisters[0x18] = 0xFF;
	IORegisters[0x19] = 0xBF;
	IORegisters[0x1A] = 0x7F;
	IORegisters[0x1B] = 0xFF;
	IORegisters[0x1C] = 0x9F;
	IORegisters[0x1D] = 0xFF;
	IORegisters[0x1E] = 0xBF;
	//IORegisters[0x1F] = 0x00;	// Not init (?)
	IORegisters[0x20] = 0xFF;
	IORegisters[0x21] = 0x00;
	IORegisters[0x22] = 0x00;
	IORegisters[0x23] = 0xBF;
	IORegisters[0x24] = 0x77;
	IORegisters[0x25] = 0xF3;
	IORegisters[0x26] = 0xF1;
	//IORegisters[0x27] = 0x00;	// Not init (?)
	// Not init until 0xFF40

	ppu.LCDC = 0x91;
	IORegisters[0x40] = 0x91;
	ppu.STAT = 0x85;
	IORegisters[0x41] = 0x85;
	IORegisters[0x42] = 0x00;
	IORegisters[0x43] = 0x00;
	IORegisters[0x44] = 0x90;	// Should be 0
	IORegisters[0x45] = 0x00;
	IORegisters[0x46] = 0xFF;
	IORegisters[0x47] = 0xFC;
	IORegisters[0x48] = 0xFF;
	IORegisters[0x49] = 0xFF;
	IORegisters[0x4A] = 0x00;
	IORegisters[0x4B] = 0x00;
	//IORegisters[0x4C] = 0x00;	// Not init (?)
	IORegisters[0x4D] = 0xFF;
	//IORegisters[0x4E] = 0x00;	// Not init (?)
	IORegisters[0x4F] = 0xFF;
	//IORegisters[0x50] = 0x00;	// Not init (?)
	IORegisters[0x51] = 0xFF;
	IORegisters[0x52] = 0xFF;
	IORegisters[0x53] = 0xFF;
	IORegisters[0x54] = 0xFF;
	IORegisters[0x55] = 0xFF;
	IORegisters[0x56] = 0xFF;

	IORegisters[0x68] = 0xFF;
	IORegisters[0x69] = 0xFF;
	IORegisters[0x6A] = 0xFF;
	IORegisters[0x6B] = 0xFF;

	IORegisters[0x70] = 0xFF;
}

int BoiBoy::Clock() {

	if (TMAChanged) {
		TMAChanged = false;
	}

	cpuCycles = cpu.Clock();

	if (DMA) {
		ppu.Write(0xFE00 + dmaOffset, Read(dmaAdd + dmaOffset));
		dmaOffset++;
		if (dmaOffset == 0xA0) {
			DMA = false;
			dmaOffset = 0;
		}
	}

	ppu.Clock(cpuCycles);

	// Timer
	Timer();

	// Interrupts
	Interrupt();

	cycles++;

	return cpuCycles;
}

void BoiBoy::Timer() {
	// Divider
	divCycles++;
	if ((divCycles >= 64 && !cpu.stopped)) {
		divCycles -= 64;
		dividerReg++;
	}

	// Timer
	if (timerEnable) {
		timerCycles++;
		if (timerCycles >= TAC) {
			TIMA++;
			if (TIMA == 0) {
				// Interrupt
				IFReg |= 0x04;
				TIMA = TMAChanged ? prevTMA : TMA;
			}
			timerCycles -= TAC;
		}
	}
}

void BoiBoy::Interrupt() {
	if ((IFReg & IEReg) != 0 && cpu.IME) {
		// Handle interrupt
		// V-Blank
		if ((IFReg & IEReg) == 0x01) {
			//std::cout << "V Blank Interrupt" << std::endl;
			cpu.Interrupt(0x0040);
			IFReg &= ~0x01;
		}
		// LCD STAT
		else if ((IFReg & IEReg) == 0x02) {
			//std::cout << "STAT Interrupt" << std::endl;
			cpu.Interrupt(0x0048);
			IFReg &= ~0x02;
		}
		// Timer
		else if ((IFReg & IEReg) == 0x04) {
			//std::cout << "Timer Interrupt" << std::endl;
			cpu.Interrupt(0x0050);
			IFReg &= ~0x04;
		}
		// Serial
		else if ((IFReg & IEReg) == 0x08) {

		}
		// Joypad
		else if ((IFReg & IEReg) == 0x10) {
			std::cout << "Joypad Interrupt" << std::endl;
			cpu.Interrupt(0x0060);
			IFReg &= ~0x10;
		}
	}
	else if (!cpu.IME && cpu.halted) {
		// Handle interrupt
		// V-Blank
		if ((IFReg & IEReg) == 0x01) {
			cpu.interrupt = true;
		}
		// LCD STAT
		else if ((IFReg & IEReg) == 0x02) {
			cpu.interrupt = true;
		}
		// Timer
		else if ((IFReg & IEReg) == 0x04) {
			cpu.interrupt = true;
		}
		// Serial
		else if ((IFReg & IEReg) == 0x08) {

		}
		// Joypad
		else if ((IFReg & IEReg) == 0x10) {
			cpu.interrupt = true;
		}
	}
}

void BoiBoy::Write(uint16_t add, uint8_t n) {
	// Cart ROM (0x0000 - 0x7FFF)
	if (add < 0x8000) {
		if (bootRom && add < 0x100)
			std::cout << "Writting in Boot Rom?!" << std::endl;

		cart->Write(add, n);
		return;
	}

	// VRAM (0x8000 - 0x9FFF)
	if (add >= 0x8000 && add < 0xA000) {
		// it should be capable of write while in mode 3 but...
		//if (ppu.mode == 3)
		//	return;

		ppu.Write(add, n);
		return;
	}

	// WRAM (0xC000 - 0xDFFF, 0xE000 - 0xFDFF are mirror of 0xC000 - 0xDDFF)
	if (add >= 0xC000 && add < 0xFE00) {
		WRAM[add & 0x1FFF] = n;
		return;
	}

	// OAM (0xFE00 - 0xFE9F)
	if (add >= 0xFE00 && add < 0xFEA0) {
		//if (ppu.mode == 3 || ppu.mode == 2)
		//	return;

		ppu.Write(add, n);
		return;
	}

	// IO Registers
	if (add >= 0xFF00 && add < 0xFF80) {
		if (add == 0xFF00) {
			controller = n;
			return;
		}
		if (add == 0xFF01) {
			Letter = (char)n;
			return;
		}
		if (add == 0xFF02) {
			if (n == 0x81)
				std::cout << Letter;
			return;
		}
		if (add == 0xFF04) {
			dividerReg = 0;
			return;
		}
		if (add == 0xFF05) {
			TIMA = n;
			return;
		}
		if (add == 0xFF06) {
			prevTMA = TMA;
			TMA = n;
			TMAChanged = true;
			return;
		}
		if (add == 0xFF07) {
			timerEnable = ((n & 0x04) != 0) ? true : false;

			switch (n & 0x3) {
			case 0:
				TAC = 256;
				break;
			case 1:
				TAC = 4;
				break;
			case 2:
				TAC = 16;
				break;
			case 3:
				TAC = 64;
				break;
			}
			return;
		}
		if (add == 0xFF0F) {
			IFReg = n | 0xE0;
			return;
		}
		if (add == 0xFF40) {
			ppu.WriteLCDC(n);
			return;
		}
		if (add == 0xFF41) {
			ppu.STAT = n | 0x80;
			return;
		}
		if (add == 0xFF42) {
			ppu.SCY = n;
			return;
		}
		if (add == 0xFF43) {
			ppu.SCX = n;
			return;
		}
		if (add == 0xFF44) {
			std::cout << "Trying to writting in LY" << std::endl;
			// Read only
			return;
		}
		if (add == 0xFF45) {
			ppu.LYC = n;
			return;
		}
		if (add == 0xFF46) {
			//std::cout << "DMA: " << (int)n << std::endl;
			if (DMA)
				return;

			dmaAdd = n << 8;
			DMA = true;
			return;
		}
		if (add == 0xFF47) {
			ppu.BGP = n;
			return;
		}
		if (add == 0xFF48) {
			ppu.OBP0 = n;
			return;
		}
		if (add == 0xFF49) {
			ppu.OBP1 = n;
			return;
		}
		if (add == 0xFF50) {
			if (n != 0) {
				bootRom = false;
				cpu.InitMem();
			}
			return;
		}
		if (add == 0xFF4A) {
			ppu.WY = n;
			return;
		}
		if (add == 0xFF4B) {
			ppu.WX = n;
			return;
		}
		IORegisters[add & 0x7F] = n;
		return;
	}

	// HRAM
	if (add >= 0xFF80 && add < 0xFFFF) {
		HRAM[add - 0xFF80] = n;
		return;
	}

	// IE
	if (add == 0xFFFF) {
		IEReg = n;
		return;
	}
}

uint8_t BoiBoy::Read(uint16_t add) {
	uint8_t value = 0;

	// Cart ROM (0x0000 - 0x7FFF)
	if (add < 0x8000) {
		if (bootRom && add < 0x100)
			value = Rom::bootRom[add];
		else
			value = cart->Read(add);
	} else

	// VRAM (0x8000 - 0x9FFF)
	if (add >= 0x8000 && add < 0xA000) {
		if (ppu.mode == 3) {
			value = 0xFF;
		}
		else {
			value = ppu.Read(add);
		}
	}

	// Cart RAM (0xA000 - 0xBFFF)
	if (add >= 0xA000 && add < 0xC000) {
		value = cart->Read(add);
	} else

	// WRAM (0xC000 - 0xDFFF, 0xE000 - 0xFDFF are mirror of 0xC000 - 0xDDFF)
	if (add >= 0xC000 && add < 0xFE00) {
		value = WRAM[add & 0x1FFF];
	} else

	if (add >= 0xFE00 && add < 0xFE9F) {
		if (ppu.mode == 3 || ppu.mode == 2)
			value = 0xFF;
		else
			value = ppu.Read(add);
	} else

	// IO Registers
	if (add >= 0xFF00 && add < 0xFF80) {
		if (add == 0xFF00) {
			if ((controller & 0x10) == 0x10) {
				controller |= (controllerInput >> 4);
			}
			else if ((controller & 0x20) == 0x20) {
				controller |= (controllerInput & 0x0F);
			}
			if (~controllerInput != 0) {
				IFReg |= 0x10;
			}
			value = controller;
		} else
		if (add == 0xFF04) {
			value = dividerReg;
		} else
		if (add == 0xFF05) {
			value = TIMA;
		} else
		if (add == 0xFF06) {
			value = TMA;
		} else
		if (add == 0xFF0F) {
			value = IFReg;
		} else
		if (add == 0xFF40) {
			value = ppu.LCDC;
		} else
		if (add == 0xFF41) {
			value = ppu.STAT;
		} else
		if (add == 0xFF42) {
			value = ppu.SCY;
		} else
		if (add == 0xFF43) {
			value = ppu.SCX;
		} else
		if (add == 0xFF44) {
			value = ppu.LY;
		} else
		if (add == 0xFF45) {
			value = ppu.LYC;
		} else
		if (add == 0xFF46) {
			std::cout << "Reading DMA?" << std::endl;
		} else
		if (add == 0xFF47) {
			value = ppu.BGP;
		} else
		if (add == 0xFF48) {
			value = ppu.OBP0;
		} else
		if (add == 0xFF49) {
			value = ppu.OBP1;
		} else
		if (add == 0xFF4A) {
			value = ppu.WY;
		} else
		if (add == 0xFF4B) {
			value = ppu.WX;
		}
		else {
			value = IORegisters[add & 0x7F];
		}
	} else

	// HRAM
	if (add >= 0xFF80 && add < 0xFFFF) {
		value = HRAM[add - 0xFF80];
	} else

	// IEReg
	if (add == 0xFFFF)
		value = IEReg;

	return value;
}
