#ifndef __SDCARD_H_INCLUDED
#define __SDCARD_H_INCLUDED

#include "FatFileSystem.h"

#define SD_FINISHED_RELEASECOMMAND "M84 X Y Z E"


class SDCard
{
public:
    SDCard();

    void init();
    void reset();

    bool openFile(char* name,bool read,bool replace_current=true);
    void openLogFile(char* name);
    void removeFile(char* name);
    void closeFile();
    void eraseForSpace();

    void startPrint();
    void pausePrint();
    void cancelPrint();

    void startFileTransfer();
    void stopFileTransfer();

    void printingHasFinished();
    int write(void* pbuffer, int size);

    bool isFileOpen(){return file.isOpen();}
    bool eof(){return sd_pos >= file_size;}
    int read(){sd_pos = file.getPosition(); return file.readChar();}
    uint8_t percentDone(){
        if(!isFileOpen()) return 0;
        if(file_size) return sd_pos/((file_size+99)/100);
        else return 0;
    }

    bool isSDPrinted(){return sd_printing;}
    bool isCardOK(){return card_ok;}
    bool getCardMode(){return file.getMode();} //0-read,1-write
private:
    bool card_ok;
    bool sd_printing;
    char file_name[128];
    int file_size;
    int sd_pos;

    int num_of_files;

    GcodeFile file;
};

extern SDCard sd_card;

#endif /* SDCARD_H_INCLUDED */
