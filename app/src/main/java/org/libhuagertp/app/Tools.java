package org.libhuagertp.app;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.res.AssetManager;
public class Tools {

    public static void copyAssetDirToFiles(Context context, String dirname)
             {
        String directoryPath="";
        directoryPath=context.getFilesDir()+"";
        File file = new File(directoryPath);
        if(!file.exists()){//判断文件目录是否存在
            file.mkdirs();
        }
        AssetManager assetManager = context.getAssets();
                 String[] children= {""};
                 try {
                     children= assetManager.list(dirname);


        for (String child : children) {
            //child =  child;
            String[] grandChildren = assetManager.list(child);
            if (0 == grandChildren.length)
                copyAssetFileToFiles(context, child);
            else
                copyAssetDirToFiles(context, child);
        }
                 } catch (Exception e)

                 {}
    }

    public static void copyAssetFileToFiles(Context context, String filename) {
        try {
            InputStream is = context.getAssets().open(filename);
            byte[] buffer = new byte[is.available()];
            is.read(buffer);
            is.close();

            File of = new File(context.getFilesDir() + "/" + filename);
            of.createNewFile();
            FileOutputStream os = new FileOutputStream(of);
            os.write(buffer);
            os.close();
        } catch (Exception e) {
        }
    }

}
