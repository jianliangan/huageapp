//
// Created by ajl on 2021/6/16.
//

#include "base.h"
#define max_uint32_NUM 4294967295 //0-4294967294
int uint32Sub(uint32_t newd,uint32_t oldd){
    uint32_t ret;
    if(newd<oldd){
        ret= (newd-0)+1+(max_uint32_NUM-1)-oldd;
        if(ret>max_uint32_NUM/2){
            return -1;
        }else{
            return ret;
        }
    }else{
        return newd-oldd;
    }
}

uint32_t uint32Add(uint32_t add1,uint32_t add2){
    return (uint32_t)((add1+add2)&(0xffffffff));
}
void toolsIntBigEndianV(uint8_t *d, uint8_t *val, uint32_t digits) {
    if (digits == 8) {
        *val=*(d++);
    } else if (digits == 16) {
        *(val+1)=*(d++);
        *(val)=*(d++);
    } else if (digits == 32) {
        *(val+3)=*(d++);
        *(val+2)=*(d++);
        *(val+1)=*(d++);
        *(val)=*(d++);
    }
}
void toolsIntBigEndian(uint8_t *d, uint32_t val, uint32_t digits) {
    if (digits == 8) {
        *(d++) = (uint8_t)(val & 0xff);
    } else if (digits == 16) {
        *(d++) = (uint8_t)((val >> 8) & 0xff); //高位
        *(d++) = (uint8_t)(val & 0xff);//低位
    } else if (digits == 32) {
        *(d++) = (uint8_t)((val >> 24) & 0xff);//高位
        *(d++) = (uint8_t)((val >> 16) & 0xff);//低位
        *(d++) = (uint8_t)((val >> 8) & 0xff);//。。
        *(d++) = (uint8_t)(val & 0xff);
    }
}
void toolsIntLitEndian(uint8_t *d, uint32_t val, uint32_t digits) {
    if (digits == 8) {
        *(d++) = (uint8_t)(val & 0xff);
    } else if (digits == 16) {
        *(d++) = (uint8_t)(val & 0xff);//低位
        *(d++) = (uint8_t)((val >> 8) & 0xff); //高位
    } else if (digits == 24) {
        *(d++) = (uint8_t)(val & 0xff);
        *(d++) = (uint8_t)((val >> 8) & 0xff);//。。
        *(d++) = (uint8_t)((val >> 16) & 0xff);//低位
    } else if (digits == 32) {
        *(d++) = (uint8_t)(val & 0xff);
        *(d++) = (uint8_t)((val >> 8) & 0xff);//。。
        *(d++) = (uint8_t)((val >> 16) & 0xff);//低位
        *(d++) = (uint8_t)((val >> 24) & 0xff);//高位
    }
}
uint64_t hgetSysTimeMicros()
{
#ifdef _WIN32
    // 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME   (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    uint64_t tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) /10;
    return tt;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
#endif // _WIN32
    return 0;
}