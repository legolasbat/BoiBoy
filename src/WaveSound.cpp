#include "WaveSound.h"

#include <iostream>

void WaveSound::Clock()
{
	currentFreq -= 1;
	if (currentFreq <= 0) {
		currentFreq = (2048 - ((highFreq << 8) | lowFreq)) * 2;
		wavePointer = (wavePointer + 1) % 32;

		if (enable && play) {
			uint8_t waveVal = WaveRAM[wavePointer / 2];
			if (wavePointer % 2 == 0) {
				waveVal = (waveVal >> 4);
			}
			waveVal &= 0x0F;

			if (shiftVol == 4) {
				output = 0;
			}
			else {
				output = (waveVal >> shiftVol);
			}
		}
		else {
			output = 0;
		}
	}
}

void WaveSound::LengthClock() {
	lengthCounter--;
	if (lengthCounter == 0 && stopping) {
		enable = false;
	}
}

uint8_t WaveSound::GetSample()
{
	return output;
}

void WaveSound::Write(uint16_t add, uint8_t n)
{
	if (add == 0xFF1A) {
		if ((n & 0x80) == 0x80) {
			play = true;
		}
		else {
			play = false;
		}
		return;
	}

	if (add == 0xFF1B) {
		lengthCounter = n;
		return;
	}

	if (add == 0xFF1C) {
		switch (((n & 0x60) >> 5) & 0x3) {
		case 0:
			shiftVol = 4;
			break;
		case 1:
			shiftVol = 0;
			break;
		case 2:
			shiftVol = 1;
			break;
		case 3:
			shiftVol = 2;
			break;
		}
		return;
	}

	if (add == 0xFF1D) {
		lowFreq = n;
		return;
	}

	if (add == 0xFF1E) {
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

	// Wave RAM
	if (add >= 0xFF30 && add < 0xFF40) {
		WaveRAM[add - 0xFF30] = n;
		return;
	}
}

uint8_t WaveSound::Read(uint16_t add)
{
	uint8_t value = 0;

	if (add == 0xFF1A) {
		if (enable) {
			value = 0xFF;
		}
		else {
			value = 0;
		}
	}
	else if (add == 0xFF1B) {
		value = (uint8_t)(-lengthCounter / (1.0 / 256.0)) + 256;
	}
	else if (add == 0xFF1C) {
		switch (shiftVol) {
		case 0:
			value = 0x9F;
			break;
		case 1:
			value = 0xBF;
			break;
		case 2:
			value = 0xDF;
			break;
		case 3:
			value = 0xFF;
			break;
		}
	}
	else if (add == 0xFF1D) {
		value = (lowFreq & 0xFF);
	}
	else if (add == 0xFF1E) {
		if (stopping) {
			value |= 0x40;
		}
		value |= (highFreq & 0x7);
	}

	return value;
}

void WaveSound::Restart() {
	enable = true;

	if (lengthCounter == 0) {
		lengthCounter = 256;
	}

	currentFreq = freq;
	wavePointer = 0;
}
