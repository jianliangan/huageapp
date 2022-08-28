//
// Created by ajl on 2022/4/21.
//
#include "stdint.h"
#include "cstring"
#include "math.h"
#include "floatdata.h"
#include <android/log.h>
#include <cassert>

floatdata::floatdata() {
}

void floatdata::find0b(int sub, floatposition &float_start, uint16_t *range_data, int curbit,
                       int curindex, int maxindex) {
    for (int j = curbit - 1; j >= 0; j--) {
        if (((sub >> j) & 0x01) == 0) {
            float_start.bpos = j;
            float_start.Bpos = curindex;
            break;
        }
    }


    if (float_start.bpos == -1) {
        for (int m = curindex + 1; m < maxindex; m++) {
            uint16_t starttmp = 0;//*(range_data + m);
            toolsIntBigEndianV((uint8_t * )(range_data + m), (uint8_t * ) & starttmp, 16);
            if (starttmp == 65535) {
                continue;
            } else {
                for (int j = 15; j >= 0; j--) {
                    if (((starttmp >> j) & 0x01) == 0) {
                        float_start.bpos = 16 - j;
                        float_start.Bpos = m;
                        break;
                    } else {
                        continue;
                    }
                }//
                if (float_start.bpos != -1) {
                    break;
                }
            }
        }
    }

}

void floatdata::find1join0(uint16_t sub, floatposition &float_start_1, floatposition &float_start_0,
                           uint16_t *range_data, int curbit, int curindex, int maxindex) {

    for (int j = curbit - 1; j >= 0; j--) {
        if (((sub >> j) & 0x01) == 1) {
            float_start_1.bpos = j;
            float_start_1.Bpos = curindex;
            break;
        } else {
            if (float_start_0.bpos == -1) {
                float_start_0.bpos = j;
                float_start_0.Bpos = curindex;
            }
        }
    }
    if (float_start_1.bpos == -1) {
        for (int m = curindex + 1; m < maxindex; m++) {
            uint16_t starttmp = 0;//*(range_data + m);
            toolsIntBigEndianV((uint8_t * )(range_data + m), (uint8_t * ) & starttmp, 16);
            if (starttmp == 0) {
                if (float_start_0.bpos == -1) {
                    float_start_0.bpos = 1;
                    float_start_0.Bpos = m;
                }
                continue;
            } else {
                for (int j = 15; j >= 0; j--) {
                    if (((starttmp >> j) & 0x01) == 1) {
                        float_start_1.bpos = 16 - j;
                        float_start_1.Bpos = m;
                        break;
                    } else {
                        if (float_start_0.bpos == -1) {
                            float_start_0.bpos = 16 - j;
                            float_start_0.Bpos = m;
                        }
                    }
                }//
                if (float_start_1.bpos != -1) {
                    break;
                }
            }
        }
    }
}

