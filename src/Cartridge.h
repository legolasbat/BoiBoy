#pragma once

#include <cstdint>
#include <vector>

class Cartridge
{
public:
	Cartridge(const char* game);

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

private:
	uint8_t type = 0;

	std::vector<uint8_t> ROM;
	std::vector<uint8_t> RAM;

	int currentRomBank = 1;
	int currentRamBank = 0;

	uint8_t nRomBanks = 0;
	uint8_t nRamBanks = 0;

	bool RamEnable = false;

	bool bankingMode = false;

	uint8_t romBank = 1;
	uint8_t secondaryBank = 0;

};

