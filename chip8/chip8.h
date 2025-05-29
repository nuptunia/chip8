#pragma once
#include <cstdint>
#include <array>
#include <string>

class Chip8 {
public:
	Chip8();
	void LoadROM(const std::string& filename); 
	void Cycle(); 
	uint8_t gfx[64 * 32];		      // 64x32 display (monochrome) 
	bool drawFlag = false; 

private:
	uint16_t opcode; 
	std::array<uint8_t, 4096> memory; // 4kb memory
	std::array<uint8_t, 16> V;		  // opcodes 
	uint16_t I;
	uint16_t pc;					  // program counter
	std::array<uint16_t, 16> stack; 
	uint16_t stackptr;
	uint8_t delayTimer;
	uint8_t soundTimer; 

	void Initialize();
};
