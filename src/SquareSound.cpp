#include "SquareSound.h"

#include <iostream>

void SquareSound::Clock()
{
	currentFreq -= 1;
	if (currentFreq <= 0) {
		currentFreq = (2048 - ((highFreq << 8) | lowFreq)) * 4;
		dutyPointer = (dutyPointer + 1) % 8;

		if (WaveDuty[duty][dutyPointer] == 1 && enable && play) {
			output = currentVol;
		}
		else {
			output = 0;
		}
	}
}

void SquareSound::LengthClock() {
	lengthCounter--;
	if (lengthCounter == 0 && stopping) {
		enable = false;
	}
}

void SquareSound::EnvClock() {

	currentEnvPer--;
	if (currentEnvPer <= 0) {
		currentEnvPer = envPer;

		if (envActive && envPer > 0) {
			if (envDir && currentVol < 15) {
				currentVol++;
			}
			else if (!envDir && currentVol > 0) {
				currentVol--;
			}
		}

		if (currentVol == 0 || currentVol == 15) {
			envActive = false;
		}
	}
}

void SquareSound::SweepClock() {

	sweepCurrentPer--;
	if (sweepCurrentPer <= 0) {
		sweepCurrentPer = sweepPer;
		if (sweepCurrentPer == 0) {
			sweepCurrentPer = 8;
		}
		if (sweepPer > 0) {

		}
	}

}

uint8_t SquareSound::GetSample()
{
	return output;
}

void SquareSound::Write(uint16_t add, uint8_t n)
{
	if (add == 0xFF10) {
		sweepPer = (n & 0x70) >> 4;

		if ((n & 0x08) == 0x08) {
			sweepNeg = true;
		}
		else {
			sweepNeg = false;
		}

		sweepShift = n & 0x07;

		return;
	}

	if (add == 0xFF16 || add == 0xFF11) {
		duty = ((n & 0xC0) >> 6) & 0x3;

		lengthCounter = n & 0x3F;

		return;
	}

	if (add == 0xFF17 || add == 0xFF12) {

		if ((n & 0xF8) != 0) {
			play = true;
		}
		else {
			play = false;
		}

		vol = (n & 0xF0) >> 4;
		currentVol = vol;

		if ((n & 0x08) == 0x08) {
			envDir = true;
		}
		else {
			envDir = false;
		}

		envPer = (n & 0x07);
		currentEnvPer = envPer;

		return;
	}

	if (add == 0xFF18 || add == 0xFF13) {
		lowFreq = n;
		return;
	}

	if (add == 0xFF19 || add == 0xFF14) {

		highFreq = (n & 0x7);

		if ((n & 0x40) == 0x40) {
			stopping = true;
		}
		else {
			stopping = false;
		}

		if ((n & 0x80) == 0x80)
			Restart();

		return;
	}
}

uint8_t SquareSound::Read(uint16_t add)
{
	uint8_t value = 0;

	if (add == 0xFF10) {
		value = sweepShift | (sweepNeg << 3) | (sweepPer << 4);
	}
	else if (add == 0xFF16 || add == 0xFF11) {
		value = (lengthCounter & 0x3F) | ((duty & 0x3) << 6);
	}
	else if (add == 0xFF17 || add == 0xFF12) {
		value = (envPer & 0x7) | (envDir << 3) | ((vol & 0xF) << 4);
	}
	else if (add == 0xFF18 || add == 0xFF13) {
		value = lowFreq;
	}
	else if (add == 0xFF19 || add == 0xFF14) {
		value = highFreq | (stopping << 6);
	}

	return value;
}

void SquareSound::Restart() {
	enable = true;

	if (lengthCounter == 0) {
		lengthCounter = 64;
	}

	currentFreq = freq;

	envActive = true;
	currentEnvPer = envPer;
	currentVol = vol;

	sweepCurrentPer = sweepPer;
	if (sweepCurrentPer == 0) {
		sweepCurrentPer = 8;
	}
	sweepEnable = sweepCurrentPer > 0 || sweepShift > 0;
}
