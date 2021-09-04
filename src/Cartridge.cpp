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
		uint8_t type;
		ifs.read((char*)&type, sizeof(uint8_t));
		if (type == 0x00) {
			std::cout << "ROM ONLY" << std::endl;
		}
		else if (type == 0x01) {
			std::cout << "MBC1" << std::endl;
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
		//else if(nRomBanks <= 0x08){
		//	std::cout << (int)nRomBanks << " ROM Banks" << std::endl;
		//}
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
		if ((n & 0xA) == 0xA && nRamBanks > 0) {
			RamEnable = true;
		}
		else {
			RamEnable = false;
		}
		return;
	}
	if (add >= 0x2000 && add < 0x4000) {
		if (nRomBanks > 2) {
			currentRomBank = (n & 0x1F) + secondaryBank;
			if(n == 0x00 || n == 0x20 || n == 0x40 || n == 0x60)
				currentRomBank += 1;
		}
		return;
	}
	if (add >= 0x4000 && add < 0x6000) {
		if (!bankingMode)
			secondaryBank = (n & 0x3) << 5;
		else
			std::cout << "Ram selection not enable" << std::endl;
		return;
	}
	if (add >= 0x6000 && add < 0x8000) {
		if (n == 0) {
			bankingMode = false;
		}
		else {
			bankingMode = true;
		}
		return;
	}
}

uint8_t Cartridge::Read(uint16_t add)
{
	uint8_t value = 0;
	if (add < 0x4000) {
		value = ROM[add];
	} else
	if (add >= 0x4000 && add < 0x8000) {
		int dir = add + (currentRomBank - 1) * 0x4000;
		value = ROM[dir];
	} else
	if (add >= 0xA000 && add < 0xC000 && nRamBanks > 0 && RamEnable) {
		RamEnable = false;
		value = RAM[add];
	}

	return value;
}
