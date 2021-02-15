#ifndef __ARMASMGEN_H__
#define __ARMASMGEN_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

void ArmAsmT_bl(uint32_t pc, uint32_t target, char instruction[4]);
void ArmAsmT_blx(uint32_t pc, uint32_t target, char instruction[4]);
void ArmAsmT_b(uint32_t pc, uint32_t target, char instruction[2]);
void ArmAsmT_bne(uint32_t pc, uint32_t target, char instruction[2]);

void ArmDasmT(uint32_t pc, char instruction[4], uint32_t* target, uint8_t* condition);

#ifdef __cplusplus
}
#endif
#endif