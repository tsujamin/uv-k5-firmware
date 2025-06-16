/* Copyright 2025 Benjamin Roberts
 * https://github.com/tsujamin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "modem.h"
#include "driver/uart.h"
#include "driver/bk4819.h"
#include "external/printf/printf.h"
#include <strings.h>

#if !defined(ENABLE_UART)
#error "Must ENABLE_UART to ENABLE_MODEM"
#endif

void Modem_Init(void)
{
    // Dump modem registers to UART
    uint16_t rReg = 0;
    char szBuf[256];
    int cBuf = 0;
    
    for (uint16_t i = 0x58; i <= 0x5E; i++)
    {
        rReg = BK4819_ReadRegister(BK4819_REG_00 + i);
        cBuf = snprintf(szBuf, sizeof(szBuf), "BK4819_REG_%02hX=0x%04hX\r\n", i, rReg);
        
        UART_Send(szBuf, cBuf + 1);
    }

    return;
}

