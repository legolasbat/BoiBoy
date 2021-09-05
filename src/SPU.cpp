#include "SPU.h"

SPU::SPU() {
	for (int i = 0; i < 0x10; i++) {
		WaveRAM[i] = 0;
	}
	samples.resize(maxSamples);

	SDL_AudioSpec audioSpec;
	
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_S16SYS;
	audioSpec.channels = 2;
	audioSpec.samples = maxSamples;

	SDL_OpenAudio(&audioSpec, NULL);
	SDL_PauseAudio(0);
}

void SPU::Clock(int cycles)
{
	if (!soundEnable)
		return;

	for (int i = 0; i < cycles * 4; i++) {

		sequenceCounter++;
		if (sequenceCounter >= 8192) {
			sequenceCounter = 0;

			switch (sequencer) {
			case 0:
				
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				CH2Env();
				break;
			}

			sequencer = (sequencer + 1) % 8;
		}

		CH1Clock();
		CH2Clock();
		CH3Clock();

		cycleCounter++;
		if (cycleCounter >= 95) {
			cycleCounter = 0;

			// CH1
			//samples.at(samplesIndex) = CH1Output;
			//samples.at(samplesIndex + 1) = CH1Output;

			int16_t sample = 0;

			int volume = (128 * leftVol) / 7;

			// CH1 Left
			if ((soundOutput & 0x10) == 0x10 && channel1) {
				sample += CH1Output * masterVolume;
			}
			// CH2 Left
			if ((soundOutput & 0x20) == 0x20 && channel2) {
				sample += CH2Output * masterVolume;
			}
			// CH3 Left
			if ((soundOutput & 0x40) == 0x40 && channel3) {
				sample += CH3Output * masterVolume;
			}
			samples.at(samplesIndex++) = sample;
			sample = 0;

			// CH1 Right
			if ((soundOutput & 0x10) == 0x10 && channel1) {
				sample += CH1Output * masterVolume;
			}
			// CH2 Right
			if ((soundOutput & 0x02) == 0x02 && channel2) {
				sample += CH2Output * masterVolume;
			}
			// CH3 Right
			if ((soundOutput & 0x04) == 0x04 && channel3) {
				sample += CH3Output * masterVolume;
			}
			samples.at(samplesIndex++) = sample;
		}
		
		if (samplesIndex >= maxSamples) {
			samplesIndex = 0;

			while (SDL_GetQueuedAudioSize(1) > maxSamples * sizeof(int16_t)) {
				//std::cout << "wait" << std::endl;
				sf::sleep(sf::microseconds(1));
			}

			SDL_QueueAudio(1, samples.data(), samples.size() * sizeof(int16_t));
		}
	}
}

void SPU::CH1Clock() {

	CH1CurrentFreq -= 1;
	if (CH1CurrentFreq <= 0) {
		CH1CurrentFreq = CH1Freq;
		CH1DutyPointer = (CH1DutyPointer + 1) % 8;
	}

	if (WaveDuty[CH1Duty][CH1DutyPointer] == 1 && CH1Enable && CH1Play) {
		CH1Output = CH1CurrentVol;
	}
	else {
		CH1Output = 0;
	}
}

void SPU::CH2Clock() {

	CH2CurrentFreq -= 1;
	if (CH2CurrentFreq <= 0) {
		CH2CurrentFreq = CH2Freq;
		CH2DutyPointer = (CH2DutyPointer + 1) % 8;
	}

	if (WaveDuty[CH2Duty][CH2DutyPointer] == 1 && CH2Enable && CH2Play) {
		CH2Output = CH1CurrentVol;
	}
	else {
		CH2Output = 0;
	}
}

void SPU::CH3Clock() {

	CH3CurrentFreq -= 1;
	if (CH3CurrentFreq <= 0) {
		CH3CurrentFreq = CH3Freq;
		CH3WavePointer = (CH3WavePointer + 1) % 32;

		if (CH3Enable && CH3Play) {
			uint8_t waveVal = WaveRAM[CH3WavePointer / 2];
			if (CH3WavePointer % 2 == 0) {
				waveVal = (waveVal >> 4);
			}
			waveVal &= 0x0F;

			if (CH3ShiftVol == 4) {
				CH3Output = 0;
			}
			else {
				CH3Output = (waveVal >> CH3ShiftVol);
			}
		}
		else {
			CH3Output = 0;
		}
	}
}