floatposition floatdata::findmin2(uint16_t sub, int curindex, int maxindex, uint16_t *range_start,
                                  uint16_t *range_end) {
//先找到找到第一个不一样的位置
    floatposition offsetbit;
    for (int j = 15; j >= 0; j--) {
        if (((sub >> j) & 0x01) == 1) {
            offsetbit.bpos = 16 - j;
            offsetbit.Bpos = curindex;
        }
    }//
//往后找，先找第一个1的位置，如果中间找到第一个0就记下来
    floatposition float_start_1;
    floatposition float_start_0;
    find1join0(sub, float_start_1, float_start_0, range_start, offsetbit.bpos, curindex, maxindex);

    floatposition float_end_1;
    floatposition float_end_0;
    find1join0(sub, float_end_1, float_end_0, range_end, offsetbit.bpos, curindex, maxindex);

    //如果后面都有1
    if (float_start_1.bpos != -1 && float_end_1.bpos != -1) {
        //第一个不一样的必定是start是0，end是0，否则就是end小于start了
        uint16_t value = 0;//*(range_start + offsetbit.Bpos)
        toolsIntBigEndianV((uint8_t * )(range_start + offsetbit.Bpos), (uint8_t * ) & value, 16);
        /*
        *(range_start + offsetbit.Bpos) =
                ((value >> (16 - offsetbit.bpos)) | 0x01) << (16 - offsetbit.bpos);*/
        uint16_t tmpbig = ((value >> (16 - offsetbit.bpos)) | 0x01) << (16 - offsetbit.bpos);
        toolsIntBigEndian((uint8_t * )(range_start + offsetbit.Bpos), tmpbig, 16);
        goto result;
    } else if (float_start_1.bpos != -1 && float_end_1.bpos == -1) {
        //否则如果 开始后面有，结束后面没有
        //如果开始后面没有0
        if (float_start_0.bpos == -1) {
            uint16_t sub = 0;
            toolsIntBigEndianV((uint8_t * )(range_start + float_start_1.Bpos), (uint8_t * )(&sub),
                               16);
            find0b(sub, float_start_0, range_start, float_start_1.bpos, float_start_1.Bpos,
                   maxindex);
        }
        if (float_start_0.bpos == -1) {
            offsetbit.bpos = 16;
            offsetbit.Bpos = maxindex - 1;
            goto result;
        } else {
            offsetbit.bpos = float_start_0.bpos;
            offsetbit.Bpos = float_start_0.Bpos;
            uint16_t value = 0;// *(range_start + offsetbit.Bpos);
            toolsIntBigEndianV((uint8_t * )(range_start + offsetbit.Bpos), (uint8_t * )(&value),
                               16);

            // *(range_start + offsetbit.Bpos) =
            //        ((value >> (16 - offsetbit.bpos)) | 0x01) << (16 - offsetbit.bpos);
            uint16_t tmpbig = ((value >> (16 - offsetbit.bpos)) | 0x01) << (16 - offsetbit.bpos);
            toolsIntBigEndian((uint8_t * )(range_start + offsetbit.Bpos), tmpbig, 16);
            goto result;
        }
    } else//(float_start_1.bpos==-1&&float_end_1.bpos!=-1) or (float_start_1.bpos==-1&&float_end_1.bpos==-1)
    {
        goto result;
    }

    //否则如果 开始后面没有，结束后面有
    result:
    return offsetbit;

}

void floatdata::init(char *datav, int count) {
    data = (uint16_t *) datav;
    b16count = count;
}

void floatdata::init() {
    char *datatmp = new char[SIZE_BLOCK_BIT8];
    memset(datatmp, '\0', SIZE_BLOCK_BIT8);
    data = (uint16_t *) datatmp;
    b16count = 0;
}

void floatdata::set(float fv) {

    if (fv <= 0.000001) {
        memset(data, '\0', SIZE_BLOCK_BIT8);
        b16count = 0;
    } else if (1 - fv <= 0.000001) {
        memset(data, '\0', SIZE_BLOCK_BIT8);
        b16count = -1;
    } else {

        uint32_t v = *(int *) (&fv);
        int preexp = ((v >> 24) << 1) | ((((v >> 16) & 0xff) >> 7) & 0x01);
        int exp = abs(preexp - 127) - 1;
        char *tmp = (char *) data + exp / 8;
        // __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd1 ", "%f", fv);
        // __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd2 ", "%u", v);
        uint32_t floatdata = (uint32_t)((v << 8) | ((uint32_t) 1 << 31));
        //   __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd3 ", "%u", floatdata);

        uint32_t *tmp32 = (uint32_t *) tmp;
        //*tmp32 = floatdata >> (exp%8-1);
        toolsIntBigEndian((uint8_t *) tmp32, floatdata >> (exp % 8), 32);




        //  for(int i=0;i<4;i++){
        //      __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd3-- ", "%u", *((uint8_t *)tmp32+i));
        //   }

        b16count = ((exp + 24) + 15) / 16;
        {
            // uint16_t tmpdata = 0;
            // __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd b16 ", "%f",fv);
            //  for (int i = 0; i < this->b16count; i++) {
            //      toolsIntBigEndianV((uint8_t * )(data + i), (uint8_t * ) & tmpdata, 16);
            //     __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd b16 ", "%u",
            //                          tmpdata);
            //  }
        }
        {

            //   __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd b8 ", "%f",fv);
            //   for (int i = 0; i < this->b16count*2; i++) {
            //        __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd b8 ", "%u",
            //                          *((uint8_t *)data+i));
            //    }
        }
        //   __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd3 ", "%u  %d  %d", *tmp32, exp,
        //                        b16count);
    }
}

