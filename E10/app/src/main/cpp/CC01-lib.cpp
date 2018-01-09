//
// Created by Administrator on 2017/12/13 0013.
//

#include <jni.h>
#include <string>

#include <malloc.h>
#include <sys/time.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

#define LOGD_A(...)  __android_log_print(ANDROID_LOG_DEBUG,"---j11",__VA_ARGS__)
#define LOGD_B(...)  __android_log_print(ANDROID_LOG_DEBUG,"---j22",__VA_ARGS__)

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型

extern "C"
{
int g_val2_threshold = 125;
//int g_resize_numerator = 5;
double g_resize_for_720 = 0.5;

double g_scale_for_rect = 0.5;
int g_width_for_scale = 0;

bool g_is_resize = true;

int g_debug_type = 0;

enum img_resolution_type{
    IMG_resolution_1280_720 = 0,
    IMG_resolution_1920_1080
};
enum img_resolution_type g_resolution_type = IMG_resolution_1280_720;
const char local_template_dirs[][8] = {
        "720", "1080"
};

#define POKER_SUIT_NUM_MAX 6
#define POKER_DATA_NUM_MAX 13
Mat img_poker_mid_suit_tp[6];   // s-spades黑桃 h-hearts红桃 d-diamonds方块 c-clubs梅花  大小王统称joker
Mat img_poker_mid_num_tp[13];    //A,2,3,...Q,K
Mat img_poker_btt_suit_tp[6];
Mat img_poker_btt_num_tp[13];

#define RECT_RC_NUM_MAX 8
struct poker_rect_info{
    int btt_num;
    int mid_num;
    Rect btt_arr[RECT_RC_NUM_MAX];
    Rect mid_arr[RECT_RC_NUM_MAX];
}g_pk_rect_info;

struct poker_rc_size_info{
    int w_max;                   //img_pk_rlt_2v_num
    int w_fix;
};

const struct poker_rc_size_info g_pk_rc_size_btt = {
        32,
        28
};
const struct poker_rc_size_info g_pk_rc_size_mid = {
        34,
        30
};

const char suits_word_arr[][8] = {
        "方块", "梅花", "红桃", "黑桃"
};

enum img_use_type{
    IMG_USE_2V = 0,
    IMG_USE_GRAY,
    IMG_USE_COLOR
};

#define POKER_MATCH_NONE 0

#define POKER_NUM_MAX 7
#define POKER_MID_NUM_MAX 5
#define POKER_BTT_NUM_MAX 2
struct poker_target_2val_info{
    int img_num;                   //img_pk_rlt_2v_num
    Mat img_arr[POKER_MID_NUM_MAX];    //img_pk_rlt_2v_arr
    unsigned char rlt_idx_suit[POKER_MID_NUM_MAX];
    unsigned char rlt_idx_data[POKER_MID_NUM_MAX];
};

struct poker_target_2val_info g_pk_tg_mid_info;
struct poker_target_2val_info g_pk_tg_btt_info;


void tp_mat_print()
{
    int i=0;
    Mat *img_ptr;

    img_ptr = &img_poker_mid_suit_tp[0];
    for(i=0; i<4; i++){
        LOGD_A("########## s. wh: %d, %d", img_ptr[i].cols, img_ptr[i].rows);
    }
    img_ptr = &img_poker_mid_num_tp[12];
    LOGD_A("########## end num. wh: %d, %d", img_ptr->cols, img_ptr->rows);

    img_ptr = &img_poker_btt_suit_tp[0];
    for(i=0; i<4; i++){
        LOGD_A("########## d. wh: %d, %d", img_ptr[i].cols, img_ptr[i].rows);
    }
    img_ptr = &img_poker_btt_num_tp[12];
    LOGD_A("########## end num. wh: %d, %d", img_ptr->cols, img_ptr->rows);
}

int img_tp_read2buf(char *buffer, const int buf_len, char *filename, AAssetManager *mgr)
{
    //打开文件
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);
    if(NULL == asset)
        return -1;
    long len = AAsset_getLength(asset);//获取文件长度
    if(len > buf_len){
        AAsset_close(asset);
        return -1;
    }
    AAsset_read(asset,buffer,len);
    AAsset_close(asset);
    return len;
}


