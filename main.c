#include "gp_driver.h"
#include "uart_driver.h"
#include "help_func.h"
#include "adc_driver.h"
#include "stdlib.h"

/* GPIO Configurations */
#define RELAY_PORT PA
#define RELAY_PIN	10
#define RESULT_LED_PORT PA
#define RESULT_LED_PIN 11
#define RESUME_PORT PA
#define RESUME_PIN 12

/* ADC Configurations */
#define ADC_PORT 1
#define LDR_PORT PA
#define LDR_PIN 1
#define LIGHT_THRESHOLD 230

/* UART Configurations */
/*
	USART2 TX:PA2 RX:PA3
	USART1 TX:PA9 RX:PA10 */
#define UART_PORT 2
#define UART_SPEED 115200

/* Mainboard Communication Configurations 
	start session E0 04 01 1A  ==> answer: E0 04 01 1B
	plug play E0 04 6C AF */
#define ENTER_KEY '\r'
#define USB_PORT '0'
#define FILE_NAME_BEGIN " 1 fusion_writer.a"	 	// begins with " 1 "
#define MAX_FILE_PART_NO 6

/* Test Conditions */
#define SHUT_DOWN_START 39000							// in ms		// first iteration of test
#define SHUT_DOWN_STEP 500								// in ms		// iteration steps
#define SHUT_DOWN_FINAL 35000							// in ms		// last iteration of test
#define SENDING_ENTER_KEY_TIME 1					// in secs 	// how long enter key will be sent after power on time
#define WELCOME_SCREEN_TIME 120						// in secs 	// how long will be waited after plug play to check LDR
#define TEST_CYCLE 5
#define OFFLINE_IMAGE_TO_FACTORY_MODE 20	// in mins	// burning offline and turning on in factory mode


void setup(void);
int shippingInit(void);
unsigned short getLightLevel(void);

struct uartManager serialPort;
//unsigned int communicationSuccess = 0xE004011B;
//char communicationSucces[] = { (char)'0xE0', (char)'0x04', (char)'0x01', (char)'0x1B', '\0' };

int light;

int main(void)
{
	int value;
	unsigned short testResult;
	char* valueStr;
	char* offsetValue[12] = { "0x0", "0x80000000", "0x100000000", "0x180000000", "0x200000000", "0x280000000" };
	char filePartSuffix[10] = { "abcdefghi" };
	
	
	setup();
	for(int trialTime=SHUT_DOWN_START, i=0; trialTime >= SHUT_DOWN_FINAL; i++)
	{
		valueStr = int2char(trialTime);
		uart_send(UART_PORT, "\n\nTEST BEGINNING for ");
		uart_send(UART_PORT, valueStr);
		uart_send(UART_PORT, " ms trial time\n");
		uart_TX(UART_PORT, ENTER_KEY);
		free(valueStr);
		
		// 2nd step: enerjiyi ver
		write_GP(RESULT_LED_PORT, RESULT_LED_PIN, HIGH);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);
		
		// 3rd step: Sürekli olarak enter komutu gönder(Ortalama kaç saniyede mboota düstügü hesaplanip o kadar süre + offset bu islem yapilabilir)
		for(int i=0; i < 100*SENDING_ENTER_KEY_TIME; i++)	// send enter key each 10ms
		{
			uart_TX(UART_PORT, ENTER_KEY);
			delayMS(10);
		}
		
		// 4th step: Uart üzerinden usb start komutu gönder.
		uart_send(UART_PORT, "usb start ");						// send USB start command // usb start <USB_PORT>
		uart_TX(UART_PORT, USB_PORT);									
		uart_TX(UART_PORT, ENTER_KEY);
		delayMS(4000);																// wait for mainboard to react
		
		// 5th step: bin2emmc <usb port number> <binary file name> <offset value> (Her dosyayi tek tek yüklet) (offset value'yu Harun'a sor)
		// 6th step: Yükleme islemi bittikten sonra reset at.
		for(int i=0; i <= MAX_FILE_PART_NO; i++)
		{
			uart_send(UART_PORT, "bin2emmc ");					// "bin2emmc "
			uart_TX(UART_PORT, USB_PORT);								// "bin2emmc 0"
			uart_send(UART_PORT, FILE_NAME_BEGIN);			// "bin2emmc 0 1 fusion_writera"
			uart_TX(UART_PORT, filePartSuffix[i]);			// "bin2emmc 0 1 fusion_writeraa"
			uart_TX(UART_PORT, ' ');										// "bin2emmc 0 1 fusion_writeraa "
			uart_send(UART_PORT, *(offsetValue+i));			// "bin2emmc 0 1 fusion_writeraa 0x80000000"
			uart_TX(UART_PORT, ';');										// "bin2emmc 0 1 fusion_writeraa 0x80000000;"
		}
		uart_send(UART_PORT, "reset");								// send reset command
		uart_TX(UART_PORT, ENTER_KEY);
		delayMS(OFFLINE_IMAGE_TO_FACTORY_MODE*60*1000);// wait 19 mins until mainboard is on in factory mode
		
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);			// turn the power off
		delayMS(4000);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		delayMS(120000);
		
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);			// turn the power off
		delayMS(4000);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		delayMS(120000);
		
		// 7th step: Shipping init yap ve gelen loglari text dosyasina kaydetmeye basla.
		if(shippingInit() == 0)	// plug play den 35 saniye sonra logo geliyor
		{
			write_GP(RELAY_PORT, RELAY_PIN, HIGH);			// turn the power off
			uart_send(UART_PORT, "\n------\noffline image burning FAILED!!\n------\n");
			while(1)
			{
				toggle_GP(RESULT_LED_PORT, RESULT_LED_PIN);
				delayMS(3000);
			}
		}
			
		
		// 8th step: trialTime süresi kadar sonra enerjiyi kes
		delayMS(trialTime);
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);			// turn the power off
		delayMS(3000);
		
		// 9th step: enerji ver
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		
		// 10th step: LDR'den gelen veriyi x dakika kadar oku eger degisim olursa en basa dön (i++; logu durdur;), olmazsa exit at.
		delayMS(WELCOME_SCREEN_TIME*1000);
		value = getLightLevel();

		if(value <= LIGHT_THRESHOLD)
		{
			valueStr = int2char(trialTime);
			uart_send(UART_PORT, "\nTEST OK for ");
			uart_send(UART_PORT, valueStr);
			uart_send(UART_PORT, " ms trial time\n");
			free(valueStr);
			testResult = 1;
		}
		else	// PROBLEM OCCURED !!
		{
			valueStr = int2char(trialTime);
			uart_send(UART_PORT, "\nTEST FAILED for ");
			uart_send(UART_PORT, valueStr);
			uart_send(UART_PORT, " ms trial time\n");
			uart_TX(UART_PORT, ENTER_KEY);
			free(valueStr);
			testResult = 0;
		}
		
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);			// turn the power off
		write_GP(RESULT_LED_PORT, RESULT_LED_PIN, LOW);
		delayMS(4000);
		
		if(i == TEST_CYCLE){
			i = 0;
			trialTime -= SHUT_DOWN_STEP;
		}
		
		if(testResult == 0) // TEST FAILED
			while(read_GP(RESUME_PORT, RESUME_PIN) == 0)
			{
				toggle_GP(RESULT_LED_PORT, RESULT_LED_PIN);
				delayMS(100);
			}

