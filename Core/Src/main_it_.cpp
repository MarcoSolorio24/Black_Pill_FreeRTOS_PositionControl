#include <main.h>

extern UART_HandleTypeDef Uart1;

extern "C"{


	void USART1_IRQHandler(void)
	{
		HAL_UART_IRQHandler(&Uart1);
	}



}


