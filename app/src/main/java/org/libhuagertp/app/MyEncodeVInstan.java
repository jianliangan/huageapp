package org.libhuagertp.app;

import android.media.MediaCodec;
import android.opengl.EGL14;
import android.opengl.EGLSurface;

import java.util.LinkedList;
import java.util.Queue;

public class MyEncodeVInstan {

    public int width;
    public int height;
    public int mBitRate ;
    public int frameRate ;
    public int iframeInternal;
/////////////
public Object lock;
    public int videoStartTime=0;
    MediaCodec.BufferInfo mBufferInfo;
    private final boolean VERBOSE = true;
    private final String TAG = "bbb";
    public final int TIMEOUT_USEC = 30000;
    public Queue<Vframe> globeOut;
    int index;
    //////////////
    public MediaCodec mMediaCodec;

    public EGLSurface mEGLSurface = EGL14.EGL_NO_SURFACE;
    private void videoptsincr(){
        videoStartTime++;
        if(videoStartTime>0xffffffff){
            videoStartTime=0;
        }
    }
    MyEncodeVInstan(int w, int h,int bitrate,int framerate,int iframeinternal) {
        width = w;
        height = h;
lock=new Object();
        mBitRate =bitrate; //2000000;
        frameRate = framerate;
        iframeInternal = iframeinternal;
        globeOut = new LinkedList<Vframe>();
    }
}
