#include "Cartridge.h"

#include <fstream>
#include <iostream>

Cartridge::Cartridge(const char* game)
{
	std::ifstream ifs;
	ifs.open(game, std::ifstream::binary);
	if (ifs.is_open()) {
		// Title
		ifs.seekg(0x134);
		char t;
		ifs.read(&t, sizeof(char));
		while (t != 0) {
			std::cout << t;
			ifs.read(&t, sizeof(char));
		}
		std::cout << std::endl;

		// Cartridge Type
		ifs.seekg(0x147);
		ifs.read((char*)&type, sizeof(uint8_t));
		if (type == 0x00) {
			std::cout << "ROM ONLY" << std::endl;
		}
		else if (type == 0x01) {
			std::cout << "MBC1" << std::endl;
		}
		else if (type == 0x02) {
			std::cout << "MBC1 + RAM" << std::endl;
		}
		else if (type == 0x03) {
			std::cout << "MBC1 + RAM + BATTERY" << std::endl;
		}
		else if (type == 0x05) {
			std::cout << "MBC2" << std::endl;
		}
		else if (type == 0x06) {
			std::cout << "MBC2 + BATTERY" << std::endl;
		}
		else {
			std::cout << "CARTRIDGE TYPE NOT HANDLED" << std::endl;
		}

		// ROM Size
		uint8_t RomSize;
		ifs.read((char*)&RomSize, sizeof(uint8_t));
		nRomBanks = (0x8000 << RomSize) / 0x4000;
		if (nRomBanks == 2) {
			std::cout << "No ROM banking" << std::endl;
		}
		else if (nRomBanks <= 0x7F) {
			std::cout << (int)nRomBanks << " ROM Banks" << std::endl;
		}
		else {
			std::cout << "CARTRIDGE ROM SIZE NOT HANDLED" << std::endl;
		}

		// RAM Size
		uint8_t RamSize;
		ifs.read((char*)&RamSize, sizeof(uint8_t));
		if (RamSize == 0x00) {
			std::cout << "No RAM" << std::endl;
			nRamBanks = 0;
		}
		else if (RamSize == 0x01) {
			std::cout << "Unused RAM" << std::endl;
			nRamBanks = 0;
		}
		else if (RamSize == 0x02) {
			std::cout << "1 Bank RAM" << std::endl;
			nRamBanks = 1;
		}
		else if (RamSize == 0x03) {
			std::cout << "4 Bank RAM" << std::endl;
			nRamBanks = 4;
		}
		else if (RamSize == 0x04) {
			std::cout << "16 Bank RAM" << std::endl;
			nRamBanks = 16;
		}
		else if (RamSize == 0x05) {
			std::cout << "8 Bank RAM" << std::endl;
			nRamBanks = 8;
		}
		else {
			std::cout << "CARTRIDGE RAM SIZE NOT HANDLED" << std::endl;
		}

		ROM.resize(nRomBanks * 0x4000);
		RAM.resize(nRamBanks * 0x8000);

		ifs.seekg(0);
		ifs.read((char*)ROM.data(), ROM.size());
	}
}

void Cartridge::Write(uint16_t add, uint8_t n)
{
	if (add < 0x2000) {
		// MBC1
		if (type == 0x1 || type == 0x2 || type == 0x3) {
			if ((n & 0xA) == 0xA && nRamBanks > 0) {
				RamEnable = true;
			}
			else {
				RamEnable = false;
			}
		}
		return;
	}
	if (add >= 0x2000 && add < 0x4000) {
		// MBC1
		if (nRomBanks > 2) {
			if (type == 0x1 || type == 0x2 || type == 0x3) {
				romBank = (n & 0x1F);
				if ((romBank == 0x00 || romBank == 0x20 || romBank == 0x40 || romBank == 0x60) && bankingMode) {
					romBank += 1;
				}
			}
		}
		return;
	}
	if (add >= 0x4000 && add < 0x6000) {
		// MBC1
		if (type == 0x1 || type == 0x2 || type == 0x3) {
			secondaryBank = (n & 0x3);
		}
		return;
	}
	if (add >= 0x6000 && add < 0x8000) {
		// MBC1
		if (type == 0x1 || type == 0x2 || type == 0x3) {
			if (n == 0) {
				bankingMode = false;
			}
			else {
				bankingMode = true;
			}
		}
		return;
	}

	if (add >= 0xA000 && add < 0xC000 && nRamBanks > 0 && RamEnable) {
		// MBC1
		if (type == 0x1 || type == 0x2 || type == 0x3) {
			if (bankingMode) {
				std::cout << "multiple ram banks not implemented" << std::endl;
			}
			else {
				RAM[add - 0xA000] = n;
			}
		}
		return;
		
	}
}

uint8_t Cartridge::Read(uint16_t add)
{
	uint8_t value = 0;
	if (add < 0x4000) {
		// MBC0
		if (type == 0)
			value = ROM[add];
		// MBC1
		else if (type == 0x1 || type == 0x2 || type == 0x3) {
			int dir = add;
			if (currentRomBank == 0x20 || currentRomBank == 0x40 || currentRomBank == 0x60) {
				dir += currentRomBank * 0x4000;
			}
			
			value = ROM[dir];
		}
	}
	else if (add >= 0x4000 && add < 0x8000) {
		// MBC0
		if (type == 0) {
			value = ROM[add];
		}
		// MBC1
		else if (type == 0x1 || type == 0x2 || type == 0x3) {
			currentRomBank = romBank + (secondaryBank << 5);
			int dir = add + (currentRomBank - 1) * 0x4000;
			value = ROM[dir];
		}
	}
	else if (add >= 0xA000 && add < 0xC000 && nRamBanks > 0 && RamEnable) {
		value = RAM[add - 0xA000];
	}

	return value;
}
