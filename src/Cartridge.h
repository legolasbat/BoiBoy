#pragma once

#include <cstdint>
#include <vector>

class Cartridge
{
public:
	Cartridge(const char* game);

	void Write(uint16_t add, uint8_t b);
	uint8_t Read(uint16_t add);

private:
	std::vector<uint8_t> ROM;
	std::vector<uint8_t> RAM;

	int currentRomBank = 1;

	uint8_t nRomBanks = 0;
	uint8_t nRamBanks = 0;

	bool RamEnable = false;

	uint8_t secondaryBank = 0;

};

