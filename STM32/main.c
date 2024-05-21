/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UartRingbuffer.h"
#include "NMEA.h"

#include <string.h>
#include <stdio.h>

#include "stdint.h"
#include "stdlib.h"
#include "strings.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ================================================ PM2.5 ================================================

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;

void parsePMS5003Data(uint8_t *buffer) {
  data.framelen = (buffer[0] << 8) | buffer[1];
  data.pm10_standard = (buffer[2] << 8) | buffer[3];
  data.pm25_standard = (buffer[4] << 8) | buffer[5];
  data.pm100_standard = (buffer[6] << 8) | buffer[7];
  data.pm10_env = (buffer[8] << 8) | buffer[9];
  data.pm25_env = (buffer[10] << 8) | buffer[11];
  data.pm100_env = (buffer[12] << 8) | buffer[13];
  data.particles_03um = (buffer[14] << 8) | buffer[15];
  data.particles_05um = (buffer[16] << 8) | buffer[17];
  data.particles_10um = (buffer[18] << 8) | buffer[19];
  data.particles_25um = (buffer[20] << 8) | buffer[21];
  data.particles_50um = (buffer[22] << 8) | buffer[23];
  data.particles_100um = (buffer[24] << 8) | buffer[25];
  data.unused = (buffer[26] << 8) | buffer[27];
  data.checksum = (buffer[30] << 8) | buffer[31];
}

uint8_t readPMSdata(UART_HandleTypeDef *huart) {
  uint8_t buffer[32];
  uint16_t sum = 0;

  /* Receive data from UART */
  if (HAL_UART_Receive(huart, buffer, 32, 1000) != HAL_OK) {
    return 0;
  }

  /* Check for start byte */
  if (buffer[0] != 0x42) {
    return 0;
  }

  /* Calculate checksum */
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  /* Compare checksums */
  /*if (sum != data.checksum) {
    return 0;
  }*/

  /* Parse received data */
  /* Implement parsePMS5003Data() function if required */
  // Parse the received data
  parsePMS5003Data(buffer);

  return 1;
}

// GPS
#define uart &huart1

#define TIMEOUT_DEF 500  // 500ms timeout
uint16_t timeout;
ring_buffer rx_buffer = { { 0 }, 0, 0};
ring_buffer tx_buffer = { { 0 }, 0, 0};

ring_buffer *_rx_buffer;
ring_buffer *_tx_buffer;

void store_char(unsigned char c, ring_buffer *buffer);


