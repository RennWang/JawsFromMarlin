/**
  ******************************************************************************
  * @file    md5.c
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    18-March-2015
  * @brief   Md5 check
  *          Bootloader for 3D Printer , is used to update firmware from sd card
  ******************************************************************************
  * 1.���
  * ��MD5�㷨�У�������Ҫ����Ϣ������䣬ʹ��λ����512����Ľ������448������
  * ��������У���ʹ��λ����512����Ľ������448����ˣ���Ϣ��λ����Bit
  * Length��������չ��N*512+448��NΪһ���Ǹ�������N�������㡣
  * ���ķ������£�
  * 1) ����Ϣ�ĺ������һ��1��������0��ֱ���������������ʱ��ֹͣ��0����Ϣ��
  *    ��䡣
  * 2) �����������渽��һ����64λ�����Ʊ�ʾ�����ǰ��Ϣ���ȣ���λΪBit����
  *    ��������Ʊ�ʾ�����ǰ��Ϣ���ȳ���64λ����ȡ��64λ��
  * �����������Ĵ�����Ϣ��λ��=N*512+448+64=(N+1��*512��������ǡ����512������
  * ������������ԭ����Ϊ������洦���ж���Ϣ���ȵ�Ҫ��
  * 2. ��ʼ������
  * ��ʼ��128λֵΪ�������ӱ�������Щ�������ڵ�һ�ֵ����㣬�Դ���ֽ�������ʾ��
  * ���Ƿֱ�Ϊ�� A=0x01234567��B=0x89ABCDEF��C=0xFEDCBA98��D=0x76543210��
  * ��ÿһ��������������ֵ�Ǹ��ֽڴ����ڴ�͵�ַ�����ֽڴ����ڴ�ߵ�ַ������
  * ���ֽ�����С���ֽ�������б���A��B��C��D��ֵ�ֱ�Ϊ0x67452301��0xEFCDAB89��
  * 0x98BADCFE��0x10325476��
  * 3. �����������
  * ÿһ������㷨�������£�
  * ��һ������Ҫ�������ĸ����ӱ������Ƶ������ĸ������У�A��a��B��b��C��c��D��d��
  * �ӵڶ����鿪ʼ�ı���Ϊ��һ���������������A = a�� B = b�� C = c�� D = d��
  * ��ѭ�������֣�MD4ֻ�����֣���ÿ��ѭ���������ơ���һ�ֽ���16�β�����ÿ�β���
  * ��a��b��c��d�е�����������һ�η����Ժ������㣬Ȼ�����ý�����ϵ��ĸ�������
  * �ı���һ���ӷ����һ���������ٽ����ý��������һ������������������a��b��c
  * ��d��֮һ������øý��ȡ��a��b��c��d��֮һ��
  * ������ÿ�β������õ����ĸ������Ժ�����ÿ��һ������
  * F( X ,Y ,Z ) = ( X & Y ) | ( (~X) & Z )
  * G( X ,Y ,Z ) = ( X & Z ) | ( Y & (~Z) )
  * H( X ,Y ,Z ) =X ^ Y ^ Z
  * I( X ,Y ,Z ) =Y ^ ( X | (~Z) )
  *��&���루And����|�ǻ�Or����~�Ƿǣ�Not����^�����Xor����
  * ���ĸ�������˵�������X��Y��Z�Ķ�Ӧλ�Ƕ����;��ȵģ���ô�����ÿһλҲӦ��
  * �����;��ȵġ�
  * F��һ����λ����ĺ������������X����ôY������Z������H����λ��ż��������
  * ����Mj��ʾ��Ϣ�ĵ�j���ӷ��飨��0��15��������ti��4294967296*abs( sin(i) ������
  * �����֣�i ȡֵ��1��64����λ�ǻ��ȡ���4294967296=232��
  * �ֶ��壺
  * FF(a ,b ,c ,d ,Mj ,s ,ti ) ����Ϊ a = b + ( (a + F(b,c,d) + Mj + ti) << s)
  * GG(a ,b ,c ,d ,Mj ,s ,ti ) ����Ϊ a = b + ( (a + G(b,c,d) + Mj + ti) << s)
  * HH(a ,b ,c ,d ,Mj ,s ,ti) ����Ϊ a = b + ( (a + H(b,c,d) + Mj + ti) << s)
  * II(a ,b ,c ,d ,Mj ,s ,ti) ����Ϊ a = b + ( (a + I(b,c,d) + Mj + ti) << s)
  * ע�⣺��<<����ʾѭ������λ����������λ��
  * �����֣���64�����ǣ�
  * ��һ��
  * FF(a ,b ,c ,d ,M0 ,7 ,0xd76aa478 )
  * FF(d ,a ,b ,c ,M1 ,12 ,0xe8c7b756 )
  * FF(c ,d ,a ,b ,M2 ,17 ,0x242070db )
  * FF(b ,c ,d ,a ,M3 ,22 ,0xc1bdceee )
  * FF(a ,b ,c ,d ,M4 ,7 ,0xf57c0faf )
  * FF(d ,a ,b ,c ,M5 ,12 ,0x4787c62a )
  * FF(c ,d ,a ,b ,M6 ,17 ,0xa8304613 )
  * FF(b ,c ,d ,a ,M7 ,22 ,0xfd469501)
  * FF(a ,b ,c ,d ,M8 ,7 ,0x698098d8 )
  * FF(d ,a ,b ,c ,M9 ,12 ,0x8b44f7af )
  * FF(c ,d ,a ,b ,M10 ,17 ,0xffff5bb1 )
  * FF(b ,c ,d ,a ,M11 ,22 ,0x895cd7be )
  * FF(a ,b ,c ,d ,M12 ,7 ,0x6b901122 )
  * FF(d ,a ,b ,c ,M13 ,12 ,0xfd987193 )
  * FF(c ,d ,a ,b ,M14 ,17 ,0xa679438e )
  * FF(b ,c ,d ,a ,M15 ,22 ,0x49b40821 )
  * �ڶ���
  * GG(a ,b ,c ,d ,M1 ,5 ,0xf61e2562 )
  * GG(d ,a ,b ,c ,M6 ,9 ,0xc040b340 )
  * GG(c ,d ,a ,b ,M11 ,14 ,0x265e5a51 )
  * GG(b ,c ,d ,a ,M0 ,20 ,0xe9b6c7aa )
  * GG(a ,b ,c ,d ,M5 ,5 ,0xd62f105d )
  * GG(d ,a ,b ,c ,M10 ,9 ,0x02441453 )
  * GG(c ,d ,a ,b ,M15 ,14 ,0xd8a1e681 )
  * GG(b ,c ,d ,a ,M4 ,20 ,0xe7d3fbc8 )
  * GG(a ,b ,c ,d ,M9 ,5 ,0x21e1cde6 )
  * GG(d ,a ,b ,c ,M14 ,9 ,0xc33707d6 )
  * GG(c ,d ,a ,b ,M3 ,14 ,0xf4d50d87 )
  * GG(b ,c ,d ,a ,M8 ,20 ,0x455a14ed )
  * GG(a ,b ,c ,d ,M13 ,5 ,0xa9e3e905 )
  * GG(d ,a ,b ,c ,M2 ,9 ,0xfcefa3f8 )
  * GG(c ,d ,a ,b ,M7 ,14 ,0x676f02d9 )
  * GG(b ,c ,d ,a ,M12 ,20 ,0x8d2a4c8a )
  * ������
  * HH(a ,b ,c ,d ,M5 ,4 ,0xfffa3942 )
  * HH(d ,a ,b ,c ,M8 ,11 ,0x8771f681 )
  * HH(c ,d ,a ,b ,M11 ,16 ,0x6d9d6122 )
  * HH(b ,c ,d ,a ,M14 ,23 ,0xfde5380c )
  * HH(a ,b ,c ,d ,M1 ,4 ,0xa4beea44 )
  * HH(d ,a ,b ,c ,M4 ,11 ,0x4bdecfa9 )
  * HH(c ,d ,a ,b ,M7 ,16 ,0xf6bb4b60 )
  * HH(b ,c ,d ,a ,M10 ,23 ,0xbebfbc70 )
  * HH(a ,b ,c ,d ,M13 ,4 ,0x289b7ec6 )
  * HH(d ,a ,b ,c ,M0 ,11 ,0xeaa127fa )
  * HH(c ,d ,a ,b ,M3 ,16 ,0xd4ef3085 )
  * HH(b ,c ,d ,a ,M6 ,23 ,0x04881d05 )
  * HH(a ,b ,c ,d ,M9 ,4 ,0xd9d4d039 )
  * HH(d ,a ,b ,c ,M12 ,11 ,0xe6db99e5 )
  * HH(c ,d ,a ,b ,M15 ,16 ,0x1fa27cf8 )
  * HH(b ,c ,d ,a ,M2 ,23 ,0xc4ac5665 )
  * ������
  * II(a ,b ,c ,d ,M0 ,6 ,0xf4292244 )
  * II(d ,a ,b ,c ,M7 ,10 ,0x432aff97 )
  * II(c ,d ,a ,b ,M14 ,15 ,0xab9423a7 )
  * II(b ,c ,d ,a ,M5 ,21 ,0xfc93a039 )
  * II(a ,b ,c ,d ,M12 ,6 ,0x655b59c3 )
  * II(d ,a ,b ,c ,M3 ,10 ,0x8f0ccc92 )
  * II(c ,d ,a ,b ,M10 ,15 ,0xffeff47d )
  * II(b ,c ,d ,a ,M1 ,21 ,0x85845dd1 )
  * II(a ,b ,c ,d ,M8 ,6 ,0x6fa87e4f )
  * II(d ,a ,b ,c ,M15 ,10 ,0xfe2ce6e0 )
  * II(c ,d ,a ,b ,M6 ,15 ,0xa3014314 )
  * II(b ,c ,d ,a ,M13 ,21 ,0x4e0811a1 )
  * II(a ,b ,c ,d ,M4 ,6 ,0xf7537e82 )
  * II(d ,a ,b ,c ,M11 ,10 ,0xbd3af235 )
  * II(c ,d ,a ,b ,M2 ,15 ,0x2ad7d2bb )
  * II(b ,c ,d ,a ,M9 ,21 ,0xeb86d391 )
  * ������Щ���֮�󣬽�a��b��c��d�ֱ���ԭ���������ټ���A��B��C��D��
  * ��a = a + A��b = b + B��c = c + C��d = d + D
  * Ȼ������һ�������ݼ������������㷨��
  * 4. ���
  * ���������a��b��c��d�ļ�����
  **/

