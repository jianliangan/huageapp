package org.libhuagertp.app;

import android.Manifest;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.os.Environment;

import android.util.Log;
import android.util.Size;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Queue;


import javax.microedition.khronos.opengles.GL10;

/**
 * Generate an MP4 file using OpenGL ES drawing commands.  Demonstrates the use of MediaMuxer
 * and MediaCodec with Surface input.
 * <p>
 * This uses various features first available in Android "Jellybean" 4.3 (API 18).  There is
 * no equivalent functionality in previous releases.
 * <p>
 * (This was derived from bits and pieces of CTS tests, and is packaged as such, but is not
 * currently part of CTS.)
 */
public class MyEncodeVs {
    private MyCamera mCamera2;
    public static SurfaceTexture msurfaceTexture;
    public static MyDraw mDraw;

    public MyEncodeVs.CodecInputSurface mInputSurface;
    private static final String TAG = "EncodeAndMuxTest";
    private static final boolean VERBOSE = false;           // lots of logging

    // where to put the output file (note: /sdcard requires WRITE_EXTERNAL_STORAGE permission)

    //private static final File OUTPUT_DIR = "/data/data/org.libhuagertp.app/55.mp4";

    // parameters for the encoder
    private static final String MIME_TYPE = "video/avc";    // H.264 Advanced Video Coding
    private static final int FRAME_RATE = 15;               // 15fps
    private static final int IFRAME_INTERVAL = 5;          // 10 seconds between I-frames

    public static Object Sync_waitpic;
    public static Object Sync_waitcamera;


    // size of a frame, in pixels
    public static int mWidth = -1;
    public static int mHeight = -1;
    // bit rate, in bits per second
    private int mBitRate = -1;
    static short aa = 0;

private long testtime=0;
    public static ArrayList<MyEncodeVInstan> mEncodeVideoInstan = new ArrayList<MyEncodeVInstan>();

