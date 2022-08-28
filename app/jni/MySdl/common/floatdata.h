//
// Created by ajl on 2022/4/21.
//

#ifndef HUAGERTP_APP_FLOATDATA_H
#define HUAGERTP_APP_FLOATDATA_H
#include "base.h"
#define SIZE_BLOCK_BIT8 512
#define EXP_NUMBER 2
typedef struct floatposition {
    int8_t bpos;
    int Bpos;

    floatposition() : bpos(-1), Bpos(-1) {}
} floatposition;


//原理基于
// a=n*L+m   n为整数，L位64位无符号最大值,m位64位无符号数字范围
//只保留小数点后面
class floatdata {
public:
    int b16count;//left to right byte count in 16bit
    uint16_t *data;

    floatdata();

    void init();

    void init(char *datav, int count);

    void set(float i);

    static void
    find0b(int sub, floatposition &float_start, uint16_t *range_data, int curbit, int curindex,
           int maxindex);

    static void find1join0(uint16_t sub, floatposition &float_start_1, floatposition &float_start_0,
                           uint16_t *range_data, int curbit, int curindex, int maxindex);

    static floatposition
    findmin2(uint16_t sub, int curindex, int maxindex, uint16_t *range_start, uint16_t *range_end);

    static int sub(floatdata *from, floatdata *sub, floatdata *out);

    static int add(floatdata *from, floatdata *add, floatdata *out);

    static void copy(floatdata *from, floatdata *to);

    static void product(floatdata *from, floatdata *pro, floatdata *out);

    static int compare(floatdata *from, floatdata *to);
    static void arith_encode(char *input, int size, char *output, int &outsize);
    static void arith_decode(char *input, int size, char *output,int &outsize);
};

#endif //HUAGERTP_APP_FLOATDATA_H
