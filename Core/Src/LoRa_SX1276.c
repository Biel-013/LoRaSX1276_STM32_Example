/*
 *      Comunicaçao chip SX1276 LoRa via Uart - LoRa_SX1276.c
 *
 *      Data: 29 de agosto, 2023
 *      Autor: Gabriel Luiz
 *      Contato: (31) 97136-4334 || gabrielluiz.eletro@gmail.com
 */
/*
 * 		- Links Úteis -
 *
 */

/* Private Includes ----------------------------------------------------------*/
/* USER CODE BEGIN PI */

#include "LoRa_SX1276.h"
#include <string.h>
#include <stdio.h>

/* USER CODE END PI */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

extern UART_HandleTypeDef huart3; /* Variável externa de configuração do UART */
/* USER CODE END EV */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LORA_HANDLER_UART &huart3 /* Ponteiro para o handle de configuração UART */

/* USER CODE END PD */

/* External functions ------------------------------------------------------------*/
/* USER CODE BEGIN EF */
/* USER CODE END EF */

/* Private variables --------------------------------------------------------*/
/* USER CODE BEGIN PV */

char LORA_UART_BUFFER[100];

LoRa_StatusTypeDef LORA_STATUS_RECEIVE = LORA_CLEAR;
unsigned char AT_RXcommand[50];
unsigned char AT_TXcommand[50];

/* USER CODE END PV */

/* Private functions ------------------------------------------------------------*/
/* USER CODE BEGIN PF */
void LORA_ReceivedCallback(uint8_t buffer[50]) {
	int posicao_inicial = 0;
	int posicao_final = 0;
	for (int i = 0; i < 70; i++) {
		if (!memcmp(buffer + i, "AT+", 3)) {
			posicao_inicial = i;
			break;
		}
	}
	for (int i = posicao_inicial; i < 70; i++) {
		if (!memcmp(buffer + i, "<OK>", 4)) {
			posicao_final = i + 4;
			break;
		}
	}
	if (posicao_inicial != 0 && posicao_final != 0) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
		for (int i = posicao_inicial; i < 100; i++) {
			if (i <= posicao_final + 1)
				LORA_UART_BUFFER[i - posicao_inicial] = buffer[i];
			else
				LORA_UART_BUFFER[i] = '\000';
		}
		LORA_STATUS_RECEIVE = LORA_OK;
		for (int i = 0; i < 15; i++)
			buffer[i] = '\0';
		return;
	}
	for (int i = 0; i < 15; i++)
		buffer[i] = '\0';
	LORA_STATUS_RECEIVE = LORA_FAILED;
}

LoRa_StatusTypeDef LORA_TransmitCommand(uint16_t _Timeout) {
	if (HAL_UART_Transmit(LORA_HANDLER_UART, AT_TXcommand,
			strlen((char*) AT_TXcommand), _Timeout) != HAL_OK) {
		HAL_Delay(20);
		return LORA_FAILED;
	}
	return LORA_OK;
}

