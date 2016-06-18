#include "SerialPort.h"

SerialPort commdSerial(SERIAL_COMMAND);
SerialPort debugSerial(SERIAL_DEBUG);

static void USART1_Init(void);
static void USART1_Transmit(uint8_t *buffer, uint16_t bufsize);

static void USART2_Init(void);
static void USART2_Transmit(uint8_t *buffer, uint16_t bufsize);

SerialPort::SerialPort(SerialPortType serial)
{
    serial_port = serial;
    flush();
    tx_buffer.datalen = 0;
}

void SerialPort::init(void)
{
    if (serial_port == SERIAL_COMMAND)
        USART1_Init();
    else if (serial_port == SERIAL_DEBUG)
        USART2_Init();
}

void SerialPort::transmit(void)
{
    is_TxBusy = 1;

    if (serial_port == SERIAL_COMMAND)
        USART1_Transmit(tx_buffer.buffer, tx_buffer.datalen);
    else if (serial_port == SERIAL_DEBUG)
        USART2_Transmit(tx_buffer.buffer, tx_buffer.datalen);

    tx_buffer.datalen = 0;
}

int SerialPort::peek(void)
{
    if (rx_buffer.head == rx_buffer.tail) {
        return -1;
    } else {
        return rx_buffer.buffer[rx_buffer.tail];
    }
}

int SerialPort::read(void)
{
    if (rx_buffer.head == rx_buffer.tail) {
        return -1;
    } else {
        unsigned char c = rx_buffer.buffer[rx_buffer.tail];
        rx_buffer.tail = (unsigned int)(rx_buffer.tail + 1) % RX_BUFFER_SIZE;
        return c;
    }
}

void SerialPort::flush(void)
{
    rx_buffer.head = rx_buffer.tail;
}

void SerialPort::write(uint8_t c)
{
    write(&c, 1);
}

void SerialPort::write(const char *str)
{
    uint16_t len = strlen(str);
    if (len > 0)
        write((const uint8_t *)str, len);
}

void SerialPort::write(const uint8_t *buffer, uint16_t bufsize)
{
    while (is_TxBusy){};

    if (bufsize > TX_BUFFER_SIZE)
        bufsize = TX_BUFFER_SIZE;

    tx_buffer.datalen = bufsize;
    memcpy(tx_buffer.buffer, buffer, tx_buffer.datalen);

    transmit();
}

void SerialPort::writeBuffer(uint8_t c)
{
    writeBuffer(&c, 1);
}

void SerialPort::writeBuffer(const char *str)
{
    uint16_t len = strlen(str);
    if (len > 0)
        writeBuffer((const uint8_t *)str, len);
}

void SerialPort::writeBuffer(const uint8_t *buffer, uint16_t bufsize)
{
    while (is_TxBusy){};

    if (tx_buffer.datalen < TX_BUFFER_SIZE){
        if (bufsize > TX_BUFFER_SIZE - tx_buffer.datalen){
            memcpy(tx_buffer.buffer + tx_buffer.datalen, buffer, TX_BUFFER_SIZE - tx_buffer.datalen);
            tx_buffer.datalen = TX_BUFFER_SIZE;
        }else{
            memcpy(tx_buffer.buffer + tx_buffer.datalen, buffer, bufsize);
            tx_buffer.datalen += bufsize;
        }
    }
}

void SerialPort::print(char c, int base)
{
    print((long) c, base);
}

void SerialPort::print(unsigned char b, int base)
{
    print((unsigned long) b, base);
}

void SerialPort::print(int n, int base)
{
    print((long) n, base);
}

void SerialPort::print(unsigned int n, int base)
{
    print((unsigned long) n, base);
}

void SerialPort::print(long n, int base)
{
    if (base == 0) {
        writeBuffer(n);
    }
    else if (base == 10) {
        if (n < 0) {
            print('-');
            n = -n;
        }
        printNumber(n, 10);
    }
    else {
        printNumber(n, base);
    }
}

void SerialPort::print(unsigned long n, int base)
{
    if (base == 0) writeBuffer(n);
    else printNumber(n, base);
}

void SerialPort::print(double n, int digits)
{
    printFloat(n, digits);
}

void SerialPort::print(float n, int digits)
{
    printFloat(n, digits);
}

