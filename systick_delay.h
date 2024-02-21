/**
  ******************************************************************************
  * @file    systick_delay.h
  * @author  Cihan Senyuz
  * @brief   Header for systick_delay.c file.
  *                   This file contains the common defines of the application.
  * 
  * @note    Source code depends on stm32f10x.h library
  ******************************************************************************
  */

#include "stm32f10x.h"

/* 
* sets control registers of system ticker, reload value of ticker
*
* @param none
* @return none
*/
void systick_init(void);

/* sets reload value of ticker to achieve 1 milisecond
* 
* @param none
* @return none
* 
* @note needs systick_init() have called firstly
*/
void createMili(void);

/* creates delay in miliseconds using createMili()
* 
* @param t desired number of milisecond
* @note needs systick_init() have called firstly
*/
void delayMS(unsigned long);

/* enables the systick interrupt by setting control registers of system ticker
* and reloading value of ticker
*
* @param none
* @return none
*/
void systick_interrup_start(void);
