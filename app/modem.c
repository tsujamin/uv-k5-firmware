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

#include "audio.h"
#include "app/modem.h"
#include "driver/uart.h"
#include "driver/bk4819.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "ui/ui.h"

#if !defined(ENABLE_UART)
#error "Must ENABLE_UART to ENABLE_MODEM"
#endif

uint16_t gModemPacket [36] = { 0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01,
                               0x55, 0x00, 0x55, 0x01 };

BK4819_ModemParams gModemParameters = {
    .BaudRate       = 1200,
    .PreambleLength = BK4819_REG_59_FSK_PREAMBLE_LENGTH_7B,
    .SyncLength     = BK4819_REG_59_FSK_SYNC_LENGTH_2B,
    .RxBandwidth    = BK4819_REG_58_FSK_RX_BANDWIDTH_FSK_1200,
    .TxMode         = BK4819_REG_58_FSK_TX_MODE_FSK_1200,
    .RxMode         = BK4819_REG_58_FSK_RX_MODE_FSK_1200,
    .SyncBytes      = {0x55, 0x44, 0x33, 0x22}
};
void Modem_Boot(void)
{
    const char szMsg[] = "Modem_Boot()\r\n";
    UART_Send(szMsg, sizeof(szMsg));
    
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

void Modem_Init()
{
    const char szMsg[] = "Modem_Init()\r\n";
    UART_Send(szMsg, sizeof(szMsg));

    // Set up the radio
    RADIO_SelectVfos();
	RADIO_SetupRegisters(false); // TODO: this probably needs to be rewritten for Modem.

    // Initialise the FSK interupts etc
    BK4819_ConfigureFSK(&gModemParameters);

    // Beep and show the UI
    gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP;

    return;
}

void Modem_HandleInterupts(uint16_t InteruptMask)
{
    return;
}

void Modem_TestTx(void)
{
    const char szMsg[] = "Modem_TestTx()\r\n";
    UART_Send(szMsg, sizeof(szMsg));

    RADIO_SetTxParameters();
    BK4819_EnableTXLink();
    BK4819_StartTransmitFSK(&gModemParameters, (uint8_t*)gModemPacket, sizeof(gModemPacket));
	BK4819_SetupPowerAmplifier(0, 0);
	BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);

    // TODO: Unclear why it idles back to main screen.

    return;
}

//
// Key Handling
//
void Modem_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
    // Return to the main/foreground display/function.
    FUNCTION_Select(FUNCTION_FOREGROUND);
    gRequestDisplayScreen = DISPLAY_MAIN;
    gFlagReconfigureVfos = true;

    return;
}

void UpdateParameter(KEY_Code_t Key)
{
    // This is a hacky way to sweep particular registry values using the keypad
    if (Key > KEY_9) {
        return;
    }

    // Preamble Length
    // gModemParameters.PreambleLength = Key << BK4819_REG_59_SHIFT_FSK_PREAMBLE_LENGTH;
    
    // Baud rate

    gModemParameters.BaudRate = ( Key == KEY_2) ? 2400 : 1200;
    BK4819_ConfigureFSK( &gModemParameters );
    return;
}

void Modem_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
    switch (Key)
    {
    case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
        if (bKeyPressed)
        {
            UpdateParameter(Key);
            Modem_TestTx();
        }
		break;
	case KEY_MENU:
		break;
	case KEY_EXIT:
		Modem_Key_EXIT(bKeyPressed, bKeyHeld);
		break;
	default:
		break;
	}

    return;
}