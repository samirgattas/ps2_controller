/*
 * ps2_controller.c
 *
 *  Created on: 9 sept. 2020
 *      Author: xx
 */

#include "ps2_controller.h"

UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi1;

void conectar_ps2() {
	int rta = 3;

	while (rta != 0) {
		rta = config_gamepad(&hspi1, false, false);
	}
	if (rta == 0) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Encontrado\n", 11, 100);
	} else if (rta == 1) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "NO encontrado\n", 14, 100);
	} else if (rta == 2) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "NO se aceptan comandos\n", 23,
				100);
	} else if (rta == 3) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "NO control de presion\n", 22,
				100);
	}

	switch (readType()) {
	case 0:
		HAL_UART_Transmit(&huart1, (uint8_t*) "Unknown Controller type found\n",
				30, 100);
		break;
	case 1:
		HAL_UART_Transmit(&huart1, (uint8_t*) "DualShock Controller found\n",
				27, 100);
		break;
	case 2:
		HAL_UART_Transmit(&huart1, (uint8_t*) "GuitarHero Controller found\n",
				28, 100);
		break;
	case 3:
		HAL_UART_Transmit(&huart1, (uint8_t*) "Wireless Controller found\n", 26,
				100);
		break;
	}
}

void acusar_botones() {
	read_gamepad(false, 0);
	if (Button(PSB_CROSS)) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Press Cross\n", 12, 100);
	}
	if (Button(PSB_SQUARE)) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Press Square\n", 13, 100);
	}
	if (Button(PSB_CIRCLE)) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Press Circle\n", 13, 100);
	}
	if (Button(PSB_TRIANGLE)) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Press Triangle\n", 15, 100);
	}
	if (Button(PSB_L1) || Button(PSB_R1)) {
		HAL_UART_Transmit(&huart1, (uint8_t*) "Stick values:\n", 14, 100);
		// Left stick, Y axis. Other options: LX, RY, RX
		if (Analog(PSS_LY) > 128) {
			HAL_UART_Transmit(&huart1, (uint8_t*) "Left v\n", 7, 100);
		} else if (Analog(PSS_LY) < 128) {
			HAL_UART_Transmit(&huart1, (uint8_t*) "Left ^\n", 7, 100);
		}
		if (Analog(PSS_RX) > 128) {
			HAL_UART_Transmit(&huart1, (uint8_t*) "Right ->\n", 9, 100);
		} else if (Analog(PSS_RX) < 128) {
			HAL_UART_Transmit(&huart1, (uint8_t*) "Right <-\n", 9, 100);
		}
	}
}