#include "string.h"
#include "md5.h"

unsigned char PADDING[]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/** \brief     ��ʼ������
 *
 * \param context MD5_CTX*  MD5�������������ݴ���Ϣ�ṹ���ָ��
 * \return void
 *
 */
void MD5Init(MD5_CTX *context)
{
     context->count[0] = 0;
     context->count[1] = 0;
     context->state[0] = 0x67452301;
     context->state[1] = 0xEFCDAB89;
     context->state[2] = 0x98BADCFE;
     context->state[3] = 0x10325476;
}

/** \brief    ����һ����Ϣ������һ��MD5״̬
 *
 * \param context MD5_CTX*      MD5�������������ݴ���Ϣ�ṹ���ָ��
 * \param input unsigned char*  ����һ����Ϣָ��
 * \param inputlen unsigned int ����һ����Ϣ����
 * \return void
 *
 */
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen)
{
    unsigned int i = 0,index = 0,partlen = 0;
    index = (context->count[0] >> 3) & 0x3F;
    partlen = 64 - index;
    context->count[0] += inputlen << 3;
    if(context->count[0] < (inputlen << 3))
       context->count[1]++;
    context->count[1] += inputlen >> 29;

    if(inputlen >= partlen)
    {
       memcpy(&context->buffer[index],input,partlen);
       MD5Transform(context->state,context->buffer);
       for(i = partlen;i+64 <= inputlen;i+=64)
           MD5Transform(context->state,&input[i]);
       index = 0;
    }
    else
    {
        i = 0;
    }
    memcpy(&context->buffer[index],&input[i],inputlen-i);
}

