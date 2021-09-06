#include "SPU.h"

SPU::SPU() {
	samples.resize(maxSamples);

	SDL_AudioSpec audioSpec;
	
	audioSpec.freq = 48000;
	audioSpec.format = AUDIO_S16SYS;
	audioSpec.channels = 2;
	audioSpec.samples = maxSamples/2;

	SDL_OpenAudio(&audioSpec, NULL);
	SDL_PauseAudio(0);
}

void SPU::Clock(int cycles)
{
	if (!soundEnable)
		return;

	for (int i = 0; i < cycles; i++) {

		sequenceCounter++;
		if (sequenceCounter >= 8192) {
			sequenceCounter = 0;

			switch (sequencer) {
			case 0:
				ch1.LengthClock();
				ch2.LengthClock();
				ch3.LengthClock();
				ch4.LengthClock();
				break;
			case 1:
				break;
			case 2:
				ch1.LengthClock();
				ch2.LengthClock();
				ch3.LengthClock();
				ch4.LengthClock();
				break;
			case 3:
				break;
			case 4:
				ch1.LengthClock();
				ch2.LengthClock();
				ch3.LengthClock();
				ch4.LengthClock();
				break;
			case 5:
				break;
			case 6:
				ch1.SweepClock();
				ch1.LengthClock();
				ch2.LengthClock();
				ch3.LengthClock();
				ch4.LengthClock();
				break;
			case 7:
				ch1.EnvClock();
				ch2.EnvClock();
				ch4.EnvClock();
				break;
			}

			sequencer = (sequencer + 1) % 8;
		}

		ch1.Clock();
		ch2.Clock();
		ch3.Clock();
		ch4.Clock();

		cycleCounter += 48000;
		if (cycleCounter >= 4194304) {
			cycleCounter -= 4194304;

			int16_t sample = 0;

			// CH1 Left
			if ((soundOutput & 0x10) == 0x10 && channel1) {
				sample += ch1.GetSample();
			}
			// CH2 Left
			if ((soundOutput & 0x20) == 0x20 && channel2) {
				sample += ch2.GetSample();
			}
			// CH3 Left
			if ((soundOutput & 0x40) == 0x40 && channel3) {
				sample += ch3.GetSample();
			}
			// CH4 Left
			if ((soundOutput & 0x80) == 0x80 && channel4) {
				sample += ch4.GetSample();
			}
			samples.at(samplesIndex++) = sample * (masterVolume * (leftVol/7));
			sample = 0;

			// CH1 Right
			if ((soundOutput & 0x01) == 0x01 && channel1) {
				sample += ch1.GetSample();
			}
			// CH2 Right
			if ((soundOutput & 0x02) == 0x02 && channel2) {
				sample += ch2.GetSample();
			}
			// CH3 Right
			if ((soundOutput & 0x04) == 0x04 && channel3) {
				sample += ch3.GetSample();
			}
			// CH4 Right
			if ((soundOutput & 0x08) == 0x08 && channel4) {
				sample += ch4.GetSample();
			}
			samples.at(samplesIndex++) = sample * (masterVolume * (rightVol / 7));

			if (samplesIndex >= maxSamples) {
				samplesIndex = 0;

				while (SDL_GetQueuedAudioSize(1) > maxSamples * sizeof(int16_t)) {
					sf::sleep(sf::microseconds(1));
				}

				SDL_QueueAudio(1, samples.data(), samples.size() * sizeof(int16_t));
			}
		}
	}
}

void SPU::Write(uint16_t add, uint8_t n)
{
	// Channel 1 (Tone and Sweep)
	if (add >= 0xFF11 && add <= 0xFF14) {
		ch1.Write(add, n);
	}

	// Channel 2 (Tone)
	if (add >= 0xFF16 && add <= 0xFF19) {
		ch2.Write(add, n);
	}

	// Channel 3
	if (add >= 0xFF1A && add <= 0xFF1E) {
		ch3.Write(add, n);
	}

	// Channel 4
	if (add >= 0xFF20 && add <= 0xFF23) {
		ch4.Write(add, n);
	}

	// Control
	if (add == 0xFF24) {
		leftVol = (n & 0x70) >> 4;
		rightVol = (n & 0x07);
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
		ch3.Write(add, n);
		return;
	}
}

uint8_t SPU::Read(uint16_t add)
{
	uint8_t value = 0xFF;

	// Channel 1 (Tone and Sweep)
	if (add >= 0xFF10 && add <= 0xFF14) {
		value = ch1.Read(add);
	}
	// Channel 2 (Tone)
	if (add >= 0xFF16 && add <= 0xFF19) {
		value = ch2.Read(add);
	}
	// Channel 3
	else if (add >= 0xFF1A && add <= 0xFF1E) {
		value = ch3.Read(add);
	}
	// Channel 4
	if (add >= 0xFF20 && add <= 0xFF23) {
		value = ch4.Read(add);
	}

	return value;
}
