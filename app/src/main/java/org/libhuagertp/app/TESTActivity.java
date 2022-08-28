//package org.libhuagertp.app;
package org.libhuagertp.app;


import android.Manifest;
import android.app.Activity;
import android.content.*;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.view.*;
import android.widget.FrameLayout;
import android.os.*;
import android.util.Log;
import android.widget.Toast;


import androidx.core.app.ActivityCompat;
import androidx.fragment.app.FragmentActivity;

public class TESTActivity extends FragmentActivity implements ActivityCompat.OnRequestPermissionsResultCallback {
    private static final String TAG = "TEST";
    OrientationEventListener mOrientationListener;

    // Main components
    private static TESTActivity mSingleton;
    private commandHandler handler;
    public static MytestView msurfaceView;
    protected static ViewGroup mLayout;

    // String[] preactive;

    public Handler commandHandler;
    static boolean islogined;
 public static TESTActivity GetContext(){
    return mSingleton;
}
    public boolean dispatchTouchEvent(MotionEvent ev) {
        //mGLView.onTouchEvent(ev); //The openGL view
        return super.dispatchTouchEvent(ev);

    }

    private void MySdlStart() {

        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String fpb = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        String sr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);

        MySdl.ApiMediaStart();

        msurfaceView = new MytestView(getApplication());
        mLayout = (FrameLayout) findViewById(R.id.mainLayout);
        mLayout.addView(msurfaceView);


        // mglSurface = new MyglSurface(getApplication());
        // msurfaceView = new MytestView(getApplication());
        //mLayout = (FrameLayout) findViewById(R.id.mainLayout);
        // msurfaceView.setBackgroundColor(Color.RED);
        //  mLayout.addView(mglSurface);
        //  mLayout.addView(msurfaceView);
    }
    private int CreateEnterRoom(int roomid) {
        return MySdl.nativeCreateEnterRoom(roomid);
    }


    public void OpenRoom(View v) {

        if(CreateEnterRoom(210)==-1){
            Toast.makeText(TESTActivity.this, "创建失败，可能是没有登录", Toast.LENGTH_SHORT).show();
            return;
        }

        Toast.makeText(TESTActivity.this, "进入房间", Toast.LENGTH_SHORT).show();
    }
    public void OpenCamera(View v) {

        MySdlStart();
        Toast.makeText(TESTActivity.this, "打开相机", Toast.LENGTH_SHORT).show();
    }

    // Setup
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        islogined = true;

        super.onCreate(savedInstanceState);
        Tools.copyAssetDirToFiles(getBaseContext(), "");
        setContentView(R.layout.fragment_camera2_basic);
        handler = new commandHandler();
        Intent intent = getIntent();
        Bundle bundle = intent.getExtras();
        //preactive = bundle.getStringArray("args");
        //
    }

    // MySdlActInter interface

    public void requestPermission(String permission, int requestCode) {
        if (Build.VERSION.SDK_INT < 23) {
            MySdl.ApiPermission(requestCode);
            return;
        }
        if (TESTActivity.mSingleton.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
            // Activity ac=this;
            //requestPermissions(new String[]{permission}, requestCode);
            ActivityCompat.requestPermissions(this, new String[]{permission}, requestCode);
        } else {
            MySdl.ApiPermission(requestCode);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSingleton = this;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case MySdl.MY_PERMISSIONS_RECORD_AUDIO: {
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay!
                    ///recordAudio();
                    MySdl.ApiPermission(MySdl.MY_PERMISSIONS_RECORD_AUDIO);
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(this, "Permissions Denied to record audio", Toast.LENGTH_LONG).show();
                }
                return;
            }
            case MySdl.MY_PERMISSIONS_RECORD_CAMERA: {
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay!
                    ///recordAudio();
                    MySdl.ApiPermission(MySdl.MY_PERMISSIONS_RECORD_CAMERA);
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(this, "Permissions Denied to record camera", Toast.LENGTH_LONG).show();
                }
                return;
            }

        }
    }

    // MySdlActInter interface

    public boolean SendMessage(int command, Object data) {
        Message msg = handler.obtainMessage();
        msg.arg1 = command;
        msg.obj = data;
        boolean result = handler.sendMessage(msg);
        return result;
    }

    //handle message
    public static class commandHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.arg1) {
                case MySdl.COMMAND_LOGINED:

                    break;

                default:
                    ;
            }
        }
    }

}



