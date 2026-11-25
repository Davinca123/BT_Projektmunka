/*
 * uart_comm.c
 *
 *  Created on: Nov 23, 2026
 *      Author: leven
 */

#include "uart_comm.h"
#include <stdio.h>

static uint8_t s_rx_buff[32u] = { 0u };
static uint8_t s_tx_buff[128u] = { 0u };
static uint8_t s_rx_index = 0u;

static LogRecord_t s_step_logs[MAX_STEP_LOGS];
static uint16_t s_step_log_count = 0u;

static LogRecord_t s_hr_logs[MAX_HR_LOGS];
static uint16_t s_hr_log_count = 0u;

uint8_t g_uart_rx_char[1u] = { 0u };
volatile uint8_t gb_flags = 0u;

void UartComm_RxCharReceived(uint8_t ch) {
	if ((ch == (uint8_t) '\n') && (s_rx_index > 0u)) {
		s_rx_buff[s_rx_index] = ch;
		s_rx_index = 0u;
		switch (s_rx_buff[1u]) {
		case '1':
			gb_flags |= REQ_ALL_FLAG_MASK;
			break;
		case '2':
			gb_flags |= TIME_FLAG_MASK;
			break;
		default:
			break;
		}
	} else if ((ch != (uint8_t) '#') && (s_rx_index == 0u)) {
		return;
	} else if (s_rx_index < (sizeof(s_rx_buff) - 1u)) {
		s_rx_buff[s_rx_index++] = ch;
	} else {
		s_rx_index = 0u;
	}
}

static void handle_time_set(void) {
	RTC_TimeTypeDef time_now = { 0 };
	RTC_DateTypeDef date_now = { 0 };
	uint32_t y, m, d, hr, min, sec;

	if (sscanf((char*) s_rx_buff, "#2 %u-%u-%u %u:%u:%u", &y, &m, &d, &hr, &min,
			&sec) == 6) {
		date_now.Year = (uint8_t) y;
		date_now.Month = (uint8_t) m;
		date_now.Date = (uint8_t) d;
		time_now.Hours = (uint8_t) hr;
		time_now.Minutes = (uint8_t) min;
		time_now.Seconds = (uint8_t) sec;

		HAL_RTC_SetDate(&hrtc, &date_now, RTC_FORMAT_BIN);
		HAL_RTC_SetTime(&hrtc, &time_now, RTC_FORMAT_BIN);
	}
}

static void handle_req_all(void) {
	int len = 0;

	for (uint16_t i = 0u; i < s_hr_log_count; i++) {
		len = snprintf((char*) s_tx_buff, sizeof(s_tx_buff),
				"%lu-%02u:%02u:%02u%s", s_hr_logs[i].value, s_hr_logs[i].hours,
				s_hr_logs[i].minutes, s_hr_logs[i].seconds,
				(i < (s_hr_log_count - 1u)) ? "," : ";");
		HAL_UART_Transmit(&huart2, s_tx_buff, (uint16_t) len, HAL_MAX_DELAY);
	}
	if (s_hr_log_count == 0u) {
		HAL_UART_Transmit(&huart2, (uint8_t*) ";", 1u, HAL_MAX_DELAY);
	}

	for (uint16_t i = 0u; i < s_step_log_count; i++) {
		len = snprintf((char*) s_tx_buff, sizeof(s_tx_buff),
				"%lu-%02u:%02u:%02u%s", s_step_logs[i].value,
				s_step_logs[i].hours, s_step_logs[i].minutes,
				s_step_logs[i].seconds,
				(i < (s_step_log_count - 1u)) ? "," : ";");
		HAL_UART_Transmit(&huart2, s_tx_buff, (uint16_t) len, HAL_MAX_DELAY);
	}
	if (s_step_log_count == 0u) {
		HAL_UART_Transmit(&huart2, (uint8_t*) ";", 1u, HAL_MAX_DELAY);
	}

	HAL_UART_Transmit(&huart2, (uint8_t*) "\r\n", 2u, HAL_MAX_DELAY);
}

static void handle_button_step(void) {
	RTC_TimeTypeDef time_now = { 0 };
	RTC_DateTypeDef date_now = { 0 };
	uint32_t next_val =
			(s_step_log_count > 0u) ?
					(s_step_logs[s_step_log_count - 1u].value + 1u) : 1u;

	if (s_step_log_count < MAX_STEP_LOGS) {
		HAL_RTC_GetTime(&hrtc, &time_now, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &date_now, RTC_FORMAT_BIN);

		LogRecord_t log = { next_val, date_now.Year, date_now.Month,
				date_now.Date, time_now.Hours, time_now.Minutes,
				time_now.Seconds };
		s_step_logs[s_step_log_count++] = log;
	}
}

void UartComm_Process(void) {
	if (gb_flags & TIME_FLAG_MASK) {
		handle_time_set();
		gb_flags &= ~TIME_FLAG_MASK;
	}
	if (gb_flags & REQ_ALL_FLAG_MASK) {
		handle_req_all();
		gb_flags &= ~REQ_ALL_FLAG_MASK;
	}
}

static void record_log(LogRecord_t *buf, uint16_t *count, uint16_t max_len,
		uint32_t val) {
	RTC_TimeTypeDef time_now = { 0 };
	RTC_DateTypeDef date_now = { 0 };

	if ((buf == NULL) || (count == NULL) || (*count >= max_len)) {
		return;
	}

	HAL_RTC_GetTime(&hrtc, &time_now, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date_now, RTC_FORMAT_BIN);

	buf[*count].value = val;
	buf[*count].year = date_now.Year;
	buf[*count].month = date_now.Month;
	buf[*count].day = date_now.Date;
	buf[*count].hours = time_now.Hours;
	buf[*count].minutes = time_now.Minutes;
	buf[*count].seconds = time_now.Seconds;
	(*count)++;
}

void UartComm_LogStep(uint32_t step_count) {
	record_log(s_step_logs, &s_step_log_count, MAX_STEP_LOGS, step_count);
}

void UartComm_LogHr(uint32_t bpm) {
	record_log(s_hr_logs, &s_hr_log_count, MAX_HR_LOGS, bpm);
}

void UartComm_ResetStepLogs(void) {
	s_step_log_count = 0u;
}
