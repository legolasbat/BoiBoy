#include "NoiseSound.h"

#include <iostream>

void NoiseSound::Clock()
{
	currentFreq -= 1;
	if (currentFreq <= 0) {
		currentFreq = divisors[divisorIndex] << clockShift;

		uint8_t result = (lfsr & 0x1) ^ ((lfsr >> 1) & 0x1);
		lfsr >>= 1;
		lfsr |= result << 14;
		if (width) {
			lfsr &= ~0x40;
			lfsr |= result << 6;
		}

		if ((lfsr & 0x1) == 0 && enable && play) {
			output = currentVol;
		}
		else {
			output = 0;
		}
	}
}

void NoiseSound::LengthClock() {

	if (lengthCounter > 0 && stopping) {
		lengthCounter--;
		if (lengthCounter == 0 && stopping) {
			enable = false;
		}
	}
}

void NoiseSound::EnvClock() {

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

uint8_t NoiseSound::GetSample()
{
	return output;
}

void NoiseSound::Write(uint16_t add, uint8_t n)
{

	if (add == 0xFF20) {
		lengthCounter = n & 0x3F;

		return;
	}

	if (add == 0xFF21) {

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

	if (add == 0xFF22) {
		clockShift = (n & 0xF0) >> 4;
		if ((n & 0x08) == 0x08) {
			width = true;
		}
		else {
			width = false;
		}
		divisorIndex = (n & 0x07);
		return;
	}

	if (add == 0xFF23) {

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

uint8_t NoiseSound::Read(uint16_t add)
{
	uint8_t value = 0;

	//if (add == 0xFF20) {
	//	lengthCounter = n & 0x3F;
	//
	//	return;
	//}
	//
	//if (add == 0xFF21) {
	//
	//	if ((n & 0xF8) != 0) {
	//		play = true;
	//	}
	//	else {
	//		play = false;
	//	}
	//
	//	vol = (n & 0xF0) >> 4;
	//	currentVol = vol;
	//
	//	if ((n & 0x08) == 0x08) {
	//		envDir = true;
	//	}
	//	else {
	//		envDir = false;
	//	}
	//
	//	envPer = (n & 0x07);
	//	currentEnvPer = envPer;
	//
	//	return;
	//}
	//
	//if (add == 0xFF22) {
	//	clockShift = (n & 0xF0) >> 4;
	//	if ((n & 0x08) == 0x08) {
	//		width = true;
	//	}
	//	else {
	//		width = false;
	//	}
	//	divisorIndex = (n & 0x07);
	//	return;
	//}
	//
	//if (add == 0xFF23) {
	//
	//	if ((n & 0x40) == 0x40) {
	//		stopping = true;
	//	}
	//	else {
	//		stopping = false;
	//	}
	//
	//	if ((n & 0x80) == 0x80)
	//		Restart();
	//
	//	return;
	//}

	return value;
}

void NoiseSound::Restart() {
	enable = true;

	if (lengthCounter == 0) {
		lengthCounter = 64;
	}

	currentFreq = divisors[divisorIndex] << clockShift;

	envActive = true;
	currentEnvPer = envPer;
	currentVol = vol;

	lfsr = 0x7FFF;
}

