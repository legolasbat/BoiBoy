#pragma once

#include <SFML/Audio.hpp>
#include <SDL_audio.h>
#include <iostream>
#include <vector>

#include "WaveSound.h"
#include "SquareSound.h"
#include "NoiseSound.h"

class SPU
{
public:
	SPU();

	void Clock(int cycles);

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

	int masterVolume = 300;

	bool channel1 = true;
	bool channel2 = true;
	bool channel3 = true;
	bool channel4 = true;

private:

	//sf::Sound outputAudio;
	//sf::SoundBuffer buffer;

	std::vector<int16_t> samples;
	int maxSamples = 2048;
	int samplesIndex = 0;

	int cycleCounter = 0;
	int sequenceCounter = 0;
	int sequencer = 0;

	bool soundEnable = false;

	uint8_t leftVol = 0;
	uint8_t rightVol = 0;

	uint8_t soundOutput = 0;

	// CH1
	SquareSound ch1;

	// CH2
	SquareSound ch2;

	// CH3
	WaveSound ch3;

	// CH4
	NoiseSound ch4;
};

