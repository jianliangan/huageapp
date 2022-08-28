package org.libhuagertp.app;

public class MyCommon {
   static public int[] freqSample = new int[]{
            96000,
            88200,
            64000,
            48000,
            44100,
            32000,
            24000,
            22050,
            16000,
            12000,
            11025,
            8000,
            7350
    };
};
class Vframe{
    public int pts;
    public byte[] data;
    public int getlen(){
        return data.length;
    }
}
