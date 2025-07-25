/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
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

#ifndef DRIVER_BK4819_h
#define DRIVER_BK4819_h

#include <stdbool.h>
#include <stdint.h>
#include "driver/bk4819-regs.h"

enum BK4819_AF_Type_t {
	BK4819_AF_MUTE = 0U,
	BK4819_AF_OPEN = 1U,
	BK4819_AF_ALAM = 2U,
	BK4819_AF_BEEP = 3U,
	BK4819_AF_CTCO = 6U,
	BK4819_AF_AM   = 7U,
	BK4819_AF_FSKO = 8U,
};

typedef enum BK4819_AF_Type_t BK4819_AF_Type_t;

enum BK4819_FilterBandwidth_t {
	BK4819_FILTER_BW_WIDE   = 0U,
	BK4819_FILTER_BW_NARROW = 1U,
};

typedef enum BK4819_FilterBandwidth_t BK4819_FilterBandwidth_t;

enum BK4819_CssScanResult_t {
	BK4819_CSS_RESULT_NOT_FOUND = 0U,
	BK4819_CSS_RESULT_CTCSS = 1U,
	BK4819_CSS_RESULT_CDCSS = 2U,
};

typedef enum BK4819_CssScanResult_t BK4819_CssScanResult_t;

extern bool gRxIdleMode;

void BK4819_Init(void);
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t Register);
void BK4819_WriteRegister(BK4819_REGISTER_t Register, uint16_t Data);
void BK4819_WriteU8(uint8_t Data);
void BK4819_WriteU16(uint16_t Data);

void BK4819_SetAGC(uint8_t Value);

void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t Pin, bool bSet);

void BK4819_SetCDCSSCodeWord(uint32_t CodeWord);
void BK4819_SetCTCSSFrequency(uint32_t BaudRate);
void BK4819_Set55HzTailDetection(void);
void BK4819_EnableVox(uint16_t Vox1Threshold, uint16_t Vox0Threshold);
void BK4819_SetFilterBandwidth(BK4819_FilterBandwidth_t Bandwidth);
void BK4819_SetupPowerAmplifier(uint16_t Bias, uint32_t Frequency);
void BK4819_SetFrequency(uint32_t Frequency);
void BK4819_SetupSquelch(
		uint8_t SquelchOpenRSSIThresh, uint8_t SquelchCloseRSSIThresh,
		uint8_t SquelchOpenNoiseThresh, uint8_t SquelchCloseNoiseThresh,
		uint8_t SquelchCloseGlitchThresh, uint8_t SquelchOpenGlitchThresh);

void BK4819_SetAF(BK4819_AF_Type_t AF);
void BK4819_RX_TurnOn(void);
void BK4819_SelectFilter(uint32_t Frequency);
void BK4819_DisableScramble(void);
void BK4819_EnableScramble(uint8_t Type);
void BK4819_DisableVox(void);
void BK4819_DisableDTMF(void);
void BK4819_EnableDTMF(void);
void BK4819_PlayTone(uint16_t Frequency, bool bTuningGainSwitch);
void BK4819_EnterTxMute(void);
void BK4819_ExitTxMute(void);
void BK4819_Sleep(void);
void BK4819_TurnsOffTones_TurnsOnRX(void);
void BK4819_SetupAircopy(void);
void BK4819_ResetFSK(void);
void BK4819_Idle(void);
void BK4819_ExitBypass(void);
void BK4819_PrepareTransmit(void);
void BK4819_TxOn_Beep(void);
void BK4819_ExitSubAu(void);

void BK4819_EnableRX(void);

void BK4819_EnterDTMF_TX(bool bLocalLoopback);
void BK4819_ExitDTMF_TX(bool bKeep);
void BK4819_EnableTXLink(void);

void BK4819_PlayDTMF(char Code);
void BK4819_PlayDTMFString(const char *pString, bool bDelayFirst, uint16_t FirstCodePersistTime, uint16_t HashCodePersistTime, uint16_t CodePersistTime, uint16_t CodeInternalTime);

void BK4819_TransmitTone(bool bLocalLoopback, uint32_t Frequency);

void BK4819_GenTail(uint8_t Tail);
void BK4819_EnableCDCSS(void);
void BK4819_EnableCTCSS(void);

uint16_t BK4819_GetRSSI(void);

bool BK4819_GetFrequencyScanResult(uint32_t *pFrequency);
BK4819_CssScanResult_t BK4819_GetCxCSSScanResult(uint32_t *pCdcssFreq, uint16_t *pCtcssFreq);
void BK4819_DisableFrequencyScan(void);
void BK4819_EnableFrequencyScan(void);
void BK4819_SetScanFrequency(uint32_t Frequency);

void BK4819_Disable(void);

void BK4819_StopScan(void);

uint8_t BK4819_GetDTMF_5TONE_Code(void);

uint8_t BK4819_GetCDCSSCodeType(void);
uint8_t BK4819_GetCTCType(void);

void BK4819_SendFSKData(uint16_t *pData);
void BK4819_PrepareFSKReceive(void);

void BK4819_PlayRoger(void);
void BK4819_PlayRogerMDC(void);

void BK4819_Enable_AfDac_DiscMode_TxDsp(void);

void BK4819_GetVoxAmp(uint16_t *pResult);
void BK4819_SetScrambleFrequencyControlWord(uint32_t Frequency);
void BK4819_PlayDTMFEx(bool bLocalLoopback, char Code);

#if defined(ENABLE_MODEM)

typedef struct {
	uint16_t BaudRate;
	uint16_t PremableType : 2;
	uint16_t TxMode : 3;
	uint16_t RxMode : 3;
	uint16_t RxBandwidth : 3;
	uint16_t SyncLength : 1;
	uint16_t PreambleLength: 8;
	uint8_t  SyncBytes[4];
#if defined (MODEM_DEBUG)
	uint16_t InvertTx : 1;
	uint16_t InvertRx : 1;
	uint16_t REG_58_67_UNKNOWN : 2;
	uint16_t REG_59_02_UNKNOWN : 3;
	uint16_t REG_5C_UNKNOWN;
	uint16_t Scramble : 1;
#endif
} BK4819_ModemParams;

void BK4819_ConfigureFSK(BK4819_ModemParams *Params);
void BK4819_StartTransmitFSK(BK4819_ModemParams *Params, uint8_t *Buf, uint16_t nBuf);

#endif 

#endif

