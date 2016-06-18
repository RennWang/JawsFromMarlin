#ifndef __FATFILESYSTEM_H_INCLUDED
#define __FATFILESYSTEM_H_INCLUDED

#include "ff_gen_drv.h"
#include "sd_diskio.h"

bool mountSDCard();
bool eraseSDCard();
int getSDFreeSpace();

void deleteFile(char* name);

enum FILEMODE
{
   FILE_READ = 0,
   FILE_WRITE = 1
};

class GcodeFile
{
public:
    GcodeFile();

    bool create(char* name);
    bool open(char* name);
    void close();

    int readChar();
    int read(char* pbuffer, int len);
    int write(char* pbuffer, int len);

    int getPosition(){return curr_pos;}
    int getFileLength(){return file_len;}
    bool isOpen(){return is_opened;}
    int getMode(){return file_mode;}
private:
    int file_mode;
    bool is_opened;
    FIL file_handle;

    int file_len;
    int curr_pos;
};



#endif /* __FATFILESYSTEM_H_INCLUDED */
