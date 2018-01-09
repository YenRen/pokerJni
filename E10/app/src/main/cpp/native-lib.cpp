#include <jni.h>
#include <string>

//#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

extern "C"
{
JNIEXPORT jintArray JNICALL
Java_com_yxsj_yz_e10_MainActivity_getGray(JNIEnv *env, jobject instance,
                                          jintArray buf, jint w, jint h);

JNIEXPORT jintArray  JNICALL
Java_com_yxsj_yz_e10_MainActivity_getRlt(JNIEnv *env, jobject instance, jobject classobj,
                                         jintArray buf, jint width, jint height);

JNIEXPORT jstring JNICALL
Java_com_yxsj_yz_e10_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello C";
    return env->NewStringUTF(hello.c_str());
}


JNIEXPORT jintArray JNICALL
Java_com_yxsj_yz_e10_MainActivity_getGray(JNIEnv *env, jobject instance,
                                          jintArray buf, jint width, jint height) {

    jint *cbuf;
    jboolean ptfalse = false;
    cbuf = env->GetIntArrayElements(buf, &ptfalse);
    if (cbuf == NULL) {
        return 0;
    }

    Mat imgData(height, width, CV_8UC4, (unsigned char *) cbuf);

    uchar *ptr = imgData.ptr(0);
    for (int i = 0; i < width * height; i++) {
        uchar grayScale = (uchar) (ptr[4 * i + 2] * 0.299 + ptr[4 * i + 1] * 0.587 +
                                   ptr[4 * i + 0] * 0.114);
        ptr[4 * i + 1] = grayScale;
        ptr[4 * i + 2] = grayScale;
        ptr[4 * i + 0] = grayScale;
    }

    int size = width * height;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}

#define MAX_SIDE_USE 640
#if 0
void reduce_img_size_1(unsigned char *cbuf, IplImage **pDstImage, int width, int height)
{
    //IplImage *pSrcImage = cvLoadImage(cbuf, CV_LOAD_IMAGE_UNCHANGED);
    IplImage *pSrcImage = cvDecodeImage(cbuf,CV_LOAD_IMAGE_COLOR);

    double fScale = 0.5;      //缩放倍数
    CvSize czSize;              //目标图像尺寸

    if(width > height && width > MAX_SIDE_USE){
        width = MAX_SIDE_USE;
        height = MAX_SIDE_USE*height/width;
        fScale = MAX_SIDE_USE/width;
        czSize.width = width;
    } else if(height>width && height > MAX_SIDE_USE){
        height = MAX_SIDE_USE;
        width = MAX_SIDE_USE*width/height;
        fScale = MAX_SIDE_USE/height;
        czSize.height = height;
    } else{
        *pDstImage = pSrcImage;
        return;
    }
    *pDstImage = cvCreateImage(czSize, pSrcImage->depth, pSrcImage->nChannels);
    cvResize(pSrcImage, pDstImage, CV_INTER_AREA);
    free(pSrcImage);
}
#endif
void reduce_img_size(Mat &imgDataSrc, Mat &imgDataDst, int width, int height)
{
    double fScale = 0.5;      //缩放倍数
    //CvSize czSize;              //目标图像尺寸
#if 0
    imgDataDst = imgDataSrc;
    return;
#else
    if(width > height && width > MAX_SIDE_USE){
        fScale = (double)MAX_SIDE_USE/width;
//        height = MAX_SIDE_USE*height/width;
//        width = MAX_SIDE_USE;
    } else if(height>width && height > MAX_SIDE_USE){
        fScale = (double)MAX_SIDE_USE/height;
//        width = MAX_SIDE_USE*width/height;
//        height = MAX_SIDE_USE;
    } else{
        imgDataDst = imgDataSrc;
        return;
    }
    width = width*fScale;
    height = height*fScale;
    //Size dsize = Size(imgDataSrc.cols*fScale, imgDataSrc.rows*fScale);
    Size dsize(width, height);
    resize(imgDataSrc, imgDataDst, dsize);
#endif
}

//jobject
JNIEXPORT jintArray  JNICALL
Java_com_yxsj_yz_e10_MainActivity_getRlt(JNIEnv *env, jobject instance, jobject classobj,
                                          jintArray buf, jint width, jint height) {

    jint *cbuf;
    jboolean ptfalse = false;
    cbuf = env->GetIntArrayElements(buf, &ptfalse);
    if (cbuf == NULL) {
        return 0;
    }

    jclass objectClass = env->FindClass("com/yxsj/yz/e10/DataPicInfo");  //com.yxsj.yz.e10;

    //jfieldID id_buf = env->GetFieldID(objectClass, "buf", "[I");
    jfieldID id_width = env->GetFieldID(objectClass, "width", "I");
    jfieldID id_height = env->GetFieldID(objectClass, "height", "I");
    //jobject classobj;

    Mat imgDataPre(height, width, CV_8UC4, (unsigned char *) cbuf);
    Mat imgData;
    reduce_img_size(imgDataPre, imgData, width, height);

#define TEST_001 0
#if TEST_001
    int size = width * height;
    uchar *ptr = imgDataPre.ptr(0);
#else
    width = imgData.cols;
    height = imgData.rows;
    int size = width * height;
    uchar *ptr = imgData.ptr(0);
#endif

    for (int i = 0; i < size; i++) {
        uchar grayScale = (uchar) (ptr[4 * i + 2] * 0.299 + ptr[4 * i + 1] * 0.587 +
                                   ptr[4 * i + 0] * 0.114);
        ptr[4 * i + 1] = grayScale;
        ptr[4 * i + 2] = grayScale;
        ptr[4 * i + 0] = grayScale;
    }

#if TEST_001
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);  //data
    env->SetIntField(classobj, id_width, width);
    env->SetIntField(classobj, id_height, height);
#else
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, (jint *)imgData.data);  //data
    env->SetIntField(classobj, id_width, width);
    env->SetIntField(classobj, id_height, height);
    // env->SetObjectField(classobj, id_buf, result);

#endif
    env->ReleaseIntArrayElements(buf, cbuf, 0);

    return result;
}
}