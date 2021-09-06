#pragma once

#include <cstdint>

class WaveSound
{
public:

	void Clock();

	void LengthClock();

	uint8_t GetSample();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

private:

	bool enable = false;
	bool play = false;
	int shiftVol = 0;
	double lengthCounter = 0.0;
	bool stopping = false;
	uint16_t lowFreq = 0;
	uint16_t highFreq = 0;
	int freq = 0;
	int currentFreq = 0;
	int wavePointer = 0;

	uint8_t output = 0;

	uint8_t WaveRAM[0x10] = {0};

	void Restart();
};

