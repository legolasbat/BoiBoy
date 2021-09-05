#pragma once

#include "Cartridge.h"
#include "CPUSharp.h"
#include "PPU.h"
#include "SPU.h"
#include "BootRom.h"

#include <vector>

class BoiBoy
{
public:
	BoiBoy(Cartridge* game);

	int Clock();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

	void InitializeIO();

	int cycles;
	int cpuCycles = 0;

	CPUSharp cpu;
	Cartridge* cart;
	PPU ppu;
	SPU spu;

	uint8_t IFReg = 0;
	uint8_t IEReg = 0;

	// Start Select B A Down Up Left Right
	uint8_t controllerInput = 0xFF;

	uint8_t TIMA = 0;

private:

	bool bootRom = false;

	int timerCycles = 0;
	uint16_t divCycles = 0;
	uint8_t dividerReg = 0;
	bool timerEnable = false;
	uint8_t prevTMA = 0;
	uint8_t TMA = 0;
	bool TMAChanged = false;
	int TAC = 0;

	bool DMA = false;
	uint16_t dmaAdd = 0;
	uint8_t dmaOffset = 0;

	uint8_t WRAM[0x2000];

	uint8_t controller = 0xCF;

	uint8_t HRAM[0x7F];

	void Timer();
	void Interrupt();
};

