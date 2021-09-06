#pragma once

#include <cstdint>

class SquareSound
{

public:

	void Clock();

	void LengthClock();
	void EnvClock();
	void SweepClock();

	uint8_t GetSample();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

private:

	bool enable = false;
	bool play = false;
	int duty = 0;
	int lengthCounter = 0;
	bool stopping = false;
	uint16_t lowFreq = 0;
	uint16_t highFreq = 0;
	int freq = 0;
	int currentFreq = 0;
	int dutyPointer = 0;
	uint8_t vol = 0;
	uint8_t currentVol = 0;
	bool envActive = false;
	bool envDir = false;
	int envPer = 0;
	int currentEnvPer = 0;

	bool sweepEnable = false;
	uint8_t sweepPer = 0;
	uint8_t sweepCurrentPer = 0;
	bool sweepNeg = false;
	uint8_t sweepShift = 0;

	uint8_t output = 0;

	int const WaveDuty[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 0}
	};

	void Restart();
};

