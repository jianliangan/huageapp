package org.libhuagertp.app;

import android.Manifest;
import android.content.Context;
import android.media.AudioManager;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;

import androidx.fragment.app.FragmentActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class MySdl {
/*
    char *roomid = argv[0];
    char *userid = argv[1];
    char *server = argv[2];
    char *port = argv[3];
    char *uname = argv[4];
 */
    public static native void nativeLogin(String[] params);

    public static native int nativeCreateEnterRoom(int roomid);

    public static native void nativeMainThRun();

    public static native void nativeSendVideEncode(ByteBuffer bytebuf, int size, int timestamp,int channel);

    public static native void nativePushAdioDataPlay(ByteBuffer bytebuf, int size);

    public static native int nativeSetupMysdl();



    public static native void nativeTest();

    public static native void nativeStartStream();



    public static native int nativeVirtualMain0();

    public static native int nativeVirtualMain1();

    public static ByteBuffer[] recBuffer = null;


    public static final int MY_PERMISSIONS_RECORD_AUDIO = 1;
    public static final int MY_PERMISSIONS_RECORD_CAMERA = 2;
    //message
    public static final int COMMAND_LOGINED = 1;


    private static boolean allowCamer = false;
    private static boolean allowMic = false;
    private static Object lockCamerAuthor = new Object();
    private static Object lockMicAuthor = new Object();

    public static Object syncTh = new Object();
    public static boolean needRender;

    public static void ApiPermission(int s) {
        if (s == MY_PERMISSIONS_RECORD_CAMERA) {
            synchronized (lockCamerAuthor) {
                allowCamer = true;
                lockCamerAuthor.notifyAll();
            }
        } else {
            synchronized (lockMicAuthor) {
                allowMic = true;
                lockMicAuthor.notifyAll();
            }
        }
    }

    /**
     * called by jni
     *
     * @param permission
     * @param requestCode
     */
    public static void NotifyReqPermission(String permission, int requestCode) {
        TESTActivity.GetContext().requestPermission(permission, requestCode);
    }

    /**
     * called by jni
     *
     * @param s
     * @return
     */
    public static boolean NotifyPermission(int s) {
        if (s == MY_PERMISSIONS_RECORD_CAMERA) {
            if (allowCamer) {
                return true;
            }
            synchronized (lockCamerAuthor) {
                try {
                    lockCamerAuthor.wait();
                    return true;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    return false;
                }
            }
        } else {
            if (allowMic) {
                return true;
            }
            synchronized (lockMicAuthor) {
                try {
                    lockMicAuthor.wait();
                    return true;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    return false;
                }
            }
        }

    }

    //must call before use MySdl
    public static void ApiLoadDll() {

        System.loadLibrary("MySdl");
        nativeSetupMysdl();
    }

    /**
     * called by jni
     *

     */


    public static void ApiMediaStart() {
        /*

////////////////////
        File f=new File("/data/data/org.libhuagertp.app/logjava.log");
        FileOutputStream fos=null;
        try {
            if(!f.exists()){
                f.createNewFile();
            }
            fos=new FileOutputStream(f);

            fos.write(content.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }finally{
            if(fos!=null){
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }*/
        /////////////////////////
        //
        RunEncodeVideo();
        RunEncodeAudio();
        RunStream();
    }

    public static void ApiRunMainTh() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                MySdl.nativeMainThRun();
            }
        });
        t.start();
    }

    private static void RunStream() {
        nativeStartStream();
    }

    private static void RunEncodeVideo() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                MyEncodeVs myc = new MyEncodeVs();
                try {
                    myc.loop();

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        t.start();
    }

    private static void RunEncodeAudio() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                    nativeVirtualMain0();
            }
        });
        t.start();
    }

    /**
     * called by jni
     */
    public static void NotifyMyTest() {

    }

    /**
     * called by jni
     *
     * @param index
     * @param pc
     */
    public static void NotifyRec(int index, long pc) {

    }
    //???????????????
public static void NotifyVideoData(byte[] data,int size,int pts){
   // Log.i("ddd", "eeeeaaae33eeeeeeeeeeeeeeeeee  add " + size);
if(1==1){
    return;
}
    if ( MyEncodeVs.mEncodeVideoInstan.get(0) == null  )
        return;
//这里原来是有同步锁的，写入和读取的问题，回头在考虑下，感觉可以优化掉
    /*
String aaa="1111111111111111111111111111111111222222222222222222222222";
    File f=new File("/data/data/org.libhuagertp.app/video.data");
    FileOutputStream fos=null;
    try {
        if (!f.exists()) {
            f.createNewFile();
        }
        fos = new FileOutputStream(f,true);

        fos.write(data);
        fos.write(aaa.getBytes());
        fos.close();
    }catch (IOException e){}
    */


        Vframe vfdata=new Vframe();
        vfdata.data=data;
        vfdata.pts=pts;
    synchronized (MyEncodeVs.mEncodeVideoInstan.get(0).lock) {
        MyEncodeVs.mEncodeVideoInstan.get(0).globeOut.add(vfdata);
        MyEncodeVs.mEncodeVideoInstan.get(0).lock.notify();
    }



}
    /**
     * called by jni
     */
    public static boolean NotifySendMessage(int command, Object data) {
        if(command==COMMAND_LOGINED){
            Login.GetContext().SendMessage(command,data);
        }
        return true;
    }



}
