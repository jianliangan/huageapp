package org.libhuagertp.app;

import android.animation.TimeAnimator;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.Image;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.Queue;

import android.util.Log;
import android.view.Surface;
class MyThread2 extends Thread {
private MyDecodeVs myde;
    public MyThread2(MyDecodeVs myde) {

        this.myde = myde;

    }

    public void run() {

        MyDecodeVs.threadrun(myde);

    }
}
public class MyDecodeVs {
    public Surface s;
    public int framerate;
  //  private TimeAnimator mTimeAnimator = new TimeAnimator();

    protected void initDecodec(Surface s)  {


       // mTimeAnimator.setTimeListener(new TimeAnimator.TimeListener() {
        //    @Override
        //    public void onTimeUpdate(final TimeAnimator animation,
       //                              final long totalTime,
        ///                             final long deltaTime) {
        this.s=s;
        MyThread2 t = new MyThread2(this);
        t.start();

     //       }
      //  });

        // We're all set. Kick off the animator to process buffers and render video frames as
        // they become available
      //  mTimeAnimator.start();


    }
public static void threadrun(MyDecodeVs myde){
    MediaCodec videoDecoder;
    MediaFormat format = MediaFormat.createVideoFormat("video/avc", 0, 0);

    // Set some properties.  Failing to specify some of these can cause the MediaCodec
    // configure() call to throw an unhelpful exception.
    // format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
    //         MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
    //format.setInteger(MediaFormat.KEY_BIT_RATE, 2000000);
    //format.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
    //format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 10);

    try {
        //mmd = myMediaDecode.fromVideoFormat(format, getHolder().getSurface());
        videoDecoder = MediaCodec.createDecoderByType("video/avc");
        //getHolder().getSurface()
        videoDecoder.configure(format, myde.s, null, 0);
        videoDecoder.start();


        ///init
    } catch (IOException e) {
        e.printStackTrace();
        return;
    }
    Queue<Vframe> globeOuttmp=new LinkedList<Vframe>();
    for(;;) {

        if (MyEncodeVs.mEncodeVideoInstan.get(0) == null)
            continue;
        Vframe vframe;
        globeOuttmp.clear();
        synchronized (MyEncodeVs.mEncodeVideoInstan.get(0).lock) {
            try {
                MyEncodeVs.mEncodeVideoInstan.get(0).lock.wait();
            } catch (InterruptedException i) {
                i.printStackTrace();
                return;
            }
            for(;;) {
                vframe = MyEncodeVs.mEncodeVideoInstan.get(0).globeOut.poll();
                if (vframe == null || vframe.getlen() <= 4) {
                    break;
                }
                globeOuttmp.add(vframe);
            }
        }
        while(globeOuttmp.size()>0) {
            vframe = globeOuttmp.poll();

            if (vframe != null && vframe.getlen() > 4) {
                try {
/*
enum nal_unit_type_e
{
    NAL_UNKNOWN     = 0,
    NAL_SLICE       = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,
                            NAL_SEI         = 6,
                                    NAL_SPS         = 7,
                                    NAL_PPS         = 8,
                                    NAL_AUD         = 9,
                                    NAL_FILLER      = 12,

                        };
 */
                    // Log.i("ddd", "eeeeaaae33eeeeeeeeeeeeeeeeee  poll " + tmp.length);
                    int type = 0;
                    if (vframe.data[0] == 0 && vframe.data[1] == 0 && vframe.data[2] == 0 && vframe.data[3] == 1) {
                        //    Log.i("ddd", "eeeeeeeeeeeeeeeeeeeeee  " + (vframe.data[5] & 31));
                        int tt = (vframe.data[4] & 31);
                        if (tt == 5) {
                            type = MediaCodec.BUFFER_FLAG_KEY_FRAME;
                        } else if (tt == 7 || tt == 8) {
                            type = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                        } else if (tt == 1) {
                            type = MediaCodec.BUFFER_FLAG_PARTIAL_FRAME;
                        }

                    } else if (vframe.data[0] == 0 && vframe.data[1] == 0 && vframe.data[2] == 1) {
                        //  Log.i("ddd", "eeeeeeeeeeeeeeeeeeeeee  " + (vframe.data[4] & 31));
                        int tt = (vframe.data[3] & 31);
                        if (tt == 5) {
                            type = MediaCodec.BUFFER_FLAG_KEY_FRAME;
                        } else if (tt == 7 || tt == 8) {
                            type = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
                        } else if (tt == 1) {
                            type = MediaCodec.BUFFER_FLAG_PARTIAL_FRAME;
                        }
                    }


                    if (videoDecoder == null) {
                        continue;
                    }

                    if (type == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {

                        myde.framerate = (int) vframe.data[vframe.getlen() - 1];
                        Log.i("ddd", "eeeeaaae33eeeeeeeeeeeeeeeeee  " + (int) vframe.data[vframe.getlen() - 1]);
                    }
                    int index;
                    while ((index = videoDecoder.dequeueInputBuffer(20)) >= 0) {
                        break;
                    }
                    if (index < 0) {
                        continue;
                    }
                    ByteBuffer buffer = videoDecoder.getInputBuffer(index);
                    buffer.clear();
                    buffer.put(vframe.data);

                    int presenttime = 0;
                    if (myde.framerate != 0) {
                        presenttime = vframe.pts * 1000000 / myde.framerate;
                    }
                    videoDecoder.queueInputBuffer(index, 0, buffer.position(), presenttime, type);
                    Log.i("ddd", "eeeeaaae33eeeeeeeeeeeeeeeeee  " + presenttime);
                    // mmd.writeSample(vframe.data, null, totalTime, type);
                } catch (MediaCodec.CryptoException e) {
                    e.printStackTrace();

                }
            }else{
                break;
            }
        }
        if (videoDecoder == null) {
            continue;
        }
        int index;
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

        index = videoDecoder.dequeueOutputBuffer(info, 0);

        switch (index) {
            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:

                break;

            case MediaCodec.INFO_TRY_AGAIN_LATER:
                break;
            default:
                break;
        }
        if (index < 0) {
            continue;
        }

        videoDecoder.releaseOutputBuffer(index, true);

    }
}
    private static byte[] YUV_420_888toNV21(Image image) {
        int width = image.getWidth();
        int height = image.getHeight();
        int ySize = (width) * height;
        int uvSize = width * height / 4;

        byte[] nv21 = new byte[ySize + uvSize * 2];

        ByteBuffer yBuffer = image.getPlanes()[0].getBuffer(); // Y
        ByteBuffer uBuffer = image.getPlanes()[1].getBuffer(); // U
        ByteBuffer vBuffer = image.getPlanes()[2].getBuffer(); // V
        byte bb = 123;

        int rowStride = image.getPlanes()[0].getRowStride();
        assert (image.getPlanes()[0].getPixelStride() == 1);

        int pos = 0;

        if (rowStride == width) { // likely
            yBuffer.get(nv21, 0, ySize);
            pos += ySize;
        } else {
            long yBufferPos = -rowStride; // not an actual position
            for (; pos < ySize; pos += width) {
                yBufferPos += rowStride;
                yBuffer.position((int) yBufferPos);
                yBuffer.get(nv21, pos, width);
            }
        }

        rowStride = image.getPlanes()[2].getRowStride();
        int pixelStride = image.getPlanes()[2].getPixelStride();

        assert (rowStride == image.getPlanes()[1].getRowStride());
        assert (pixelStride == image.getPlanes()[1].getPixelStride());


        // other optimizations could check if (pixelStride == 1) or (pixelStride == 2),
        // but performance gain would be less significant

        for (int row = 0; row < height / 2; row++) {
            for (int col = 0; col < width / 2; col++) {
                int vuPos = col * pixelStride + row * rowStride;
                nv21[pos++] = vBuffer.get(vuPos);
                nv21[pos++] = uBuffer.get(vuPos);
            }
        }

        return nv21;
    }
}
