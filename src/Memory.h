#pragma once

#include "Cartridge.h"
#include "CPUSharp.h"

#include <cstdint>
#include <vector>

class BoiBoy
{
public:
	BoiBoy(Cartridge* game);

	void Clock();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

	void InitializeIO();

	int cycles;
	int cpuCycles = 0;

private:

	CPUSharp cpu;
	Cartridge* cart;

	int timerCycles = 0;
	uint16_t divCycles = 0;
	uint8_t dividerReg = 0;
	bool timerEnable = false;
	uint8_t TIMA = 0;
	uint8_t prevTMA = 0;
	uint8_t TMA = 0;
	bool TMAChanged = false;
	int TAC = 0;

	uint8_t WRAM[0x2000];

	uint8_t IORegisters[0x80];

	uint8_t HRAM[0x7F];

	uint8_t IEReg = 0;
	uint8_t IFReg = 0;

	void Timer();
	void Interrupt();

	char Letter;
};

