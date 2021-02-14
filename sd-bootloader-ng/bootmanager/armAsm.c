#include "armAsm.h"

static uint16_t swap_bytes_word(uint16_t word) {
	uint16_t b0 = word & 0x00ff;
	uint16_t b1 = word & 0xff00;

	return (b0<<8|b1>>8);
}

static bool get_bit(uint32_t bits, uint8_t index) {
  uint32_t bitval = bits&(1<<index);
  return (bitval != 0);
}
static uint32_t set_bit(uint32_t value, uint8_t index, bool bit) {
  uint32_t mask = 1<<index;
  value &= ~mask;
  if (bit) {
    value |= mask;
  }
  return value;
}

static uint32_t set_bits(uint32_t value, uint8_t offset, uint32_t bits,  uint8_t nbits) {
  for (uint8_t i = 0; i < nbits; i++) {
    bool bit = get_bit(bits, i);
    value = set_bit(value, i+offset, bit);
  }
  return value;
}

static uint32_t get_bits(int32_t value, uint8_t start, uint8_t end) {
  //assert(end < 32);
  //assert(start >= 0);

  uint8_t mask_bits = end - start + 1;

  int32_t mask = 0; //TODO: Check signed/unsigned
  for (uint8_t i = 0; i < mask_bits; i++) {
    mask = mask + (1<<i);
  }

  return (uint32_t)((value>>start)&mask);
}

static void armAsmT_bl_blx(uint32_t pc, uint32_t target, uint32_t* instruction, bool x) { 
    //Base from
    //https://gist.github.com/jeremy-allen-cs/c93bd333b5b585c2b840 
    //https://www.cs.utexas.edu/~simon/378/resources/ARMv7-AR_TRM.pdf

    uint32_t result = 0;

    pc = pc + 4; //Arm pipeline preloading
    int32_t offset = target - pc;

    uint32_t s = get_bits(offset, 24, 24);
    uint32_t i1 = get_bits(offset, 23, 23);
    uint32_t i2 = get_bits(offset, 22, 22);
    uint32_t imm10h = get_bits(offset, 12, 21);
    uint32_t imm10l = get_bits(offset, 2, 11);
    uint32_t last2 = get_bits(offset, 0, 1);

    //Mask all but the last bit
    uint32_t m = 0xfffffffe;
    uint32_t j1 = (i1 ^ 0x1); // bitwise not
    uint32_t j2 = (i2 ^ 0x1); // bitwise not
    j1 = j1 ^ s;
    j2 = j2 ^ s;

	//bits 27-31 = 0b11110
	result = set_bits(result, 27, 0x1e, 5);

	//bit 26 = S
	result = set_bit(result, 26, s);

	//imm10h, bits 16-25
	result = set_bits(result, 16, imm10h, 10);

	//static
	result = set_bits(result, 14, 0x3, 2);

	//j1 
	result = set_bit(result, 13, j1);

	//blx = 0x0, bl = 0x1 
    uint8_t x_flag = 0x0;
    if (!x)
	    x_flag = 0x1;
	result = set_bit(result, 12, x_flag);

	//j2 
	result = set_bit(result, 11, j2);

	//imm10l
	result = set_bits(result, 1, imm10l, 10);

	//last bit
	result = set_bit(result, 0, 0x0);

	uint32_t top = 0xffff0000 & result;
	uint32_t bot = 0x0000ffff & result;
	top = swap_bytes_word(top>>16);
	bot = swap_bytes_word(bot);

	result = top << 16 | bot;

    *instruction = result;
}
void ArmAsmT_bl(uint32_t pc, uint32_t target, uint32_t* instruction) { 
    armAsmT_bl_blx(pc, target, instruction, false);
}
void ArmAsmT_blx(uint32_t pc, uint32_t target, uint32_t* instruction) { 
    armAsmT_bl_blx(pc, target, instruction, true);
}
void ArmAsmT_b(uint32_t pc, uint32_t target, uint16_t* instruction) { 
    //A8.6.16 B
}