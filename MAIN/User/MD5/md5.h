/**
  ******************************************************************************
  * @file    md5.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    18-March-2015
  * @brief   Md5 check
  *          Bootloader for 3D Printer , is used to update firmware from sd card
  ******************************************************************************
  */
#ifndef MD5_H_INCLUDED
#define MD5_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct
{
    unsigned int count[2];              //64λ�ļ�λ����
    unsigned int state[4];              //ABCD4������
    unsigned char buffer[64];           //512λ��Ϣ�ݴ�ռ�
}MD5_CTX;


#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) \
          { \
          a += F(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define GG(a,b,c,d,x,s,ac) \
          { \
          a += G(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define HH(a,b,c,d,x,s,ac) \
          { \
          a += H(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define II(a,b,c,d,x,s,ac) \
          { \
          a += I(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);
void MD5Final(MD5_CTX *context,unsigned char digest[16]);
void MD5Transform(unsigned int state[4],unsigned char block[64]);
void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* MD5_H_INCLUDED */
