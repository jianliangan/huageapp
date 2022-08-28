package org.libhuagertp.app;


import android.content.Context;



public class CrashLogCatch {
    public static final String THREAD_NAME_MAIN = "com.example.ABC";   //主线程名称
    public static final String THREAD_NAME_REMOTE = "com.example.ABC:remote_service";

    public static void initCrashLog(final Context context) {
        final Thread.UncaughtExceptionHandler oriHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            public void uncaughtException(Thread thread, Throwable e) {
                try {
                    System.out.println("ddddddddddd Exception message:" + e);
                } catch (Exception ex) {}
            }
        });
    }

    /**
     * 获取当前进程名
     */


}
