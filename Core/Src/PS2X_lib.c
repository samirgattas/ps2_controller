#include "PS2X_lib.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "main.h"

//#include "WProgram.h"
//#include "pins_arduino.h"

static uint8_t enter_config[] = { 0x01, 0x43, 0x00, 0x01, 0x00 };
static uint8_t set_mode[] = { 0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
		0x00 };
static uint8_t set_bytes_large[] = { 0x01, 0x4F, 0x00, 0xFF, 0xFF, 0x03, 0x00,
		0x00, 0x00 };
static uint8_t exit_config[] = { 0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A,
		0x5A };
static uint8_t enable_rumble[] = { 0x01, 0x4D, 0x00, 0x00, 0x01 };
static uint8_t type_read[] = { 0x01, 0x45, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
		0x5A };

void ATT_SET(void);
void ATT_CLR(void);

/****************************************************************************************/
_Bool NewSomeButtonState() {
	return ((last_buttons ^ buttons) > 0);
}

/****************************************************************************************/
_Bool NewButtonState(unsigned int button) {
	return (((last_buttons ^ buttons) & button) > 0);
}

/****************************************************************************************/
_Bool ButtonPressed(unsigned int button) {
	return (NewButtonState(button) & Button(button));
}

/****************************************************************************************/
_Bool ButtonReleased(unsigned int button) {
	return ((NewButtonState(button)) & ((~last_buttons & button) > 0));
}

/****************************************************************************************/
_Bool Button(uint16_t button) {
	return ((~buttons & button) > 0);
}

/****************************************************************************************/
unsigned int ButtonDataByte() {
	return (~buttons);
}

/****************************************************************************************/
uint8_t Analog(uint8_t button) {
	return PS2data[button];
}

/****************************************************************************************/
unsigned char _gamepad_shiftinout(char byte) {
	uint8_t *rx_data = 0;
	HAL_SPI_Receive(&spi, (uint8_t*)rx_data, 1, 100);
	return (char)*rx_data;
}

/****************************************************************************************/
/*void read_gamepad() {
 read_gamepad(false, 0x00);
 }*/

