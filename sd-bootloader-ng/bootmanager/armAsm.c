#include "armAsm.h"
#include <stddef.h>
#include "logger.h"

//A8.3 Conditional execution
#define COND_EQ 0b0000 //Equal Equal Z == 1
#define COND_NE 0b0001 //Not equal Not equal, or unordered Z == 0
#define COND_CS 0b0010 //b Carry set Greater than, equal, or unordered C == 1
#define COND_CC 0b0011 //c Carry clear Less than C == 0
#define COND_MI 0b0100 //Minus, negative Less than N == 1
#define COND_PL 0b0101 //Plus, positive or zero Greater than, equal, or unordered N == 0
#define COND_VS 0b0110 //Overflow Unordered V == 1
#define COND_VC 0b0111 //No overflow Not unordered V == 0
#define COND_HI 0b1000 //Unsigned higher Greater than, or unordered C == 1 and Z == 0
#define COND_LS 0b1001 //Unsigned lower or same Less than or equal C == 0 or Z == 1
#define COND_GE 0b1010 //Signed greater than or equal Greater than or equal N == V
#define COND_LT 0b1011 //Signed less than Less than, or unordered N != V
#define COND_GT 0b1100 //Signed greater than Greater than Z == 0 and N == V
#define COND_LE 0b1101 //Signed less than or equal Less than, equal, or unordered Z == 1 or N != V
#define COND_AL 0b1110 //Always (unconditional) Always (unconditional) Any

//A8.6.16 B
#define INST_B_T1 0b1101000000000000
#define INSM_B_T1 0b1111000000000000
#define INST_B_T2 0b1110000000000000
#define INSM_B_T2 0b1111100000000000
//A8.6.23 BL, BLX (immediate)
#define INST_BL_T1 0b1111000000000000110100000000
#define INSM_BL_T1 0b1111100000000000110100000000

#define INST_BLX_T2 0b1111000000000000110000000000
#define INSM_BLX_T2 0b1111100000000000110100000001

static void conv_ui_ca32(uint32_t num, char arr[4]) {
  arr[0] = (0xff&(num));
  arr[1] = (0xff&(num>>8));
  arr[2] = (0xff&(num>>16));
  arr[3] = (0xff&(num>>24));
}
static uint32_t conv_ca_ui32(char arr[4]) {
  return (arr[3]<<24) + (arr[2]<<16) + (arr[1]<<8) + arr[0];
}
static void conv_ui_ca16(uint16_t num, char arr[2]) {
  arr[0] = (0xff&(num));
  arr[1] = (0xff&(num>>8));
}
static uint16_t conv_ca_ui16(char arr[2]) {
  return (arr[1]<<8) + arr[0];
}

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

static void armAsmT_bl_blx(uint32_t pc, uint32_t target, bool x, char instruction[4]) { 
  //A8.6.23 BL, BLX (immediate) T1/T2
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

  uint32_t imm11 = get_bits(offset, 1, 11); //BL
  uint32_t imm10l = get_bits(offset, 2, 11); //BLX
  //uint32_t last2 = get_bits(offset, 0, 1); //BLX

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

	//static 0b11
	result = set_bits(result, 14, 0x3, 2);

	//j1 
	result = set_bit(result, 13, j1);

	//blx = 0x0, bl = 0x1 
  uint8_t x_flag = 0x1;
  if (x)
    x_flag = 0x0;
	result = set_bit(result, 12, x_flag);

	//j2 
	result = set_bit(result, 11, j2);

  if (x) { //BLX
    //imm10l
    result = set_bits(result, 1, imm10l, 10);

    //last bit
    result = set_bit(result, 0, 0x0);
  } else { //BL
    //imm11
    result = set_bits(result, 0, imm11, 11);
  }

/*
	uint32_t top = 0xffff0000 & result;
	uint32_t bot = 0x0000ffff & result;
	top = swap_bytes_word(top>>16);
	bot = swap_bytes_word(bot);

	result = top << 16 | bot;
  */
 
	uint32_t top = 0xffff0000 & result;
	uint32_t bot = 0x0000ffff & result;

	result = (top >> 16) + (bot << 16);
  

  conv_ui_ca32(result, instruction);
}
static void armDasmT_bl_blx(uint32_t pc, char instruction[4], bool x, uint32_t* target) {
  //assert(!x && (INST_BL_T1 == INSM_BL_T1&instruction32));
  //assert(x && (INST_BLX_T2 == INSM_BLX_T2&instruction32));
  /*
  //swap endi

  uint32_t imm10h = get_bits(instruction32, 12, 21);
  uint32_t imm10l = get_bits(instruction32, 1, 10); //BLX
  uint32_t imm11 = get_bits(instruction32, 0, 10); //BL
  uint32_t imm32;
  if (x) { //BLX
    imm32 = (imm10l<<2);
  } else { //BL
    imm32 = (imm11<<1);
  }
  imm32 += (imm10h<<12);
  imm32 += (i2<<13);
  imm32 += (i1<<14);
  imm32 += (s<<15);

  *target = imm32;*/
}

