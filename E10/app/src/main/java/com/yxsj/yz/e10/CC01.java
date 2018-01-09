package com.yxsj.yz.e10;

import android.content.Context;
import android.content.res.AssetManager;

import java.nio.ByteBuffer;

/**
 * Created by Administrator on 2017/12/13 0013.
 */

public class CC01 {
    static {
        System.loadLibrary("cc01-lib");
    }

    /**
     *  软件启动时的初始
     *  成功返回true;失败返回false。
     */
    public boolean useAssetsInit(Context context) {
        try {
            AssetManager mAssetManger = context.getAssets();
//            String fileNames[] = mAssetManger.list("");
//            Log.i("a01", "list num: "+String.valueOf(fileNames.length));
            return templateInit(mAssetManger, 720);//
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    /**
     *  图像匹配
     */
    public int[] picMatchDeal(int[] pixels, int w, int h)
    {
        return picMatch(pixels, w, h);
    }
    public int[] picMatchDeal(ByteBuffer pixels, int w, int h)
    {
        return picMatchBuff(pixels, w, h);
    }
    public int[] picMatchDeal(byte[] pixels, int w, int h, long dataLen)
    {
        return picMatchByte(pixels, w, h, dataLen);
    }
    /**
     * 匹配结果格式化到StringBuilder
     * rlt_arr为picMatchDeal接口返回的结果
     * tm_pre为调用picMatchDeal前System.currentTimeMillis()的值
    * */
    public void pic_rlt_to_show(int[] rlt_arr, StringBuilder rlt_show, long tm_pre)
    {
        long tMsNow =  System.currentTimeMillis();
        rlt_show.delete(0, rlt_show.length());
        if(null == rlt_arr)
            return;
        int rlt_poker_num_idx = 1;
        int pk_num = rlt_arr[rlt_poker_num_idx];
        int idx_max_btt = rlt_poker_num_idx + 4;
        int idx_max_mid = rlt_poker_num_idx + pk_num*2;
        if(pk_num>=2 || pk_num > 7){
            rlt_show.append("底牌：");
            for (int i = rlt_poker_num_idx+1; i <idx_max_btt; i+=2) {
                str_name_set(rlt_arr, i, rlt_show);
            }
            rlt_show.append("\n池中：");
            for (int i = idx_max_btt+1; i < idx_max_mid; i+=2) {
                str_name_set(rlt_arr, i, rlt_show);
            }
            rlt_show.append("\n时间1：");
            rlt_show.append(rlt_arr[rlt_poker_num_idx +1+2*7]);

            rlt_show.append(" 时间2：");
            rlt_show.append(tMsNow-tm_pre);
        }
        else{
            rlt_show.append("未识别！");
        }
    }

    //初始C接口。
    public static native boolean templateInit(AssetManager assetManager, int resolutionHeight);  //String[] fileList  String file_name, int files_num,
    //图像匹配C接口
    public static native int[] picMatch(int[] pixels, int width, int height);
    public static native int[] picMatchBuff(ByteBuffer pixels, int width, int height);
    public static native int[] picMatchByte(byte[] pixels, int width, int height, long len);

    public static native void debugSet(int dbg_type);   //dbg_type非0时开启某些打印

    private static final String[] suit_name = {"方块", "梅花", "红桃", "黑桃", "null"};
    private static final String[] data_name = {"A,", "2,", "3,", "4,", "5,", "6,", "7,", "8,", "9,", "10,", "J,", "Q,", "K,", "-,"};

    private void str_name_set(int[] rlt_arr, int i,  StringBuilder rlt_show)
    {
        if (rlt_arr[i] <= 4) {
            rlt_show.append(suit_name[rlt_arr[i]-1]);
            if(rlt_arr[i+1]<=13)
                rlt_show.append(data_name[rlt_arr[i+1]-1]);
            else
                rlt_show.append(data_name[13]);
        } else {
            rlt_show.append(suit_name[4]);
            if(rlt_arr[i+1]<=13)
                rlt_show.append(data_name[rlt_arr[i+1]-1]);
            else
                rlt_show.append(data_name[13]);
        }
    }

}