/****************************************************************************************/
_Bool read_gamepad(_Bool motor1, uint8_t motor2) {
	double temp = millis() - last_read;

	if (temp > 1500) //waited to long
		reconfig_gamepad();

	if (temp < read_delay)  //waited too short
		HAL_Delay(read_delay - temp);

	/*if (motor2 != 0x00)
		motor2 = map(motor2, 0, 255, 0x40, 0xFF); //noting below 40 will make it spin
	*/
	uint8_t dword[9] = { 0x01, 0x42, 0, motor1, motor2, 0, 0, 0, 0 };
	uint8_t dword2[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	// Try a few times to get valid data...
	for (uint8_t RetryCnt = 0; RetryCnt < 5; RetryCnt++) {
		ATT_CLR(); // low enable joystick

		//delayMicroseconds(CTRL_BYTE_DELAY);
		//Send the command to send button and joystick data;
		for (int i = 0; i < 9; i++) {
			PS2data[i] = _gamepad_shiftinout(dword[i]);
		}

		if (PS2data[1] == 0x79) { //if controller is in full data return mode, get the rest of data
			for (int i = 0; i < 12; i++) {
				PS2data[i + 9] = _gamepad_shiftinout(dword2[i]);
			}
		}

		ATT_SET(); // HI disable joystick
		// Check to see if we received valid data or not.
		// We should be in analog mode for our data to be valid (analog == 0x7_)
		if ((PS2data[1] & 0xf0) == 0x70)
			break;

		// If we got to here, we are not in analog mode, try to recover...
		reconfig_gamepad(); // try to get back into Analog mode.
		HAL_Delay(read_delay);
	}

	// If we get here and still not in analog mode (=0x7_), try increasing the read_delay...
	if ((PS2data[1] & 0xf0) != 0x70) {
		if (read_delay < 10)
			read_delay++;   // see if this helps out...
	}

	last_buttons = buttons; //store the previous buttons states

	buttons = (uint16_t) (PS2data[4] << 8) + PS2data[3]; //store as one value for multiple functions
	last_read = millis();
	return ((PS2data[1] & 0xf0) == 0x70);  // 1 = OK = analog mode - 0 = NOK
}

/****************************************************************************************/
/*uint8_t config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat) {
 return config_gamepad(clk, cmd, att, dat, false, false);
 }*/

/****************************************************************************************/
uint8_t config_gamepad(SPI_HandleTypeDef *_spi, _Bool pressures, _Bool rumble) {
	spi = *_spi;

	uint8_t temp[sizeof(type_read)];

	//new error checking. First, read gamepad a few times to see if it's talking
	read_gamepad(false, 0x00);
	read_gamepad(false, 0x00);

	//see if it talked - see if mode came back.
	//If still anything but 41, 73 or 79, then it's not talking
	if (PS2data[1] != 0x41 && PS2data[1] != 0x42 && PS2data[1] != 0x73
			&& PS2data[1] != 0x79) {
		return 1; //return error code 1
	}

	//try setting mode, increasing delays if need be.
	read_delay = 1;

	for (int y = 0; y <= 10; y++) {
		sendCommandString(enter_config, sizeof(enter_config)); //start config run

		//read type
		//delayMicroseconds(CTRL_BYTE_DELAY);

		ATT_CLR(); // low enable joystick

		//delayMicroseconds(CTRL_BYTE_DELAY);

		for (int i = 0; i < 9; i++) {
			temp[i] = _gamepad_shiftinout(type_read[i]);
		}

		ATT_SET(); // HI disable joystick

		controller_type = temp[3];

		sendCommandString(set_mode, sizeof(set_mode));
		if (rumble) {
			sendCommandString(enable_rumble, sizeof(enable_rumble));
			en_Rumble = true;
		}
		if (pressures) {
			sendCommandString(set_bytes_large, sizeof(set_bytes_large));
			en_Pressures = true;
		}
		sendCommandString(exit_config, sizeof(exit_config));

		read_gamepad(false, 0x00);

		if (pressures) {
			if (PS2data[1] == 0x79)
				break;
			if (PS2data[1] == 0x73)
				return 3;
		}

		if (PS2data[1] == 0x73)
			break;

		if (y == 10) {
			return 2; //exit function with error
		}
		read_delay += 1; //add 1ms to read_delay
	}
	return 0; //no error if here
}

/****************************************************************************************/
void sendCommandString(uint8_t string[], uint8_t len) {
	ATT_CLR(); // low enable joystick
	//delayMicroseconds(CTRL_BYTE_DELAY);
	for (int y = 0; y < len; y++)
		_gamepad_shiftinout(string[y]);
	ATT_SET(); //high disable joystick
	HAL_Delay(read_delay);                  //wait a few
}

/****************************************************************************************/
uint8_t readType() {
	if (controller_type == 0x03)
		return 1;
	else if (controller_type == 0x01 && PS2data[1] == 0x42)
		return 4;
	else if (controller_type == 0x01 && PS2data[1] != 0x42)
		return 2;
	else if (controller_type == 0x0C)
		return 3;  //2.4G Wireless Dual Shock PS2 Game Controller

	return 0;
}

/****************************************************************************************/
void enableRumble() {
	sendCommandString(enter_config, sizeof(enter_config));
	sendCommandString(enable_rumble, sizeof(enable_rumble));
	sendCommandString(exit_config, sizeof(exit_config));
	en_Rumble = true;
}

/****************************************************************************************/
_Bool enablePressures() {
	sendCommandString(enter_config, sizeof(enter_config));
	sendCommandString(set_bytes_large, sizeof(set_bytes_large));
	sendCommandString(exit_config, sizeof(exit_config));

	read_gamepad(false, 0x00);
	read_gamepad(false, 0x00);

	if (PS2data[1] != 0x79)
		return false;

	en_Pressures = true;
	return true;
}

/****************************************************************************************/
void reconfig_gamepad() {
	sendCommandString(enter_config, sizeof(enter_config));
	sendCommandString(set_mode, sizeof(set_mode));
	if (en_Rumble)
		sendCommandString(enable_rumble, sizeof(enable_rumble));
	if (en_Pressures)
		sendCommandString(set_bytes_large, sizeof(set_bytes_large));
	sendCommandString(exit_config, sizeof(exit_config));
}

/****************************************************************************************/

// On pic32, use the set/clr registers to make them atomic...
// Esta escribiendo en los registros de los puertos
void ATT_SET(void) {
	HAL_GPIO_WritePin(CS, GPIO_PIN_SET);
}

void ATT_CLR(void) {
	HAL_GPIO_WritePin(CS, GPIO_PIN_RESET);
}
