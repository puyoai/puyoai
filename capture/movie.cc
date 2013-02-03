#include "movie.h"

Movie::Movie(const char* filename)
    : filename_(filename),
      sws_(NULL) {
  format_ = NULL;

  if (avformat_open_input(&format_, filename_, NULL, NULL) != 0) {
    fprintf(stderr, "Couldn't open file: %s\n", filename_);
    return;
  }

  if (av_find_stream_info(format_) < 0) {
    fprintf(stderr, "Couldn't find stream infomation\n");
    return;
  }

  av_dump_format(format_, 0, filename, false);

  video_index_ = -1;
  for (size_t i = 0; i < format_->nb_streams; i++) {
    if (AVMEDIA_TYPE_VIDEO == format_->streams[i]->codec->codec_type) {
      video_index_ = i;
      break;
    }
  }
  if (video_index_ == -1) {
    fprintf(stderr, "Couldn't find a video stream\n");
    return;
  }

  codec_ = format_->streams[video_index_]->codec;
  width_ = codec_->width;
  height_ = codec_->height;

  AVCodec* c = avcodec_find_decoder(codec_->codec_id);
  if(c == NULL) {
    fprintf(stderr, "Codec not found\n");
    return;
  }
  if (c->capabilities & CODEC_CAP_TRUNCATED) {
    codec_->flags |= CODEC_FLAG_TRUNCATED;
  }

  if (avcodec_open(codec_, c) < 0) {
    fprintf(stderr, "Couldn't open codec\n");
    return;
  }

  frame_ = avcodec_alloc_frame();
  if (frame_rgb_ == NULL) {
    fprintf(stderr, "Couldn't allocate frame memory\n");
    return;
  }

  frame_rgb_ = avcodec_alloc_frame();
  if (frame_rgb_ == NULL) {
    fprintf(stderr, "Couldn't allocate frame_rgb memory\n");
    return;
  }

  int buf_len = avpicture_get_size(PIX_FMT_RGB24, width_, height_);
  uint8_t* buffer = (uint8_t*)calloc(sizeof(uint8_t), buf_len);
  avpicture_fill((AVPicture*)frame_rgb_, buffer,
                 PIX_FMT_RGB24, width_, height_);

#if 1
  surf_ = SDL_CreateRGBSurfaceFrom(frame_rgb_->data[0], width_, height_, 24,
                                   buf_len / height_,
                                   255, 255 << 8, 255 << 16, 0);
#else
  surf_ = SDL_CreateRGBSurface(SDL_SWSURFACE, width_, height_, 24,
                               255, 255 << 8, 255 << 16, 0);
#endif

  fprintf(stderr,"Parsed movie: width=%d height=%d pitch=%d\n",
          width_, height_, surf_->pitch);

  ok_ = true;
}

Movie::~Movie() {
  // TODO(hamaji): Free resource.
}

SDL_Surface* Movie::getNextFrame() {
  int frame_finished;
  while (1) {
    if (av_read_frame(format_, &packet_) < 0)
      return NULL;

    if (packet_.stream_index == video_index_) {
      avcodec_decode_video2(codec_, frame_, &frame_finished, &packet_);

      if (frame_finished) {
        sws_ = sws_getCachedContext(sws_,
                                    width_, height_, codec_->pix_fmt,
                                    width_, height_, PIX_FMT_RGB24,
                                    SWS_BICUBIC,
                                    NULL,
                                    NULL,
                                    NULL);
        sws_scale(sws_, frame_->data, frame_->linesize, 0, height_,
                  frame_rgb_->data, frame_rgb_->linesize);

#if 0
        for (int y = 0; y < height_; y++) {
          memcpy((char*)surf_->pixels + y * surf_->pitch,
                 frame_rgb_->data[0] + y * frame_rgb_->linesize[0],
                 frame_rgb_->linesize[0]);
        }
#endif

        break;
      }
    }

    av_free_packet(&packet_);
  }

  return surf_;
}

void Movie::init() {
  av_register_all();
}
