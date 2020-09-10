/*
 * ps2_controller.c
 *
 *  Created on: 9 sept. 2020
 *      Author: xx
 */

// ############################################################################
// INCLUDE
#include "ps2_controller.h"

// ############################################################################
// DEFINE
# define DEBUG_PS2 1
// ############################################################################
// VARIABLES EXTERNAS

UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi1;

// ############################################################################
// VARIABLES PRIVADAS

// Estado y subestado de la interfaz para comunicacion con ps2 controller
uint16_t estado_ps2 = 0;
uint16_t sub_estado_ps2 = 0;

// Time Out 1 = 1 [ms]
uint32_t tout_ps2 = 0;

// Reintentos de conexion con control de PS2
uint16_t intentos_conexion = 0;

// ############################################################################
// CABECERA DE FUNCIONES PRIVADAS

void set_tout(uint32_t ms);

// ############################################################################
// FUNCIONES PRIVADAS

void set_tout(uint32_t ms) {
	tout_ps2 = ms;
}

// ############################################################################
// FUNCIONES PUBLICAS

int8_t conectar_ps2() {
	uint8_t rta = 3;

	rta = config_gamepad(&hspi1, false, false);
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
	return rta;
}

_Bool acusar_botones() {
	_Bool isAnalogMode;
	isAnalogMode = read_gamepad(false, 0);
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
	return isAnalogMode;
}

void ps2_controller() {
	switch (estado_ps2) {
	case 0:
		tout_ps2 = 0;
		estado_ps2++;
		sub_estado_ps2 = 1;
		intentos_conexion = 0;
		break;
	case 1:
		switch (sub_estado_ps2) {
		case 1:
			if (!conectar_ps2()) {
				// Entra si conectó
				estado_ps2++;
			} else if (intentos_conexion > 2) {
				// Entra si intento 3 veces
				set_tout(10000);	// Espera 10
				if (DEBUG_PS2 == 1){
					HAL_UART_Transmit(&huart1, (uint8_t*) "-> Espera 10s\n", 14, 100);
				}
				sub_estado_ps2++;
			}
			intentos_conexion++;
			break;
		case 2:
			if (tout_ps2 == 0) {
				if(DEBUG_PS2 == 1){
					HAL_UART_Transmit(&huart1, (uint8_t*) "-> Podes reintentar\n", 20, 100);
				}
				estado_ps2 = 0;
			}
			break;
		}
		break;
	case 2:
		if (!acusar_botones()) {
			estado_ps2 = 0;
		}
		break;
	}
}
