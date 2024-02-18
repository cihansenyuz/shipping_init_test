/**
  ******************************************************************************
  * @file    main.h
  * @author  Cihan Senyuz
  * @brief   Header for main.c file.
  *                   This file contains the common defines of the application.
  *     
  *     An MCU-controlled hardware and software are developed to check some conditions
  * of the TV and to communicate with the TV in order to perform the power cut-off
  * process after shipping init of the TV product in a certain test time. The test
  * scenario is developed as like TV is on the final assembly line.
  *     This project uses my own libraries and some standard libraries for STM32F103C6.
  * This project is designed in a modular way to be used in different mainboard projects
  * of Android TVs.
  *     Before starting the test, must follow the below instructions list
  *     - Check and configure macros for GPIO, ADC, and UART if needed.
  *     - Plug the USB disk into the USB0 port. May be configurated in macros if needed.
  *     - Set MAX_FILE_PART_NO and define offsetValue array variables according to part quantity.
  *     - Set test conditions by SHUT_DOWN_START, SHUT_DOWN_STEP, SHUT_DOWN_FINAL, TEST_CYCLE macros. For example; run for each 0,5 from 50 secs to 40 secs, and each step 10 times.
  *     - Configurate WELCOME_SCREEN_TIME if needed.
  *     - Set OFFLINE_IMAGE_TO_FACTORY_MODE if needed.
  *     - Build the project and upload it to STM32.
  *     - Open a serial port on a PC and connect it to UART TX of the MCU board to see test information logs.
  *     - Open another serial port on PC and connect it UART TX of the TV to see logs of the mainboard.
  *     - Prepare the setup by doing all hardware connections. Use the "schematics.pdf" file for reference.
  *     - Once the hardware setup connection is ready, power it to run the test.
  * 
  * @note   Source code depends on gp_driver.h, uart_driver.h, help_func.h,
  * adc_driver.h and stdlib.h libraries
  ******************************************************************************
  */

#include "gp_driver.h"
#include "uart_driver.h"
#include "help_func.h"
#include "adc_driver.h"
#include "stdlib.h"

/* GPIO Configurations */
#define RELAY_PORT PA       
#define RELAY_PIN	10      // connected to relay to power on/off TV
#define INFO_LED_PORT PA
#define INFO_LED_PIN 11     // connected to LED to see test status
/* Info actions
	on: ongoing test
	off: test cycle is over
	blinking slow: whole test is over and no problem
	blinking fast: test is interrupted due to no welcome screen
*/
#define RESUME_PORT PA
#define RESUME_PIN 12       // connected to button to continue in case of false call

/* ADC Configurations */
#define ADC_PORT 1
#define LDR_PORT PA
#define LDR_PIN 1           // connected to LDR to capture welcome screen
#define LIGHT_THRESHOLD 230 // threshold value of LDR when there is welcome text on the screen

/* UART Configurations */
/*
	USART2 TX:PA2 RX:PA3
	USART1 TX:PA9 RX:PA10 */
#define UART_PORT 2
#define UART_SPEED 115200

/* Mainboard Communication Configurations 
	start session E0 04 01 1A  ==> answer: E0 04 01 1B
	plug play E0 04 6C AF
*/
/* Generic defines */
#define ENTER_KEY '\r'
#define USB_PORT '0'
#define FILE_NAME_BEGIN " 1 fusion_writer.a"// begins with " 1 "
#define MAX_FILE_PART_NO 6

/* Test Conditions */
#define SHUT_DOWN_START 39000				// in ms	// first iteration of test
#define SHUT_DOWN_STEP 500					// in ms	// iteration steps
#define SHUT_DOWN_FINAL 35000				// in ms	// last iteration of test
#define SENDING_ENTER_KEY_TIME 1			// in secs 	// how long enter key will be sent after power on time
#define WELCOME_SCREEN_TIME 120				// in secs 	// how long will be waited after plug play to check LDR
#define TEST_CYCLE 5
#define OFFLINE_IMAGE_TO_FACTORY_MODE 20	// in mins	// burning offline and turning on in factory mode

void setup(void);
void shippingInit(void);
unsigned short getLightLevel(void);
