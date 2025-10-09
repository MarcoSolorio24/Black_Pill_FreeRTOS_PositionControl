#include "main.h"

extern TIM_HandleTypeDef HTIM1;
extern TIM_HandleTypeDef HTIM2;
extern UART_HandleTypeDef Uart1;


void System_Init(void)
{
	GPIO_Init();
	Clock_Init();
	TIM2_Init();
	UART1_Init();
	TIM1_Init();
}

void Clock_Init(void)
{
	RCC_OscInitTypeDef Osc_Config = {0};
	Osc_Config.HSIState = RCC_HSI_OFF; //ON
	Osc_Config.HSEState = RCC_HSE_ON;  //OF
	Osc_Config.LSEState = RCC_LSE_OFF;
	Osc_Config.LSIState = RCC_LSI_OFF;
	Osc_Config.OscillatorType = RCC_OSCILLATORTYPE_HSE; //HSI
	Osc_Config.PLL.PLLState = RCC_PLL_ON;
	Osc_Config.PLL.PLLSource = RCC_PLLSOURCE_HSE; //HSI
	Osc_Config.PLL.PLLM = 25; //4
	Osc_Config.PLL.PLLN = 200; //50
	Osc_Config.PLL.PLLP = RCC_PLLP_DIV2;
	Osc_Config.PLL.PLLQ = 4;

	if(HAL_RCC_OscConfig(&Osc_Config) != HAL_OK)
		Error_Handler();
	RCC_ClkInitTypeDef Clk_Config = {0};
	Clk_Config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	Clk_Config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	Clk_Config.AHBCLKDivider = RCC_SYSCLK_DIV1;
	Clk_Config.APB1CLKDivider = RCC_SYSCLK_DIV2;
	Clk_Config.APB2CLKDivider = RCC_SYSCLK_DIV1;

	if(HAL_RCC_ClockConfig(&Clk_Config, FLASH_LATENCY_4) != HAL_OK)
			Error_Handler();

}
void TIM2_Init(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef qei_pin = {0};
	qei_pin.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	qei_pin.Mode = GPIO_MODE_AF_OD;
	qei_pin.Pull = GPIO_PULLUP;
	qei_pin.Alternate = GPIO_AF1_TIM2;//Enables the timer of advanced control
	HAL_GPIO_Init(GPIOA, &qei_pin);

	HTIM2.Instance = TIM2;
	HTIM2.Init.Period = 0xFFFFFFFF;//The max count reachable
	HTIM2.Init.Prescaler = 0;
	HAL_TIM_Base_Init(&HTIM2);

	TIM_Encoder_InitTypeDef qei_config = {0};
	qei_config.EncoderMode = TIM_ENCODERMODE_TI12;
	qei_config.IC1Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
	qei_config.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	qei_config.IC2Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
	qei_config.IC2Selection = TIM_ICSELECTION_DIRECTTI;

	if(HAL_TIM_Encoder_Init(&HTIM2, &qei_config) != HAL_OK)
		Error_Handler();


	HAL_TIM_Encoder_Start(&HTIM2, TIM_CHANNEL_ALL);
}

void UART1_Init(void)
{
	//1. Habilitar la seÃ±al del reloj del periferio/
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//2. Cponfigurar los pines del periferico/
	GPIO_InitTypeDef Tx_Pin = {0};
	Tx_Pin.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	Tx_Pin.Mode = GPIO_MODE_AF_PP;
	Tx_Pin.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &Tx_Pin);

	//3. Dar de alta una interrupcion/

	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	//4. Configuracion de alto nivel/
	Uart1.Instance = USART1;
	Uart1.Init.BaudRate = 115200;
	Uart1.Init.Mode = UART_MODE_TX_RX;

	if(HAL_UART_Init(&Uart1) != HAL_OK)
		Error_Handler();
}

void TIM1_Init(void)
{
	//1. Habilitar la seÃ±al del reloj del periferio/
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//2. Cponfigurar los pines del periferico/
	GPIO_InitTypeDef OC_Pin = {0};
	OC_Pin.Pin = GPIO_PIN_8;
	OC_Pin.Mode = GPIO_MODE_AF_PP;
	OC_Pin.Alternate = GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOA, &OC_Pin);

	//3. Dar de alta una interrupcion/

	//4. Configuracion de alto nivel/
	HTIM1.Instance = TIM1;
	HTIM1.Init.Prescaler = 3;
	HTIM1.Init.Period = 999;

	HAL_TIM_Base_Init(&HTIM1);

	TIM_OC_InitTypeDef Oc_Config = {0};
	Oc_Config.OCMode = TIM_OCMODE_PWM1;
	Oc_Config.Pulse = 0;
	Oc_Config.OCPolarity = TIM_OCPOLARITY_HIGH;
	HAL_TIM_PWM_ConfigChannel(&HTIM1, &Oc_Config, TIM_CHANNEL_1);

	//5. Arrancar el periferico/
	HAL_TIM_PWM_Start(&HTIM1, TIM_CHANNEL_1);

}

void GPIO_Init(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef Led_pin = {0};
	Led_pin.Pin = Led_Builtin;
	Led_pin.Mode = GPIO_MODE_OUTPUT_PP;
	Led_pin.Pull = GPIO_NOPULL;
	Led_pin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &Led_pin);


	GPIO_InitTypeDef dir_pin = {0};
	dir_pin.Pin = GPIO_PIN_14 |GPIO_PIN_15;
	dir_pin.Mode = GPIO_MODE_OUTPUT_PP;
	dir_pin.Pull = GPIO_NOPULL;
	dir_pin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &dir_pin);
}

void Error_Handler(void)
{
	GPIO_Init();
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
}
