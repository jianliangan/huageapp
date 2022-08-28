package org.libhuagertp.app;
/**
 * 这个类用来测试解码后显示的
 */

import android.animation.TimeAnimator;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.Image;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import androidx.annotation.NonNull;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ReadOnlyBufferException;

public class MytestView extends SurfaceView implements SurfaceHolder.Callback,
        View.OnKeyListener, View.OnTouchListener, SensorEventListener {
    protected static SurfaceView mtestsurface;
    public static String TAG = "aaaaaaaaaa";

    private MyDecodeVs mdv=new MyDecodeVs();
    //private MyDecodeAs mda=new MyDecodeAs();
    // public static myMediaDecode mmd;


    // Is SurfaceView ready for rendering

    // Startup
    public MytestView(Context context) {

        super(context);
        getHolder().addCallback(this);

    }

    public void handlePause() {

    }

    public void handleResume() {

    }

    public Surface getNativeSurface() {
        return getHolder().getSurface();
    }

    // Called when we have a valid drawing surface
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v("my", "surfaceCreated()");
        mdv.initDecodec(getHolder().getSurface());
//mda.initDecodec();

//        mTimeAnimator.setTimeListener(new TimeAnimator.TimeListener() {
//            @Override
//            public void onTimeUpdate(final TimeAnimator animation,
//                                     final long totalTime,
//                                     final long deltaTime) {
//
//              //  loop1();
//
//            }
//        });

        // We're all set. Kick off the animator to process buffers and render video frames as
        // they become available
        // mTimeAnimator.start();

    }

    // Called when we lose the surface
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v("my", "surfaceDestroyed()");

    }

    // Called when the surface is resized
    @Override
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {


    }

    // Key events
    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {


        return true;
    }

    // Touch events
    @Override
    public boolean onTouch(View v, MotionEvent event) {


        return true;
    }

    // Sensor events
    public void enableSensor(int sensortype, boolean enabled) {

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO
    }

    @Override
    public void onSensorChanged(SensorEvent event) {

    }

    // Captured pointer events for API 26.
    public boolean onCapturedPointerEvent(MotionEvent event) {

        return true;
    }




}
