//
// Created by ajl on 2022/5/6.
//
#include <cassert>
#include "func_test.h"
/*
 0.015123
 3 223 25 216
  00000011  11011111  00011001  11011000
0.015
 3 215 10 60
  00000011  11010111  00001010  00111100
0.025
 6 102 102 104
 00000110  01100110  01100110  01101000
0.025123
 6 110 118 0
 00000110  01101110  01110110  00000000
 */
/*
 0.015123*0.025123=下面了
 */
void func_test::copy_test() {
    {
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025);
        floatdata::copy(&ratio_fix1, &ratio_fix2);

        int max = ratio_fix1.b16count > ratio_fix2.b16count ? ratio_fix1.b16count
                                                            : ratio_fix2.b16count;
        for (int i = 0; i < max; i++) {
            assert(ratio_fix1.data[i] == ratio_fix2.data[i]);
            assert(ratio_fix1.b16count == ratio_fix2.b16count);
        }
    }

}

void func_test::add_test() {
    {
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025);
        //int floatdata::add(floatdata *from, floatdata *add, floatdata *out)
        floatdata::add(&ratio_fix1, &ratio_fix2, &ratio_fix2);

        // int max = ratio_fix1.b16count > ratio_fix2.b16count ? ratio_fix1.b16count
        //                                                     : ratio_fix2.b16count;
        assert(*((uint8_t *) ratio_fix2.data + 0) == 10);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 61);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 112);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 164);
        /*
        for (int i = 0; i < max * 2; i++) {
            __android_log_print(ANDROID_LOG_INFO, "testtesttesttestadd ", "%u",
                                *((uint8_t *) ratio_fix2.data + i));
        }*/
    }
    {
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015123);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025);
        //int floatdata::add(floatdata *from, floatdata *add, floatdata *out)
        floatdata::add(&ratio_fix1, &ratio_fix2, &ratio_fix2);
        assert(*((uint8_t *) ratio_fix2.data + 0) == 10);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 69);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 128);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 64);

    }
    {
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025123);
        //int floatdata::add(floatdata *from, floatdata *add, floatdata *out)
        floatdata::add(&ratio_fix1, &ratio_fix2, &ratio_fix2);

        assert(*((uint8_t *) ratio_fix2.data + 0) == 10);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 69);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 128);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 60);
    }
}

void func_test::product_test() {
    {

        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.2);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.2);
        //int floatdata::add(floatdata *from, floatdata *add, floatdata *out)
        floatdata::product(&ratio_fix1, &ratio_fix2, &ratio_fix2);

        assert(*((uint8_t *) ratio_fix2.data + 0) == 10);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 61);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 112);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 168);
        assert(*((uint8_t *) ratio_fix2.data + 4) == 245);
        assert(*((uint8_t *) ratio_fix2.data + 5) == 194);
        assert(*((uint8_t *) ratio_fix2.data + 6) == 144);
        assert(*((uint8_t *) ratio_fix2.data + 7) == 0);
    }

    {
        /*
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015123);

        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025123);
        //int floatdata::add(floatdata *from, floatdata *add, floatdata *out)
        floatdata::product(&ratio_fix1, &ratio_fix2, &ratio_fix2);
        assert(*((uint8_t *) ratio_fix2.data + 0) == 0);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 24);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 230);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 65);
        assert(*((uint8_t *) ratio_fix2.data + 4) == 0);
        assert(*((uint8_t *) ratio_fix2.data + 5) == 185);
        assert(*((uint8_t *) ratio_fix2.data + 6) == 144);
        assert(*((uint8_t *) ratio_fix2.data + 7) == 0);*/
    }


}

void func_test::sub_test() {
    floatdata ratio_fix1;
    ratio_fix1.init();
    ratio_fix1.set(0.03556);

    floatdata ratio_fix2;
    ratio_fix2.init();
    ratio_fix2.set(0.0251);

    floatdata::sub(&ratio_fix1, &ratio_fix2, &ratio_fix2);

    assert(*((uint8_t *) ratio_fix2.data + 0) == 2);
    assert(*((uint8_t *) ratio_fix2.data + 1) == 173);
    assert(*((uint8_t *) ratio_fix2.data + 2) == 129);
    assert(*((uint8_t *) ratio_fix2.data + 3) == 176);
}

void func_test::set_test() {
    {
        floatdata ratio_fix1;
        ratio_fix1.init();
        ratio_fix1.set(0.015);
        assert(*((uint8_t *) ratio_fix1.data + 0) == 3);
        assert(*((uint8_t *) ratio_fix1.data + 1) == 215);
        assert(*((uint8_t *) ratio_fix1.data + 2) == 10);
        assert(*((uint8_t *) ratio_fix1.data + 3) == 60);
    }
    {
        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.025);
        assert(*((uint8_t *) ratio_fix2.data + 0) == 6);
        assert(*((uint8_t *) ratio_fix2.data + 1) == 102);
        assert(*((uint8_t *) ratio_fix2.data + 2) == 102);
        assert(*((uint8_t *) ratio_fix2.data + 3) == 104);
    }
    {
        floatdata ratio_fix2;
        ratio_fix2.init();
        ratio_fix2.set(0.2);
        for (int i = 0; i < ratio_fix2.b16count ; i++) {

            uint16_t fromtmp = 0;

            toolsIntBigEndianV((uint8_t * )(ratio_fix2.data + i), (uint8_t * ) & fromtmp, 16);
            __android_log_print(ANDROID_LOG_INFO, "testtesttesttestset 0.2 ", "%u",
                                fromtmp);
        }
    }

/*
    for (int i = 0; i < ratio_fix1.b16count * 2; i++) {
        __android_log_print(ANDROID_LOG_INFO, "testtesttesttestset 0.015 ", "%u",
                            *((uint8_t *) ratio_fix1.data + i));
    }
    for (int i = 0; i < ratio_fix2.b16count * 2; i++) {
        __android_log_print(ANDROID_LOG_INFO, "testtesttesttestset 0.025 ", "%u",
                            *((uint8_t *) ratio_fix2.data + i));
    }*/
}