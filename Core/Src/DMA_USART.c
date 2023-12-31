/*
 *      Comunicação UART via DMA - DMA_USART.c
 *
 *      Data: 27 de julho, 2023
 *      Autor: Gabriel Luiz
 *      Contato: (31) 97136-4334 || gabrielluiz.eletro@gmail.com
 */
/*
 * 		- Links Úteis -
 *
 *      DATASHEET:
 */

#include "DMA_USART.h"

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

//extern UART_HandleTypeDef huart2; /* */
extern UART_HandleTypeDef huart3; /* */
extern DMA_HandleTypeDef hdma_usart3_rx;
/* USER CODE END EV */

/* External functions ------------------------------------------------------------*/
/* USER CODE BEGIN EF */

extern void Error_Handler(); /* */
extern void LORA_ReceivedCallback();
extern uint8_t LORA_UART_BUFFER[100];
char *fistTERM = "\r\n<OK>\r\n\nAT+";
/* USER CODE END EF */

/* Private variables --------------------------------------------------------*/
/* USER CODE BEGIN PV */

uint8_t DMA_RX_Buffer_3[DMA_RX_BUFFER_SIZE]; /* */
uint16_t size = 0;
uint8_t POSICAO_INICIAL = 0;
/* USER CODE END PV */

/* Private functions ------------------------------------------------------------*/
/* USER CODE BEGIN PF */

/**
 * @brief
 * @param
 * @param
 * @retval ***NONE***
 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	/* Prevent unused argument(s) compilation warning */

	if (huart->Instance == USART3) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
		HAL_UART_DMAPause(&huart3);
		for (int i = 0; i < 10; i++)
			if (!memcmp(DMA_RX_Buffer_3 + i, fistTERM, strlen(fistTERM))) {
				LORA_ReceivedCallback(DMA_RX_Buffer_3);

			}
		HAL_UART_DMAResume(&huart3);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3, DMA_RX_Buffer_3,
		DMA_RX_BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
	}
}

/**
 * @brief
 * @param
 * @param
 * @retval ***NONE***
 */
void USART_Init(void) {
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, DMA_RX_Buffer_3,
	DMA_RX_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
//	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, DMA_RX_Buffer_2, DMA_RX_BUFFER_SIZE);
}

/* USER CODE END PF */