void SerialPort::printNumber(unsigned long n, uint8_t base)
{
    unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
    unsigned long i = 0;

    if (n == 0) {
        print('0');
        return;
    }

    while (n > 0) {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
        print((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}

void SerialPort::printFloat(double number, uint8_t digits)
{
    // Handle negative numbers
    if (number < 0.0){
        print('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i=0; i<digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
        print(".");

    // Extract digits from the remainder one at a time
    while (digits-- > 0){
        remainder *= 10.0;
        int toPrint = int(remainder);
        print(toPrint);
        remainder -= toPrint;
    }
}

static UART_HandleTypeDef       commandUart_HandleStruct;
static UART_HandleTypeDef       debugUart_HandleStruct;
static DMA_HandleTypeDef        commandUart_DMA_tx;
static DMA_HandleTypeDef        debugUart_DMA_tx;

static void USART1_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

//    __HAL_RCC_AFIO_CLK_ENABLE();
//    AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

     /* Configure the DMA handler for Transmission process */
    commandUart_DMA_tx.Instance                 = DMA1_Channel4;
    commandUart_DMA_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    commandUart_DMA_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    commandUart_DMA_tx.Init.MemInc              = DMA_MINC_ENABLE;
    commandUart_DMA_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    commandUart_DMA_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    commandUart_DMA_tx.Init.Mode                = DMA_NORMAL;
    commandUart_DMA_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&commandUart_DMA_tx);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&commandUart_HandleStruct, hdmatx, commandUart_DMA_tx);

    /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    commandUart_HandleStruct.Instance        = USART1;
    commandUart_HandleStruct.Init.BaudRate   = 115200;
    commandUart_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
    commandUart_HandleStruct.Init.StopBits   = UART_STOPBITS_1;
    commandUart_HandleStruct.Init.Parity     = UART_PARITY_NONE;
    commandUart_HandleStruct.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    commandUart_HandleStruct.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_DeInit(&commandUart_HandleStruct) != HAL_OK)
        return;

    if(HAL_UART_Init(&commandUart_HandleStruct) != HAL_OK)
        return;

    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&commandUart_HandleStruct, UART_IT_RXNE);
}

static void USART1_Transmit(uint8_t *buffer, uint16_t bufsize)
{
    HAL_UART_Transmit_DMA(&commandUart_HandleStruct, buffer, bufsize);
}

static void USART2_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

     /* Configure the DMA handler for Transmission process */
    debugUart_DMA_tx.Instance                 = DMA1_Channel7;
    debugUart_DMA_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    debugUart_DMA_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    debugUart_DMA_tx.Init.MemInc              = DMA_MINC_ENABLE;
    debugUart_DMA_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    debugUart_DMA_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    debugUart_DMA_tx.Init.Mode                = DMA_NORMAL;
    debugUart_DMA_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&debugUart_DMA_tx);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&debugUart_HandleStruct, hdmatx, debugUart_DMA_tx);

    debugUart_HandleStruct.Instance        = USART2;
    debugUart_HandleStruct.Init.BaudRate   = 115200;
    debugUart_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
    debugUart_HandleStruct.Init.StopBits   = UART_STOPBITS_1;
    debugUart_HandleStruct.Init.Parity     = UART_PARITY_NONE;
    debugUart_HandleStruct.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    debugUart_HandleStruct.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_DeInit(&debugUart_HandleStruct) != HAL_OK)
        return;

    if(HAL_UART_Init(&debugUart_HandleStruct) != HAL_OK)
        return;
}

static void USART2_Transmit(uint8_t *buffer, uint16_t bufsize)
{
    HAL_UART_Transmit_DMA(&debugUart_HandleStruct, buffer, bufsize);
}

void USART1_IRQHandler(void)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
    uint8_t tmp_data = 0;

    tmp_flag = __HAL_UART_GET_FLAG(&commandUart_HandleStruct, UART_FLAG_RXNE);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(&commandUart_HandleStruct, UART_IT_RXNE);

    if((tmp_flag != RESET) && (tmp_it_source != RESET)){
        tmp_data = (uint8_t)(commandUart_HandleStruct.Instance->DR & (uint16_t)0x00FF);
        commdSerial.receive(tmp_data);
    }

    tmp_flag = __HAL_UART_GET_FLAG(&commandUart_HandleStruct, UART_FLAG_TC);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(&commandUart_HandleStruct, UART_IT_TC);

    if((tmp_flag != RESET) && (tmp_it_source != RESET)){
        __HAL_UART_DISABLE_IT(&commandUart_HandleStruct, UART_IT_TC);

        if(commandUart_HandleStruct.State == HAL_UART_STATE_BUSY_TX_RX){
            commandUart_HandleStruct.State = HAL_UART_STATE_BUSY_RX;
        }else{
            commandUart_HandleStruct.State = HAL_UART_STATE_READY;
        }
        commdSerial.is_TxBusy = 0;
        //HAL_UART_DMAStop(&debugUart_HandleStruct);
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&commandUart_DMA_tx);
}
