#include "main.h"

TIM_HandleTypeDef HTIM1 = {0};
TIM_HandleTypeDef HTIM2 = {0};
UART_HandleTypeDef Uart1 = {0};

TimerHandle_t xTimer;
SemaphoreHandle_t xSem;
SemaphoreHandle_t zSem;
QueueHandle_t queue;

int32_t reference = 0, position = 0;
int16_t error = 0;

float Kp = 1.1938;//1.9;//1.68; //4.9; //1.1
float Td = 0.0061;//0.0383;//0.0287;//1.05; //0.0355
float Ti = 0.025;//0.035;//3.8; //0.025

float uout = 0.0;


uint8_t idx = 0;
char buffer[32] = {0}, byteRec = 0;

int32_t pos_samples[1000];

int main(int argc, char **argv)
{
	System_Init();
	xSem = xSemaphoreCreateBinary();
	zSem = xSemaphoreCreateBinary();
	queue = xQueueCreate(128,sizeof(char));

	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(250), pdTRUE,NULL, blinkTimer);
	xTaskCreate(sample_task, "sample", configMINIMAL_STACK_SIZE * 2, NULL, 0, NULL);
	xTaskCreate(control_task, "control", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate(command_task, "command", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTimerStart(xTimer, 10);
	vTaskStartScheduler();
	return EXIT_SUCCESS;
}

void control_task(void *pvParameters)
{
	int16_t pre_error = 0;
	int32_t sum_error = 0;

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(1);

	UNUSED(pvParameters);

	xLastWakeTime = xTaskGetTickCount();

	while(true)
	{

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		position = __HAL_TIM_GET_COUNTER(&HTIM2);

		error = reference - position;
		float Up = Kp * error;

		sum_error += error;

		if(sum_error > 1000) sum_error = 1000;
		if(sum_error < -1000) sum_error = -1000;

		float Ui = Kp * (Ts/Ti) * sum_error;
		float Ud = Kp * (Td/Ts) * (error - pre_error);

		pre_error = error;

		uout = Up + Ui + Ud;

		int16_t usat = (int16_t)uout;

		if(usat > 999) usat = 999;
		if(usat < -999) usat = -999;


		if(usat >= 0)
		{
			__HAL_TIM_SET_COMPARE(&HTIM1, TIM_CHANNEL_1, usat);

			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, HIGH);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, LOW);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&HTIM1, TIM_CHANNEL_1, -usat);

			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, LOW);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, HIGH);
		}

	}
}

void command_task(void *pvParameters)
{
	char *token;
	char bufout[32], buflen;

	UNUSED(pvParameters);

	HAL_UART_Receive_IT(&Uart1, (uint8_t *)&byteRec, 1);

	while(true)
	{
		xSemaphoreTake(xSem, portMAX_DELAY);
		token = strtok(buffer," ");
		if(strcmp(token, "REF") == 0){
			token = strtok(NULL, " ");
			reference = atol(token);
			xSemaphoreGive(zSem);
			buflen = sprintf(bufout, "Ok\tReference: %lu\r\n",reference);
			transmitData(bufout,buflen);
		}
		else if(strcmp(token,"POS") == 0){

			buflen = sprintf(bufout, "Position: %lu\r\n",position);
			transmitData(bufout,buflen);
		}
		else if(strcmp(token,"OUT") == 0){

			buflen = sprintf(bufout, "Out: %.2f\r\n",uout);
			transmitData(bufout,buflen);
		}
		else if(strcmp(token,"ERR") == 0){

			buflen = sprintf(bufout, "Error: %i\r\n",error);
			transmitData(bufout,buflen);
		}else{
			buflen = sprintf(bufout, "Unknown cmd\r\n");
			transmitData(bufout,buflen);
		}
	}
}
void transmitData(const char *ptr,uint8_t size)
{
	for(int i = 0; i < size; i++)
		xQueueSend(queue, ptr + i,portMAX_DELAY);
}

extern "C"
{
	void vApplicationIdleHook(void)
	{
		char byteToSend;
		if(xQueueReceive(queue, &byteToSend,0) == pdTRUE)
		{
			HAL_UART_Transmit(&Uart1, (uint8_t *)&byteToSend, 1, HAL_MAX_DELAY);
		}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xTaskWoken = pdFALSE;

	UNUSED(huart);

	if(huart->Instance == USART1)
	{
		if(byteRec == '\n')
		{
			buffer[idx++] = '\0';
			xSemaphoreGiveFromISR(xSem, &xTaskWoken);
			idx = 0;
		}
		else
		{
			buffer[idx++] = byteRec;
		}
	}

	HAL_UART_Receive_IT(&Uart1, (uint8_t *)&byteRec, 1);
	if(xTaskWoken == pdTRUE)
		portYIELD_FROM_ISR(xTaskWoken);
}

void sample_task(void *pvParameters){

	char bufout[32], buflen;

	UNUSED(pvParameters);

	while(1){

		xSemaphoreTake(zSem,portMAX_DELAY);

		for(int i = 0 ; i < 1000; i++){

			pos_samples[i] = position;
			vTaskDelay(pdMS_TO_TICKS(1));
		}

		buflen = sprintf(bufout, "Time\tPosition\r\n");
		HAL_UART_Transmit(&Uart1, (uint8_t *)bufout, buflen, HAL_MAX_DELAY);

		for(int i = 0; i < 1000; i++){

			buflen = sprintf(bufout, "%.3f\t%lu\r\n", i*Ts, pos_samples[i]);
			HAL_UART_Transmit(&Uart1, (uint8_t *)bufout, buflen, HAL_MAX_DELAY);
		}
	}
}



void blinkTimer(TimerHandle_t xTimer)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
