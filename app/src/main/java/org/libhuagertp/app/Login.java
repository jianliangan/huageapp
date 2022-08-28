package org.libhuagertp.app;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import androidx.fragment.app.FragmentActivity;

public class Login extends FragmentActivity {
    private EditText et_roomid;
    private EditText et_uname;
    private EditText et_userid;
    private EditText et_server;
    private EditText et_port;
    private CheckBox cb_isCheck;
    private commandHandler handler;
    private static Login mSingleton;

    public static Login GetContext() {
        return mSingleton;
    }

    public void isLogin(View v) {
        MySdl.nativeLogin(getArguments());
    }

    protected String[] getArguments() {
        return new String[]{et_roomid.getText().toString().trim(), et_userid.getText().toString().trim(), et_server.getText().toString(), et_port.getText().toString(),et_uname.getText().toString()};
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        mSingleton = this;
        setContentView(R.layout.activity_main);
        MySdl.ApiLoadDll();
        MySdl.ApiRunMainTh();
        handler = new commandHandler();
        /*[1]找到控件*/
        et_roomid = (EditText) findViewById(R.id.et_roomid);
        et_userid = (EditText) findViewById(R.id.et_userid);
        cb_isCheck = (CheckBox) findViewById(R.id.cb_isCheck);
        et_server = (EditText) findViewById(R.id.et_server);
        et_port = (EditText) findViewById(R.id.et_port);
        et_uname=(EditText) findViewById(R.id.et_uname);
        /*[1.1]获取登陆名和密码*/
        //Map<String,String> map = saveInfoUtils.readInfo();
        //Map<String,String> map = saveInfoUtils.readInfo(MainActivity.this);
        //if(map !=null){
        //    String name = map.get("name");
        //    String password = map.get("password");
        et_roomid.setText("210");
        et_userid.setText("12345");
        et_server.setText("192.168.22.158");
        et_uname.setText("安松");
        et_port.setText("89");
        // }


        //return;

    }


    public void onLogined() {
        /*[2.1]获取用户名和密码*/


        System.out.println("登陆成功");
        Intent intent = new Intent(Login.this, TESTActivity.class);
        //将text框中的值传入

        //为了接受SecondActivity中的值，不用startAcitivity(intent)
        startActivity(intent);

        //两个动画之间的动画描述
        //overridePendingTransition(0, 0);
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
                    //Log.i("dd", "aaaa");
                    Login.GetContext().onLogined();
                    break;

                default:
                    ;
            }
        }
    }
}
