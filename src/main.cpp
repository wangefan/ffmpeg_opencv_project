#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
 extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

int XError(int res) {
    char buf[1024] = {0};
    av_strerror(res, buf, sizeof(buf));
    std::cout << buf << std::endl;
    getchar();
    return -1;
}

int main() {
  // ==1. open inUrl
  const std::string inUrl = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4";
  const std::string outUrl = "rtmp://10.82.82.200/live";
  std::cout << "==1. open inUrl: " << inUrl << std::endl;
  cv::VideoCapture cap;
  if(!cap.open(inUrl)) {
    std::cout << "open inUrl failed" << std::endl;
    exit(0);
  }
  std::cout << "open inUrl ok" << std::endl;

  std::cout << "==2. prepare converting format" << inUrl << std::endl;
  int nSrcWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  int nSrcHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  int nFPS = cap.get(cv::CAP_PROP_FPS);
  SwsContext* pSct = NULL;
  pSct = sws_getCachedContext(pSct, nSrcWidth, nSrcHeight, AV_PIX_FMT_BGR24, nSrcWidth, nSrcHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
  if(!pSct) {
    std::cout << "sws_getCachedContext failed" << std::endl;
    exit(0);
  }
  std::cout << "sws_getCachedContext ok, nSrcWidth = " << nSrcWidth << ", nSrcHeight = " << nSrcHeight << std::endl;
  AVFrame* pYUVFrame = av_frame_alloc();
  pYUVFrame->format = AV_PIX_FMT_YUV420P;
  pYUVFrame->width = nSrcWidth;
  pYUVFrame->height = nSrcHeight;
  pYUVFrame->pts = 0;
  std::cout << "pYUVFrame->width = " << pYUVFrame->width << ", pYUVFrame->height = " << pYUVFrame->height << std::endl;
  int res = av_frame_get_buffer(pYUVFrame, 0);
  if(res != 0) {
    return XError(res);
  }

  std::cout << "==3. encode.. " << std::endl;
  const AVCodec* pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if(!pH264Codec) {
    std::cout << "avcodec_find_encoder failed" << std::endl;
    exit(0);
  }
  AVCodecContext* pAvCodecContext = avcodec_alloc_context3(pH264Codec);
  if(!pAvCodecContext) {
    std::cout << "avcodec_alloc_context3 failed" << std::endl;
    exit(0);
  }
  pAvCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //
  pAvCodecContext->codec_id = pH264Codec->id;
  pAvCodecContext->thread_count = 8;
  pAvCodecContext->bit_rate = 50 * 1024 * 8; // sampling video 50kB/sec
  pAvCodecContext->width = nSrcWidth;
  pAvCodecContext->height = nSrcHeight;
  pAvCodecContext->time_base = {1, nFPS};
  pAvCodecContext->framerate = {nFPS, 1};
  pAvCodecContext->gop_size = 50; //how many frames contains 1 key frame.
  pAvCodecContext->max_b_frames = 0; // workaround for simplifying dts
  pAvCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

  res = avcodec_open2(pAvCodecContext, 0, 0);
  if(res != 0) {
    return XError(res);
  }

  std::cout << "==4. process frame loop begin.. " << std::endl;
  while (true) {
    if(!cap.grab()) { // read and decode
      std::cout << "cap.grab() fail!" << std::endl;
      continue;
    }
    cv::Mat inFrame;
    if(!cap.retrieve(inFrame)) { // output inFrame
      std::cout << "cap.retrieve() fail!" << std::endl;
      continue;
    }
    cv::imshow("RTSP", inFrame);   
    cv::waitKey(1);

    // RGB to YUV, now RGB
    uint8_t *inData[AV_NUM_DATA_POINTERS] = {0};
    //BGRBGR..
    inData[0] = inFrame.data;
    int inStride[AV_NUM_DATA_POINTERS] = {0};
    inStride[0] = inFrame.cols * inFrame.elemSize();
    int h = sws_scale(pSct, inData, inStride, 0, inFrame.rows, pYUVFrame->data, pYUVFrame->linesize);
    if(h <= 0) {
      continue;
    }
    std::cout << h << " " << std::flush;
  }   

  sws_freeContext(pSct);
  pSct = NULL;

  av_frame_free(&pYUVFrame);
  pYUVFrame = NULL;

  if(pAvCodecContext) {
    avcodec_free_context(&pAvCodecContext);
  }
  
  return 0;
}
