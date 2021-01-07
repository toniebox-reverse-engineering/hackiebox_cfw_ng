//*****************************************************************************
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the
//  distribution.
//
//  Neither the name of Texas Instruments Incorporated nor the names of
//  its contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission.
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

//****************************************************************************
//
//! \addtogroup app2
//! @{
//
//****************************************************************************

#include <stdlib.h>
#include "app.h"

#if defined(ccs)
#pragma DATA_SECTION(fptr_table, ".funcvec")
const s_fptr fptr_table =
#else
/* Function table */
const s_fptr fptr_table @ ".funcvec" =
#endif
{
    Init,
    Display,
    AddNums,
    SubNums
};

s_wptr *gp_wptr;

#ifdef EXE
int main(char *argc, char **argv)
{
    /* Dummy main */
    fptr_table.p_init(NULL);
    fptr_table.p_display("Dummy");
    fptr_table.p_add(0, 0);
    fptr_table.p_sub(0, 0);

    return 0;
}
#endif /* EXE */

/**/
int Init(s_wptr *p_wrapper)
{
    gp_wptr = p_wrapper;
    return 0;
}

/**/
void Display(char *p_app_name)
{
    gp_wptr->p_display(p_app_name);
}

/**/
int AddNums(int num_1, int num_2)
{
    gp_wptr->p_display("This function returns twice the summation of the arguments" );
    return 2 * (num_1 + num_2);
}

/**/
int SubNums(int num_1, int num_2)
{
    gp_wptr->p_display("This function returns twice the difference of arguments"
                       " 2*(first - second)");
    return 2 * (num_1 - num_2);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
