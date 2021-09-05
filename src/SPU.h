#pragma once

#include <SFML/Audio.hpp>
#include <SDL_audio.h>
#include <iostream>
#include <cstdint>
#include <vector>

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
	int maxSamples = 4096;
	int samplesIndex = 0;

	int cycleCounter = 0;
	int sequenceCounter = 0;
	int sequencer = 0;

	bool soundEnable = false;

	uint8_t leftVol = 0;

	uint8_t soundOutput = 0;

	// CH1
	bool CH1Enable = false;
	bool CH1Play = false;
	int CH1Duty = 0;
	double CH1TimeToLife = 0.0;
	bool CH1Stopping = false;
	uint16_t CH1RawFreq = 0;
	int CH1Freq = 0;
	int CH1CurrentFreq = 0;
	int CH1DutyPointer = 0;
	uint8_t CH1Vol = 0;
	uint8_t CH1CurrentVol = 0;
	bool CH1EnvActive = false;
	bool CH1EnvDir = false;
	int CH1EnvPer = 0;
	int CH1CurrentEnvPer = 0;

	uint8_t CH1Output = 0;

	// CH2
	bool CH2Enable = false;
	bool CH2Play = false;
	int CH2Duty = 0;
	double CH2TimeToLife = 0.0;
	bool CH2Stopping = false;
	uint16_t CH2RawFreq = 0;
	int CH2Freq = 0;
	int CH2CurrentFreq = 0;
	int CH2DutyPointer = 0;
	uint8_t CH2Vol = 0;
	uint8_t CH2CurrentVol = 0;
	bool CH2EnvActive = false;
	bool CH2EnvDir = false;
	int CH2EnvPer = 0;
	int CH2CurrentEnvPer = 0;

	uint8_t CH2Output = 0;

	// CH3
	bool CH3Enable = false;
	bool CH3Play = false;
	int CH3ShiftVol = 0;
	double CH3TimeToLife = 0.0;
	bool CH3Stopping = false;
	uint16_t CH3RawFreq = 0;
	int CH3Freq = 0;
	int CH3CurrentFreq = 0;
	int CH3WavePointer = 0;

	uint8_t CH3Output = 0;



	uint8_t WaveRAM[0x10];

	int const WaveDuty[4][8] = {
		{1, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 0, 0}
	};

	void CH1Clock();
	void CH2Clock();
	void CH3Clock();

	void CH1Env();
	void CH2Env();

	void RestartCH1();
	void RestartCH2();
	void RestartCH3();

	/*
	virtual bool onGetData(Chunk& data) {

		data.samples = samples.data() + sampleSize * region;
		data.sampleCount = sampleSize;
		region = (region + 1) % (maxSamples / sampleSize);

		while (getPlayingOffset().asMicroseconds() > time.asMicroseconds() + sf::microseconds(16670).asMicroseconds()) {
			//std::cout << getPlayingOffset().asMicroseconds() << ", " << time.asMicroseconds() << std::endl;
		}

		time = getPlayingOffset();

		//std::cout << time.asMicroseconds() << std::endl;

		

		return true;
	}

	virtual void onSeek(sf::Time timeOffset) {}*/

};

