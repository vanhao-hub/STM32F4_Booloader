#include "stm32_ota_update_app.h"
#include "string.h"
#include "stdio.h"
#include "driver/uart.h"
#include "esp_log.h"

#define delay(a) vTaskDelay(a / portTICK_PERIOD_MS);
#define RS232_PollComport(a,b,c) 		uart_read_bytes(a, b, ETX_OTA_PACKET_MAX_SIZE, 3000 / portTICK_RATE_MS)
#define RS232_SendByte(a,b) 			sendData(&b)
#define RX_BUF_SIZE  1024
uint8_t DATA_BUF[ETX_OTA_PACKET_MAX_SIZE];


int sendData(uint8_t *data) {
	uart_write_bytes(UART_NUM_2, data, 1);
	return 0;
}

/* read the response */
uint8_t is_ack_resp_received(int comport) {
	uint8_t is_ack = 0;
	uint8_t i = 0;

	memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);
	uint16_t len =  RS232_PollComport( comport, DATA_BUF, sizeof(ETX_OTA_RESP_));
	for(i=0; i < sizeof(ETX_OTA_RESP_); i++)
		printf("byte %d : %x \n",i,DATA_BUF[i]);
	if( len > 0 )
	{
		printf("vanhao_______ len >0 \n");
		ETX_OTA_RESP_ *resp = (ETX_OTA_RESP_*) DATA_BUF;
		if( resp->packet_type == ETX_OTA_PACKET_TYPE_RESPONSE )
		{
			printf("vanhao_______ ETX_OTA_PACKET_TYPE_RESPONSE \n");
		  //TODO: Add CRC check
		  if( resp->status == ETX_OTA_ACK )
		  {
			  printf("vanhao_______ ETX_OTA_ACK \n");

			//ACK received
			is_ack = 1;
		  }
		}
	}
	return is_ack;
}


/* Build the OTA START command */
int send_ota_start(int comport)
{
	uint16_t len;
	ETX_OTA_COMMAND_ *ota_start = (ETX_OTA_COMMAND_*)DATA_BUF;
	int ex = 0;

	memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

	ota_start->sof          = ETX_OTA_SOF;
	ota_start->packet_type  = ETX_OTA_PACKET_TYPE_CMD;
	ota_start->data_len     = 1;
	ota_start->cmd          = ETX_OTA_CMD_START;
	ota_start->crc          = 0x00;               //TODO: Add CRC
	ota_start->eof          = ETX_OTA_EOF;

	len = sizeof(ETX_OTA_COMMAND_);

	//send OTA START
	for(int i = 0; i < len; i++)
	{
		delay(1);
		if( RS232_SendByte(comport, DATA_BUF[i]) )
		{
			//some data missed.
			printf("OTA START : Send Err\n");
			ex = -1;
			break;
		}
	}
	uart_flush(comport);
	if( ex >= 0 )
	{
		if( !is_ack_resp_received( comport ) )
		{
			//Received NACK
			printf("OTA START : NACK\n");
			ex = -1;
		}
	}
	printf("OTA START [ex = %d]\n", ex);
	return ex;
}


/* Build and Send the OTA END command */
uint16_t send_ota_end(int comport)
{
	uint16_t len;
	ETX_OTA_COMMAND_ *ota_end = (ETX_OTA_COMMAND_*)DATA_BUF;
	int ex = 0;

	memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

	ota_end->sof          = ETX_OTA_SOF;
	ota_end->packet_type  = ETX_OTA_PACKET_TYPE_CMD;
	ota_end->data_len     = 1;
	ota_end->cmd          = ETX_OTA_CMD_END;
	ota_end->crc          = 0x00;               //TODO: Add CRC
	ota_end->eof          = ETX_OTA_EOF;

	len = sizeof(ETX_OTA_COMMAND_);

	//send OTA END
	for(int i = 0; i < len; i++)
	{
		delay(1);
		if( RS232_SendByte(comport, DATA_BUF[i]) )
		{
			//some data missed.
			printf("OTA END : Send Err\n");
			ex = -1;
			break;
		}
	}
	uart_flush(comport);
	if( ex >= 0 )
	{
		if( !is_ack_resp_received( comport ) )
		{
			//Received NACK
			printf("OTA END : NACK\n");
			ex = -1;
		}
	}
	printf("OTA END [ex = %d]\n", ex);
	return ex;
}


/* Build and send the OTA Header */
int send_ota_header(int comport, meta_info *ota_info)
{
	uint16_t len;
	ETX_OTA_HEADER_ *ota_header = (ETX_OTA_HEADER_*)DATA_BUF;
	int ex = 0;

	memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

	ota_header->sof          = ETX_OTA_SOF;
	ota_header->packet_type  = ETX_OTA_PACKET_TYPE_HEADER;
	ota_header->data_len     = sizeof(meta_info);
	ota_header->crc          = 0x00;               //TODO: Add CRC
	ota_header->eof          = ETX_OTA_EOF;

	memcpy(&ota_header->meta_data, ota_info, sizeof(meta_info) );

	len = sizeof(ETX_OTA_HEADER_);

	//send OTA Header
	for(int i = 0; i < len; i++)
	{
		delay(1);
		if( RS232_SendByte(comport, DATA_BUF[i]) )
		{
			//some data missed.
			printf("OTA HEADER : Send Err\n");
			ex = -1;
			break;
		}
	}
	uart_flush(comport);
	if( ex >= 0 )
	{
		if( !is_ack_resp_received( comport ) )
		{
			//Received NACK
			printf("OTA HEADER : NACK\n");
			ex = -1;
		}
	}
	printf("OTA HEADER [ex = %d]\n", ex);
	return ex;
}


/* Build and send the OTA Data */
int send_ota_data(int comport, char *data, uint16_t data_len)
{
	uint16_t len;
	ETX_OTA_DATA_ *ota_data = (ETX_OTA_DATA_*)DATA_BUF;
	int ex = 0;
	static uint8_t count=0;
	memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

	ota_data->sof          = ETX_OTA_SOF;
	ota_data->packet_type  = ETX_OTA_PACKET_TYPE_DATA;
	ota_data->data_len     = data_len;

	len = 4;

	//Copy the data
	memcpy(&DATA_BUF[len], data, data_len );
	len += data_len;
	uint32_t crc = 0u;        //TODO: Add CRC

	//Copy the crc
	memcpy(&DATA_BUF[len], (uint8_t*)&crc, sizeof(crc) );
	len += sizeof(crc);

	//Add the EOF
	DATA_BUF[len] = ETX_OTA_EOF;
	len++;

	printf("Sending %d Data\n", len);

	//send OTA Data
	for(int i = 0; i < len; i++)
	{
		delay(1);
		if( RS232_SendByte(comport, DATA_BUF[i]) )
		{
			//some data missed.
			printf("OTA DATA : Send Err\n");
			ex = -1;
			break;
		}
		//printf("Gui byte thu %d \n", i);
	}
	uart_flush(comport);
	printf("Da gui part data thu %d \n", count);
	if( ex >= 0 )
	{
		if( !is_ack_resp_received( comport ) )
		{
			//Received NACK
			printf("OTA DATA : NACK\n");
			ex = -1;
		}
	}
	printf("OTA DATA [ex = %d]\n", ex);
	count++;
	return ex;
}
