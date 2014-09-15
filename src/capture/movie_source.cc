#include "capture/movie_source.h"

#include <iostream>

using namespace std;

MovieSource::MovieSource(const char* filename) :
    filename_(filename),
    waitUntilTrue_(true),
    sws_(NULL),
    surf_(makeUniqueSDLSurface(nullptr))
{
    format_ = NULL;

    if (avformat_open_input(&format_, filename_, NULL, NULL) != 0) {
        fprintf(stderr, "Couldn't open file: %s\n", filename_);
        return;
    }

    if (avformat_find_stream_info(format_, nullptr) < 0) {
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
    if (c == NULL) {
        fprintf(stderr, "Codec not found\n");
        return;
    }
    if (c->capabilities & CODEC_CAP_TRUNCATED) {
        codec_->flags |= CODEC_FLAG_TRUNCATED;
    }

    if (avcodec_open2(codec_, c, NULL) < 0) {
        fprintf(stderr, "Couldn't open codec\n");
        return;
    }

    frame_ = avcodec_alloc_frame();
    if (frame_ == NULL) {
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
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(frame_rgb_->data[0], width_, height_, 24,
                                                 buf_len / height_,
                                                 255, 255 << 8, 255 << 16, 0);
#else
    SDL_Surface* surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width_, height_, 24,
                                             255, 255 << 8, 255 << 16, 0);
#endif
    surf_ = makeUniqueSDLSurface(surf);

    fprintf(stderr,"Parsed movie: width=%d height=%d pitch=%d\n",
            width_, height_, surf_->pitch);

    ok_ = true;
}

MovieSource::~MovieSource()
{
    // TODO(hamaji): Free resource.
}

void MovieSource::nextStep()
{
    waitUntilTrue_ = true;
}

UniqueSDLSurface MovieSource::getNextFrame()
{
    int frame_finished;
    while (true) {
        if (av_read_frame(format_, &packet_) < 0)
            return emptyUniqueSDLSurface();

        if (packet_.stream_index == video_index_) {
            avcodec_decode_video2(codec_, frame_, &frame_finished, &packet_);

            if (frame_finished) {
                sws_ = sws_getCachedContext(sws_,
                                            width_, height_, codec_->pix_fmt,
                                            width_, height_, PIX_FMT_RGB24,
                                            SWS_BICUBIC, NULL, NULL, NULL);

                sws_scale(sws_, frame_->data, frame_->linesize, 0, height_, frame_rgb_->data, frame_rgb_->linesize);

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

    // Wait until next frame.
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - lastTaken_;
    if (fps_ == 0) {
        while (!waitUntilTrue_) {
            SDL_Delay(10);
        }
        waitUntilTrue_ = false;
        cout << "space detected" << endl;
    } else if (static_cast<int>(elapsed) < 1000 / fps_) {
        int d = 1000 / fps_ - elapsed;
        SDL_Delay(d);
        Uint32 hoge = SDL_GetTicks();
        cout << "waiting... " << d << "ms ... " << (hoge - currentTime) << endl;
    }
    lastTaken_ = SDL_GetTicks();

    return makeUniqueSDLSurface(SDL_ConvertSurface(surf_.get(), surf_->format, 0));
}

void MovieSource::init()
{
    av_register_all();
}