void floatdata::product(floatdata *from, floatdata *pro, floatdata *out) {

    if (pro == nullptr) {
        return;
    }
    if (from->b16count == -1) {
        copy(pro, out);
        return;
    }
    if (pro->b16count == -1) {
        copy(from, out);
        return;
    }
    int b16count2 = pro->b16count;
    uint16_t *data2 = pro->data;

    uint16_t *fromdata = from->data;
    int fromb16count = from->b16count;
    int maxlen = SIZE_BLOCK_BIT8 / 2;
    uint16_t *outdatablock = data2;
//uint16_t outdatablock[SIZE_BLOCK_BIT8]={0};
    int left16len = fromb16count + pro->b16count;

    int outdatalen = 0;
   //  __android_log_print(ANDROID_LOG_INFO, "ttttttttttttttttttdd 00000000000000000", "%s","rrrrr");
  // uint64_t tmptime=hgetSysTimeMicros();
  // uint64_t addtotal1=0;
  //  uint64_t addtotal2=0;
    bool all0 = true;
    for (int j = b16count2 - 1; j >= 0; j--) {
        int tmpindex = left16len - 1 - (b16count2 - 1 - j);
        int carry = 0;//进位
        uint16_t datatmp = 0;
        toolsIntBigEndianV((uint8_t * )(data2 + j), (uint8_t * ) & datatmp, 16);
        *(data2 + j) = 0;
        for (int i = fromb16count - 1; i >= 0; i--) {
            if (tmpindex <= maxlen) {
                uint16_t fromtmp = 0;
                //uint64_t tmptime=hgetSysTimeMicros();
               // addtotal1 +=(hgetSysTimeMicros()-tmptime);
                toolsIntBigEndianV((uint8_t * )(fromdata + i), (uint8_t * ) & fromtmp, 16);


                uint32_t value =
                        fromtmp * datatmp + carry;//(*(fromdata + i)) * (*(data2 + j)) + carry;
               // uint64_t tmptime3=hgetSysTimeMicros();
               // addtotal2 +=(hgetSysTimeMicros()-tmptime3);
                if (value != 0 && all0 == true) {
                    all0 = false;
                }

                if (value >= 65536) {
                    carry = value / 65536;
                    value = value % 65536;
                } else {
                    carry = 0;
                }
                uint16_t outdatatmp = 0;



            //    __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd  30000", "%ld ,%u",(uint64_t)outdatablock,tmpindex);
                toolsIntBigEndianV((uint8_t * )(outdatablock + tmpindex), (uint8_t * ) & outdatatmp,
                                   16);
                value = value + outdatatmp;//*(outdatablock + tmpindex);
                if (value >= 65536) {
                    carry += value / 65536;
                    value = value % 65536;
                } else {
                    carry += 0;
                }
            //      __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd  pp30000", "%u,%u",value,(tmpindex));
                toolsIntBigEndian((uint8_t * )(outdatablock + tmpindex), value, 16);
                //
                //*(outdatablock + tmpindex) = value;
                if (i == 0 && carry != 0) {
                    //*(outdatablock + tmpindex - 1) = carry;
                    toolsIntBigEndian((uint8_t * )(outdatablock + tmpindex - 1), carry, 16);

                 //        __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd  --pp30000", "%u,%u",carry,(tmpindex - 1));

                    outdatalen = tmpindex - 1;
                } else {
                    outdatalen = tmpindex;
                }

            }
            tmpindex--;

        }


    }
    //uint64_t tmptime_=hgetSysTimeMicros()-tmptime;
    //__android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd  55555555", "%ld %ld %ld",tmptime_,addtotal1,addtotal2);
    if (all0) {
        left16len = 0;
    }
    int b16countmax = SIZE_BLOCK_BIT8 / 2;
    left16len = (left16len > b16countmax) ? b16countmax : left16len;
    //memcpy(out->data, (char *) outdatablock, left16len * 2);
    out->b16count = left16len;
}

