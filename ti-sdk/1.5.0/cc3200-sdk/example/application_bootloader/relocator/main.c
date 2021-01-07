//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
// Initial location for boot manager
//*****************************************************************************
#define  INIT_LOCATION          0x20004100

//*****************************************************************************
// Final location for boot manager
//*****************************************************************************
#define  RELOC_LOCATION         0x20000000


//*****************************************************************************
// Pointer to destination location
//*****************************************************************************
unsigned long * const __init_location = (unsigned long *)INIT_LOCATION;


//*****************************************************************************
// Pointer to destination location
//*****************************************************************************
unsigned long * const __recloc_location = (unsigned long *)RELOC_LOCATION;

//*****************************************************************************
// Fucntion Decleration
//*****************************************************************************
void RunRelocated(unsigned long ulBaseLoc);

//*****************************************************************************
//
//! Executed the application from given location
//!
//! \param ulBaseLoc is the base address of the application
//!
//! This function execution the application loaded at \e ulBaseLoc. It assumes
//! the vector table is placed at the base adderss thus sets the new Stack
//! pointer from the first word.
//!
//! \return None.
//
//*****************************************************************************
#ifndef ccs
void RunRelocated(unsigned long ulBaseLoc)
{

  //
  // Set the SP
  //
  __asm("	ldr    sp,[r0]\n"
	"	add    r0,r0,#4");

  //
  // Jump to entry code
  //
  __asm("	ldr    r1,[r0]\n"
        "	bx     r1");
}
#else
__asm("    .sect \".text:RunRelocated\"\n"
      "    .clink\n"
      "    .thumbfunc RunRelocated\n"
      "    .thumb\n"
      "RunRelocated:\n"
      "    ldr    sp,[r0]\n"
      "    add    r0,r0,#4\n"
      "    ldr    r1,[r0]\n"
      "    bx     r1");
#endif

//*****************************************************************************
// Main entry function
//*****************************************************************************
void main()
{
  unsigned long ulNdx;

  for( ulNdx =0; ulNdx < 4*1024; ulNdx++)
  {
      __recloc_location[ulNdx] =  __init_location[ulNdx];
  }

  RunRelocated(RELOC_LOCATION);

  while(1)
  {

  }
}