    private int createTextureID() {
        int[] texture = new int[1];
        GLES20.glGenTextures(1, texture, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture[0]);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);
        return texture[0];
    }

    /**
     * Tests encoding of AVC video from a Surface.  The output is saved as an MP4 file.
     */
    public void loop() throws IOException {

        mEncodeVideoInstan.add(new MyEncodeVInstan(1920, 1080,1000000,20,5));
        //mEncodeVideoInstan.add(new MyEncodeVInstan(800, 500,200000,20,5));
        //mEncodeVideoInstan.add(new MyEncodeVInstan(300, 300,80000,20,5));
        Sync_waitpic = new Object();
        Sync_waitcamera = new Object();
        boolean Permis = false;

        // QVGA at 2Mbps
        mWidth = 1920;
        mHeight = 1080;
        mBitRate = 2000000;
        prepareEncoder(mEncodeVideoInstan.size());


        MySdl.NotifyReqPermission(Manifest.permission.CAMERA, MySdl.MY_PERMISSIONS_RECORD_CAMERA);
        Permis = MySdl.NotifyPermission(MySdl.MY_PERMISSIONS_RECORD_CAMERA);
        if (Permis == false) {
            //Log.i(TAG, "permission error");
            return;
        }
        mCamera2 = MyCamera.newInstance(mWidth, mHeight);
        int texture;


        texture = createTextureID();
        msurfaceTexture = new SurfaceTexture(texture);


        mDraw = new MyDraw(texture);

        mCamera2.msurfacetexture = msurfaceTexture;
        msurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                synchronized (MyEncodeVs.Sync_waitpic) {
                    MyEncodeVs.Sync_waitpic.notifyAll();
                }
                MySdl.needRender = true;
                // requestRender();
            }
        });

        /*暂时关闭*/
        mCamera2.openCamera();
        synchronized (MyEncodeVs.Sync_waitcamera) {
            try {
                MyEncodeVs.Sync_waitcamera.wait(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
                //Log.i(TAG, "openCamera error");
                return;
            }
        }
        Size size = mCamera2.getPreviewSize();
        GLES20.glViewport(0, 0, (size.getHeight()) / 3, (size.getWidth()) / 3);

        while (true) {
           // Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh111111111111111111111111111111111");
            // Feed any pending encoder output into the muxer.
            // Generate a new frame of input.
            // generateSurfaceFrame(i);
            long t0 = System.currentTimeMillis();
            synchronized (MyEncodeVs.Sync_waitpic) {//等待照片数据
              //  Log.i(TAG,"11111111111111111111111111111111");
                if (!MySdl.needRender) {
                    try {
                     //   Log.i(TAG,"222222222222222222222222222222222222222222");

                        MyEncodeVs.Sync_waitpic.wait(100);

                      //  Log.i(TAG,"3333333333333333333333333333333333333");
                        if (!MySdl.needRender) {
                            // TODO: if "spurious wakeup", continue while loop
                            //Log.i(TAG,"Camera frame wait timed out");
                            continue;
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                } else {

                }
                MySdl.needRender = false;
            }
           // Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh222222222222222222222222222222222222222");

            msurfaceTexture.updateTexImage();

          //  Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh333333333333333333333333333333333333333333333");
            for (int nn = 0; nn < mEncodeVideoInstan.size(); nn++) {
                //Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh333333333333333333333333333333333333333333333---0---------");
                mInputSurface.makeCurrent(mEncodeVideoInstan.get(nn).mEGLSurface);
               // Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh333333333333333333333333333333333333333333333---1");
                mDraw.draw();
              //  Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh333333333333333333333333333333333333333333333---2");
               // mInputSurface.setPresentationTime(computePresentationTimeNsec(i), mEncodeVideoInstan[nn].mEGLSurface);
                // Submit it to the encoder.  The eglSwapBuffers call will block if the input
                // is full, which would be bad if it stayed full until we dequeued an output
                // buffer (which we can't do, since we're stuck here).  So long as we fully drain
                // the encoder before supplying additional input, the system guarantees that we
                // can supply another frame without blocking.
               // Log.i(TAG, "sending frame " + i + " to encoder");
                mInputSurface.swapBuffers(mEncodeVideoInstan.get(nn).mEGLSurface);
               // Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh333333333333333333333333333333333333333333333---3");
            }
          //  Log.i(TAG,"hhhhhhhhhhhhhhhhhhhhhhhhhhhhh4444444444444444444444444444444444444444");
            long t1 = System.currentTimeMillis();
            int s0 = Integer.parseInt(String.valueOf(t1 - t0));


            outLoop();

        }

    }

    private void outLoop(){

        for (int nn = 0; nn < mEncodeVideoInstan.size(); nn++) {
            MediaCodec.BufferInfo bufferInfo=mEncodeVideoInstan.get(nn).mBufferInfo;
            MediaCodec encoder=mEncodeVideoInstan.get(nn).mMediaCodec;
            Object lock=mEncodeVideoInstan.get(nn).lock;
            Queue<Vframe> globeOut=mEncodeVideoInstan.get(nn).globeOut;
            int encoderStatus = encoder.dequeueOutputBuffer(bufferInfo, mEncodeVideoInstan.get(nn).TIMEOUT_USEC);
            if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // no output available yet

                //  if (VERBOSE) Log.d(TAG, "no output available, spinning to await EOS");

            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // not expected for an encoder

            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                // should happen before receiving buffers, and should only happen once
//                if (mMuxerStarted) {
//                    throw new RuntimeException("format changed twice");
//                }
                MediaFormat newFormat = encoder.getOutputFormat();
                Log.d(TAG, "encoder output format changed: " + newFormat);
                // now that we have the Magic Goodies, start the muxer
                //  mTrackIndex = mMuxer.addTrack(newFormat);
                //  mMuxer.start();
                //  mMuxerStarted = true;
            } else if (encoderStatus < 0) {
                Log.w(TAG, "unexpected result from encoder.dequeueOutputBuffer: " +
                        encoderStatus);
                // let's ignore it
            } else {


                ByteBuffer encodedData = encoder.getOutputBuffer(encoderStatus);

                if (encodedData == null) {
                    throw new RuntimeException("encoderOutputBuffer " + encoderStatus +
                            " was null");
                }


                if (mEncodeVideoInstan.get(nn).mBufferInfo.size != 0) {


                    if (encodedData.hasRemaining()) {
                        //  Log.i("www","999999999999990000000000>>> "+(encodedData.limit()-encodedData.position()));


                       /*ajltest*/
                        int length=encodedData.limit() - encodedData.position();
                        byte[] btmp = new byte[5];
                        encodedData.get(btmp);
                        /**/

/*ajlformal*/
                        mEncodeVideoInstan.get(nn).videoStartTime++;
                        //如果要相对精确对齐一下，可以算出来编码用的偏移量，相当于多少个pts，然后传过去的时候加上偏移量就行了，暂时只携带编码以后的pts
                        //以后可能会加上pts偏移量
                        int type=0;
                        if (btmp.length == 5) {
                            if (btmp[0] == 0 && btmp[1] == 0 && btmp[2] == 0 && btmp[3] == 1) {
                                int tt = (btmp[4] & 31);
//                                if (tt == 5) {
//                                    type = MediaCodec.BUFFER_FLAG_KEY_FRAME;
//                                } else
                                    if (tt == 7 || tt == 8) {
                                    type = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                                }
//                                    else if (tt == 1) {
//                                    type = MediaCodec.BUFFER_FLAG_PARTIAL_FRAME;
//                                }

                            } else if (btmp[0] == 0 && btmp[1] == 0 && btmp[2] == 1) {
                                //  Log.i("ddd", "eeeeeeeeeeeeeeeeeeeeee  " + (tmp[4] & 31));
                                int tt = (btmp[3] & 31);
//                                if (tt == 5) {
//                                    type = MediaCodec.BUFFER_FLAG_KEY_FRAME;
//                                } else
                                    if (tt == 7 || tt == 8) {
                                    type = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                                }
//                                    else if (tt == 1) {
//                                    type = MediaCodec.BUFFER_FLAG_PARTIAL_FRAME;
//                                }
                            }
                        }
                        if(type==MediaCodec.BUFFER_FLAG_CODEC_CONFIG){
                            ByteBuffer encodedDataAppend=ByteBuffer.allocateDirect(bufferInfo.size+1);
                            encodedData.position(0);
                            encodedDataAppend.put(encodedData);
                            encodedDataAppend.put((byte)25);
                            encodedDataAppend.flip();
                            MySdl.nativeSendVideEncode(encodedDataAppend,bufferInfo.size+1,mEncodeVideoInstan.get(nn).videoStartTime,nn);
                        }else{
                            MySdl.nativeSendVideEncode(encodedData,bufferInfo.size,mEncodeVideoInstan.get(nn).videoStartTime,nn);
                        }

/**/
                        /* ajltest*/
                     //   synchronized (lock) {
                    //        globeOut.add(btmp);
                    //   }
                        /**/
                        encodedData.position(bufferInfo.offset);
                        encodedData.limit(bufferInfo.offset + bufferInfo.size);

                    } else {

                    }
                    // mMuxer.writeSampleData(mTrackIndex, encodedData, mBufferInfo);
                    //  if (VERBOSE) Log.d(TAG, "sent " + mBufferInfo.size + " bytes to muxer");
                }

                encoder.releaseOutputBuffer(encoderStatus, false);
//if(aa>=221){
//    this.releaseEncoder();
//    Log.i("dd","okoooooooooooooooooooooo");
//}
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {

                    if (VERBOSE) Log.d(TAG, "end of stream reached");


                }
            }
        }
    }
    /**
     * Configures encoder and muxer state, and prepares the input Surface.
     */

    private void prepareEncoder(int num) throws IOException {
        mInputSurface = new CodecInputSurface();
        mInputSurface.eglSetup();
        for (int i = 0; i < num; i++) {
            mEncodeVideoInstan.get(i).mBufferInfo = new MediaCodec.BufferInfo();
//宽高从surface 0，0圆点截取像素
            MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE, mEncodeVideoInstan.get(i).height, mEncodeVideoInstan.get(i).width);

            // Set some properties.  Failing to specify some of these can cause the MediaCodec
            // configure() call to throw an unhelpful exception.
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                    MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
            format.setInteger(MediaFormat.KEY_BIT_RATE, mBitRate);//mBitRate = 2000000;
            format.setInteger(MediaFormat.KEY_FRAME_RATE, FRAME_RATE);//FRAME_RATE = 15;
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, IFRAME_INTERVAL);//IFRAME_INTERVAL = 5;

            if (VERBOSE) Log.d(TAG, "format: " + format);

            // Create a MediaCodec encoder, and configure it with our format.  Get a Surface
            // we can use for input and wrap it with a class that handles the EGL work.
            //
            // If you want to have two EGL contexts -- one for display, one for recording --
            // you will likely want to defer instantiation of CodecInputSurface until after the
            // "display" EGL context is created, then modify the eglCreateContext call to
            // take eglGetCurrentContext() as the share_context argument.

            mEncodeVideoInstan.get(i).mMediaCodec = MediaCodec.createEncoderByType(MIME_TYPE);
            mEncodeVideoInstan.get(i).mMediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mEncodeVideoInstan.get(i).mEGLSurface = mInputSurface.eglSurface(mEncodeVideoInstan.get(i).mMediaCodec.createInputSurface());
            try {
                mEncodeVideoInstan.get(i).mMediaCodec.start();
            } catch (RuntimeException e) {
                e.printStackTrace();
            }


            mEncodeVideoInstan.get(i).index = i;
            //mEncodeVideoInstan[i].mMediaCoOut.start();
        }
        mInputSurface.makeCurrent(mEncodeVideoInstan.get(0).mEGLSurface);
        // Output filename.  Ideally this would use Context.getFilesDir() rather than a
        // hard-coded output directory.
        //String outputPath = "/data/data/org.libhuagertp.app/55.mp4";
        // Log.d(TAG, "output file is " + outputPath);


        // Create a MediaMuxer.  We can't add the video track and start() the muxer here,
        // because our MediaFormat doesn't have the Magic Goodies.  These can only be
        // obtained from the encoder after it has started processing data.
        //
        // We're not actually interested in multiplexing audio.  We just want to convert
        // the raw H.264 elementary stream we get from MediaCodec into a .mp4 file.
        // try {
        //     mMuxer = new MediaMuxer(outputPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
        //  } catch (IOException ioe) {
        //      throw new RuntimeException("MediaMuxer creation failed", ioe);
        //   }

        //   mTrackIndex = -1;
        //    mMuxerStarted = false;
    }

    /**
     * Releases encoder resources.  May be called after partial / failed initialization.
     */
    private void releaseEncoder() {
        if (VERBOSE) Log.d(TAG, "releasing encoder objects");
        for (int i = 0; i < mEncodeVideoInstan.size(); i++) {
            if (mEncodeVideoInstan.get(i).mMediaCodec != null) {
                mEncodeVideoInstan.get(i).mMediaCodec.stop();
                mEncodeVideoInstan.get(i).mMediaCodec.release();
                mEncodeVideoInstan.get(i).mMediaCodec = null;
                EGL14.eglDestroySurface(CodecInputSurface.mEGLDisplay, mEncodeVideoInstan.get(i).mEGLSurface);
            }
        }

        //   if (mMuxer != null) {
        //         mMuxer.stop();
        //       mMuxer.release();
        //       mMuxer = null;
        //   }
    }


    /**
     * Generates the presentation time for frame N, in nanoseconds.
     */
    private static long computePresentationTimeNsec(int frameIndex) {
        final long ONE_BILLION = 1000000000;
        return frameIndex * ONE_BILLION / FRAME_RATE;
    }

    /**
     * Holds state associated with a Surface used for MediaCodec encoder input.
     * <p>
     * The constructor takes a Surface obtained from MediaCodec.createInputSurface(), and uses that
     * to create an EGL window surface.  Calls to eglSwapBuffers() cause a frame of data to be sent
     * to the video encoder.
     * <p>
     * This object owns the Surface -- releasing this will release the Surface too.
     */
    public static class CodecInputSurface {
        private static final int EGL_RECORDABLE_ANDROID = 0x3142;

        public static EGLDisplay mEGLDisplay = EGL14.EGL_NO_DISPLAY;
        public static EGLContext mEGLContext = EGL14.EGL_NO_CONTEXT;
        public static EGLConfig mEGLConfig;


        /**
         * Creates a CodecInputSurface from a Surface.
         */
        public CodecInputSurface() {

        }

        /**
         * Prepares EGL.  We want a GLES 2.0 context and a surface that supports recording.
         */
        public void eglSetup() {
            mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
            if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
                throw new RuntimeException("unable to get EGL14 display");
            }
            int[] version = new int[2];
            if (!EGL14.eglInitialize(mEGLDisplay, version, 0, version, 1)) {
                throw new RuntimeException("unable to initialize EGL14");
            }

            // Configure EGL for recording and OpenGL ES 2.0.
            int[] attribList = {
                    EGL14.EGL_RED_SIZE, 8,
                    EGL14.EGL_GREEN_SIZE, 8,
                    EGL14.EGL_BLUE_SIZE, 8,
                    EGL14.EGL_ALPHA_SIZE, 8,
                    EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                    EGL_RECORDABLE_ANDROID, 1,
                    EGL14.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] numConfigs = new int[1];
            EGL14.eglChooseConfig(mEGLDisplay, attribList, 0, configs, 0, configs.length,
                    numConfigs, 0);
            checkEglError("eglCreateContext RGB888+recordable ES2");

            // Configure context for OpenGL ES 2.0.
            int[] attrib_list = {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL14.EGL_NONE
            };
            mEGLConfig = configs[0];
            mEGLContext = EGL14.eglCreateContext(mEGLDisplay, mEGLConfig, EGL14.EGL_NO_CONTEXT,
                    attrib_list, 0);
            checkEglError("eglCreateContext");

            // Create a window surface, and attach it to the Surface we received.

        }

        public EGLSurface eglSurface(Surface ms) {
            int[] surfaceAttribs = {
                    EGL14.EGL_NONE
            };
            EGLSurface egls = EGL14.eglCreateWindowSurface(mEGLDisplay, mEGLConfig, ms,
                    surfaceAttribs, 0);
            checkEglError("eglCreateWindowSurface");
            return egls;
        }

        /**
         * Discards all resources held by this class, notably the EGL context.  Also releases the
         * Surface that was passed to our constructor.
         */
        public void release() {
            if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
                EGL14.eglMakeCurrent(mEGLDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE,
                        EGL14.EGL_NO_CONTEXT);

                EGL14.eglDestroyContext(mEGLDisplay, mEGLContext);
                EGL14.eglReleaseThread();
                EGL14.eglTerminate(mEGLDisplay);
            }

            mEGLDisplay = EGL14.EGL_NO_DISPLAY;
            mEGLContext = EGL14.EGL_NO_CONTEXT;

        }

        /**
         * Makes our EGL context and surface current.
         */
        public void makeCurrent(EGLSurface egls) {
            EGL14.eglMakeCurrent(mEGLDisplay, egls, egls, mEGLContext);
            checkEglError("eglMakeCurrent");
        }

        /**
         * Calls eglSwapBuffers.  Use this to "publish" the current frame.
         */
        public boolean swapBuffers(EGLSurface egls) {
            boolean result = EGL14.eglSwapBuffers(mEGLDisplay, egls);
            checkEglError("eglSwapBuffers");
            return result;
        }

        /**
         * Sends the presentation time stamp to EGL.  Time is expressed in nanoseconds.
         */
        public void setPresentationTime(long nsecs, EGLSurface egls) {
            EGLExt.eglPresentationTimeANDROID(mEGLDisplay, egls, nsecs);
            checkEglError("eglPresentationTimeANDROID");
        }

        /**
         * Checks for EGL errors.  Throws an exception if one is found.
         */
        private void checkEglError(String msg) {
            int error;
            if ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
                throw new RuntimeException(msg + ": EGL error: 0x" + Integer.toHexString(error));
            }
        }
    }
}