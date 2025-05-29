# include "chip8.h"
# include "fstream" // file stream 
# include <iostream>
# include <cstring>

Chip8::Chip8() {
	Initialize();
}

void Chip8::Initialize() {
	pc = 0x200;
	opcode = 0;
	I = 0;
	stackptr = 0;

	memory.fill(0);
	V.fill(0);
	stack.fill(0);
	std::memset(gfx, 0, sizeof(gfx)); 

	delayTimer = 0;
	soundTimer = 0;

	uint8_t fontset[80] = {
		0xf0, 0x90, 0x90, 0x90, 0x90, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
		0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
		0x90, 0x90, 0xf0, 0x10, 0x10, // 4
		0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
		0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
		0xf0, 0x10, 0x20, 0x40, 0x40, // 7 
		0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8 
		0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9 
		0xf0, 0x90, 0xf0, 0x90, 0x90, // A 
		0xe0, 0x90, 0xe0, 0x90, 0xe0, // B 
		0xf0, 0x80, 0x80, 0x80, 0xf0, // C
		0xe0, 0x90, 0x90, 0x90, 0xe0, // D 
		0xf0, 0x80, 0xf0, 0x80, 0xf0, // E 
		0xf0, 0x80, 0xf0, 0x80, 0x80, // F
	};

	for (int i = 0; i < 80; ++i) {
		memory[0x50 + i] = fontset[i];
	}
}

void Chip8::LoadROM(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate); 
	if (!file) {
		std::cerr << "Failed to read " << filename << "\n";
		return;
	}

	std::streamsize size = file.tellg(); 
	file.seekg(0, std::ios::beg); 
	file.read(reinterpret_cast<char*>(&memory[0x200]), size); 

	std::cout << "Loaded ROM: " << size << "bytes\n";
}

void Chip8::Cycle() {
	opcode = memory[pc] << 8 | memory[pc + 1]; 
	std::cout << "Opcode: " << std::hex << opcode << "\n";

	switch (opcode & 0xf000) {
	case 0x0000:
		switch (opcode & 0x00ff) {
		case 0x00e0: // clear the screen | cls
			std::memset(gfx, 0, sizeof(gfx));
			pc += 2;
			break;
		case 0x00ee:
			--stackptr;
			pc = stack[stackptr];
			pc += 2;
			break;
		default:
			std::cout << "ERROR: Unknown Opcode [0x0000]: " << std::hex << opcode << "\n";
			pc += 2;
			break;
		}
		break;
	
	case 0x1000: // jump address
		pc = opcode & 0x0fff;
		break;

	case 0x2000: // 2nnn call subroutine at nnn 
		stack[stackptr++] = pc;
		pc = opcode & 0x0fff;
		break;

	case 0x3000: { // vx == nn
		uint8_t X = (opcode & 0x0f00) >> 8;
		uint8_t NN = opcode & 0x00ff;
		if (V[X] == NN) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	}

	case 0x6000: { // 6xnn set vx = nn
		uint8_t X = (opcode & 0x0f00) >> 8;
		uint8_t NN = opcode & 0x00ff;
		V[X] = NN;
		pc += 2;
		break; }

	case 0x7000: { // 7xnn nn to vx
		uint8_t X = (opcode & 0x0f00) >> 8;
		uint8_t NN = opcode & 0x00ff;
		V[X] += NN;
		pc += 2;
		break; }
	
	case 0xf007: {
		uint8_t X = (opcode & 0x0f00) >> 8;
		V[X] = delayTimer;
		pc += 2;
		break;
	}

	case 0xa000:
		I = opcode & 0x0fff;
		pc += 2;
		break;

	case 0xd000: { // drawing and displaying

		std::cout << "[INFO] Drawing (DXYN): " << opcode << "\n";
		uint8_t x = V[(opcode & 0x0f00) >> 8];
		uint8_t y = V[(opcode & 0x00f0) >> 4];
		uint8_t height = opcode & 0x000f;
		V[0xf] = 0; // Reset collision flag

		for (int row = 0; row < height; ++row) {
			uint8_t spriteByte = memory[I + row];

			for (int col = 0; col < 8; ++col) {
				if ((spriteByte & (0x80 >> col)) != 0) {
					int xCoord = (x + col) % 64;
					int yCoord = (y + row) % 32;
					int index = xCoord + (yCoord * 64);

					if (gfx[index] == 1) {
						V[0xf] = 1; // Collision detected
					}
					gfx[index] ^= 1; // XOR pixel
				}
			}
		}
		drawFlag = true;
		pc += 2;
		break;
	}

	default:
		std::cout << "ERROR: Unknown Opcode: " << std::hex << opcode << "\n"; 
		pc += 2;
		break;
	}

	if (delayTimer > 0) --delayTimer;
	if (soundTimer > 0) --soundTimer;


}

/*
	case 0xf000: {
		uint8_t X = (opcode & 0x0f00) >> 8;
		switch (opcode & 0x00ff) {
		case 0x07:
			V[X] = delayTimer;
			pc += 2;
			break;
		}

	case 0x0a:
		for (int i = 0; i < 16; ++i) {
			if (key[i]) {
				V[X] = i;
				pc += 2;
				break;
			}
		}
		// stall
		break;

	case 0x15:
		delayTimer = V[X];
		pc += 2;
		break;

	case 0x18:
		soundTimer = V[X];
		pc += 2;
		break;

	case 0x1e:
		I += V[X];
		pc += 2;
		break;

	case 0x29:
		I = V[X] * 5;
		pc += 2;
		break;

	case 0x33:
		memory[I] = V[X] / 100;
		memory[I + 1] = (V[X] / 10) % 10;
		memory[I + 2] = V[X] % 10;
		pc += 2;
		break;

	case 0x55:
		for (int i = 0; i <= X; ++i)
			memory[I + i] = V[i];
		pc += 2;
		break;

	case 0x65:
		for (int i = 0; i <= X; ++i)
			V[I] = memory[i + I];
		pc += 2;
		break;

	default:
		std::cout << "ERROR: Unknown F000 opcode: " << std::hex << opcode << "\n";
		pc += 2;
		break;
	}

*/