int floatdata::add(floatdata *from, floatdata *add, floatdata *out) {
    if (add == nullptr) {
        return 0;
    }
    if (from->b16count == -1) {
        return -1;
    }
    if (add->b16count == -1) {
        return -1;
    }
    int b16counttotal = 0;
    int fromb16count = from->b16count;
    b16counttotal = (fromb16count > add->b16count) ? fromb16count : add->b16count;

    uint16_t *fromdata = from->data;

    uint16_t *data2 = add->data;
    uint16_t *outdata = out->data;
    int carry = 0;//进位

    for (int i = b16counttotal - 1; i >= 0; i--) {
        uint16_t fromtmp = 0;
        uint16_t datatmp = 0;
        toolsIntBigEndianV((uint8_t * )(fromdata + i), (uint8_t * ) & fromtmp, 16);
        toolsIntBigEndianV((uint8_t * )(data2 + i), (uint8_t * ) & datatmp, 16);

        //   __android_log_print(ANDROID_LOG_INFO, "ddddddddddddddddddddd30000", "%u,%u",fromtmp,datatmp);

        int value = fromtmp + datatmp + carry;

        if (value >= 65536) {
            //value-=65536;
            carry = 1;
        } else {
            carry = 0;
        }
        toolsIntBigEndianV((uint8_t * ) & value, (uint8_t * )(outdata + i), 16);
        //*(outdata + i) = value;
    }
    out->b16count = b16counttotal;
    return 0;
}

int floatdata::sub(floatdata *from, floatdata *sub, floatdata *out) {
    if (sub == nullptr) {
        return 0;
    }
    if (sub->b16count == -1) {
        return -1;
    }
    if (sub->b16count == 0) {
        copy(from, out);
        return 0;
    }
    int b16counttotal = 0;
    int fromb16count = from->b16count;
    b16counttotal = (fromb16count > sub->b16count) ? fromb16count : sub->b16count;

    uint16_t *data2 = sub->data;
    uint16_t *fromdata = from->data;

    uint16_t *outdata = out->data;
    int carry = 0;//进位
    int tail0 = 0;
    bool isfirst = true;
    for (int i = b16counttotal - 1; i >= 0; i--) {

        uint16_t fromtmp = 0;
        uint16_t datatmp = 0;
        toolsIntBigEndianV((uint8_t * )(fromdata + i), (uint8_t * ) & fromtmp, 16);
        toolsIntBigEndianV((uint8_t * )(data2 + i), (uint8_t * ) & datatmp, 16);


        int value = fromtmp - datatmp + carry;
        if (value < 0) {
            value = 65536 + fromtmp - datatmp;
            carry = -1;
        } else {
            carry = 0;
            if (value != 0) {
                isfirst = false;
            }
            if (value == 0 && isfirst) {
                tail0++;
            }
        }
        toolsIntBigEndianV((uint8_t * ) & value, (uint8_t * )(outdata + i), 16);
        //*(outdata + i) = value;
    }

    out->b16count = b16counttotal - tail0;
    return 0;
}

void floatdata::copy(floatdata *from, floatdata *to) {
    to->b16count = from->b16count;
    if (from->b16count == -1) {
        memset(to->data, '\0', SIZE_BLOCK_BIT8);
    } else
        memcpy(to->data, from->data, from->b16count * 2);
}

