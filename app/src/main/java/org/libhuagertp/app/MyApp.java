package org.libhuagertp.app;

import android.app.Application;

public class MyApp extends Application {

    @Override
    public void onCreate() {
        CrashLogCatch.initCrashLog(this);   //注意这里
        super.onCreate();
    }
}