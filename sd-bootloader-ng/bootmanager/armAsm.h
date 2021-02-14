#ifndef __ARMASMGEN_H__
#define __ARMASMGEN_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

void ArmAsmT_bl(uint32_t pc, uint32_t target, uint32_t* instruction);
void ArmAsmT_blx(uint32_t pc, uint32_t target, uint32_t* instruction);

#ifdef __cplusplus
}
#endif
#endif