//		while(1)
//		{
//			light = getLightLevel();
//			valueStr = int2char(light); // for debug
//			uart_send(UART_PORT, valueStr); // for debug
//			uart_TX(UART_PORT, ENTER_KEY); // for debug
//			free(valueStr);
//		}
	}
	while(1)		// TEST IS DONE SUCCESSFULLY
	{
		toggle_GP(RESULT_LED_PORT, RESULT_LED_PIN);
		delayMS(1000);
	}
//	while(1)
//	if(serialPort.signal == 1)	// TRUE once we have uart message
//	{
//		// gelen mesaja dair islem
//		
//		uart_send(2,serialPort.message); 
//		serialPort.signal = 0;
//		str_empty(serialPort.message);
//	}
		
	return 0;
}

void setup(void)
{
	systick_init();																			// for systick delays
	gpio_init(RELAY_PORT, RELAY_PIN, OUT10, OUT_GP_PP);	// initilize GPIO for relay
	write_GP(RELAY_PORT, RELAY_PIN, HIGH);								// condition on start
	gpio_init(RESULT_LED_PORT, RESULT_LED_PIN, OUT10, OUT_GP_PP);	// initilize GPIO for result LED
	write_GP(RESULT_LED_PORT, RESULT_LED_PIN, LOW);								// condition on start
	adc_init(ADC_PORT, LDR_PORT, LDR_PIN);							// initilize ADC for LDR
	uart_init(UART_PORT, UART_SPEED);										// initilize uart for communication with TV mainboard
	delayMS(100);
	
	/* uart comfigurations */
	serialPort.mode = 0; 					/* 0: process /// 1/2/3: brigde to uart1/2/3 */
	serialPort.signal = 0;				/* message recieved signal */
	serialPort.strategy = 1; 			/* 1:terminator /// 0:interrupt */
	serialPort.terminator = '\n';
}

int shippingInit(void)
{
	serialPort.signal = 0;
	str_empty(serialPort.message);
	
	/* start session command */
	uart_TX_hex(UART_PORT, 0xE0);
	uart_TX_hex(UART_PORT, 0x04);
	uart_TX_hex(UART_PORT, 0x01);
	uart_TX_hex(UART_PORT, 0x1A);
	delayMS(1000);
//	if(serialPort.signal == 1 && str_exact(serialPort.message, communicationSuccess) )
	{
		/* plug play command */
		uart_TX_hex(UART_PORT, 0xE0);
		uart_TX_hex(UART_PORT, 0x04);
		uart_TX_hex(UART_PORT, 0x6C);
		uart_TX_hex(UART_PORT, 0xAF);
		return 1;
	}
//	return 0;
}

unsigned short getLightLevel(void)
{
	unsigned short sumData;
	
	for(int i=0; i<100;){
		if(adc_check(ADC_PORT, LDR_PORT, LDR_PIN)){
			i++;
			sumData += adc_get(ADC_PORT, LDR_PORT, LDR_PIN);
			
		}
		delayMS(10);
	}
	return(sumData / 100);
}

void USART2_IRQHandler(void)
{
	uart_ISR(UART_PORT, &serialPort);
}