void SPU::CH1Env() {

	CH1CurrentEnvPer--;
	if (CH1CurrentEnvPer <= 0) {
		CH1CurrentEnvPer = CH1EnvPer;

		if (CH1EnvActive && CH1EnvPer > 0) {
			if (CH1EnvDir && CH1CurrentVol < 15) {
				CH1CurrentVol++;
			}
			else if (!CH1EnvDir && CH1CurrentVol > 0) {
				CH1CurrentVol--;
			}
		}

		if (CH1CurrentVol == 0 || CH1CurrentVol == 15) {
			CH1EnvActive = false;
		}
	}
}

void SPU::CH2Env() {

	CH2CurrentEnvPer--;
	if (CH2CurrentEnvPer <= 0) {
		CH2CurrentEnvPer = CH2EnvPer;

		if (CH2EnvActive && CH2EnvPer > 0) {
			if (CH2EnvDir && CH2CurrentVol < 15) {
				CH2CurrentVol++;
			}
			else if (!CH2EnvDir && CH2CurrentVol > 0) {
				CH2CurrentVol--;
			}
		}

		if (CH2CurrentVol == 0 || CH2CurrentVol == 15) {
			CH2EnvActive = false;
		}
	}
}

void SPU::RestartCH1() {
	CH1Enable = true;

	// Check length counter

	CH1CurrentFreq = CH1Freq;

	CH1EnvActive = true;
	CH1CurrentEnvPer = CH1EnvPer;
	CH1CurrentVol = CH1Vol;
}

void SPU::RestartCH2() {
	CH2Enable = true;

	// Check length counter

	CH2CurrentFreq = CH2Freq;

	CH2EnvActive = true;
	CH2CurrentEnvPer = CH2EnvPer;
	CH2CurrentVol = CH2Vol;
}

void SPU::RestartCH3() {
	CH3Enable = true;

	// Check length counter

	CH3CurrentFreq = CH3Freq;
	CH3WavePointer = 0;
}