bool read_tp2Mat(const char *dir_ptr, AAssetManager *mgr)
{
    char* buffer;
    const int buff_len = 2048;
    int rd_len;
    int i = 0;
    Mat *img_ptr;
    char filename[64];

    buffer = (char*) malloc(buff_len);
    buffer[buff_len-1]='\0';

    img_ptr = &img_poker_mid_suit_tp[0];
    for(i=0; i<4; i++){
        snprintf(filename, sizeof(filename), "%s/up_s_%d.png", dir_ptr, i);
        rd_len = img_tp_read2buf(buffer, buff_len, filename, mgr);
        if(rd_len < 0)
            goto rlt_end_back;
        std::vector<uchar> data(buffer, buffer+rd_len);
        img_ptr[i] = cv::imdecode(data, IMREAD_UNCHANGED);
    }

    img_ptr = &img_poker_mid_num_tp[0];
    for(i=1; i<=13; i++){
        snprintf(filename, sizeof(filename), "%s/up_d_%d.png", dir_ptr, i);
        rd_len = img_tp_read2buf(buffer, buff_len, filename, mgr);
        if(rd_len < 0)
            goto rlt_end_back;
        std::vector<uchar> data(buffer, buffer+rd_len);
        img_ptr[i-1] = cv::imdecode(data, IMREAD_UNCHANGED);
    }

    //bottom
    img_ptr = &img_poker_btt_suit_tp[0];
    for(i=0; i<4; i++){
        snprintf(filename, sizeof(filename), "%s/bt_s_%d.png", dir_ptr, i);
        rd_len = img_tp_read2buf(buffer, buff_len, filename, mgr);
        if(rd_len < 0)
            goto rlt_end_back;
        std::vector<uchar> data(buffer, buffer+rd_len);
        img_ptr[i] = cv::imdecode(data, IMREAD_UNCHANGED);
    }
    img_ptr = &img_poker_btt_num_tp[0];
    for(i=1; i<=13; i++){
        snprintf(filename, sizeof(filename), "%s/bt_d_%d.png", dir_ptr, i);
        rd_len = img_tp_read2buf(buffer, buff_len, filename, mgr);
        if(rd_len < 0)
            goto rlt_end_back;
        std::vector<uchar> data(buffer, buffer+rd_len);
        img_ptr[i-1] = cv::imdecode(data, IMREAD_UNCHANGED);
    }

rlt_end_back:
    free(buffer);
    if(rd_len < 0)
        return false;
    else
        return true;
}

JNIEXPORT jboolean JNICALL
Java_com_yxsj_yz_e10_CC01_templateInit(JNIEnv  *env, jclass clazz, jobject assetManager, int resolutionHeight)   //int files_num,
{
    const char *dir_ptr = NULL;

    if(720 == resolutionHeight){
        g_resolution_type = IMG_resolution_1280_720;
        dir_ptr = local_template_dirs[0];
    }
    else if(1080 == resolutionHeight) {
        g_resolution_type = IMG_resolution_1920_1080;
        dir_ptr = local_template_dirs[1];
    }
    else
        return false;

    //创建一个AssetManager对象
    AAssetManager *mgr = AAssetManager_fromJava(env,assetManager);


    if(!read_tp2Mat(dir_ptr, mgr)){
        return false;
    }
    else{
        tp_mat_print();
    }

    return true;
}
JNIEXPORT void JNICALL
Java_com_yxsj_yz_e10_CC01_debugSet(JNIEnv  *env, jclass clazz, int dbg_type)   //int files_num,
{
    g_debug_type = dbg_type;
}

void poker_target_rlt_reset()
{
    struct poker_target_2val_info *pk_info = &g_pk_tg_mid_info;

    pk_info->img_num = 0;
    memset(pk_info->rlt_idx_suit, 0xF, sizeof(pk_info->rlt_idx_suit));
    memset(pk_info->rlt_idx_data, 0xF, sizeof(pk_info->rlt_idx_data));

    pk_info = &g_pk_tg_btt_info;
    pk_info->img_num = 0;
    memset(pk_info->rlt_idx_suit, 0xF, sizeof(pk_info->rlt_idx_suit));
    memset(pk_info->rlt_idx_data, 0xF, sizeof(pk_info->rlt_idx_data));
}

