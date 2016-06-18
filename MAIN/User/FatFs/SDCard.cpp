#include "SDCard.h"
#include "Command.h"
#include "stepper.h"

#include "usbd_cdc_interface.h"

//#include <stdio.h>
//#include <string.h>
SDCard sd_card;

SDCard::SDCard():card_ok(false),sd_printing(false),file_size(0),sd_pos(0)
{
    memset(file_name,0,sizeof(file_name));
}

void SDCard::init()
{
    if (mountSDCard())
        card_ok = true;
}

void SDCard::reset()
{
    init();
}

bool SDCard::openFile(char* name, bool read, bool replace_current)
{
    if (!card_ok)
        init();
    if (file.isOpen()){
        file.close();
    }

    sd_printing = false;
    sd_pos = 0;
    file_size = 0;

    if(read){
        if (!file.open(name)){
            card_ok = false;
            return false;
        }
        file_size = file.getFileLength();
        strcpy(file_name, name);
    }else{
        //eraseSDCard();
        if (!file.create(name)){
            card_ok = false;
            return false;
        }
        strcpy(file_name, name);
    }
    return true;
}

void SDCard::removeFile(char* name)
{
    deleteFile(name);
}

void SDCard::closeFile()
{
    if (file.isOpen()){
        file.close();
    }
}

void SDCard::eraseForSpace()
{
    if (file.isOpen())
        file.close();

    int ret = getSDFreeSpace();
    if (ret == -1)
        return;

    //保证剩余空间有500M
    if (ret < 512*1024){
        eraseSDCard();
        HAL_Delay(500);
    }
}

int SDCard::write(void* pbuffer, int size)
{
    return file.write((char*)pbuffer, size);
}

void SDCard::startPrint()
{
    if(card_ok)
    {
        sd_printing = true;
    }
}

void SDCard::pausePrint()
{
    if(sd_printing)
    {
        sd_printing = false;
    }
}

void SDCard::cancelPrint()
{
    if(sd_printing)
    {
        sd_printing = false;
    }
    //
    quickStop();
    file.close();

    //
    command::enquecommand("G92 E5");
    command::enquecommand("G1 E0");
    //
    command::enquecommand("G28");
    //
    command::enquecommand(SD_FINISHED_RELEASECOMMAND);
    //
    command::enquecommand("M104 S0");
}

void SDCard::printingHasFinished()
{
    st_synchronize();

    quickStop();
    file.close();
    sd_printing = false;

    //
    command::enquecommand("G28");

    command::enquecommand(SD_FINISHED_RELEASECOMMAND);
    //
    command::enquecommand("M104 S0");
    //autotempShutdown();
}

void SDCard::startFileTransfer()
{
    USBFileTransferStart();
}

void SDCard::stopFileTransfer()
{
    USBFileTransferStop();
}

int32_t USBFileWrite(uint8_t* pbuffer, int32_t size)
{
    return sd_card.write(pbuffer,size);
}

/**
  * @}
  */

/*****************************END OF FILE****/