/** \brief  ����һ��MD5���㣬���MD5ֵ
 *
 * \param context MD5_CTX*          MD5�������������ݴ���Ϣ�ṹ���ָ��
 * \param digest[16] unsigned char  ���MD5ֵ
 * \return void
 *
 */
void MD5Final(MD5_CTX *context,unsigned char digest[16])
{
    unsigned int index = 0,padlen = 0;
    unsigned char bits[8];
    index = (context->count[0] >> 3) & 0x3F;
    padlen = (index < 56)?(56-index):(120-index);
    MD5Encode(bits,context->count,8);
    MD5Update(context,PADDING,padlen);
    MD5Update(context,bits,8);
    MD5Encode(digest,context->state,16);
}

/** \brief
 *
 * \param output unsigned char*
 * \param input unsigned int*
 * \param len unsigned int
 * \return void
 *
 */
void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len)
{
    unsigned int i = 0,j = 0;
    while(j < len)
    {
         output[j] = input[i] & 0xFF;
         output[j+1] = (input[i] >> 8) & 0xFF;
         output[j+2] = (input[i] >> 16) & 0xFF;
         output[j+3] = (input[i] >> 24) & 0xFF;
         i++;
         j+=4;
    }
}

/** \brief
 *
 * \param output unsigned int*
 * \param input unsigned char*
 * \param len unsigned int
 * \return void
 *
 */
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len)
{
     unsigned int i = 0,j = 0;
     while(j < len)
     {
           output[i] = (input[j]) |
                       (input[j+1] << 8) |
                       (input[j+2] << 16) |
                       (input[j+3] << 24);
           i++;
           j+=4;
     }
}