static void armAsmT_b(uint32_t pc, uint32_t target, uint8_t condition, char instruction[2]) { 
  //A8.6.16 B - T1+T2
  
  uint16_t result = 0;

  pc = pc + 4; //Arm pipeline preloading
  int32_t offset = target - pc;

  if (condition == COND_AL) {
    uint16_t imm11 = (uint16_t)(offset>>1);

    result = set_bits(result, 11, 0b11100, 5);
    result = set_bits(result, 0, imm11, 11);
  } else {
    uint8_t imm8 = (uint8_t)(offset>>1);

    result = set_bits(result, 12, 0b1101, 4);
    result = set_bits(result, 8, condition, 4);
    result = set_bits(result, 0, imm8, 8);
  }

  conv_ui_ca16(result, instruction);
}
static void armDasmT_b1(uint32_t pc, uint16_t instruction16, uint32_t* target, uint8_t* condition) {
  int32_t offset = get_bits(instruction16, 0, 7);
  offset = offset<<1;

  if (condition != NULL)
    *condition = get_bits(instruction16, 8, 11);
  if (target != NULL)
    *target = offset + pc+4; //Arm pipeline preloading
}
static void armDasmT_b2(uint32_t pc, uint16_t instruction16, uint32_t* target) {
  int32_t offset = get_bits(instruction16, 0, 11);
  offset = offset<<1;

  if (target != NULL)
    *target = offset + pc+4; //Arm pipeline preloading
}

void ArmAsmT_bl(uint32_t pc, uint32_t target, char instruction[4]) { 
  armAsmT_bl_blx(pc, target, false, instruction);
  Logger_debug("ArmAsmT_bl: pc=0x%x target=0x%x | instr=%02x%02x%02x%02x", pc, target, instruction[0], instruction[1], instruction[2], instruction[3]);

}
void ArmAsmT_blx(uint32_t pc, uint32_t target, char instruction[4]) { 
  armAsmT_bl_blx(pc, target, true, instruction);
  Logger_debug("ArmAsmT_blx: pc=0x%x target=0x%x | instr=%02x%02x%02x%02x", pc, target, instruction[0], instruction[1], instruction[2], instruction[3]);
}
void ArmAsmT_b(uint32_t pc, uint32_t target, char instruction[2]) { 
  armAsmT_b(pc, target, COND_AL, instruction);
  Logger_debug("ArmAsmT_b: pc=0x%x target=0x%x | instr=%02x%02x", pc, target, instruction[0], instruction[1]);
}
void ArmAsmT_bne(uint32_t pc, uint32_t target, char instruction[2]) { 
  armAsmT_b(pc, target, COND_NE, instruction);
  Logger_debug("ArmAsmT_bne: pc=0x%x target=0x%x | instr=%02x%02x", pc, target, instruction[0], instruction[1]);
}
void ArmDasmT(uint32_t pc, char instruction[4], uint32_t* target, uint8_t* condition) {
  uint16_t instr16 = conv_ca_ui16(instruction);
  uint32_t instr32 = conv_ca_ui32(instruction);

  if (INST_B_T1 == (INSM_B_T1&instr16)) {
    armDasmT_b1(pc, instr16, target, condition);
    Logger_debug("armDasmT_b1: pc=0x%x instr=%02x%02x | target=0x%x condition=0x%x", pc, instruction[0], instruction[1], *target, *condition);
  } else if (INST_B_T2 == (INSM_B_T2&instr16)) {
    armDasmT_b2(pc, instr16, target);
    Logger_debug("armDasmT_b2: pc=0x%x instr=%02x%02x | target=0x%x", pc, instruction[0], instruction[1], *target);
  } else if (INST_BL_T1 == INSM_BL_T1&instr32) {
    Logger_fatal("ArmDasmT for INST_BL_T1 not implemented");
    //armDasmT_bl_blx(pc, instruction, false, target);
  } else if (INST_BLX_T2 == INSM_BLX_T2&instr32) {
    Logger_fatal("ArmDasmT for INSM_BLX_T2 not implemented");
    //armDasmT_bl_blx(pc, instruction, true, target);
  }
}