void poker_target_rlt_print()
{
    int i;
    int idx;
    struct poker_target_2val_info *pk_info = &g_pk_tg_mid_info;
    for(i=0; i<pk_info->img_num; i++){
        idx = pk_info->rlt_idx_suit[i];
        if(idx < 4)
            LOGD_A("---m %s %d\n", suits_word_arr[idx], pk_info->rlt_idx_data[i]);
        else
            LOGD_A("---m NO MATCH...%d\n", idx);
    }
    pk_info = &g_pk_tg_btt_info;
    for(i=0; i<pk_info->img_num; i++){
        idx = pk_info->rlt_idx_suit[i];
        if(idx < 4)
            LOGD_A("---b %s %d\n", suits_word_arr[idx], pk_info->rlt_idx_data[i]);
        else
            LOGD_A("---b NO MATCH...%d\n", idx);
    }
}
#define RLT_POKER_NUM_IDX 1
void poker_target_rlt_set(int rlt_arr[16])
{
    int i;
    int j = RLT_POKER_NUM_IDX+1;

    struct poker_target_2val_info *pk_info = &g_pk_tg_btt_info;
    if(pk_info->img_num != 2){
        rlt_arr[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
        return;
    }
    for(i=0; i<pk_info->img_num; i++){
        rlt_arr[j++] = pk_info->rlt_idx_suit[i]+1;
        rlt_arr[j++] = pk_info->rlt_idx_data[i];
    }

    pk_info = &g_pk_tg_mid_info;
    if(pk_info->img_num > 5){
        rlt_arr[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
        return;
    }
    for(i=0; i<pk_info->img_num; i++){
        rlt_arr[j++] = pk_info->rlt_idx_suit[i]+1;
        rlt_arr[j++] = pk_info->rlt_idx_data[i];
    }
    rlt_arr[RLT_POKER_NUM_IDX] = 2+pk_info->img_num;
}

void _width_scale_check(int width)
{
    if(width != g_width_for_scale){
        if(width > 720){
            g_scale_for_rect = 720*g_resize_for_720/width;
        }
        else{
            g_scale_for_rect = g_resize_for_720;
        }
        g_width_for_scale = width;
    }
}
#define MAX_SIDE_USE 320
void _pic_mat_resize(const Mat &img_src, Mat &img_dst)
{
    _width_scale_check(img_src.cols);
    double fScale = g_scale_for_rect;      //缩放倍数

    int width = img_src.cols;
    int height = img_src.rows;
    width = width*fScale;
    height = height*fScale;
    if(height < (MAX_SIDE_USE) && width < (MAX_SIDE_USE)){
        return;
    }
    resize(img_src, img_dst, Size(width, height));
}

void _pic_2gray_8UC1(const Mat &img_src, Mat &img_dst)
{
    double scal = 255.0/0xffff;

    if(img_src.channels() == 3)
        cvtColor(img_src, img_dst, CV_BGR2GRAY); // 转为灰度图像
    else
        cvtColor(img_src, img_dst, CV_BGRA2GRAY);

    if(img_dst.depth() > CV_8S){
        if(img_dst.depth() > CV_16S)
            scal = 255.0/0xffffffff;
        img_dst.convertTo(img_dst, CV_8UC1, scal, 0);
    }
}

bool _pic_2value(Mat &image)
{
    Mat result;
    //创建同原始图像等大小的图像空间

    _pic_2gray_8UC1(image, image);

    //二值化
    //CV_THRESH_OTSU参数自动生成阈值，跟第三个参数也就没有关系了。
    threshold(image, result, g_val2_threshold, 255,  CV_THRESH_BINARY);
    //获取处理后的结果
    if(!result.data){
        return false;
    }

    image = result;

    return true;
}


#define PIC_PARALLEL_MIN       3
#define PIC_MID_WIDTH_MIN       32
#define PIC_MID_HEIGHT_MIN      48
#define PIC_BUTT_WIDTH_MIN      25
#define PIC_BUTT_HEIGHT_MIN     45
int parallel_ts = PIC_PARALLEL_MIN;
int pic_width_min = PIC_MID_WIDTH_MIN;
int pic_height_min = PIC_MID_HEIGHT_MIN;
int pic_epsilon_num = 4;

void _rect_filter_01(vector<vector<Point> > &contours_src, vector<vector<Point> > &contours_dst, Mat &img)  //vector<vector<Point> > &contours_dst
{
    //parallel_ts = 6;

    vector<Point> points;
    bool is_rlt;
    for(unsigned int i=0;i<contours_src.size();i++){
        approxPolyDP(contours_src.at(i), points, pic_epsilon_num, true);

        if(points.size() == 4){
            is_rlt = false;
            if(abs(((Point)points.at(0)).x - ((Point)points.at(1)).x) < parallel_ts){
                if((abs(((Point)points.at(0)).x - ((Point)points.at(2)).x) > pic_width_min)
                   &&(abs(((Point)points.at(0)).y - ((Point)points.at(1)).y) > pic_height_min)){
                    is_rlt = true;
                }
            }
            else if(abs(((Point)points.at(0)).x - ((Point)points.at(2)).x) < parallel_ts){
                if((abs(((Point)points.at(0)).x - ((Point)points.at(2)).x) > pic_width_min)
                   &&(abs(((Point)points.at(0)).y - ((Point)points.at(2)).y) > pic_height_min)){
                    is_rlt = true;
                }
            }


            if(is_rlt){
                contours_dst.push_back(contours_src.at(i));
                drawContours(img,contours_src,i,Scalar(0),1);
            }

            //qDebug() << "pts num:" << ((vector<Point>)contours_src.at(i)).size() << is_rlt;
        }
    }
}

bool pic_find_rectangle(Mat &img_org, Mat &img_us, vector<vector<Point> > &contours_rlt)
{
    if(g_is_resize){
        _pic_mat_resize(img_org, img_us);
        if(g_debug_type)
            LOGD_B("wh:%d,%d\n", img_us.cols, img_us.rows);
    }
    else{
        img_us = img_org.clone();
    }
    if(!_pic_2value(img_us)){
        std::cout << "failed! 2valu deal" <<std::endl;
        return false;
    }
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(img_us,contours,hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    //cout << "contours: " << contours.size() << endl;

    Mat contoursRt(img_us.rows,img_us.cols,CV_8U,Scalar(255));
    _rect_filter_01(contours, contours_rlt, contoursRt);
    if(g_debug_type)
        LOGD_B("ct_rect:%d\n", contours_rlt.size());
    return true;
}

void _rect_revise(Rect &rt_data, const Mat &img)
{
    //printf("b1 x,y: %d,%d. w,h: %d,%d. %d,%d\n", rt_data.x, rt_data.y, rt_data.width, rt_data.height, img.cols, img.rows);
    if(rt_data.x + rt_data.width > img.cols)
        rt_data.width = img.cols - rt_data.x;
    if(rt_data.y + rt_data.height > img.rows)
        rt_data.height = img.rows - rt_data.y;
    //printf("b2 x,y: %d,%d. w,h: %d,%d\n", rt_data.x, rt_data.y, rt_data.width, rt_data.height);
}

/*
rt_org: contours所在图相对img_src的比例（0.0~1）
*/
void _pic_get_part_rect(const Mat &img_src, vector<vector<Point> > &contours, vector<Rect> &rects, double rt_org, double rt_w, double rt_h)
{
    int num = contours.size();
    bool is_ratio = true;
    Rect rt_info;

    if(rt_org > 1 || rt_org<=0.001)  //0.999
        return;
    if(rt_w >= 0.999 || rt_w <= 0.001)
        is_ratio = false;
    else if(rt_h >= 0.999 || rt_h <= 0.001)
        is_ratio = false;
    cout << "ratio " << rt_org << endl;
    for(int i=0;i<num;i++){
        rt_info = boundingRect(contours[i]);
        if(is_ratio){
            rt_info.width = rt_info.width * rt_w - 2;
            rt_info.height = rt_info.height * rt_h - 2;
            rt_info.x++;
            rt_info.y++;
        }
        //printf("a1 x,y: %d,%d. w,h: %d,%d\n", rt_info.x, rt_info.y, rt_info.width, rt_info.height);
        rt_info.x = rt_info.x/rt_org;
        rt_info.y = rt_info.y/rt_org;
        rt_info.width = rt_info.width/rt_org;
        rt_info.height = rt_info.height/rt_org;
        _rect_revise(rt_info, img_src);

        //printf("x,y: %d-%d. w,h: %d,%d\n", rt_info.x, rt_info.y, rt_info.width, rt_info.height);
        rects.push_back(rt_info);
    }
}


/*
从彩图中取目标图中的左上角，并转为二值图放到img_dst、及g_pk_tg_info的img_arr
img_src：原彩图
img_dst：用于显示结果的。CV_8U类型
*/
void _pic_cut_part(const Mat &img_src, Mat &img_dst, vector<Rect> &rects, enum img_use_type u_type , bool is_save)
{
    int num = rects.size();
    char filename[48];
    Mat roi_tmp;
    Rect rt_info;
    struct poker_target_2val_info *img_ptr;

    poker_target_rlt_reset();
    if(num > POKER_NUM_MAX)
        num = POKER_NUM_MAX;

    for(int i=0;i<num;i++){
        rt_info = rects[i];
        roi_tmp = img_src(rt_info).clone();

        if(IMG_USE_GRAY == u_type)
            cvtColor(roi_tmp, roi_tmp, CV_BGR2GRAY);
        else if(IMG_USE_COLOR == u_type){
            cout << "color reserve";
            return;
        }
        else
            _pic_2value(roi_tmp);

        //
        roi_tmp.copyTo(img_dst(rt_info));
        if(rt_info.y < img_src.rows/2){
            img_ptr = &g_pk_tg_mid_info;
            if(img_ptr->img_num > POKER_MID_NUM_MAX)
                continue;
            img_ptr->img_arr[img_ptr->img_num] = roi_tmp;
            img_ptr->img_num++;
        }
        else{
            img_ptr = &g_pk_tg_btt_info;
            if(img_ptr->img_num > POKER_BTT_NUM_MAX)
                continue;
            img_ptr->img_arr[img_ptr->img_num] = roi_tmp;
            img_ptr->img_num++;
        }
        if(0==i){
            cv::circle(img_dst,
                       cv::Point(rt_info.x + rt_info.width/2, rt_info.y + rt_info.height/2),
                       20, cv::Scalar(0),
                       2, 8, 0);
        }

        if(is_save){
            snprintf(filename, sizeof(filename), "pic_%d.png", i);
            cout << "file is" << filename << endl;
            imwrite(filename, roi_tmp);
        }
    }
}

void _rect_ratio_chg(const Mat &img_src, Rect &rt_info, int add)
{
    double rt_org = g_scale_for_rect;
    double rt_w = 0.4;
    double rt_h = 0.6;

    rt_info.width = rt_info.width * rt_w - 2;
    rt_info.height = rt_info.height * rt_h - 2;
    rt_info.x++;
    rt_info.y++;
    //qDebug("a1 x,y: %d,%d. w,h: %d,%d\n", rt_info.x, rt_info.y, rt_info.width, rt_info.height);
    rt_info.x = rt_info.x/rt_org;
    rt_info.y = rt_info.y/rt_org;
    rt_info.width = rt_info.width/rt_org;
    if(0.5 != g_scale_for_rect){
        rt_info.height = rt_info.height/rt_org + add;
        //qDebug() << "add.." << add;
    }
    else
        rt_info.height = rt_info.height/rt_org;
    _rect_revise(rt_info, img_src);
}
void _record_poker(const Mat &img_src, Mat &img_dst, bool is_btt)
{
    const struct poker_rc_size_info *p_size = &g_pk_rc_size_mid;
    int w = img_src.cols;
    int h = img_src.rows;
    Mat img_tmp;
    if(is_btt)
        p_size = &g_pk_rc_size_btt;
    if(w > p_size->w_max){
        h = h * p_size->w_fix/w;
        w = p_size->w_fix;
        resize(img_src, img_tmp, Size(w,h));
    }
    else
        img_tmp = img_src;
    _pic_2gray_8UC1(img_tmp, img_dst);
}

void _remove_out_btt()
{
    struct poker_rect_info *p_rts = &g_pk_rect_info;
    Rect rt_tmp = p_rts->btt_arr[0];

    int i=1;
    for(; i<p_rts->btt_num; i++){
        if(abs(rt_tmp.y - p_rts->btt_arr[i].y) < 4)
            break;
    }
    if(i==1)
        return;
    if(i>=p_rts->btt_num){
        p_rts->btt_arr[0] = p_rts->btt_arr[2];
    }
}
void _remove_out_mid(int width)
{
    struct poker_rect_info *p_rts = &g_pk_rect_info;

    for(int i=0; i<p_rts->mid_num; i++){
        if(p_rts->mid_arr[i].x - p_rts->mid_arr[i].width < 0)
            p_rts->mid_arr[i].width = 0;
        else if(p_rts->mid_arr[i].x + p_rts->mid_arr[i].width*3 > width)
            p_rts->mid_arr[i].width = 0;
    }
}

void _pic_get_pk_rect(const Mat &img_line, vector<vector<Point> > &contours)
{
    bool is_ok = false;
    int num_btt = 0;
    int num_mid = 0;
    int w_mid = img_line.cols/2;
    int h_mid = img_line.rows/2;
    struct poker_rect_info *p_rts = &g_pk_rect_info;

    Rect rt_tmp;
    vector<vector<Point>>::iterator it;
    for(it=contours.begin();it!=contours.end();++it)
    {
        rt_tmp = boundingRect(*it);

        if(rt_tmp.y < img_line.rows*3/4)        //mid
        {
            if(rt_tmp.y < h_mid && (rt_tmp.y+rt_tmp.height) > h_mid){       //abs((rt_tmp.y+rt_tmp.height/2) - h_mid) < 10
                if(num_mid < RECT_RC_NUM_MAX-1)
                    p_rts->mid_arr[num_mid++] = rt_tmp;
            }
            continue;
        }
        //btt
        if(rt_tmp.x >= w_mid){
            if((rt_tmp.x - rt_tmp.width/6) < w_mid)
                is_ok = true;
        }
        else{
            if((rt_tmp.x + rt_tmp.width + rt_tmp.width/6) > w_mid)
                is_ok = true;
        }
        if(is_ok){
            is_ok = false;
            if(num_btt < RECT_RC_NUM_MAX-1)
                p_rts->btt_arr[num_btt++] = rt_tmp;
        }
    }
    p_rts->btt_num = num_btt;
    p_rts->mid_num = num_mid;

    if(num_btt > 2){
        LOGD_B("!! btt num big: %d\n", num_btt);
        _remove_out_btt();
    }
    if(num_mid > 5){
        LOGD_B("!! mid num big: %d\n", num_mid);
        _remove_out_mid(img_line.cols);
    }
    if(g_debug_type)
        LOGD_B("num btt: %d, mid: %d\n", num_btt, num_mid);
}

int _pic_rec_btt_pk(const Mat &img_src)
{
    struct poker_target_2val_info *tg_info_ptr = &g_pk_tg_btt_info;
    struct poker_rect_info *p_rts = &g_pk_rect_info;
    if(p_rts->btt_num < 2){
        return -1;
    }

    _rect_ratio_chg(img_src, p_rts->btt_arr[0], 2);
    _record_poker(img_src(p_rts->btt_arr[0]), tg_info_ptr->img_arr[0], true);
    _rect_ratio_chg(img_src, p_rts->btt_arr[1], 2);
    _record_poker(img_src(p_rts->btt_arr[1]), tg_info_ptr->img_arr[1], true);
    tg_info_ptr->img_num = 2;
    return 0;
}

int _pic_rec_mid_pk(const Mat &img_src)
{
    struct poker_target_2val_info *tg_info_ptr = &g_pk_tg_mid_info;
    struct poker_rect_info *p_rts = &g_pk_rect_info;
    if(p_rts->mid_num < 3){
        return -1;
    }

    int j = 0;
    for(int i=0; i<p_rts->mid_num; i++){
        if(p_rts->mid_arr[i].width>0){
            _rect_ratio_chg(img_src, p_rts->mid_arr[i], 0);
            _record_poker(img_src(p_rts->mid_arr[i]), tg_info_ptr->img_arr[j++], false);
        }
    }
    tg_info_ptr->img_num = j;
    return 0;
}

double pic_do_tp_match(cv::Mat image, cv::Mat tepl, cv::Point &point, int method)
{
    int result_cols =  image.cols - tepl.cols + 1;
    int result_rows = image.rows - tepl.rows + 1;
    if(result_cols<=0 || result_rows<=0){
        printf("err dw:%d, dh:%d\n", result_cols, result_rows);
        return -1;
    }
    if(image.type() != tepl.type()){
        printf("err type %d-%d\n", image.type(), tepl.type());
        return -2;
    }

    cv::Mat result = cv::Mat( result_cols, result_rows, CV_32FC1 );   //
    cv::matchTemplate( image, tepl, result, method );

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    switch(method)
    {
        case CV_TM_SQDIFF:
        case CV_TM_SQDIFF_NORMED:
            point = minLoc;
            return minVal;
            break;
        case CV_TM_CCORR:
        case CV_TM_CCOEFF:
        case CV_TM_CCORR_NORMED:
        case CV_TM_CCOEFF_NORMED:
        default:
            point = maxLoc;
            return maxVal;
            break;
    }
}

int _tp_match_one(bool is_mid)
{
    int i;
    int j;
    int num_tp;
    int method = CV_TM_SQDIFF_NORMED;
    struct poker_target_2val_info *pk_info = &g_pk_tg_btt_info;
    Mat *img_tp_suit = &img_poker_btt_suit_tp[0];
    Mat *img_tp_num = &img_poker_btt_num_tp[0];
    double value;
    double min_value;
    int min_id;
    cv::Point matchLoc;

    if(is_mid){
        pk_info = &g_pk_tg_mid_info;
        img_tp_suit = &img_poker_mid_suit_tp[0];
        img_tp_num = &img_poker_mid_num_tp[0];
    }

    for(i=0; i<pk_info->img_num; i++){
        /* suit */
        num_tp = 6;
        min_value = 100.0;
        min_id = -1;
        for(j=0; j<num_tp; j++){
            if(img_tp_suit[j].rows<10)
                break;

            value = pic_do_tp_match(pk_info->img_arr[i], img_tp_suit[j], matchLoc, method);
            if(value < 0.02){   //method <= CV_TM_SQDIFF_NORMED &&
                //set suit
                min_value = value;
                min_id = j;
                break;
            }
            if(value < min_value){
                min_value = value;
                min_id = j;
            }
        }
        if(min_id >= 0 && min_value < 0.18){
            pk_info->rlt_idx_suit[i] = min_id;
            if(min_value < 0.02)
                ;//qDebug() << "suit ok." << i << "id" << min_id << value;
        }
        else{
            min_id = pk_info->rlt_idx_suit[i];
            if(g_debug_type)
                LOGD_B("suit none XX. %d, id:%d, val:%f\n", i, min_id, min_value);
            return -1;
        }
        /* num */
        if(min_id > 3)
            continue;  //大小王等
        num_tp = 13;  //
        min_value = 100.0;
        min_id = -1;
        for(j=0; j<num_tp; j++){
            if(img_tp_num[j].rows<10)
                break;

            value = pic_do_tp_match(pk_info->img_arr[i], img_tp_num[j], matchLoc, method);
            if(method <= CV_TM_SQDIFF_NORMED && value < 0.09){
                //set num
                min_value = value;
                min_id = j;
                break;
            }
            if(value < min_value){
                min_value = value;
                min_id = j;
            }
        }
        if(min_id >= 0 && min_value < 0.22){
            pk_info->rlt_idx_data[i] = min_id+1;
            if(min_value < 0.09){
                ;//qDebug() << "num ok." << i << "id" << min_id << min_value;
            }
        }
        else{
            if(g_debug_type)
                LOGD_B("num none XX. %d, id:%d, val:%f\n", i, min_id, min_value);
            return -2;
        }
    }
    return 0;
}

int _pic_match_suit()  //, bool is_save
{
    //mid
    if(_tp_match_one(true)<0)
        return -1;
    //btt
    if(_tp_match_one(false)<0)
        return -2;

    return 0;
}

void pic_matching_deal_a(Mat &img_org, int rlt_arr[15])
{
    Mat img_us;
    vector<vector<Point> > contours_rlt;

    if(!pic_find_rectangle(img_org, img_us, contours_rlt))
        return;

    Mat rltImage(img_org.rows,img_org.cols,CV_8U,Scalar(255));
    vector<Rect> rects;
    _pic_get_part_rect(img_org, contours_rlt, rects,
                         g_scale_for_rect,
                         0.4,
                         0.6);
    _pic_cut_part(img_org, rltImage, rects,
                    (enum img_use_type)1,
                    false);


    _pic_match_suit();
    poker_target_rlt_print();
    poker_target_rlt_set(rlt_arr);
}
int pic_matching_deal(Mat &img_org, int rlt_arr[16])
{
    Mat img_us;
    vector<vector<Point> > contours_rlt;

    if(!pic_find_rectangle(img_org, img_us, contours_rlt))
        return -1;

    poker_target_rlt_reset();
    _pic_get_pk_rect(img_us, contours_rlt);
    if(_pic_rec_btt_pk(img_org)<0){
        LOGD_A("!!! no btt\n");
        return -2;
    }
    _pic_rec_mid_pk(img_org);

    if(_pic_match_suit()<0)
        return -3;
    poker_target_rlt_print();
    poker_target_rlt_set(rlt_arr);
    return 0;
}

#define RLT_BACK_NUM_MAX 18
struct timeval tvafter,tvpre;
int rlt_info[RLT_BACK_NUM_MAX];
cv::Mat g_img_cv;

void pic_match_to_rlt(JNIEnv *env, Mat &imgData, jintArray &result)
{
    memset(rlt_info, 0, sizeof(rlt_info));

    if(!imgData.empty()){
        if(pic_matching_deal(imgData, rlt_info)<0)
            rlt_info[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
    }
    else{
        rlt_info[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
    }
    gettimeofday (&tvafter , NULL);

    int idx = RLT_POKER_NUM_IDX+1+2*7;
    rlt_info[idx] = (tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000;
    if(g_debug_type)
        LOGD_A("花费时间:%d \n", rlt_info[idx]);
    env->SetIntArrayRegion(result, 0, RLT_BACK_NUM_MAX, rlt_info);
}

void pic_match_to_rlt_b(Mat &imgData)
{
    memset(rlt_info, 0, sizeof(rlt_info));

    if(!imgData.empty()){
        if(pic_matching_deal(imgData, rlt_info)<0)
            rlt_info[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
    }
    else{
        rlt_info[RLT_POKER_NUM_IDX] = POKER_MATCH_NONE;
    }
    gettimeofday (&tvafter , NULL);

    int idx = RLT_POKER_NUM_IDX+1+2*7;
    rlt_info[idx] = (tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000;
    if(g_debug_type)
        LOGD_A("花费时间:%d \n", rlt_info[idx]);
}

JNIEXPORT jintArray JNICALL
Java_com_yxsj_yz_e10_CC01_picMatch(JNIEnv *env, jobject instance,
                                  jintArray buf, jint width, jint height)
{
    jint *cbuf;
    jboolean ptfalse = false;
    gettimeofday (&tvpre , NULL);
    cbuf = env->GetIntArrayElements(buf, &ptfalse);
    if (cbuf == NULL) {
        return 0;
    }

    Mat imgData(height, width, CV_8UC4, (unsigned char *) cbuf);
    g_img_cv = imgData;
    if(imgData.empty())
        return 0;

    pic_match_to_rlt_b(g_img_cv);

    env->ReleaseIntArrayElements(buf, cbuf, 0);
    if(rlt_info[RLT_POKER_NUM_IDX]<=0)
        return 0;
    jintArray result = env->NewIntArray(RLT_BACK_NUM_MAX);
    env->SetIntArrayRegion(result, 0, RLT_BACK_NUM_MAX, rlt_info);
    return result;
}

int g_image_type = -1;

int read_2_mat(unsigned char *cbuf, jlong buf_len)
{
    std::vector<uchar> data(cbuf, cbuf+buf_len);
    g_img_cv = cv::imdecode(data, IMREAD_UNCHANGED);
    if(g_img_cv.empty()){
        LOGD_A("img none a. len:%d\n", (int)buf_len);
        return -1;
    }
    g_image_type = g_img_cv.type();
    return 1;
}
int get_buff_to_mat(unsigned char *cbuf, jlong buf_len, int width, int height)
{
    if(buf_len < 0)
        return -1;
    if(g_image_type < CV_8U || width < 0){
        if(read_2_mat(cbuf, buf_len)>0) {
            LOGD_A("img type: %d\n", g_image_type);
            return 1;
        } else
            return -2;
    }
    if(g_image_type >= CV_8U){
        Mat img(height, width, g_image_type, cbuf);
        g_img_cv = img;
        if(img.empty() || width != img.cols || height != img.rows){
            LOGD_A("img ...\n");
            if(read_2_mat(cbuf, buf_len)>0) {
                LOGD_A("img type: %d\n", g_image_type);
                return 1;
            } else
                return -3;
        }
        return 1;
    }
    return -4;
}

JNIEXPORT jintArray JNICALL
Java_com_yxsj_yz_e10_CC01_picMatchBuff(JNIEnv *env, jobject instance,
                                   jobject pixels, jint width, jint height)
{
    unsigned char *cbuf;
    gettimeofday (&tvpre , NULL);
    jclass cls = env->GetObjectClass(instance);
    jfieldID fid = env->GetFieldID(cls, "pixels","Ljava/nio/ByteBuffer;");
    if(NULL == fid){
        LOGD_A("!!! fid null\n");
        return 0;
    }
    jobject bar = env->GetObjectField(instance, fid);
    cbuf = (unsigned char *)env->GetDirectBufferAddress(pixels);
    //unsigned char *cbuf_2 = (unsigned char *)env->GetDirectBufferAddress(bar);
    jlong buf_len = env->GetDirectBufferCapacity(pixels);

    if(NULL == cbuf){
        LOGD_A("!!! buf get null\n");
        return 0;
    }
    int rlt = get_buff_to_mat(cbuf, buf_len, width, height);
    if(rlt < 0){
        LOGD_A("!!! buf to mat: %d\n", rlt);
        return 0;
    }

    jintArray result = env->NewIntArray(sizeof(rlt_info));
    pic_match_to_rlt(env, g_img_cv, result);

    return result;
}

JNIEXPORT jintArray JNICALL
Java_com_yxsj_yz_e10_CC01_picMatchByte(JNIEnv *env, jobject instance,
                                     jbyteArray pixels, jint width, jint height, jlong dataLen)
{
    gettimeofday (&tvpre , NULL);
    jboolean ptfalse = false;
    jbyte* bbuf = (jbyte *)env->GetByteArrayElements(pixels, &ptfalse);
    unsigned char *cbuf = (unsigned char *)bbuf;

    if(NULL == cbuf){
        LOGD_A("!!! buf get null\n");
        return 0;
    }
    int rlt = get_buff_to_mat(cbuf, dataLen, width, height);
    if(rlt < 0){
        LOGD_A("!!! buf to mat: %d\n", rlt);
        return 0;
    }

    jintArray result = env->NewIntArray(RLT_BACK_NUM_MAX);
    pic_match_to_rlt(env, g_img_cv, result);

    env->ReleaseByteArrayElements(pixels, (jbyte *)cbuf, 0);
    return result;
}

}