/** \brief
 *
 * \param state[4] unsigned int
 * \param block[64] unsigned char
 * \return void
 *
 */
void MD5Transform(unsigned int state[4],unsigned char block[64])
{
     unsigned int a = state[0];
     unsigned int b = state[1];
     unsigned int c = state[2];
     unsigned int d = state[3];
     unsigned int x[64];

     MD5Decode(x,block,64);
     FF(a, b, c, d, x[ 0], 7, 0xd76aa478); /* 1 */
     FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */
     FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */
     FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */
     FF(a, b, c, d, x[ 4], 7, 0xf57c0faf); /* 5 */
     FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */
     FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */
     FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */
     FF(a, b, c, d, x[ 8], 7, 0x698098d8); /* 9 */
     FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
     FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
     FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
     FF(a, b, c, d, x[12], 7, 0x6b901122); /* 13 */
     FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
     FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
     FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */

     /* Round 2 */
     GG(a, b, c, d, x[ 1], 5, 0xf61e2562); /* 17 */
     GG(d, a, b, c, x[ 6], 9, 0xc040b340); /* 18 */
     GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
     GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
     GG(a, b, c, d, x[ 5], 5, 0xd62f105d); /* 21 */
     GG(d, a, b, c, x[10], 9,  0x2441453); /* 22 */
     GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
     GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
     GG(a, b, c, d, x[ 9], 5, 0x21e1cde6); /* 25 */
     GG(d, a, b, c, x[14], 9, 0xc33707d6); /* 26 */
     GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
     GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
     GG(a, b, c, d, x[13], 5, 0xa9e3e905); /* 29 */
     GG(d, a, b, c, x[ 2], 9, 0xfcefa3f8); /* 30 */
     GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
     GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */

     /* Round 3 */
     HH(a, b, c, d, x[ 5], 4, 0xfffa3942); /* 33 */
     HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
     HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
     HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
     HH(a, b, c, d, x[ 1], 4, 0xa4beea44); /* 37 */
     HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
     HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
     HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
     HH(a, b, c, d, x[13], 4, 0x289b7ec6); /* 41 */
     HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
     HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
     HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
     HH(a, b, c, d, x[ 9], 4, 0xd9d4d039); /* 45 */
     HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
     HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
     HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */

     /* Round 4 */
     II(a, b, c, d, x[ 0], 6, 0xf4292244); /* 49 */
     II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
     II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
     II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
     II(a, b, c, d, x[12], 6, 0x655b59c3); /* 53 */
     II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
     II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
     II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
     II(a, b, c, d, x[ 8], 6, 0x6fa87e4f); /* 57 */
     II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
     II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
     II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
     II(a, b, c, d, x[ 4], 6, 0xf7537e82); /* 61 */
     II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
     II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
     II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */
     state[0] += a;
     state[1] += b;
     state[2] += c;
     state[3] += d;
}
