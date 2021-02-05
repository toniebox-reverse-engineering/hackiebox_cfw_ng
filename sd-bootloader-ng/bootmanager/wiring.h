#ifndef __WIRING_H__
#define __WIRING_H__
#ifdef __cplusplus
extern "C"
{
#endif

void xPortSysTickHandler(void);
unsigned long micros(void);
unsigned long millis(void);

#ifdef __cplusplus
}
#endif
#endif