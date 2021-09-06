#pragma once

#include <cstdint>

class NoiseSound
{
public:

	void Clock();

	void LengthClock();
	void EnvClock();

	uint8_t GetSample();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

private:

	bool enable = false;
	bool play = false;
	double lengthCounter = 0.0;
	bool stopping = false;
	uint8_t vol = 0;
	uint8_t currentVol = 0;
	uint16_t rawFreq = 0;
	int freq = 0;
	int currentFreq = 0;
	int wavePointer = 0;
	bool envActive = false;
	bool envDir = false;
	int envPer = 0;
	int currentEnvPer = 0;

	uint16_t lfsr = 0;

	uint8_t clockShift = 0;
	bool width = false;
	uint8_t divisorIndex = 0;

	uint8_t output = 0;

	uint16_t const divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

	void Restart();
};