LoRa_StatusTypeDef LORA_ReceiveCommand(uint16_t _TimerWait, uint16_t _Periodo) {
	HAL_UART_Transmit(LORA_HANDLER_UART, AT_RXcommand,
			strlen((char*) AT_RXcommand), 100);
	HAL_Delay(20);

	uint32_t Timer_start = HAL_GetTick();
	while (LORA_STATUS_RECEIVE != LORA_OK) {
		if (((HAL_GetTick() - Timer_start) % _Periodo) == 0)
			if (HAL_UART_Transmit(LORA_HANDLER_UART, AT_RXcommand,
					strlen((char*) AT_RXcommand), 100) != HAL_OK)
				return LORA_FAILED;
		if ((HAL_GetTick() - Timer_start) > _TimerWait)
			return LORA_TIMEOUT;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*---- IDENTIDICADOR DO DISPOSITIVO FINAL --------------------------------------------------------------*/

/**
 * @brief Identificador de dispositivo final no espaço de endereço IEEE EUI64
 * @tparam AT+DEVEUI <devEUI> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Identifier: Identificador de dispositivo final
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_EndDeviceIdentifier(LoRa_OperationTypeDef _Operacao,
		LoRa_Id *_Identifier) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+DEVEUI\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx%8lx\r\n", AT_RXcommand,
				&(((uint32_t*) _Identifier)[1]),
				&(((uint32_t*) _Identifier)[0]));
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+DEVEUI %08lX%08lX\r\n",
				((uint32_t*) _Identifier)[1], ((uint32_t*) _Identifier)[0]);
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ID DE APLICATIVO GLOBAL -----------------------------------------------------------*/

/**
 * @brief ID de aplicativo global no espaço de endereço IEEE EUI64
 * @tparam AT+APPEUI <AppEUI> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Identifier: ID de aplicativo global
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_AppEUIAdress(LoRa_OperationTypeDef _Operacao,
		LoRa_Id *_Identifier) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+APPEUI\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx%8lx\r\n", AT_RXcommand,
				&(((uint32_t*) _Identifier)[1]),
				&(((uint32_t*) _Identifier)[0]));
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+APPEUI %08lX%08lX\r\n",
				((uint32_t*) _Identifier)[1], ((uint32_t*) _Identifier)[0]);
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CHAVE DO DISPOSITIVO FINAL -----------------------------------------------------------*/

/**
 * @brief Chave de raiz AES-128 específica para o dispositivo final
 * @tparam AT+APPKEY <AppKey> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Keyword: Chave do dispositivo final
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ApplicationKey(LoRa_OperationTypeDef _Operacao,
		LoRa_KeyTypeDef *_Keyword) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+APPKEY\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx%8lx%8lx%8lx\r\n", AT_RXcommand,
				&(_Keyword->LoRa_HighKey[1]), &(_Keyword->LoRa_HighKey[0]),
				&(_Keyword->LoRa_LowKey[1]), &(_Keyword->LoRa_LowKey[0]));
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+APPKEY %08lX%08lX%08lX%08lX\r\n",
				(_Keyword->LoRa_HighKey[1]), (_Keyword->LoRa_HighKey[0]),
				(_Keyword->LoRa_LowKey[1]), (_Keyword->LoRa_LowKey[0]));
		if (LORA_TransmitCommand(500) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- STATUS DA REDE PÚBLICA -----------------------------------------------------------*/

/**
 * @brief Status do modo de rede pública
 * @tparam AT+PNM <0 | 1> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Status: Status da rede pública
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_PublicNetworkModeStatus(LoRa_OperationTypeDef _Operacao,
		LoRa_PublicNetworkTypeDef *_Status) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+PNM\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Status);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+PNM %d\r\n", (*_Status));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONFIGURAÇÃO DO MODO DE INGRESSO NA REDE -----------------------------------------------------------*/

/**
 * @brief Comando para configuração do modo de ingresso na rede, reinicie após a configuração ser atualizada
 * @tparam AT+NJM <0 | 1> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Mode: Modo de ingresso na rede
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_NetworkJoinMode(LoRa_OperationTypeDef _Operacao,
		LoRa_NetworkJoinModeTypeDef *_Mode) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+NJM\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Mode);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+NJM %d\r\n", (*_Mode));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DEFINIÇÃO DE LORA MAC -----------------------------------------------------------*/

/**
 * @brief Comando para definir a classe LoRa MAC
 * @tparam AT+CLASS <LoRa MAC Class> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Class: Classe de MAC
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_LoRaMacClass(LoRa_OperationTypeDef _Operacao,
		LoRa_MacClassTypeDef *_Class) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+CLASS\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%c\r\n", AT_RXcommand, (char*) _Class);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+CLASS %c\r\n", (*_Class));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- INGRESSO NA REDE LORA -----------------------------------------------------------*/

/**
 * @brief Comando para ingressar no servidor de rede LoRa
 * @tparam AT+JOIN <ENTER>
 * @param ***NONE***
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_JoinRequestNetworkServer(void) {
	sprintf((char*) AT_TXcommand, "AT+JOIN\r\n");
	if (LORA_TransmitCommand(100) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- STATUS DE INGRESSO NA REDE LORA -----------------------------------------------------------*/

/**
 * @brief Comando para verificar o status de ingresso na rede LoRa
 * @tparam AT+NJS <ENTER>
 * @param _Status: Status de ingresso
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_JoinNetworkServerStatus(LoRa_NetworkJoinTypeDef *_Status) {
	sprintf((char*) AT_RXcommand, "AT+NJS\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(500, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, (uint16_t*) _Status);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- SOLICITAÇÃO DE INGRESSO NA REDE AUTOMÁTICO -----------------------------------------------------------*/

/**
 * @brief Comando para definir a solicitação de ingresso na rede automática quando o dispositivo é inicializado
 * @tparam AT+AJOIN <0 | 1> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Status: Status de solicitação de ingresso automático
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_AutoJoinNetworkServer(LoRa_OperationTypeDef _Operacao,
		LoRa_AutoNetworkJoinTypeDef *_Status) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+AJOIN\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Status);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+AJOIN %d\r\n", (*_Status));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CHAVE DA SESSÃO DE REDE -----------------------------------------------------------*/

/**
 * @brief Chave da sessão de rede
 * @tparam AT+NWKSKEY <Network Session Key> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Keyword: Chave da sessão
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_NetworkSessionKey(LoRa_OperationTypeDef _Operacao,
		LoRa_KeyTypeDef *_Keyword) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+NWKSKEY\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx%8lx%8lx%8lx\r\n", AT_RXcommand,
				&(_Keyword->LoRa_HighKey[1]), &(_Keyword->LoRa_HighKey[0]),
				&(_Keyword->LoRa_LowKey[1]), &(_Keyword->LoRa_LowKey[0]));
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+NWKSKEY %08lX%08lX%08lX%08lX\r\n",
				(_Keyword->LoRa_HighKey[1]), (_Keyword->LoRa_HighKey[0]),
				(_Keyword->LoRa_LowKey[1]), (_Keyword->LoRa_LowKey[0]));
		if (LORA_TransmitCommand(500) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CHAVE DA SESSÃO DO APLICATIVO -----------------------------------------------------------*/

/**
 * @brief Chave de sessão do aplicativo
 * @tparam AT+APPSKEY <APPSKEY> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Keyword: Chave da sessão
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ApplicationSessionKey(LoRa_OperationTypeDef _Operacao,
		LoRa_KeyTypeDef *_Keyword) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+APPSKEY\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx%8lx%8lx%8lx\r\n", AT_RXcommand,
				&(_Keyword->LoRa_HighKey[1]), &(_Keyword->LoRa_HighKey[0]),
				&(_Keyword->LoRa_LowKey[1]), &(_Keyword->LoRa_LowKey[0]));
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+APPSKEY %08lX%08lX%08lX%08lX\r\n",
				(_Keyword->LoRa_HighKey[1]), (_Keyword->LoRa_HighKey[0]),
				(_Keyword->LoRa_LowKey[1]), (_Keyword->LoRa_LowKey[0]));
		if (LORA_TransmitCommand(500) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ENDEREÇO DO DISPOSITIVO -----------------------------------------------------------*/

/**
 * @brief Endereço do dispositivo
 * @tparam AT+DADDR <Device Address> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Adress: Endereço
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_DeviceAddress(LoRa_OperationTypeDef _Operacao,
		LoRa_Adress *_Adress) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+DADDR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx\r\n", AT_RXcommand,
				(uint32_t*) _Adress);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+DADDR %08lX\r\n",
				((uint32_t*) _Adress)[0]);
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- IDENTIFICADOR DA REDE -----------------------------------------------------------*/

/**
 * @brief Valor de exibição do identificador da rede
 * @tparam AT+NWKID <Network Address> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Identifier: Identificador da rede
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_NetworkIdentifier(LoRa_OperationTypeDef _Operacao,
		LoRa_Adress *_Identifier) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+NWKID\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%8lx\r\n", AT_RXcommand,
				(uint32_t*) _Identifier);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+NWKID %06lX\r\n",
				((uint32_t*) _Identifier)[0]);
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONFIGURAÇOES ATIVAS -----------------------------------------------------------*/

/**
 * @brief Retorna as configurações ativas
 * @tparam AT+AINF <ENTER>
 * @attention Essa função não foi continuada, por haver outras funções que realizam o mesmo trabalho
 * @param _hSettings: Configurações ativas
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ActivationSettingValue(
		LoRa_ActivationSettingTypeDef *_hSettings) {
	return LORA_FAILED;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ENVIO DE DADOS (STRING) -----------------------------------------------------------*/

/**
 * @brief Comando LoRa para envio de dados do tipo string
 * @tparam AT+SEND <application port>:<data> <ENTER>
 * @param _Port: Porta para envio de dados
 * @param _Data: Dados para envio
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_DataUplinkText(LoRa_Value _Port, LoRa_Data _Data[5]) {
	sprintf((char*) AT_TXcommand, "AT+SEND %hu:%5s\r\n", _Port, _Data);
	if (LORA_TransmitCommand(100) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ENVIO DE DADOS (HEXADECIMAL) -----------------------------------------------------------*/

/**
 * @brief Comando LoRa para envio de dados do tipo hexacimal
 * @tparam  AT+SENDB <application port>:<data> <ENTER>
 * @param _Port: Porta para envio de dados
 * @param _Data: Dados para envio
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_DataUplinkHexadecimal(LoRa_Value _Port,
		LoRa_Data _Data[5]) {
	sprintf((char*) AT_TXcommand, "AT+SENDB %hu:%02hx%02hx%02hx%02hx%02hx\r\n",
			_Port, _Data[4], _Data[3], _Data[2], _Data[1], _Data[0]);
	if (LORA_TransmitCommand(100) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- RECEBIMENTO DE DADOS (STRING) -----------------------------------------------------------*/

/**
 * @brief Comando para leitura de dados de recebidos
 * @tparam AT+RECV <ENTER>
 * @param _Port: Porta dos dados recebidos
 * @param _Data: Dados recebidos
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ConfirmDownlinkDataText(LoRa_Value *_Port,
		LoRa_Data _Data[5]) {
	sprintf((char*) AT_RXcommand, "AT+RECV\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%hu:%s\r\n", AT_RXcommand, (uint16_t*) _Port,
			_Data);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- RECEBIMENTO DE DADOS (HEXADECIMAL) -----------------------------------------------------------*/

/**
 * @brief Comando para leitura de dados de recebidos
 * @tparam AT+RECVB <ENTER>
 * @param _Port: Porta dos dados recebidos
 * @param _Data: Dados recebidos
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ConfirmDownlinkDataHexadecimal(LoRa_Value *_Port,
		LoRa_Data _Data[5]) {
	sprintf((char*) AT_RXcommand, "AT+RECVB\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%hu:%s\r\n", AT_RXcommand, (uint16_t*) _Port,
			_Data);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- VALOR DO RSSI DE LEITURA -----------------------------------------------------------*/

/**
 * @brief Valor RSSI de leitura dos últimos dados recebidos
 * @tparam AT+RSSI <ENTER>
 * @param  _Value: Valor do RSSI
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ReturnRSSI(LoRa_RSSI *_Value) {
	sprintf((char*) AT_RXcommand, "AT+RSSI\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%hd\r\n", AT_RXcommand, (int16_t*) _Value);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- VALOR DO SNR DE LEITURA -----------------------------------------------------------*/

/**
 * @brief Leitura do valor SNR (relação sinal-ruído) dos últimos dados recebidos
 * @tparam AT+SNR <ENTER>
 * @param _Value: Valor do SNR
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ReturnsSNR(LoRa_Value *_Value) {
	sprintf((char*) AT_RXcommand, "AT+SNR\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(500, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%hd\r\n", AT_RXcommand, (int16_t*) _Value);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONFIGURAÇÃO DA REGIÃO LORAMAC -----------------------------------------------------------*/

/**
 * @brief Retorna a configuração da região LoRaMAC. Reinicie após a atualização da configuração
 * @tparam  AT+REGION <0 - 9> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Region: Região de LoRaMAC
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_LoRaMacRegion(LoRa_OperationTypeDef _Operacao,
		LoRa_LoraMacRegionTypeDef *_Region) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+REGION\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hd\r\n", AT_RXcommand,
				(uint16_t*) _Region);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+REGION %hu\r\n", (*_Region));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TAXA DE DADOS AUTOMÁTICO -----------------------------------------------------------*/

/**
 * @brief Configuração de Auto Data Rate (ADR)
 * @tparam AT+ADR <0 | 1> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Status: Status do ADR
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_AutoDateRate(LoRa_OperationTypeDef _Operacao,
		LoRa_AutoDataRateTypeDef *_Status) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+ADR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hd\r\n", AT_RXcommand,
				(uint16_t*) _Status);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+ADR %hu\r\n", (*_Status));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONFIGURAÇÃO DE TAXA DE DADOS -----------------------------------------------------------*/

/**
 * @brief Status da configuração da taxa de dados. O fator de espalhamento (SF) pode variar de acordo com a região
 * @tparam AT+DR <data rate> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _DateRate: Taxa de dados
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_DataRateCommand(LoRa_OperationTypeDef _Operacao,
		LoRa_DataRateTypeDef *_DateRate) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+DR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hd\r\n", AT_RXcommand,
				(uint16_t*) _DateRate);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+DR %hu\r\n", (*_DateRate));
		if (LORA_TransmitCommand(100) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- FREQUÊNCIA RX DA JANELA 2 -----------------------------------------------------------*/

/**
 * @brief Comando da frequência Rx da janela 2
 * @tparam AT+RX2FQ <Rx Window 2> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Rate: Frequência
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RxWindow2Frequency(LoRa_OperationTypeDef _Operacao,
		LoRa_Rate *_Rate) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+RX2FQ\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%lu\r\n", AT_RXcommand, _Rate);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+RX2FQ %lu\r\n", (*_Rate));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TAXA DE DADOS RX DA JANELA 2 -----------------------------------------------------------*/

/**
 * @brief Comando da taxa de dados da janela Rx 2 (0-7 correspondente a DR_X)
 * @tparam AT+RX2DR <0-7 corresponding to DR_X > <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Taxa de dados
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RxWindow2DataRate(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+RX2DR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+RX2DR %hu\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DELAY ENTRE JANELAS TX E RX1 -----------------------------------------------------------*/

/**
 * @brief Define o atraso entre o final da janela TX e a janela Rx 1 em ms
 * @tparam AT+RX1DL <delay> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Delay em ms
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_TxRxWindow1Delay(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+RX1DL\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+RX1DL %hu\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DELAY ENTRE JANELAS TX E RX2 -----------------------------------------------------------*/

/**
 * @brief Define o atraso entre o final do TX e a janela Rx 2 em ms
 * @tparam  AT+RX2DL <delay> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Delay em ms
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_TxRxWindow2Delay(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+RX2DL\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+RX2DL %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DELAY DE JUNÇÃO DA JANELA RX1 -----------------------------------------------------------*/

/**
 * @brief Comando para acessar o atraso de junção na janela RX 1 em ms
 * @tparam AT+JN1DL <delay> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Delay em ms
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_TxRxWindow1JoinDelay(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+JN1DL\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+JN1DL %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DELAY DE ACEITAÇÃO DE JUNÇÃO DAS JANELAS TX E RX2 ------------------------------------------------*/

/**
 * @brief Defina o atraso de aceitação de junção entre o final do TX e a junção da janela Rx 2 em ms
 * @tparam AT+JN2DL <delay> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Delay em ms
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_TxRxWindow2JoinDelay(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+JN2DL\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+JN2DL %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- REPETIÇÃO DE UPLINKS NÃO CONFIRMADOS -----------------------------------------------------------*/

/**
 * @brief Comando para repetir o uplink não confirmado sem aguardar o reconhecimento do servidor (1 - 15)
 * @tparam AT+MUFR <number> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Número de repetiçoes
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RepeatUnconfirmedUplink(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+MUFR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+MUFR %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*----  -----------------------------------------------------------*/

/**
 * @brief Comando para reenvio de uplink confirmado. O envio se repete até que uma confirmação servidor chegue (1 - 8)
 * @tparam  AT+MCFR <number> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Número de repetiçoes
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ResendConfirmedUplink(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+MCFR\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+MCFR %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ÍNDICE DE POTÊNCIA TX -----------------------------------------------------------*/

/**
 * @brief Comando de Índice de Potência Tx
 * @tparam AT+TXP <index(0 ~ 10)> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Índice de Potência
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_TxPowerIndex(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+TXP\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+TXP %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONTADOR DE UPLINKS -----------------------------------------------------------*/

/**
 * @brief Comando para acesso ao contador de uplinks (0 - 65535)
 * @tparam AT+FCU <number> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Número de uplinks
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_UplinkCounter(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+FCU\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+FCU %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONTADOR DE DOWNLINKS -----------------------------------------------------------*/

/**
 * @brief Comando para acesso ao contador de downlinks (0 - 65535)
 * @tparam AT+FCD <number> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Value: Número de downlinks
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_DownlinkCounter(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Value) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+FCD\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Value);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+FCD %u\r\n", (*_Value));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- NÍVEL DE BATERIA DO DISPOSITIVO FINAL -----------------------------------------------------------*/

/**
 * @brief Este comando permite ao usuário acessar o nível da bateria do dispositivo final
 * @tparam AT+BAT <Battery level> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Level: Nível de bateria
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_BatteryLevel(LoRa_OperationTypeDef _Operacao,
		LoRa_BateryLevelTypeDef *_Level) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+FCD\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Level);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+FCD %u\r\n", (*_Level));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- VERIFICAÇÃO DE LINK -----------------------------------------------------------*/

/**
 * @brief Comando usado para verificar se o link está funcionando corretamente
 * @attention Essa função não foi continuada, por não apresentar utilidade para o fim dessa biblioteca
 * @tparam AT+LCHK <ENTER>
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_MacLineCheckRequest(void) {
	sprintf((char*) AT_TXcommand, "AT+LCHK\r\n");
	if (LORA_TransmitCommand(300) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CRIPTOGRAFIA DE LEITURA -----------------------------------------------------------*/

/**
 * @brief Configuração de criptografia de leitura
 * @tparam AT+CRYPTO <number> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Encryption: Modo de encriptação
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_EncryptionConfiguration(LoRa_OperationTypeDef _Operacao,
		LoRa_ReadoutEncryptionTypeDef *_Encryption) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+CRYPTO\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Encryption);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+CRYPTO %u\r\n", (*_Encryption));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- LEITURA E ATUALIZAÇÃO DE CONFIGURAÇÃO DE CANAL -----------------------------------------------------------*/

/**
 * @brief Comando para ler as configurações atuais do canal, e atualizar-las
 * @tparam AT+CH [ freq? / drrange? / status? / <ch> ? ] <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _ChOperation: Configuração a ser lida ou modificada
 * @param _hConfiguration: Handler de configurações do canal
 * @retval Status de execução do comando
 * AT+CH 1?\rfreq 902500000 drrange 0 to 3 status 1\r\n\
 */
LoRa_StatusTypeDef AT_ChannelConfiguration(LoRa_OperationTypeDef _Operacao,
		LoRa_ChannelOperationTypeDef _ChOperation,
		LoRa_ChannelConfigurationTypeDef *_hConfiguration) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		switch (_ChOperation) {
		case AT_CHANNEL_OPERATION_CHANNEL:
			sprintf((char*) AT_RXcommand, "AT+CH %hu?\r\n",
					_hConfiguration->LoRa_Channel);
			LORA_STATUS_RECEIVE = LORA_CLEAR;
			if (LORA_ReceiveCommand(1000, 10) != LORA_OK)
				return LORA_FAILED;
			sscanf(LORA_UART_BUFFER,
					"%s %hu?\rfreq %lu drrange %hu to %hu status %hu\r\n",
					AT_RXcommand, &_hConfiguration->LoRa_Channel,
					&_hConfiguration->LoRa_Frequency,
					&_hConfiguration->LoRa_MinDrRange,
					&_hConfiguration->LoRa_MaxDrRange,
					&_hConfiguration->LoRa_StatusChannel);
			break;
		case AT_CHANNEL_OPERATION_FREQ:
			sprintf((char*) AT_RXcommand, "AT+CH %hu freq?\r\n",
					_hConfiguration->LoRa_Channel);
			LORA_STATUS_RECEIVE = LORA_CLEAR;
			if (LORA_ReceiveCommand(750, 20) != LORA_OK)
				return LORA_FAILED;
			sscanf(LORA_UART_BUFFER, "%s %hu freq?\r%lu\r\n", AT_RXcommand,
					&_hConfiguration->LoRa_Channel,
					&_hConfiguration->LoRa_Frequency);
			break;
		case AT_CHANNEL_OPERATION_DRRANGE:
			sprintf((char*) AT_RXcommand, "AT+CH %hu drrange?\r\n",
					_hConfiguration->LoRa_Channel);
			LORA_STATUS_RECEIVE = LORA_CLEAR;
			if (LORA_ReceiveCommand(500, 10) != LORA_OK)
				return LORA_FAILED;
			sscanf(LORA_UART_BUFFER, "%s %hu drrange?\r%hu to %hu\r\n",
					AT_RXcommand, &_hConfiguration->LoRa_Channel,
					&_hConfiguration->LoRa_MinDrRange,
					&_hConfiguration->LoRa_MaxDrRange);
			break;
		case AT_CHANNEL_OPERATION_STATUS:
			sprintf((char*) AT_RXcommand, "AT+CH %hu status?\r\n",
					_hConfiguration->LoRa_Channel);
			LORA_STATUS_RECEIVE = LORA_CLEAR;
			if (LORA_ReceiveCommand(500, 10) != LORA_OK)
				return LORA_FAILED;
			sscanf(LORA_UART_BUFFER, "%s %hu status?\r%hu\r\n", AT_RXcommand,
					&_hConfiguration->LoRa_Channel,
					&_hConfiguration->LoRa_StatusChannel);
			break;
		default:
			return LORA_FAILED_COMMAND;
		}
		break;
	case AT_OPERATION_WRITE:
		switch (_ChOperation) {
		case AT_CHANNEL_OPERATION_FREQ:
			sprintf((char*) AT_TXcommand, "AT+CH %hu freq=%lu\r\n",
					_hConfiguration->LoRa_Channel,
					_hConfiguration->LoRa_Frequency);
			break;
		case AT_CHANNEL_OPERATION_DRRANGE:
			sprintf((char*) AT_TXcommand, "AT+CH %hu drrange=%hu %hu\r\n",
					_hConfiguration->LoRa_Channel,
					_hConfiguration->LoRa_MinDrRange,
					_hConfiguration->LoRa_MaxDrRange);
			break;
		case AT_CHANNEL_OPERATION_STATUS:
			sprintf((char*) AT_TXcommand, "AT+CH %hu status=%hu\r\n",
					_hConfiguration->LoRa_Channel,
					_hConfiguration->LoRa_StatusChannel);
			break;
		default:
			return LORA_FAILED_COMMAND;
		}
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- REINICIALIZAÇÃO DO SISTEMA -----------------------------------------------------------*/

/**
 * @brief Comando de reinicialização do canal ou do sistema
 * @tparam AT+RESET <ENTER>
 * @tparam AT+RESET channels<ENTER>
 * @param _Mode: Modo de reinicialização
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_SystemReboot(LoRa_SystemRebootModeTypeDef _Mode) {
	switch (_Mode) {
	case AT_REBOOT_SYSTEM:
		sprintf((char*) AT_TXcommand, "AT+RESET\r\n");
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	case AT_REBOOOT_CHANNEL:
		sprintf((char*) AT_TXcommand, "AT+RESET channels\r\n");
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- INFORMAÇÕES DO SISTEMA -----------------------------------------------------------*/

/**
 * @brief Comando para ler informações do sistema
 * @tparam AT+SINF <ENTER>
 * @attention Essa função não foi continuada, por haver outras funções que realizam o mesmo trabalho
 * @param _hInfo: Handler de informações do sistema
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_SystemInformation(LoRa_SystemInfoTypeDef *_hInfo) {
	return LORA_FAILED;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- VERSÃO DO FIRMWARE -----------------------------------------------------------*/

/**
 * @brief Informações sobre a versão do firmware
 * @tparam AT+VER <ENTER>
 * @param _Version: Versão do firmaware
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_FirmwareVersion(LoRa_Float *_Version) {
	sprintf((char*) AT_RXcommand, "AT+VER\r\n");
	LORA_STATUS_RECEIVE = LORA_CLEAR;
	if (LORA_ReceiveCommand(500, 10) != LORA_OK)
		return LORA_FAILED;
	sscanf(LORA_UART_BUFFER, "%s\r%f\r\n", AT_RXcommand, _Version);
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- GANHO DE ANTENA -----------------------------------------------------------*/

/**
 * @brief O comando para acesso ao ganho da antena (-4 e 6)
 * @tparam AT+SAG <gain> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Gain: Ganho de antena
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_AntennaGain(LoRa_OperationTypeDef _Operacao,
		LoRa_Float *_Gain) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+SAG\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%f\r\n", AT_RXcommand, _Gain);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+SAG %lf\r\n", (*_Gain));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TIPO DE PACOTE DE UPLINK -----------------------------------------------------------*/

/**
 * @brief Comando de leitura e definição de tipo de pacote uplink (0 ou 1)
 * @tparam AT+CFM <type> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Type: Tipo de pacote
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_UplinkPacketType(LoRa_OperationTypeDef _Operacao,
		LoRa_UplinkTypePacketTypeDef *_Type) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+CFM\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Type);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+CFM %hu\r\n", (*_Type));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- MODO DE BAIXO CONSUMO -----------------------------------------------------------*/

/**
 * @brief Comando para entrar no modo de baixo consumo (usar reset para voltar ao normal)
 * @tparam AT+SLEEP <ENTER>
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_EntersLowPowerMode(void) {
	sprintf((char*) AT_TXcommand, "AT+SLEEP\r\n");
	if (LORA_TransmitCommand(300) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ALARME DO RTC -----------------------------------------------------------*/

/**
 * @brief Hora de despertar do "real time clock" (RTC)
 * @tparam AT+ALARM <ver> <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Time: Tempo para despertar (em segundos)
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RTCWakeupTime(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Time) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+ALARM\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(1000, 15) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand, _Time);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+ALARM %hu\r\n", (*_Time));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- RELÓGIO DO RTC -----------------------------------------------------------*/

/**
 * @brief Comando de acessor ao relógio do "real time clock" (RTC)
 * @tparam AT+TIME Hour(2 digit):Min(2 digit):Sec(2 digit):ms(3 digit)
 * @param _Operacao: Modo de operação do comando
 * @param _Time: Tempo do RTC
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RTCTime(LoRa_OperationTypeDef _Operacao,
		LoRa_TimeTypeDef *_Time) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+TIME\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu:%hu:%f\r\n", AT_RXcommand,
				&_Time->LoRa_Horas, &_Time->LoRa_Minutos,
				&_Time->LoRa_Segundos);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+TIME %02hu:%02hu:%02hu\r\n",
				_Time->LoRa_Horas, _Time->LoRa_Minutos,
				(uint16_t) _Time->LoRa_Segundos);
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- DATA DO RTC -----------------------------------------------------------*/

/**
 * @brief Comando de acessor a data do "real time clock" (RTC)
 * @tparam AT+DATE year(4 digit):monthe(2 digit):date(2 digit)
 * @param _Operacao: Modo de operação do comando
 * @param _Date: Data do RTC
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_RTCDate(LoRa_OperationTypeDef _Operacao,
		LoRa_DateTypeDef *_Date) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+DATE\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu:%hu:%hu\r\n", AT_RXcommand,
				&_Date->LoRa_Ano, &_Date->LoRa_Mes, &_Date->LoRa_Dia);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+DATE %04hu:%02hu:%02hu\r\n",
				_Date->LoRa_Ano, _Date->LoRa_Mes, (uint16_t) _Date->LoRa_Dia);
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- RETORNO DE ECO -----------------------------------------------------------*/

/**
 * @brief Comando de retorno de eco
 * @tparam AT+ECHO < 0 | 1 > <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Echo: Retorno do eco
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ECHO(LoRa_OperationTypeDef _Operacao,
		LoRa_LoraEchoTypeDef *_Echo) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+ECHO\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Echo);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+ECHO %hu\r\n", (*_Echo));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- REDEFINIÇÃO DE CONFIGURAÇÕES -----------------------------------------------------------*/

/**
 * @brief Comando para redefinir as configuração
 * @tparam AT+FRESET <ENTER>
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_ResetConfiguration(void) {
	sprintf((char*) AT_TXcommand, "AT+FRESET\r\n");
	if (LORA_TransmitCommand(100) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- MODO DE DEBUG -----------------------------------------------------------*/

/**
 * @brief Comando para acesso ao modo de dubug
 * @tparam AT+DBG < 0 | 1 > <ENTER>
 * @param _Operacao: Modo de operação do comando
 * @param _Status: Status do modo debug
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_DebugMessageStatus(LoRa_OperationTypeDef _Operacao,
		LoRa_DebugMessageTypeDef *_Status) {
	switch (_Operacao) {
	case AT_OPERATION_READ:
		sprintf((char*) AT_RXcommand, "AT+DBG\r\n");
		LORA_STATUS_RECEIVE = LORA_CLEAR;
		if (LORA_ReceiveCommand(500, 10) != LORA_OK)
			return LORA_FAILED;
		sscanf(LORA_UART_BUFFER, "%s\r%hu\r\n", AT_RXcommand,
				(uint16_t*) _Status);
		break;
	case AT_OPERATION_WRITE:
		sprintf((char*) AT_TXcommand, "AT+DBG %hu\r\n", (*_Status));
		if (LORA_TransmitCommand(300) != LORA_OK)
			return LORA_FAILED;
		break;
	default:
		break;
	}
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TESTE DE FORÇA TX -----------------------------------------------------------*/

/**
 * @brief Modo de onda contínua FSK Tx (teste de força Tx)
 * @tparam AT+TXCW <frequency> <Power in dBm> <Timeout (Sec)> <ENTER>
 * @param _Frequency: Frequência de operação do teste
 * @param _Power: Potência de transmissão na operação do teste
 * @param _Timeout: Tempo total de teste (em segundos)
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_FSKTxContinuousWaveMode(LoRa_Rate _Frequency,
		LoRa_Value _Power, LoRa_Value _Timeout) {
	sprintf((char*) AT_TXcommand, "AT+TXCW %lu %hu %hu\r\n", _Frequency, _Power,
			_Timeout);
	if (LORA_TransmitCommand(300) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TESTE DE FORÇA DA ANTENA PARA RECEBIMENTO -----------------------------------------------------------*/

/**
 * @brief LoRa Rx (teste de força de RF)
 * @tparam AT+RXTT <Spreading Factor (Data rate)> <Bandwidth> <ENTER>
 * @param _Frequency: Frequência de operação do teste
 * @param _DataRate: Configuração de taxa de dados do teste (0 - 7)
 * @param _TBaund: Largura de banda do teste (0 a 2)
 * @retval Status de execução do comando
 */
LoRa_StatusTypeDef AT_LoRaRxSignalStrengthTest(LoRa_Rate _Frequency,
		LoRa_Value _DataRate, LoRa_StrenghtTestBaundTypeDef _TBaund) {
	sprintf((char*) AT_TXcommand, "AT+RXTT %lu %hu %hu\r\n", _Frequency, _DataRate,
			_TBaund);
	if (LORA_TransmitCommand(300) != LORA_OK)
		return LORA_FAILED;
	return LORA_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- TESTE DE FORÇA DA ANTENA PARA TRANSMISSÃO -----------------------------------------------------------*/

/**
 * @brief LoRa Tx (teste de força de RF)
 * @tparam
 * @param _Operacao: Modo de operação do comando
 * @param _Frequency: Frequência de operação do teste
 * @param _Power: Potência de transmissão na operação do teste
 * @param _DataRate: Configuração de taxa de dados do teste (0 - 7)
 * @param _TBaund: Largura de banda do teste (0 a 3)
 * @param _NumberBytes: Número de bytes para transmissão
 * @param _Period: Periodo de transmissão dos bytes
 * @param _Status: Status de final do teste
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_LoRaTxSignalStrengthTest(LoRa_OperationTypeDef _Operacao,
		LoRa_Rate _Frequency, LoRa_Value _Power, LoRa_Value _DataRate,
		LoRa_StrenghtTestBaundTypeDef _TBaund, LoRa_Value _NumberBytes,
		LoRa_Value _Period, LoRa_StrenghtTestStatusTypeDef *_Status);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- FINALIZAÇÃO DE TESTE DE RF -----------------------------------------------------------*/

/**
 * @brief Comando para finalização de teste de RF
 * @tparam
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_StopRFTest(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CONFIGURAÇÃO DE PINO GPIO -----------------------------------------------------------*/

/**
 * @brief Comando para acesso a configuração de pinos GPIO MS500
 * @tparam
 * @param _Operacao: Modo de operação do comando
 * @param _GPIO: Pino GPIO para acesso
 * @param _Config: Configuração do pino GPIO
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_GPIOPinInformation(LoRa_OperationTypeDef _Operacao,
		LoRa_PinTypeDef _GPIO, LoRa_PinConfigurationTypeDef *_Config);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CANAL DE COMUNICAÇÃO P2P -----------------------------------------------------------*/

/**
 * @brief Comando para listar canais de comunicação diponíveis, e seleção de canal para comunicação P2P
 * @tparam
 * @param _Operacao: Modo de operação do comando
 * @param _Channel: Canal de comunicação P2P
 * @param _ChannelsList: Lista de canais disponíveis
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_RegionalChannelListP2P(LoRa_OperationTypeDef _Operacao,
		LoRa_Value _Channel, LoRa_ChannelsTypeDef *_ChannelsList);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- ENDEREÇO DE COMUNICAÇÃO P2P -----------------------------------------------------------*/

/**
 * @brief  Defina o endereço do dispositivo P2P (4 bytes) para comunicação
 * @tparam
 * @param _Operacao: Modo de operação do comando
 * @param _Adress: Endereço de comunicação P2P
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_DeviceAdressP2P(LoRa_OperationTypeDef _Operacao,
		LoRa_Adress *_Adress);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---- CHAVE DE SINCRONIZAÇÃO -----------------------------------------------------------*/

/**
 * @brief Comando para definição de chave de sincronização
 * @tparam
 * @param _Operacao: Modo de operação do comando
 * @param _Word: Chave de sincronização (1 a 255)
 * @retval Status de execução do comando
 */

LoRa_StatusTypeDef AT_SyncWordP2P(LoRa_OperationTypeDef _Operacao,
		LoRa_Value *_Word);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* USER CODE END PF */
