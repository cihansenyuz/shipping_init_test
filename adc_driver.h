/**
  ******************************************************************************
  * @file    adc_driver.h
  * @author  Cihan Senyuz
  * @brief   Header for adc_driver.c file.
  *                 This file contains the common defines of the application.
  *     
  *     Setup
  *
  *     8.3.7 Enable alternate func. AFIO and the clock for ADC (RCC->APB2ENR)
  *     Find ADC pins in datasheet 3. section table5
  *     Configurate those pins as GPIO input (GPIOx_CRL or CRH)
  *
  *     11.12.2 set all registers zero to avoid any unnecessary setup (ADCx->CR2)
  *     11.12.11 Setup the channel you are going to use (ADCx->SQR3)
  *     11.12.3 Put 1 twice to turn on A/D converter ADON (ADCx->CR2)
  *     11.12.3 Turn on continues conversation CONT (ADCx->CR2)
  *
  *     ADC check
  *
  *     11.12.1 Check status register to know end of conversion EOC (ADCx->SR)
  *
  *     ADC read
  *
  *     11.12.14 Read data register (ADCx->DR)
  * 	
  * 
  * @note   Source code depends on stm32f10x.h, gp_driver.h and systick_delay.h libraries
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "gp_driver.h"
#include "systick_delay.h"

#define adc1	1
#define adc2	2

/* 
* Configurates GPIO port, pin and ADC to initiliaze
*
* @param adc desired ADC
* @param port GPIO port to be used
* @param pin GPIO pin to be used
* @return 1: successful, 0: given GPIO port/pin does not match with ADC channels
*/
char adc_init(char adc, short port, short pin);

/* 
* Checks if value read from ADC is ready to be taken.
*
* @param adc desired ADC
* @param port GPIO port to be used
* @param pin GPIO pin to be used
* @return 1: data is ready, 0: not ready
*/
char adc_check(char adc, short port, short pin);

/* 
* Gets the ADC value
*
* @param adc desired ADC
* @param port GPIO port to be used
* @param pin GPIO pin to be used
* @return 1: data is read, 0: data couldnot be read
* @note Use adc_check() function before to use this function to be sure that data is ready to be read.
*/
int adc_get(char adc, short port, short pin);
