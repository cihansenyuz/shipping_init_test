#include "main.h"

struct uartManager serialPort;

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
		// step 1: print test information
		valueStr = int2char(trialTime);
		uart_send(UART_PORT, "\n\nTEST BEGINNING for ");
		uart_send(UART_PORT, valueStr);
		uart_send(UART_PORT, " ms trial time\n");
		uart_TX(UART_PORT, ENTER_KEY);
		free(valueStr);
		
		// step 2: TV power on 
		write_GP(INFO_LED_PORT, INFO_LED_PIN, HIGH);		// test info LED is on: test is ongoing
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// power on
		
		// step 3: send Enter key at start-up
		for(int i=0; i < 100*SENDING_ENTER_KEY_TIME; i++)	// send enter key each 10ms
		{
			uart_TX(UART_PORT, ENTER_KEY);
			delayMS(10);
		}
		
		// step 4: read USB stick
		uart_send(UART_PORT, "usb start ");					// "usb start "
		uart_TX(UART_PORT, USB_PORT);						// "usb start 0"
		uart_TX(UART_PORT, ENTER_KEY);						// "usb start 0\r"
		delayMS(4000);										// wait for mainboard to react
		
		// step 5: send offline burn commands
		for(int i=0; i <= MAX_FILE_PART_NO; i++)
		{
			uart_send(UART_PORT, "bin2emmc ");				// "bin2emmc "
			uart_TX(UART_PORT, USB_PORT);					// "bin2emmc 0"
			uart_send(UART_PORT, FILE_NAME_BEGIN);			// "bin2emmc 0 1 fusion_writera"
			uart_TX(UART_PORT, filePartSuffix[i]);			// "bin2emmc 0 1 fusion_writeraa"
			uart_TX(UART_PORT, ' ');						// "bin2emmc 0 1 fusion_writeraa "
			uart_send(UART_PORT, *(offsetValue+i));			// "bin2emmc 0 1 fusion_writeraa 0x80000000"
			uart_TX(UART_PORT, ';');						// "bin2emmc 0 1 fusion_writeraa 0x80000000;"
		}
		
		// step 6: reset TV after burn
		uart_send(UART_PORT, "reset");						// send reset command
		uart_TX(UART_PORT, ENTER_KEY);
		delayMS(OFFLINE_IMAGE_TO_FACTORY_MODE*60*1000);		// wait 19 mins until mainboard is on in factory mode
		
		// step 7: power off & on twice to simulate final assambly line
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);				// turn the power off
		delayMS(4000);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		delayMS(120000);
		
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);				// turn the power off
		delayMS(4000);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		delayMS(120000);
		
		// step 8: send shipping init (plug&play) command
		shippingInit();	// brand logo appears after command 35 secs later

		// step 9: wait defined test period and power off and on TV
		delayMS(trialTime);
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);				// turn the power off
		delayMS(3000);
		write_GP(RELAY_PORT, RELAY_PIN, LOW);				// turn the power on
		
		// step 10: sense the light of the specific pixels of screen
		delayMS(WELCOME_SCREEN_TIME*1000);
		value = getLightLevel();
		
		// step 11: decide if welcome screen appeared or not
		if(value <= LIGHT_THRESHOLD) // Welcome screen arrived
		{
			valueStr = int2char(trialTime);
			uart_send(UART_PORT, "\nTEST OK for ");
			uart_send(UART_PORT, valueStr);
			uart_send(UART_PORT, " ms trial time\n");
			free(valueStr);
			testResult = 1;
		}
		else	// no welcome screen! PROBLEM OCCURED !!
		{
			valueStr = int2char(trialTime);
			uart_send(UART_PORT, "\nTEST FAILED for ");
			uart_send(UART_PORT, valueStr);
			uart_send(UART_PORT, " ms trial time\n");
			uart_TX(UART_PORT, ENTER_KEY);
			free(valueStr);
			testResult = 0;
		}
		
		// step 12: power off TV
		write_GP(RELAY_PORT, RELAY_PIN, HIGH);				// turn the power off
		write_GP(INFO_LED_PORT, INFO_LED_PIN, LOW);			// test info LED is off: test is over
		delayMS(4000);
		
		// step 13: check if the test is repated as many as defined
		if(i == TEST_CYCLE){
			i = 0;
			trialTime -= SHUT_DOWN_STEP;
		}
		
		if(testResult == 0) 								// no welcome screen
			while(read_GP(RESUME_PORT, RESUME_PIN) == 0)	// until tester's resume signal, wait unlimited time
			{
				toggle_GP(INFO_LED_PORT, INFO_LED_PIN);		// test info LED is blinking fast: no welcome screen
				delayMS(100);
			}
	}
	
	while(1)																				// TEST IS DONE SUCCESSFULLY
	{
		toggle_GP(INFO_LED_PORT, INFO_LED_PIN);				// test info LED is blinking slow: test is over and no problem occured
		delayMS(1000);
	}
		
	return 0;
}

/* 
* Configurates and initiliazes necessary GPIOs, UART, ADC.
*
* @param none
* @return none
*/
void setup(void)
{
	systick_init();											// for systick delays
	gpio_init(RELAY_PORT, RELAY_PIN, OUT10, OUT_GP_PP);		// initilize GPIO for relay
	write_GP(RELAY_PORT, RELAY_PIN, HIGH);					// condition on start
	gpio_init(INFO_LED_PORT, INFO_LED_PIN, OUT10, OUT_GP_PP);// initilize GPIO for result LED
	write_GP(INFO_LED_PORT, INFO_LED_PIN, LOW);				// condition on start
	adc_init(ADC_PORT, LDR_PORT, LDR_PIN);					// initilize ADC for LDR
	uart_init(UART_PORT, UART_SPEED);						// initilize uart for communication with TV mainboard
	delayMS(100);
	
	/* uart comfigurations */
	serialPort.mode = 0; 				/* 0: process /// 1/2/3: brigde to uart1/2/3 */
	serialPort.signal = 0;				/* message recieved signal */
	serialPort.strategy = 1; 			/* 1:terminator /// 0:interrupt */
	serialPort.terminator = '\n';
}

/* 
* Sends hexadecimal commands to UART to set TV shipping mode
*
* @param none
* @return none
*/
void shippingInit(void)
{
	/* start session command */
	uart_TX_hex(UART_PORT, 0xE0);
	uart_TX_hex(UART_PORT, 0x04);
	uart_TX_hex(UART_PORT, 0x01);
	uart_TX_hex(UART_PORT, 0x1A);
	delayMS(1000);
	
	/* plug play command */
	uart_TX_hex(UART_PORT, 0xE0);
	uart_TX_hex(UART_PORT, 0x04);
	uart_TX_hex(UART_PORT, 0x6C);
	uart_TX_hex(UART_PORT, 0xAF);
}

/* 
* Reads ADC value 100 times in a second, and calculates the average.
*
* @param none
* @return value of ADC
*/
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

/* 
* This function is from STM32 library to handle UART interrupts
*
* @param none
* @return none
*/
void USART2_IRQHandler(void)
{
	uart_ISR(UART_PORT, &serialPort);
}
