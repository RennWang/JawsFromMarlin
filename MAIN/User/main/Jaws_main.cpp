/**
  ******************************************************************************
  * @file    main.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    7-April-2015
  * @brief
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Jaws_main.h"
#include "Command.h"
#include "delay.h"
#include "EepromMap.h"
#include "JawsBoard.h"
#include "planner.h"
#include "stepper.h"
#include "Fan.h"
#include "RGB_LED.h"
#include "hw_gpio.h"
#include "digpot_mcp44xx.h"

static void FirmwareInit(void);
static void FirmwareLoop(void);

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    if (!eeprom_read_word(0))
        eeprom_write_word(0,1);
    ///固件初始化
    FirmwareInit();

    while(1){
        FirmwareLoop();
    }

//    digipot_set(0,100);
//    digipot_set(1,100);
//    digipot_set(2,100);
//    digipot_set(3,100);
//
//    SET_OUTPUT(X_ENABLE_PIN);
//    WRITE(X_ENABLE_PIN,false);
//
//    SET_OUTPUT(X_DIR_PIN);
//    WRITE(X_DIR_PIN,true);
//
//    SET_OUTPUT(X_STEP_PIN);
//    WRITE(X_STEP_PIN,true);
//
//    while(1) {
//        int i;
//
//        HAL_Delay(500);
//        WRITE(X_DIR_PIN, false); //set the direction: low
//
//        for(i=0; i<10000; i++) //iterate for 3200 steps
//        {
//            WRITE(X_STEP_PIN, false);
//            HAL_Delay_us(50); //wait 2ms
//            WRITE(X_STEP_PIN, true);
//            HAL_Delay_us(50); //wait 2ms
//        }
//
//        HAL_Delay(500); //wait 500ms
//
//        WRITE(X_DIR_PIN, true); //set the direction: high
//
//        for(i=0; i<10000; i++) //iterate for 3200 steps
//        {
//            WRITE(X_STEP_PIN, false);
//            HAL_Delay_us(50); //wait 2ms
//            WRITE(X_STEP_PIN, true);
//            HAL_Delay_us(50); //wait 2ms
//        }
//
//        HAL_Delay(500); //wait 500ms
//    }

//      piezoaudio.init();
//    piezoaudio.playTune(TUNE_SAILFISH_STARTUP);
//    while(1){
//        piezoaudio.runPiezoSlice();
//    }
    while(1);
    /* not run here forever */
    return 0;
}



/** \brief  固件初始化
 *
 * \param void
 * \return void
 */
static void FirmwareInit(void)
{
    //串口初始化
    commandSerial.init();

    SERIAL_ECHO_START;
    SERIAL_ECHOLN("Firmware initializing!");

    //基本参数初始化
    eeprom::init();
    command::init();
    Jawsboard::getBoard().init();

    plan_init();
    st_init();
}

/** \brief  固件执行循环
 *
 * \param void
 * \return void
 */
static void FirmwareLoop(void)
{
    //SERIAL_ECHO_START;
    //SERIAL_ECHOLN("loop\n");
    command::runCommandSlice();
    Jawsboard::getBoard().runboardSlice();
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
