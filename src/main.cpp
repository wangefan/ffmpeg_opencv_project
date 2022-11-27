#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
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

  std::cout << "==2. prepare converting format stuff, RGB to YUV" << inUrl << std::endl;
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
  int nYUVPts = 0;
  std::cout << "pYUVFrame->width = " << pYUVFrame->width << ", pYUVFrame->height = " << pYUVFrame->height << std::endl;
  int res = av_frame_get_buffer(pYUVFrame, 0);
  if(res != 0) {
    return XError(res);
  }

  std::cout << "==3. Prepare encode stuff, encode to H264 " << std::endl;
  avcodec_register_all();
  const AVCodec* pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  AVPacket pktEncoded;
  memset(&pktEncoded, 0, sizeof(AVPacket));
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

  std::cout << "==4. prepare container stuff" << std::endl;
  av_register_all();
  avformat_network_init();
  AVFormatContext* pAvFormatContext = NULL;
  res = avformat_alloc_output_context2(&pAvFormatContext, 0, "flv", outUrl.c_str()); // here is for container format
  if(res < 0) {
    return XError(res);
  }
  // video stream
  AVStream* pVStream = avformat_new_stream(pAvFormatContext, NULL);
  if(!pVStream) {
    std::cout << "avformat_new_stream failed" << std::endl;
    exit(0);
  }
  pVStream->codecpar->codec_tag = 0; //means no set
  avcodec_parameters_from_context(pVStream->codecpar, pAvCodecContext); // means copy from encoder context
  av_dump_format(pAvFormatContext, 0, outUrl.c_str(), 1);

  // here is for network protocol, ex:RTMP
  res = avio_open(&pAvFormatContext->pb, outUrl.c_str(), AVIO_FLAG_WRITE);
  if(res < 0) {
    return XError(res);
  }
  res = avformat_write_header(pAvFormatContext, NULL);
  if(res < 0) {
    return XError(res);
  }

  std::cout << "==5. process frame loop begin.. " << std::endl;
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
    // h is supposed to be the height of the video
    int h = sws_scale(pSct, inData, inStride, 0, inFrame.rows, pYUVFrame->data, pYUVFrame->linesize);
    if(h <= 0) {
      continue;
    }
    // encode to H264
    pYUVFrame->pts = nYUVPts++;
    //std::cout << "pYUVFrame->pts:" << pYUVFrame->pts;
    res = avcodec_send_frame(pAvCodecContext, pYUVFrame);
    if(res != 0)
      continue;
	// get encoded packet
    res = avcodec_receive_packet(pAvCodecContext, &pktEncoded);
    // Could observe the pack size, the bigger size would be occurred
    // very pAvCodecContext->gop_size frames since that would be the key
    // frame.
    if(res == 0 && pktEncoded.size > 0) {
      //std::cout << "pktEncoded.size:" << pktEncoded.size << "\n" << std::flush;
	  // push stream
	  pktEncoded.pts = av_rescale_q(pktEncoded.pts, pAvCodecContext->time_base, pVStream->time_base);
	  pktEncoded.dts = av_rescale_q(pktEncoded.dts, pAvCodecContext->time_base, pVStream->time_base);
	  res = av_interleaved_write_frame(pAvFormatContext, &pktEncoded);
	  if(res == 0) { // success
	  	
	  }
	}
  }   

  sws_freeContext(pSct);
  pSct = NULL;

  av_frame_free(&pYUVFrame);
  pYUVFrame = NULL;

  if(pAvCodecContext) {
    avcodec_free_context(&pAvCodecContext);
  }
  if(pAvFormatContext) {
  	avio_closep(&pAvFormatContext->pb);
	avformat_free_context(pAvFormatContext);
  }
  
  return 0;
}
