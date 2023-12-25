An MCU-controlled hardware and software are developed to check some conditions of the TV and to communicate with the TV in order to perform the power cut-off process after shipping init of the TV product in a certain test time. The test scenario is developed as like TV is on the final assembly line.

This project uses my own libraries and some standard libraries for STM32F103C6.
This project is designed in a modular way to be used in different mainboard projects of Android TVs.

Before starting the test, must follow the below instructions list
- Check and configure macros for GPIO, ADC, and UART if needed.
- Plug the USB disk into the USB0 port. May be configurated in macros if needed.
- Set MAX_FILE_PART_NO and define offsetValue array variables according to part quantity.
- Set test conditions by SHUT_DOWN_START, SHUT_DOWN_STEP, SHUT_DOWN_FINAL, TEST_CYCLE macros. For example; run for each 0,5 from 50 secs to 40 secs, and each step 10 times.
- Configurate WELCOME_SCREEN_TIME if needed.
- Set OFFLINE_IMAGE_TO_FACTORY_MODE if needed.
- Build the project and upload it to STM32.
- Open a serial port on a PC and connect it to UART TX of the MCU board to see test information logs.
- Open another serial port on PC and connect it UART TX of the TV to see logs of the mainboard.
- Prepare the setup by doing all hardware connections. Use the "schematics.pdf" file for reference.
- Once the hardware setup connection is ready, power it to run the test.

Project photos, schematics, and flow charts can be found in the "Project Documentation" folder.

Cihan Şenyüz
25.12.2023
