#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <unistd.h>


#include <raspicam/raspicam_cv.h>

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/avutil.h>
  #include <libavutil/pixdesc.h>
  #include <libswscale/swscale.h>
  #include "libavutil/imgutils.h"
  #include <libavutil/opt.h>
}

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/img_hash.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <bitset>

using namespace std;
using namespace cv;



void av_image_fill_arrays(void *frame, uint8_t *ptr, enum AVPixelFormat pix_fmt, int width, int height)
{
	avpicture_fill((AVPicture*)frame, ptr, pix_fmt, width, height);
	
}
void cvmat_to_avframe(cv::Mat *input_image, AVPixelFormat output_pix_fmt, AVFrame *ret_frame)

{
	AVFrame *intput_frame;
	AVPixelFormat input_pix_fmt=AV_PIX_FMT_BGR24;
	const unsigned char* data_frame=(const uchar*)input_image->data;
	int step=(int)input_image->step;
	int width=(int)input_image->cols;
	int height=(int)input_image->rows;
	int cn=(int)input_image->channels();
	int origin=0;
	unsigned char *aligned_input=NULL;
	size_t aligned_input_size=0;
	//cout<<"step:"<<step<<","<<"width:"<<width<<",height:"<<height<<",cn:"<<cn<<",";
	const int CV_STEP_ALIGNMENT = 32;
	const size_t CV_SIMD_SIZE = 32;
	const size_t CV_PAGE_MASK = ~(4096 - 1);
	const uchar* dataend = data_frame + ((size_t)height * step);

	if (step % CV_STEP_ALIGNMENT != 0 ||
        	(((size_t)dataend - CV_SIMD_SIZE) & CV_PAGE_MASK) != (((size_t)dataend + CV_SIMD_SIZE) & CV_PAGE_MASK)){
		int aligned_step = (step + CV_STEP_ALIGNMENT - 1) & ~(CV_STEP_ALIGNMENT - 1);

        	size_t new_size = (aligned_step * height + CV_SIMD_SIZE);

		if (!aligned_input || aligned_input_size < new_size)
		{
		    if (aligned_input)
		        av_freep(&aligned_input);
		    aligned_input_size = new_size;
		    aligned_input = (unsigned char*)av_mallocz(aligned_input_size);
		}
		if (origin == 1)
		    for( int y = 0; y < height; y++ )
		        memcpy(aligned_input + y*aligned_step, data_frame + (height-1-y)*step, step);
		else
		    for( int y = 0; y < height; y++ )
		        memcpy(aligned_input + y*aligned_step, data_frame + y*step, step);
		data_frame = aligned_input;
        	step = aligned_step;
	}

	if ( input_pix_fmt != output_pix_fmt ){
		
		intput_frame = av_frame_alloc();
		av_image_fill_arrays(intput_frame, (uint8_t *) data_frame,
                       (AVPixelFormat)input_pix_fmt, width, height);
		int numBytesYUV=av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height,1);
		
		intput_frame->format = (AVPixelFormat)input_pix_fmt;
		intput_frame->width = width;
		intput_frame->height = height;
		intput_frame->linesize[0] = step;
		
		ret_frame->format = (AVPixelFormat)output_pix_fmt;
		ret_frame->width = width;
		ret_frame->height = height;
				
		struct SwsContext *img_convert_ctx;
		img_convert_ctx = sws_getContext(width,
	                                     height,
	                                     (AVPixelFormat)input_pix_fmt,
	                                     width,
	                                     height,
	                                     (AVPixelFormat)output_pix_fmt,
	                                     SWS_BICUBIC,
	                                     NULL, NULL, NULL);
		
		sws_scale(img_convert_ctx, intput_frame->data,
		               intput_frame->linesize, 0,
		               height, ret_frame->data, ret_frame->linesize);
		
		sws_freeContext(img_convert_ctx);
		av_free(intput_frame);
	
	}else{
		
		av_image_fill_arrays(ret_frame, (uint8_t *) data_frame,
		               (AVPixelFormat)output_pix_fmt, width, height);
		ret_frame->format = output_pix_fmt;
		ret_frame->width = width;
		ret_frame->height = height;
		ret_frame->linesize[0] = step;
	}
	
        
	if (aligned_input)
		av_freep(&aligned_input);
	//return ret_frame;
}