int floatdata::compare(floatdata *from, floatdata *to) {
    int maxindex = (from->b16count > to->b16count) ? from->b16count : to->b16count;
    for (int i = 0; i < maxindex; i++) {

        uint16_t fromtmp = 0;
        uint16_t datatmp = 0;
        toolsIntBigEndianV((uint8_t * )(from->data + i), (uint8_t * ) & fromtmp, 16);
        toolsIntBigEndianV((uint8_t * )(to->data + i), (uint8_t * ) & datatmp, 16);

        if (fromtmp > datatmp) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}

/*
int floatdata::maxindex(floatdata *d1,floatdata *d2){
    if(d1->b16count==-1)
        return d2->b16count;
    int ret=d1->b16count > d2->b16count ? d1->b16count : d2->b16count;
    return ret;
}*/

void
floatdata::arith_decode(char *input, int size, char *output, int &outsize) {

    float ratio0 = *((float *) (input + 2));
    int orgBsize = *((uint16_t *) input);
    int orgbsize = orgBsize * 8;


    floatdata inputdata;
    inputdata.init(input, size / 2);

    floatdata ratio_fix;
    ratio_fix.init();
    ratio_fix.set(ratio0);

    floatdata *start_ptr;
    floatdata *middle_ptr;
    floatdata *end_ptr;
    floatdata *tmp_ptr;
    floatdata store_box[3];
    store_box[0].init();
    store_box[1].init();
    store_box[2].init();

    store_box[0].set(0);
    floatdata::copy(&ratio_fix, &store_box[1]);
    store_box[2].set(1);
    start_ptr = &store_box[0];
    middle_ptr = &store_box[1];
    end_ptr = &store_box[2];
//回头
    int index = 0;
    int boffset = 0;
    char *curB = nullptr;
    curB = output + index;
    outsize = orgBsize;
    for (int i = 0; i < orgbsize; i++) {

        if (floatdata::compare(&inputdata, start_ptr) >= 0 &&
            floatdata::compare(&inputdata, middle_ptr) < 0) {
            *curB = (*curB) | (1 << (7 - boffset));

            floatdata::sub(middle_ptr, start_ptr, end_ptr);
            floatdata::product(&ratio_fix, end_ptr, end_ptr);
            floatdata::add(start_ptr, end_ptr, end_ptr);
            tmp_ptr=middle_ptr;
            middle_ptr=end_ptr;
            end_ptr=tmp_ptr;

        } else {

            floatdata::sub(end_ptr, middle_ptr, start_ptr);
            floatdata::product(&ratio_fix, start_ptr, start_ptr);
            floatdata::add(middle_ptr, start_ptr, start_ptr);
            tmp_ptr=middle_ptr;
            middle_ptr=start_ptr;
            start_ptr=tmp_ptr;
        }



        if (boffset == 7) {
            boffset = 0;
            index++;
            curB = output + index;
        } else {
            boffset++;
        }
    }
}

void floatdata::arith_encode(char *input, int size, char *output, int &outsize) {
    int count1 = 0;
    int count0 = 0;

    for (int i = 0; i < size; i++) {
        char tmp = *(input + i);
        for (int j = 0; j < 8; j++) {
            if (((tmp >> j) & 1) == 1) {
                count1++;
            } else {
                count0++;
            }
        }
    }

    floatdata ratio_fix;
    ratio_fix.init();
    float rat = (float) count0 / (size * 8);

    ratio_fix.set(rat);

    floatdata *start_ptr;
    floatdata *middle_ptr;
    floatdata *end_ptr;
    floatdata *tmp_ptr;
    floatdata store_box[3];
    store_box[0].init();
    store_box[1].init();
    store_box[2].init();

    store_box[0].set(0);
    floatdata::copy(&ratio_fix, &store_box[1]);
    store_box[2].set(1);

    start_ptr = &store_box[0];
    middle_ptr = &store_box[1];
    end_ptr = &store_box[2];
////到这里了，往下
    floatdata *range_start = nullptr;
    floatdata *range_end = nullptr;

    for (int i = 0; i < size; i++) {
        char tmp = *(input + i);
        for (int j = 0; j < 8; j++) {
            if (((tmp >> j) & 1) == 0) {
                if (i == size - 1 && j == 7) {
                    range_start = start_ptr;
                    range_end = middle_ptr;
                    break;
                }
                floatdata::sub(middle_ptr, start_ptr, end_ptr);
                floatdata::product(&ratio_fix, end_ptr, end_ptr);
                floatdata::add(start_ptr, end_ptr, end_ptr);
                tmp_ptr = middle_ptr;
                middle_ptr = end_ptr;
                end_ptr = tmp_ptr;
            } else {
                if (i == size - 1 && j == 7) {
                    range_start = middle_ptr;
                    range_end = end_ptr;
                    break;
                }
                floatdata::sub(end_ptr, middle_ptr, start_ptr);
                floatdata::product(&ratio_fix, start_ptr, start_ptr);
                floatdata::add(middle_ptr, start_ptr, start_ptr);
                tmp_ptr = middle_ptr;
                middle_ptr = start_ptr;
                start_ptr = tmp_ptr;
            }
        }
    }
    return;

    /*
     * range_start
     * range_end
     * 取一个值就是结果
     */
    floatposition offsetbit;
    int max = range_start->b16count > range_end->b16count ? range_start->b16count
                                                          : range_end->b16count;
    //第一个不一样的位

    for (int i = 0; i < max; i++) {
        uint16_t start = *(range_start->data + i);
        uint16_t end = *(range_end->data + i);
        if (start == end) {
            continue;
        } else {
            uint16_t sub = end - start;
            offsetbit = floatdata::findmin2(sub, i, max, range_start->data, range_end->data);
            break;
        }
    }
//还要算出来去掉末尾的一个字节
    outsize = (offsetbit.Bpos + 1) * 2;

    uint16_t *Bcount = (uint16_t *) output;
    float *ratiotmp = (float *) (output + 2);
    *ratiotmp = count0 / (float) size;
    *Bcount = size;
    // memcpy(output+6, range_start->data, outsize);
}