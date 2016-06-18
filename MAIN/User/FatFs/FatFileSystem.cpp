#include "FatFileSystem.h"
#include "sd_diskio.h"


static FATFS SDFatFs;
static char SDPath[4];
static bool isDriverLink = false;

bool mountSDCard()
{
    if (isDriverLink){
        FATFS_UnLinkDriver(SDPath);
        isDriverLink = false;
    }

    if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
    {
        isDriverLink = true;
        if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) == FR_OK)
        {
            return true;
        }
    }
    return false;
}

bool eraseSDCard()
{
    if(f_mkfs((TCHAR const*)SDPath, 0, 0) == FR_OK)
    {
        return true;
    }
    return false;
}

int getSDFreeSpace()
{
    FATFS *fs1;
    DWORD fre_clust,fre_sect;
    DWORD free = 0;
    FRESULT res;
    res = f_getfree((const TCHAR*)SDPath, (DWORD*)&fre_clust, &fs1);
    if(res==FR_OK)
	{
	    fre_sect=fre_clust*fs1->csize;

		free=fre_sect>>1;
		return free;
 	}
 	return -1;
}

void deleteFile(char* name)
{
    f_unlink(name);
}

GcodeFile::GcodeFile():file_mode(0),is_opened(false),file_len(0),curr_pos(0)
{

}

bool GcodeFile::create(char* name)
{
    FRESULT ret;
    if ((ret = f_open(&file_handle, name, FA_CREATE_ALWAYS|FA_WRITE)) == FR_OK){
        file_mode = FILE_WRITE;
        file_len = 0;
        curr_pos = 0;
        is_opened = true;
        return true;
    }
    return false;
}

bool GcodeFile::open(char* name)
{
    if (f_open(&file_handle, name, FA_READ) == FR_OK){
        file_mode = FILE_READ;
        file_len = file_handle.fsize;
        curr_pos = 0;
        is_opened = true;
        return true;
    }
    return false;
}

void GcodeFile::close()
{
    f_close(&file_handle);
    is_opened = false;
}

int GcodeFile::readChar()
{
    unsigned char tmpchar;
    UINT tmpnum = 0;
    if ((f_read(&file_handle, &tmpchar, 1, &tmpnum) != FR_OK) || (tmpnum == 0)){
        return -1;
    }
    curr_pos++;
    return tmpchar;
}

int GcodeFile::read(char* pbuffer, int len)
{
    UINT tmpnum = 0;
    if (f_read(&file_handle, &pbuffer, len, &tmpnum) != FR_OK){
        return -1;
    }
    curr_pos += tmpnum;
    return (int)tmpnum;
}

int GcodeFile::write(char* pbuffer, int len)
{
    UINT tmpnum = 0;
    if (f_write(&file_handle, pbuffer, len, &tmpnum) != FR_OK){
        return -1;
    }
    return (int)tmpnum;
}

