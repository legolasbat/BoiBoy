#pragma once

#include <cstdint>

class BoiBoy;

class PPU
{
public:
	void ConnectMem(BoiBoy* mem) { memory = mem; }

	void Clock(int cycles);

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);

	uint8_t fullScreen[256 * 256 * 3];
	// Gameboy screen
	uint8_t frameBufferA[160 * 144 * 4];
	uint8_t data[128 * 128 * 4];

	// 0xFF40
	uint8_t LCDC = 0x91;

	// 0xFF41
	uint8_t STAT = 0x85;

	// 0xFF42
	uint8_t SCY = 0;

	// 0xFF43
	uint8_t SCX = 0;

	// 0xFF44
	uint8_t LY = 0;

	// 0xFF45
	uint8_t LYC = 0;

	// 0xFF47
	uint8_t BGP = 0xFC;

	// 0xFF48
	uint8_t OBP0 = 0xFF;

	// 0xFF49
	uint8_t OBP1 = 0xFF;

	// 0xFF4A
	uint8_t WY = 0;

	// 0xFF4B
	uint8_t WX = 0;

	uint8_t* GetPixels();
	uint8_t* GetBackground();
	uint8_t* GetScreen();

	int mode = 1;
	int cnt = 28;

	bool frameComplete = false;

private:

	BoiBoy* memory = nullptr;

	int scanlineCycles = 0;

	int cntOffset = 0;

	// Grey
	//uint32_t pallete[4] = { 0x000000FF, 0x808080FF, 0xD3D3D3FF, 0xFFFFFFFF };
	// Inverse grey
	//uint32_t pallete[4] = { 0xFFFFFFFF, 0xD3D3D3FF, 0x808080FF, 0x000000FF };
	// Green
	uint32_t pallete[4] = { 0xE0F8D0FF, 0x88C070FF, 0x346856FF, 0x081820FF };

	// Memory range 0x8000 - 0x97FF
	uint8_t tileData[0x1800];

	// Memory range 0x9800 - 0x9BFF
	uint8_t tileMap0[0x400];

	// Memory range 0x9C00 - 0x9FFF
	uint8_t tileMap1[0x400];

	// Memory range 0xFE00 - 0xFE9F
	uint8_t OAM[0xA0];

	void WriteLCDC(uint8_t n);

	void GetBGRow(uint8_t r);
	void GetWinRow(uint8_t r);
	void GetSpRow(uint8_t r);

	void CopyBuffer(uint8_t r);

	void GetSpIndex();
	int spriteCont = 0;
	uint8_t spritesIndex[10];

	bool spriteFetch = false;
	bool drew = false;
	bool copy = false;

	int spriteHeight = 8;

	void CheckLY();

};