void SPU::Write(uint16_t add, uint8_t n)
{

#pragma region CH1
	// Channel 1 (Tone and Sweep)
	if (add == 0xFF11) {
		CH1Duty = ((n & 0xC0) >> 6) & 0x3;

		CH1TimeToLife = (64 - (n & 0x3F)) * (1.0 / 256.0);

		return;
	}

	if (add == 0xFF12) {
		if ((n & 0xF8) != 0) {
			CH1Play = true;
		}
		else {
			CH1Play = false;
		}

		CH1Vol = (n & 0xF0) >> 4;
		CH1CurrentVol = CH1Vol;

		if ((n & 0x08) == 0x08) {
			CH1EnvDir = true;
		}
		else {
			CH1EnvDir = false;
		}

		CH1EnvPer = (n & 0x07);
		CH1CurrentEnvPer = CH1EnvPer;

		return;
	}

	if (add == 0xFF13) {
		CH1RawFreq &= 0xFF00;
		CH1RawFreq |= n;
		CH1Freq = (2048 - CH1RawFreq) * 4; // Cycles
		CH1CurrentFreq = CH1Freq;
		return;
	}

	if (add == 0xFF14) {
		CH1RawFreq &= 0xF8FF;
		CH1RawFreq |= (n & 0x7);
		CH1Freq = (2048 - CH1RawFreq) * 4; // Cycles
		CH1CurrentFreq = CH1Freq;

		if ((n & 0x40) == 0x40) {
			CH1Stopping = true;
		}
		else {
			CH1Stopping = false;
		}

		if ((n & 0x80) == 0x80)
			RestartCH1();

		return;
	}
#pragma endregion

#pragma region CH2
	// Channel 2 (Tone)
	if (add == 0xFF16) {
		CH2Duty = ((n & 0xC0) >> 6) & 0x3;

		CH2TimeToLife = (64 - (n & 0x3F)) * (1.0 / 256.0);

		return;
	}

	if (add == 0xFF17) {

		if ((n & 0xF8) != 0) {
			CH2Play = true;
		}
		else {
			CH2Play = false;
		}

		CH2Vol = (n & 0xF0) >> 4;
		CH2CurrentVol = CH2Vol;

		if ((n & 0x08) == 0x08) {
			CH2EnvDir = true;
		}
		else {
			CH2EnvDir = false;
		}

		CH2EnvPer = (n & 0x07);
		CH2CurrentEnvPer = CH2EnvPer;

		return;
	}

	if (add == 0xFF18) {
		CH2RawFreq &= 0xFF00;
		CH2RawFreq |= n;
		CH2Freq = (2048 - CH2RawFreq) * 4; // Cycles
		CH2CurrentFreq = CH2Freq;
		return;
	}

	if (add == 0xFF19) {


		CH2RawFreq &= 0xF8FF;
		CH2RawFreq |= (n & 0x7);
		CH2Freq = (2048 - CH2RawFreq) * 4; // Cycles
		CH2CurrentFreq = CH2Freq;

		if ((n & 0x40) == 0x40) {
			CH2Stopping = true;
		}
		else {
			CH2Stopping = false;
		}

		if ((n & 0x80) == 0x80)
			RestartCH2();

		return;
	}
#pragma endregion


#pragma region CH3
	// Channel 3 (Wave)
	if (add == 0xFF1A) {
		if ((n & 0x80) == 0x80) {
			CH3Play = true;
		}
		else {
			CH3Play = false;
		}
		return;
	}

	if (add == 0xFF1B) {
		CH3TimeToLife = (256 - n) * (1.0 / 256.0);
		return;
	}

	if (add == 0xFF1C) {
		switch (((n & 0x60) >> 5) & 0x3) {
		case 0:
			CH3ShiftVol = 4;
			break;
		case 1:
			CH3ShiftVol = 0;
			break;
		case 2:
			CH3ShiftVol = 1;
			break;
		case 3:
			CH3ShiftVol = 2;
			break;
		}
		return;
	}

	if (add == 0xFF1D) {
		CH3RawFreq &= 0xFF00;
		CH3RawFreq |= n;
		CH3Freq = (2048 - CH3RawFreq) * 2; // Cycles
		CH3CurrentFreq = CH3Freq;
		return;
	}

	if (add == 0xFF1E) {
		if ((n & 0x40) == 0x40) {
			CH3Stopping = true;
		}
		else {
			CH3Stopping = false;
		}
		CH3RawFreq &= 0xF8FF;
		CH3RawFreq |= ((uint16_t)n & 0x7) << 8;
		CH3Freq = (2048 - CH3RawFreq) * 2; // Cycles
		CH3CurrentFreq = CH3Freq;

		if ((n & 0x80) == 0x80)
			RestartCH3();

		return;
	}
#pragma endregion

	// Control
	if (add == 0xFF24) {
		leftVol = (n & 0x70) >> 4;
		return;
	}

	if (add == 0xFF25) {
		soundOutput = n;
		return;
	}

	if (add == 0xFF26) {
		if ((n & 0x80) == 0x80) {
			soundEnable = true;
		}
		else {
			soundEnable = false;
		}
		return;
	}

	// Wave RAM
	if (add >= 0xFF30 && add < 0xFF40) {
		WaveRAM[add - 0xFF30] = n;
		return;
	}
}

uint8_t SPU::Read(uint16_t add)
{
	uint8_t value = 0;

#pragma region CH3
	// Channel 3 (Wave)
	if (add == 0xFF1A) {
		if (CH3Enable) {
			value = 0xFF;
		}
		else {
			value = 0;
		}
	} else

	if (add == 0xFF1B) {
		value = (uint8_t)(-CH3TimeToLife / (1.0 / 256.0)) + 256;
	} else

	if (add == 0xFF1C) {
		switch (CH3ShiftVol) {
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
	} else

	if (add == 0xFF1D) {
		value = CH3RawFreq & 0xFF;
	} else

	if (add == 0xFF1E) {
		if (CH3Stopping) {
			value |= 0x40;
		}
		value |= (CH3RawFreq >> 8) & 0x7;
	}
#pragma endregion

	return value;
}
