/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/usbd_cdc_interface.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    18-December-2015
  * @brief   Source file for USBD CDC interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright © 2015 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"
#include "usbd_desc.h"
#include "usbd_cdc_interface.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FS_PACKET_SIZE      COMM_BLOCK_SIZE
#define APP_RX_DATA_SIZE    512
#define APP_TX_DATA_SIZE    512

#define FILE_RX_DATA_SIZE   FILE_BLOCK_SIZE

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USBD_CDC_LineCodingTypeDef LineCoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };

static uint8_t packetRxBuffer[FS_PACKET_SIZE];/* Received Data over USB are stored in this buffer */

static uint8_t serialRxBuffer[APP_RX_DATA_SIZE];
static uint32_t rx_head = 0;
static uint32_t rx_tail = 0;

static uint8_t serialTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
static uint32_t tx_len = 0;

static __IO uint32_t flag_usbinit = 0;

//file transfer
static uint8_t fileRxBuffer[FILE_RX_DATA_SIZE];

//
static void USBSerialReceive(uint8_t c);

///* USB handler declaration */
//extern USBD_HandleTypeDef  USBD_Device;
USBD_HandleTypeDef USBD_Device;

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init     (void);
static int8_t CDC_Itf_DeInit   (void);
static int8_t CDC_Itf_Control  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive  (uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_Itf_FileReceive  (uint8_t* pbuf, uint32_t *Len);


USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive,
  CDC_Itf_FileReceive
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Itf_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
  /*##-5- Set Application Buffers ############################################*/
  USBD_CDC_SetTxBuffer(&USBD_Device, serialTxBuffer, 0);
  USBD_CDC_SetRxBuffer(&USBD_Device, packetRxBuffer);

  rx_head = rx_tail;
  tx_len = 0;

  flag_usbinit = 1;

  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
    flag_usbinit = 0;

  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                            (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format     = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype   = pbuf[6];

    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;

    /* Add your code here */
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* Add your code here */
    break;

  case CDC_SEND_BREAK:
     /* Add your code here */
    break;

  default:
    break;
  }

  return (USBD_OK);
}

#define FILE_TRANSFER_START     "FILE START\n"
#define FILE_TRANSFER_STOP      "FILE STOP\n"
/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Receive(uint8_t* Buf, uint32_t *Len)
{
    uint32_t tmplen = *Len;

    if (tmplen>0){
        char tmp[]= FILE_TRANSFER_START;
        if (!strncmp( tmp, Buf, strlen(tmp)-1)){
            USBD_CDC_FileSetRxBuffer(&USBD_Device,fileRxBuffer);
            USBD_CDC_FileReceivePacket(&USBD_Device);
        }else{
            while (tmplen != 0){
                USBSerialReceive(*Buf);
                Buf++;
                tmplen--;
            }
            USBD_CDC_ReceivePacket(&USBD_Device);
        }
    }else{
        USBD_CDC_ReceivePacket(&USBD_Device);
    }

    return (USBD_OK);
}

static int8_t CDC_Itf_FileReceive(uint8_t* Buf, uint32_t *Len)
{
    int32_t ret = 0;
    uint32_t tmplen = *Len;

    if (tmplen>0){
        char tmp[]= FILE_TRANSFER_STOP;
        if (!strncmp( tmp, Buf, strlen(tmp)-1)){
            USBD_CDC_SetRxBuffer(&USBD_Device, packetRxBuffer);
            USBD_CDC_ReceivePacket(&USBD_Device);
        }else{
            ret = USBFileWrite(Buf, tmplen);

            if (ret != tmplen){
                snprintf(serialTxBuffer,512,"File write %d\n", ret);
                USBD_CDC_SetTxBuffer(&USBD_Device,serialTxBuffer,strlen(serialTxBuffer));
                USBD_CDC_TransmitPacket(&USBD_Device);
            }

            USBD_CDC_FileReceivePacket(&USBD_Device);
        }
    }else{
        USBD_CDC_FileReceivePacket(&USBD_Device);
    }

    return (USBD_OK);
}

void USBSerialReceive(uint8_t c)
{
    int i = (unsigned int)(rx_head + 1) % APP_RX_DATA_SIZE;

    if (i != rx_tail) {
        serialRxBuffer[rx_head] = c;
        rx_head = i;
    }
}

void USBSerialFlush()
{
    rx_head = rx_tail;
}

void USBSerialTransmit()
{
    if (flag_usbinit){
        while (USBD_CDC_IsTransmitting(&USBD_Device));
        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&serialTxBuffer, tx_len);
        USBD_CDC_TransmitPacket(&USBD_Device);
    }

    tx_len = 0;
}

void USBSerialWrite(const uint8_t* buf, uint32_t len)
{
    if (len > APP_TX_DATA_SIZE)
        len = APP_TX_DATA_SIZE;

    memcpy(serialTxBuffer, buf, len);
    tx_len = len;

    USBSerialTransmit();
}

void USBSerialWriteBuffer(const uint8_t* buf, uint32_t len)
{
    if (tx_len < APP_TX_DATA_SIZE){
        if (len > APP_TX_DATA_SIZE - tx_len){
            memcpy(serialTxBuffer + tx_len, buf, APP_TX_DATA_SIZE - tx_len);
            tx_len = APP_TX_DATA_SIZE;
        }else{
            memcpy(serialTxBuffer + tx_len, buf, len);
            tx_len += len;
        }
    }
}

int32_t USBSerialPeek()
{
    if (rx_head == rx_tail) {
        return -1;
    } else {
        return serialRxBuffer[rx_tail];
    }
}

int32_t USBSerialAvailable()
{
    return (uint32_t)(APP_RX_DATA_SIZE + rx_head - rx_tail) % APP_RX_DATA_SIZE;
}

int32_t USBSerialRead()
{
    if (rx_head == rx_tail) {
        return -1;
    } else {
        unsigned char c = serialRxBuffer[rx_tail];
        rx_tail = (unsigned int)(rx_tail + 1) % APP_RX_DATA_SIZE;
        return c;
    }
}

void USBSerialInit()
{
  /* Init Device Library */
  USBD_Init(&USBD_Device, &VCP_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&USBD_Device);

  rx_head = rx_tail;
  tx_len = 0;
}

//File Transfer TODO:
void USBFileTransferStart()
{

}

//File Transfer TODO:
void USBFileTransferStop()
{

}

__weak int32_t USBFileWrite(uint8_t* pbuffer, int32_t size)   //USB   file write  callback
{

    return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

