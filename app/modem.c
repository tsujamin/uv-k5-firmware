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

typedef struct {
    bool               UARTLoggingState : 1;
    uint16_t           PacketBuffer[36];
    BK4819_ModemParams ModemParams;
} ModemState;

ModemState gModemState = {
    .UARTLoggingState = 0,
    .PacketBuffer = { 0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01,
                      0x55, 0xff, 0x55, 0x01 },
    .ModemParams = {
        .BaudRate       = 1200,
        .PreambleLength = BK4819_REG_59_FSK_PREAMBLE_LENGTH_7B,
        .SyncLength     = BK4819_REG_59_FSK_SYNC_LENGTH_2B,
        .RxBandwidth    = BK4819_REG_58_FSK_RX_BANDWIDTH_FSK_1200,
        .TxMode         = BK4819_REG_58_FSK_TX_MODE_FSK_1200,
        .RxMode         = BK4819_REG_58_FSK_RX_MODE_FSK_1200,
        .SyncBytes      = {0x55, 0x44, 0x33, 0x22},
        .TxMode         = 0,
        .RxMode         = 0
    }
};

void Modem_Boot(void)
{
#if defined(MODEM_DEBUG)
    const char szMsg[] = "Modem_Boot()\r\n";
    UART_LogSend(szMsg, sizeof(szMsg));
    
    // Dump modem registers to UART
    uint16_t rReg = 0;
    char szBuf[256];
    int cBuf = 0;
    
    for (uint16_t i = 0x58; i <= 0x5E; i++)
    {
        rReg = BK4819_ReadRegister(BK4819_REG_00 + i);
        cBuf = snprintf(szBuf, sizeof(szBuf), "BK4819_REG_%02hX=0x%04hX\r\n", i, rReg);
        
        UART_LogSend(szBuf, cBuf + 1);
    }
#endif

    // Cache initial UART logging state.
    gModemState.UARTLoggingState = UART_IsLogEnabled;

    return;
}

void Modem_Init()
{
    // Disable UART logging so the modem isn't interupted.
    gModemState.UARTLoggingState = UART_IsLogEnabled;
    UART_IsLogEnabled = false;

    // Set up the radio
    RADIO_SelectVfos();
	RADIO_SetupRegisters(false); // TODO: this probably needs to be rewritten for Modem.

    // Initialise the FSK interupts etc
    BK4819_ConfigureFSK(&gModemState.ModemParams);

    // Beep and show the UI
    gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP;

    return;
}

void Modem_Exit()
{
    // Resture UART logging
    if (gModemState.UARTLoggingState == true)
    {
        UART_IsLogEnabled = true;
    }

    // Return to the main/foreground display/function.
    FUNCTION_Select(FUNCTION_FOREGROUND);
    gRequestDisplayScreen = DISPLAY_MAIN;
    gFlagReconfigureVfos = true;
}

void Modem_HandleInterupts(uint16_t InteruptMask)
{
    return;
}

void Modem_TestTx(void)
{
    RADIO_SetTxParameters();
    BK4819_EnableTXLink();
    BK4819_StartTransmitFSK(&gModemState.ModemParams, (uint8_t*)gModemState.PacketBuffer, sizeof(gModemState.PacketBuffer));
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
    Modem_Exit();

    return;
}

#if defined(MODEM_DEBUG)
void UpdateParameter(KEY_Code_t Key)
{
    // This is a hacky way to sweep particular registry values using the keypad
    if (Key > KEY_9) {
        return;
    }
    // Preamble Length
    // gModemState.ModemParams.PreambleLength = Key << BK4819_REG_59_SHIFT_FSK_PREAMBLE_LENGTH;
    
    // Baud rate
    // gModemState.ModemParams.BaudRate = (Key == KEY_2) ? 2400 : 1200;
    
    // Tx Mode (3 bits, so map 0->7)
    // if ( Key <= 0b111 )
    // {
    //     gModemState.ModemParams.TxMode = Key << BK4819_REG_58_SHIFT_FSK_TX_MODE;
    // }

    // Preamble Type
    // if ( Key <= 0b11 )
    // {
    //     gModemState.ModemParams.PremableType = Key << BK4819_REG_58_SHIFT_FSK_PREAMBLE_TYPE;
    // }

    // Inversion
    // if (Key == KEY_8)
    // {
    //     gModemState.ModemParams.InvertTx = 0;
    // }
    // else if (Key == KEY_9)
    // {
    //     gModemState.ModemParams.InvertTx = 1;
    // }

    // The unknown 2 bits in REG_58
    // if (Key <= 0b11)
    // {
    //     gModemState.ModemParams.REG_58_67_UNKNOWN = Key;
    // }

    // The unknown 3 bits in REG_59
    // 0b0XX looks unchanged
    // 0b110 looks scrambled
    // 0b10X pins to high or low frequency respectively
    // 0b111 is modulates 1010101010
    if (Key <= 0b111)
    {
        gModemState.ModemParams.REG_59_02_UNKNOWN = Key;
    }

    // Sweep bits of the 5C register
    // if (Key <= 0b111)
    // {
    //     // Nothing obvious
    //     // gModemState.ModemParams.REG_5C_UNKNOWN = (Key << 0) & ~((uint16_t)BK4819_REG_5C_FSK_ENABLE_CRC);
    //     gModemState.ModemParams.REG_5C_UNKNOWN = ((uint16_t)Key << 13) & ~((uint16_t)BK4819_REG_5C_FSK_ENABLE_CRC);
    // } else if (Key == KEY_8)
    // {
    //     // From MDC1200 
    //     gModemState.ModemParams.REG_5C_UNKNOWN = 0xAA30;
    // } else if (Key == KEY_9)
    // {
    //     // From AirCopy, but withour CRC
    //     gModemState.ModemParams.REG_5C_UNKNOWN = 0x5625 & ~((uint16_t)BK4819_REG_5C_FSK_ENABLE_CRC);
    // }
    // else
    // {
    //     gModemState.ModemParams.REG_5C_UNKNOWN = 0;
    // }

    if (Key == KEY_9)
    {

        gModemState.ModemParams.Scramble = 1;
    }
    else
    {
        gModemState.ModemParams.Scramble = 0;
    }
    
    BK4819_ConfigureFSK(&gModemState.ModemParams);
    
    return;
}
#endif

void Modem_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
    switch (Key)
    {
    case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
        if (bKeyPressed)
        {
#if defined(MODEM_DEBUG)
            UpdateParameter(Key);
            Modem_TestTx();
#endif
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