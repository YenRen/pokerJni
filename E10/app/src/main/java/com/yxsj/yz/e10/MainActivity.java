package com.yxsj.yz.e10;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public CC01 img_deal = new CC01();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        Button GiveFaceButton = (Button) findViewById(R.id.GiveFaceButton);  //灰度图
        GiveFaceButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                Intent myIntent = new Intent();

                //开启picture画面Type设置为image*
                myIntent.setType("image/*");

                //使用Intent.ACTION_GET_CONTENT这个Action
                myIntent.setAction(Intent.ACTION_GET_CONTENT);

                /*取得相片后返回本画面*/
                startActivityForResult(myIntent, 1);
            }
        });
        Button GetFaceButton = (Button) findViewById(R.id.GetFaceButton);   //原图
        GetFaceButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                Intent myIntent = new Intent();

                //开启picture画面Type设置为image*
                myIntent.setType("image/*");

                //使用Intent.ACTION_GET_CONTENT这个Action
                myIntent.setAction(Intent.ACTION_GET_CONTENT);

                /*取得相片后返回本画面*/
                startActivityForResult(myIntent, 2);
            }
        });

        CC01.debugSet(1);
        img_deal.useAssetsInit(this);    //getApplicationContext()
    }

    public static native int[] getGray(int[] pixels, int width, int height);

    private DataPicInfo img_rlt = new DataPicInfo();

    public static native int[] getRlt(DataPicInfo cobj, int[] pixels, int width, int height);

    /**
     * int[]到byte[]
     */
    public static byte[] intArrayToByteArray(int[] i_arr, int iLen)
    {
        int i=0, j=0;
        byte[] result = new byte[iLen*4];
        for(; i<iLen; i++){
            //由高位到低位
            result[j] = (byte)((i_arr[i] >> 24) & 0xFF);
            result[j+1] = (byte)((i_arr[i] >> 16) & 0xFF);
            result[j+2] = (byte)((i_arr[i] >> 8) & 0xFF);
            result[j+3] = (byte)(i_arr[i] & 0xFF);
            j+=4;
        }
        return result;
    }
//            throws IOException
    public static final byte[] input2byte(InputStream inStream)
    {
        ByteArrayOutputStream swapStream = new ByteArrayOutputStream();
        byte[] buff = new byte[100];
        int rc = 0;
        try {
            while ((rc = inStream.read(buff, 0, 100)) > 0) {
                swapStream.write(buff, 0, rc);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] in2b = swapStream.toByteArray();
        return in2b;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case 1:
                Uri uri = data.getData();
                Log.e("uri", uri.toString());
                ContentResolver cr = this.getContentResolver();

                try {
                    InputStream input = cr.openInputStream(uri);
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    Bitmap bitmap = BitmapFactory.decodeStream(input, null, options);

                    int w = bitmap.getWidth();
                    int h = bitmap.getHeight();
                    int[] pix = new int[w * h];
                    bitmap.getPixels(pix, 0, w, 0, 0, w, h);

                    long tMsPre =  System.currentTimeMillis();
                    int[] resultPixels;
                    if(true){
                        resultPixels = img_deal.picMatchDeal(pix, w, h);
                        pic_match_rlt_show(resultPixels, tMsPre);
                    }
                    else{
                        int w_pre = w;
                        int h_pre = h;
                        int bytes = bitmap.getByteCount();
                        ByteBuffer buf = ByteBuffer.allocate(bytes);
                        bitmap.copyPixelsToBuffer(buf);

                        byte[] byteArray;
                        if(false){
                            byteArray = buf.array();
                        }
                        else {
                            byteArray = input2byte(cr.openInputStream(uri));
                            w = -1;
                            h = -1;
                        }

                        if(false)
                            resultPixels = img_deal.picMatchDeal(buf, w, h);
                        else
                            resultPixels = img_deal.picMatchDeal(byteArray, w, h, byteArray.length);

                        pic_match_rlt_show(resultPixels, tMsPre);
                        w = w_pre;
                        h = h_pre;
                    }

                    /*以下进行灰度化处理*/
                    if (false) {
                        resultPixels = getGray(pix, w, h);
                        //int[] resultPixels = CC01.picDeal(pix, w, h);
                    } else {
                        //img_rlt.buf = getRlt(img_rlt, pix, w, h);
                        //resultPixels = img_rlt.buf;
                        resultPixels = getRlt(img_rlt, pix, w, h);
                        w = img_rlt.width;
                        h = img_rlt.height;
                    }
                    Bitmap result = Bitmap.createBitmap(w, h, Bitmap.Config.RGB_565);
                    result.setPixels(resultPixels, 0, w, 0, 0, w, h);

//                    w = result.getWidth();
//                    h = result.getHeight();
                    ImageView imageView = (ImageView) findViewById(R.id.IV01);
                    /*将处理好的灰度图设定到ImageView*/
                    imageView.setImageBitmap(result);
                } catch (FileNotFoundException e) {
                    Log.e("Exception", e.getMessage(), e);
                }
                break;
            case 2:
                Uri uri2 = data.getData();
                Log.e("uri2", uri2.toString());
                ContentResolver cr2 = this.getContentResolver();

                try {
                    InputStream input = cr2.openInputStream(uri2);
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    options.inJustDecodeBounds = true;
                    BitmapFactory.decodeStream(input, null, options);
                    if (options.outWidth > 450 || options.outHeight > 450) {
                        options.inSampleSize = Math.max(options.outWidth / 450, options.outHeight / 450);
                    }
                    options.inJustDecodeBounds = false;
                    Bitmap bitmap = BitmapFactory.decodeStream(cr2.openInputStream(uri2), null, options);

                    int w = bitmap.getWidth();
                    int h = bitmap.getHeight();
                    ImageView imageView = (ImageView) findViewById(R.id.IV02);

                    /*将Bitmap设定到ImageView*/
                    imageView.setImageBitmap(bitmap);
                } catch (FileNotFoundException e) {
                    Log.e("Exception", e.getMessage(), e);
                }
                break;
            default:
                break;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }
    protected void pic_match_rlt_show(int[] rlt_arr, long tm_pre) {
        StringBuilder rlt_show = new StringBuilder();

        img_deal.pic_rlt_to_show(rlt_arr, rlt_show, tm_pre);
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(rlt_show.toString());

//        ByteBuffer buffer = ByteBuffer.allocate(16);
//        byte[] b_arr =  buffer.array();
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