int main(int argc, char * argv[]) {
	
	
	av_register_all(); 

	int conf_fps=25;
	int conf_video_len_sec=10;
	int conf_video_width=960;
	int conf_video_height=720;
	int conf_bit_rate=600000;
	int packetId=0;
	
	
	////create raspi camera object
	raspicam::RaspiCam_Cv Camera;
	Camera.set(CV_CAP_PROP_FORMAT, CV_8UC3);
	Camera.set(CV_CAP_PROP_FRAME_WIDTH, conf_video_width );
	Camera.set(CV_CAP_PROP_FRAME_HEIGHT, conf_video_height );
	Camera.set(CV_CAP_PROP_FPS, conf_fps );

	if (!Camera.open()) {
		cout<<"Error opening the camera"<<endl;
		return 0; 
	}
	

	AVCodec *codec;
	AVCodecContext *cContext= NULL;
	int i, ret, got_output;
	FILE *outputFile_H264;
	AVPacket pkt;
	
	codec = avcodec_find_encoder_by_name("h264_omx");
	if (!codec) {
		cout<<"codec not found"<<endl;
		exit(1);
	}
	cout<<"encoder is found"<<endl;
	
	cContext = avcodec_alloc_context3(codec);
	if (!cContext) {
		cout<<"could not allocate video codec context"<<endl;
		exit(1);
	}
	cout<<"video codec is allocated"<<endl;
	
	cContext->width = conf_video_width;
	cContext->height = conf_video_height;
	cContext->time_base= (AVRational){1,conf_fps};

	cContext->codec_type = AVMEDIA_TYPE_VIDEO;
	cContext->gop_size = 17; 
	cContext->bit_rate = conf_bit_rate;
   
   //omx just support YUV420P format
	cContext->pix_fmt = AV_PIX_FMT_YUV420P; 
	cContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	av_opt_set(cContext->priv_data, "profile", "baseline", AV_OPT_SEARCH_CHILDREN);
	// it must be zero and not be neglected
	av_opt_set(cContext->priv_data, "zerocopy", "0", 0); 
	
	// open the codec context
	if (avcodec_open2(cContext, codec, NULL) < 0) {
		
		cout<<"could not open codec"<<endl;
		exit(1);
	}
	cout<<"the codec is opened"<<endl;
	
		
	//Allocating an avframe
	AVFrame *video_avFrame;
	video_avFrame = av_frame_alloc();
	if (!video_avFrame) {
		cout<<"could not allocate video frame"<<endl;
		exit(1);
    }
    video_avFrame->format = cContext->pix_fmt;
    video_avFrame->width  = cContext->width;
    video_avFrame->height = cContext->height;
	
	ret = av_image_alloc(video_avFrame->data, video_avFrame->linesize, cContext->width, cContext->height,
                          cContext->pix_fmt, 32);
	
	outputFile_H264 = fopen("./sample.h264", "wb");
	if (!outputFile_H264) {
		cout<<"could not open the file"<<endl;
		exit(1);
	}
	
	
	while (packetId < (conf_video_len_sec * conf_fps)){
	
		Mat video_matFrame;
		Camera.grab();
		Camera.retrieve(video_matFrame);
		
		////convert opencv frame to avframe
		cvmat_to_avframe(&video_matFrame, AV_PIX_FMT_YUV420P, video_avFrame); 
		
		av_init_packet(&pkt);
		pkt.data = NULL; 
		pkt.size = 0;
		pkt.stream_index = packetId;
		
		fflush(stdout);
		video_avFrame->pts = cContext->frame_number;
		
		//// encode the frame
		ret = avcodec_encode_video2(cContext, &pkt, video_avFrame, &got_output);
		if (ret < 0) {
			cout<<"Error encoding frame"<<endl;
			exit(1);
		}
	
		if (got_output && pkt.size>0){

			fwrite(cContext->extradata, 1, cContext->extradata_size, outputFile_H264);
			fwrite(pkt.data, 1, pkt.size, outputFile_H264);
			
			av_free_packet(&pkt);
		}
		packetId++;

	}
	
	cout<<"coding the delayed frames"<<endl;
	for (got_output = 1; got_output; i++) {
		fflush(stdout);
		ret = avcodec_encode_video2(cContext, &pkt, NULL, &got_output);
		if (ret < 0) {
			cout<<"Error encoding frame"<<endl;
			exit(1);
		}
		if (got_output) {
			
			fwrite(cContext->extradata, 1, cContext->extradata_size, outputFile_H264);
			fwrite(pkt.data, 1, pkt.size, outputFile_H264);
			av_free_packet(&pkt);
		}
	}
	
	fclose(outputFile_H264);
	avcodec_close(cContext);
    av_free(cContext);
	
	cout<<"your file is ready to use"<<endl;

  return 0;
}