void Ringbuf_init(void)
{
  _rx_buffer = &rx_buffer;
  _tx_buffer = &tx_buffer;

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(uart, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(uart, UART_IT_RXNE);
}

void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % UART_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if(i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

/* checks, if the entered string is present in the giver buffer ?
 */
static int check_for (char *str, char *buffertolookinto)
{
	int stringlength = strlen (str);
	int bufferlength = strlen (buffertolookinto);
	int so_far = 0;
	int indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx])
		{
			indx++;
			if (indx>stringlength) return 0;
		}
	if (str[so_far] == buffertolookinto[indx])
	{
		while (str[so_far] == buffertolookinto[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == stringlength);
	else
	{
		so_far =0;
		if (indx >= bufferlength) return -1;
		goto repeat;
	}

	if (so_far == stringlength) return 1;
	else return -1;
}

int Uart_read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if(_rx_buffer->head == _rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
    return c;
  }
}

/* writes a single character to the uart and increments head
 */
void Uart_write(int c)
{
	if (c>=0)
	{
		int i = (_tx_buffer->head + 1) % UART_BUFFER_SIZE;
		while (i == _tx_buffer->tail);

		_tx_buffer->buffer[_tx_buffer->head] = (uint8_t)c;
		_tx_buffer->head = i;

		__HAL_UART_ENABLE_IT(uart, UART_IT_TXE); // Enable UART transmission interrupt
	}
}

/* checks if the new data is available in the incoming buffer
 */
int IsDataAvailable(void)
{
  return (uint16_t)(UART_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % UART_BUFFER_SIZE;
}

/* sends the string to the uart
 */
void Uart_sendstring (const char *s)
{
	while(*s) Uart_write(*s++);
}

void GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto)
{
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int so_far = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[so_far] != buffertocopyfrom[indx]) indx++;
	if (startString[so_far] == buffertocopyfrom[indx])
	{
		while (startString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == startStringLength) startposition = indx;
	else
	{
		so_far =0;
		goto repeat1;
	}

	so_far = 0;

repeat2:
	while (endString[so_far] != buffertocopyfrom[indx]) indx++;
	if (endString[so_far] == buffertocopyfrom[indx])
	{
		while (endString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == endStringLength) endposition = indx-endStringLength;
	else
	{
		so_far =0;
		goto repeat2;
	}

	so_far = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		buffertocopyinto[indx] = buffertocopyfrom[i];
		indx++;
	}
}

void Uart_flush (void)
{
	memset(_rx_buffer->buffer,'\0', UART_BUFFER_SIZE);
	_rx_buffer->head = 0;
	_rx_buffer->tail = 0;
}

int Uart_peek()
{
  if(_rx_buffer->head == _rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

/* copies the data from the incoming buffer into our buffer
 * Must be used if you are sure that the data is being received
 * it will copy irrespective of, if the end string is there or not
 * if the end string gets copied, it returns 1 or else 0
 * Use it either after (IsDataAvailable) or after (Wait_for) functions
 */
int Copy_upto (char *string, char *buffertocopyinto)
{
	int so_far =0;
	int len = strlen (string);
	int indx = 0;

again:
	while (Uart_peek() != string[so_far])
		{
			buffertocopyinto[indx] = _rx_buffer->buffer[_rx_buffer->tail];
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
			indx++;
			while (!IsDataAvailable());

		}
	while (Uart_peek() == string [so_far])
	{
		so_far++;
		buffertocopyinto[indx++] = Uart_read();
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

/* must be used after wait_for function
 * get the entered number of characters after the entered string
 */
int Get_after (char *string, uint8_t numberofchars, char *buffertosave)
{
	for (int indx=0; indx<numberofchars; indx++)
	{
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);  // wait until some data is available
		if (timeout == 0) return 0;  // if data isn't available within time, then return 0
		buffertosave[indx] = Uart_read();  // save the data into the buffer... increments the tail
	}
	return 1;
}

/* Waits for a particular string to arrive in the incoming buffer... It also increments the tail
 * returns 1, if the string is detected
 */
// added timeout feature so the function won't block the processing of the other functions
int Wait_for (char *string)
{
	int so_far =0;
	int len = strlen (string);

again:
	timeout = TIMEOUT_DEF;
	while ((!IsDataAvailable())&&timeout);  // let's wait for the data to show up
	if (timeout == 0) return 0;
	while (Uart_peek() != string[so_far])  // peek in the rx_buffer to see if we get the string
	{
		if (_rx_buffer->tail != _rx_buffer->head)
		{
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		}

		else
		{
			return 0;
		}
	}
	while (Uart_peek() == string [so_far]) // if we got the first letter of the string
	{
		// now we will peek for the other letters too
		so_far++;
		_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}




void Uart_isr (UART_HandleTypeDef *huart)
{
	  uint32_t isrflags   = READ_REG(huart->Instance->SR);
	  uint32_t cr1its     = READ_REG(huart->Instance->CR1);

    /* if DR is not empty and the Rx Int is enabled */
    if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
    	 /******************
    	    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	    	      *          sequence: a read operation to USART_SR register followed by a read
    	    	      *          operation to USART_DR register.
    	    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	    	      *          USART_SR register followed by a write operation to USART_DR register.
    	    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	 *********************/
		huart->Instance->SR;                       /* Read status register */
        unsigned char c = huart->Instance->DR;     /* Read data register */
        store_char (c, _rx_buffer);  // store data in buffer
        return;
    }

    /*If interrupt is caused due to Transmit Data Register Empty */
    if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
    {
    	if(tx_buffer.head == tx_buffer.tail)
    	    {
    	      // Buffer empty, so disable interrupts
    	      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

    	    }

    	 else
    	    {
    	      // There is more data in the output buffer. Send the next byte
    	      unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    	      tx_buffer.tail = (tx_buffer.tail + 1) % UART_BUFFER_SIZE;

    	      /******************
    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	      *          sequence: a read operation to USART_SR register followed by a read
    	      *          operation to USART_DR register.
    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	      *          USART_SR register followed by a write operation to USART_DR register.
    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	      *********************/

    	      huart->Instance->SR;
    	      huart->Instance->DR = c;

    	    }
    	return;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
int GMT = +7;
int inx =0;
int hr=0,min=0,day=0,mon=0,yr=0;
int daychange=0;

int decodeGGA(char *GGAbuffer, GGASTRUCT *gga){
	inx =0;
	char buffer[12];
	int i=0;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	while(GGAbuffer[inx] != ',') inx++;
	inx++;
	if((GGAbuffer[inx]== '1') || (GGAbuffer[inx]== '2') || (GGAbuffer[inx]== '6')){
		gga->isfixValid =1;
		inx=0;
	}else{
		gga->isfixValid=0;
		return 1;
	}
	while(GGAbuffer[inx]!= ',')inx++;

	inx++;   // reach the first number in time
		memset(buffer, '\0', 12);
		i=0;
		while (GGAbuffer[inx] != ',')  // copy upto the we reach the after time ','
		{
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}

		hr = (atoi(buffer)/10000) + GMT/100;   // get the hours from the 6 digit number

		min = ((atoi(buffer)/100)%100) + GMT%100;  // get the minutes from the 6 digit number

		// adjust time.. This part still needs to be tested
		if (min > 59)
		{
			min = min-60;
			hr++;
		}
		if (hr<0)
		{
			hr=24+hr;
			daychange--;
		}
		if (hr>=24)
		{
			hr=hr-24;
			daychange++;
		}

		// Store the time in the GGA structure
		gga->tim.hour = hr;
		gga->tim.min = min;
		gga->tim.sec = atoi(buffer)%100;
		/***************** Get LATITUDE  **********************/
		inx++;   // Reach the first number in the lattitude
		memset(buffer, '\0', 12);
		i=0;
		while (GGAbuffer[inx] != ',')   // copy upto the we reach the after lattitude ','
		{
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}
		if (strlen(buffer) < 6) return 2;  // If the buffer length is not appropriate, return error
		int16_t num = (atoi(buffer));   // change the buffer to the number. It will only convert upto decimal
		int j = 0;
		while (buffer[j] != '.') j++;   // Figure out how many digits before the decimal
		j++;
		int declen = (strlen(buffer))-j;  // calculate the number of digit after decimal
		int dec = atoi ((char *) buffer+j);  // conver the decimal part a a separate number
		float lat = (int)(num/100) + (((num%100) + (dec/pow(10, declen)))/60);  // 1234.56789 = 12.3456789
		if(lat > 500) return 2;
		gga->location.latitude = lat;  // save the lattitude data into the strucure
		inx++;
		gga->location.NS = GGAbuffer[inx];  // save the N/S into the structure
		//Grt long
		inx++;
		inx++;
		memset(buffer, '\0',12);
		i=0;
		while(GGAbuffer[inx]!=','){
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}
		num = (atoi(buffer));
		j=0;
		while(buffer[j]!='.')j++;
		j++;
		declen = (strlen(buffer))-j;
		dec =  atoi ((char *)buffer+j);
		lat = (float)((int)(num/100) + (((num%100) + (dec/pow(10, declen)))/60));
		if(lat > 500) return 2;
		gga->location.longitude=lat;
		inx++;
		gga->location.EW=GGAbuffer[inx];

		inx++;
		inx++;
		inx++;

		inx++;
		memset(buffer, '\0',12);
		i=0;
		while(GGAbuffer[inx]!=','){
			buffer[i]= GGAbuffer[inx];
			i++;
			inx++;
		}
		gga->numofsat = atoi(buffer);

		inx++;
		while(GGAbuffer[inx] != ',')inx++;

		inx++;
		memset(buffer,'\0',12);
		i=0;
		while(GGAbuffer[inx]!=','){
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}
		num = atoi(buffer);
		j=0;
		while(buffer[j]!='.')j++;
		j++;
		declen = (strlen(buffer))-j;
		dec =  atoi ((char *)buffer+j);
		lat = (num)+(dec/pow(10, (declen)));
		gga->alt.altitude=lat;
		inx++;
		gga->alt.unit= GGAbuffer[inx];
		return 0;
}


char GGA[100];

GPSSTRUCT gpsData;


int flagGGA = 0;
char lcdBuffer [50];


int VCCTimeout = 5000;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  Ringbuf_init();
  HAL_Delay(500);

  uint8_t printBuffer[200];
  uint8_t sendBuffer[200];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (Wait_for("GGA") == 1)
	  {
		  VCCTimeout = 5000;  // Reset the VCC Timeout indicating the GGA is being received
		  Copy_upto("*", GGA);
		  if (decodeGGA(GGA, &gpsData.ggastruct) == 0) flagGGA = 2;  // 2 indicates the data is valid
		  else flagGGA = 1;  // 1 indicates the data is invalid
	  }
	  //char printBuffer[200];
	  //sprintf(printBuffer, "latitude: %.6f, longitude: %.6f\r\n", &(&gpsData.ggastruct)->location.latitude, &(&gpsData.ggastruct)->location.longitude);
	  //HAL_UART_Transmit(&huart2, (uint8_t*)printBuffer, strlen(printBuffer), HAL_MAX_DELAY);
	  if ((flagGGA == 2 && data.pm25_standard != 0))
		  {
			  sprintf(printBuffer, "latitude: %.6f, longitude: %.6f\r\n", gpsData.ggastruct.location.latitude, gpsData.ggastruct.location.longitude);
			  HAL_UART_Transmit(&huart2, (uint8_t*)printBuffer, strlen(printBuffer), HAL_MAX_DELAY);
			  sprintf(printBuffer, "PM1: %d, PM2.5: %d, PM10: %d\r\n", data.pm10_standard, data.pm25_standard, data.pm100_standard);
			  HAL_UART_Transmit(&huart2, (uint8_t *)printBuffer, strlen(printBuffer), HAL_MAX_DELAY);

			  sprintf(sendBuffer, "%.6f,%.6f,%d,\r\n	", gpsData.ggastruct.location.latitude, gpsData.ggastruct.location.longitude, data.pm25_standard);
			  HAL_UART_Transmit(&huart6, (uint8_t*)sendBuffer, strlen(sendBuffer), HAL_MAX_DELAY);
			  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
			  data.pm25_standard = 0;
		  }

	 else if ((flagGGA == 1))
	 {
		// Instead of clearing the display, it's better if we print spaces.
		// This will avoid the "refreshing" part
		 char x[10];
		 sprintf(x,"GPS data is invalid\r\n");
		 HAL_UART_Transmit(&huart2, (uint8_t*)x, strlen(x), HAL_MAX_DELAY);
	 }
	 else {
		 char x[10];
		 sprintf(x,"No recent PM2.5 data\r\n");
		 HAL_UART_Transmit(&huart2, (uint8_t*)x, strlen(x), HAL_MAX_DELAY);
	 }
	 if (VCCTimeout <= 0)
	 {
		VCCTimeout = 5000;  // Reset the timeout
		flagGGA=0;
		 char x = '1';
		 //HAL_UART_Transmit(&huart2, x, strlen(x), HAL_MAX_DELAY);
	 }
	 if(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)){
		 uint8_t test[] = "13.746180,100.539258,999\r\n";
		 HAL_UART_Transmit(&huart6, (uint8_t*)test, strlen(test), HAL_MAX_DELAY);
		 HAL_UART_Transmit(&huart2, (uint8_t*)test, strlen(test), HAL_MAX_DELAY);
		 while(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
	 }

	 while(data.pm25_standard == 0){
		 if (readPMSdata(&huart6))
		{
		  // Reading data was successful
		  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
		  // Print data to PuTTY or other serial monitor
		  //char printBuffer[200]; // Adjust size according to your needs
		  //sprintf(printBuffer, "PM1: %d, PM2.5: %d, PM10: %d\r\n", data.pm10_standard, data.pm25_standard, data.pm100_standard);
		  //HAL_UART_Transmit(&huart2, (uint8_t *)printBuffer, strlen(printBuffer), 100);
		}
		 if(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)){
			 uint8_t test[] = "13.746180,100.539258,999\r\n";
			 HAL_UART_Transmit(&huart6, (uint8_t*)test, strlen(test), HAL_MAX_DELAY);
			 HAL_UART_Transmit(&huart2, (uint8_t*)test, strlen(test), HAL_MAX_DELAY);
			 while(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
		 }
	 }

	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
