#ifndef __ARMASMGEN_H__
#define __ARMASMGEN_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

void ArmAsm_bl(uint32_t pc, uint32_t target, uint32_t* instruction);
void ArmAsm_blx(uint32_t pc, uint32_t target, uint32_t* instruction);

#ifdef __cplusplus
}
#endif
#endif