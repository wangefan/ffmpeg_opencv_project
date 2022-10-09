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
  int res = av_frame_get_buffer(pYUVFrame, 32);
  if(res != 0) {
    return XError(res);
  }

  std::cout << "==3. process frame loop begin.. " << std::endl;
  while (true) {
    if(!cap.grab()) { // read and decode
      std::cout << "cap.grab() fail!" << std::endl;
      continue;
    }
    cv::Mat frame;
    if(!cap.retrieve(frame)) { // output frame
      std::cout << "cap.retrieve() fail!" << std::endl;
      continue;
    }
    cv::imshow("RTSP", frame);   
    cv::waitKey(1);

    // RGB to YUV, now RGB
    uint8_t *inData[AV_NUM_DATA_POINTERS] = {0};
    //BGRBGR..
    inData[0] = frame.data;
    int inSize[AV_NUM_DATA_POINTERS] = {0};
    inSize[0] = frame.cols * frame.elemSize();
    int h = sws_scale(pSct, inData, inSize, 0, frame.rows, pYUVFrame->data, pYUVFrame->linesize);
    if(h <= 0) {
      continue;
    }
    std::cout << h << " " << std::flush;
  }   

  sws_freeContext(pSct);
  pSct = NULL;

  av_frame_free(&pYUVFrame);
  pYUVFrame = NULL;
  
  return 0;
}
