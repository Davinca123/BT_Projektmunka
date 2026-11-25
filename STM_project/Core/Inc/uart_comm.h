#ifndef INC_UART_COMM_H_
#define INC_UART_COMM_H_

#include <stdint.h>
#include "main.h"

#define MAX_STEP_LOGS        1000u
#define MAX_HR_LOGS          100u

#define REQ_ALL_FLAG_MASK    0x01u
#define TIME_FLAG_MASK       0x02u

typedef struct {
	uint32_t value;      // Lépésszám (kumulatív) vagy Pulzus (BPM)
	uint8_t year;        // Év (pl. 26 -> 2026)
	uint8_t month;       // Hónap (1-12)
	uint8_t day;         // Nap (1-31)
	uint8_t hours;       // Óra (0-23)
	uint8_t minutes;     // Perc (0-59)
	uint8_t seconds;     // Másodperc (0-59)
} LogRecord_t;

extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;

extern uint8_t g_uart_rx_char[1u];
extern volatile uint8_t gb_flags;

void UartComm_RxCharReceived(uint8_t ch);
void UartComm_Process(void);
void UartComm_LogStep(uint32_t step_count);
void UartComm_LogHr(uint32_t bpm);
void UartComm_ResetStepLogs(void);

#endif /* INC_UART_COMM_H_ */
