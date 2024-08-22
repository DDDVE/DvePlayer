#include <QDebug>
#include <windows.h>
#include <map>

extern "C"
{


}

#include "log.h"
#include "player/player.h"
#include "mainwindow.h"

/* Without this av_err2str() will report error */
//#if defined(_MSC_VER)
//static char av_error[AV_ERROR_MAX_STRING_SIZE] = { 0 };
//#define av_err2str(errnum) \
//    av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)
//#else
//#define av_err2str(errnum) \
//    av_make_error_string((char[AV_ERROR_MAX_STRING_SIZE]){0}, AV_ERROR_MAX_STRING_SIZE, errnum)
//#endif

#define MAX_AUDIO_FRAME_SIZE 192000
#define PAUSE_SLEEP_TIME    100 // ms

QMutex gTimeDiffMut;
QMutex gImgMut;
QImage gImg;
//QPixmap gPixmap;

std::map<int, int> gSampleFmt2BitNum = {{AV_SAMPLE_FMT_NONE, -1},
                                        {AV_SAMPLE_FMT_U8, 8},
                                        {AV_SAMPLE_FMT_S16, 16},
                                        {AV_SAMPLE_FMT_S32, 32},
                                        {AV_SAMPLE_FMT_FLT, 32},
                                        {AV_SAMPLE_FMT_DBL, 64},
                                        {AV_SAMPLE_FMT_U8P, 8},
                                        {AV_SAMPLE_FMT_S16P, 16},
                                        {AV_SAMPLE_FMT_S32P, 32},
                                        {AV_SAMPLE_FMT_FLTP, 32},
                                        {AV_SAMPLE_FMT_DBLP, 64},
                                        {AV_SAMPLE_FMT_S64, 64},
                                        {AV_SAMPLE_FMT_S64P, 64}};

/*=====================================Player========================================*/

Player::Player(QWidget &_mMainWindow)
    : mMainWindow(_mMainWindow)
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    mIsPause = false;
    mNeedStop = false;
    mVideoTimeBase = 0.0;
    mAudioTimeBase = 0.0;
    mTimeDiff = 0.0;
    mAudioVolume = 0.5;

    mVideoPlayer = new VideoPlayer(mIsPause, mAudioTimeBase, mVideoTimeBase, mTimeDiff, mNeedStop, mFilePath, _mMainWindow, *this);
    mAudioPlayer = new AudioPlayer(mIsPause, mAudioTimeBase, mNeedStop, mFilePath, _mMainWindow, *this, mAudioVolume);



}

Player::~Player()
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    if (!mVideoPlayer)
    {
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
    }
    if (!mAudioPlayer)
    {
        delete mAudioPlayer;
        mAudioPlayer = nullptr;
    }
}

void Player::Start(const QString _filePath)
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    if (!mVideoPlayer || !mAudioPlayer)
    {
        qDebug() << "[ERROR] Have nullptr. "
                 << LOG_FUNCTION_AND_LINE
                 << getMiliSecondTimeStamp();
        return;
    }
    if (mVideoPlayer->isRunning() || mAudioPlayer->isRunning())
    {
        if (!mIsPause)
        {
            // TODO: end two threads, change filePath, restart
            qDebug() << "[INFO] Player is running, try again later."
                     << LOG_FUNCTION_AND_LINE
                     << getMiliSecondTimeStamp();
            mNeedStop = true;
            while (mVideoPlayer->isRunning())
            {
                Sleep(50);
            }
            while (mAudioPlayer->isRunning())
            {
                Sleep(50);
            }
            qDebug() << "[INFO] threads end." << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        }
        else
        {
            qDebug() << "[INFO] Running but pause, play new file."
                     << LOG_FUNCTION_AND_LINE
                     << getMiliSecondTimeStamp();
            mNeedStop = true;
            while (mVideoPlayer->isRunning())
            {
                Sleep(50);
            }
            while (mAudioPlayer->isRunning())
            {
                Sleep(50);
            }
            qDebug() << "[INFO] threads end." << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        }
    }
    mFilePath = _filePath;
    mNeedStop = false;
    mIsPause = false;

    // start this thread
    this->start();
}

void Player::run()
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    mVideoTimeBase = -1;
    mAudioTimeBase = -1;

    // init video and audio parameter
    int audioInitRet = mAudioPlayer->InitRunParam();
    int videoInitRet = mVideoPlayer->InitRunParam();

    if (!videoInitRet)
    {
        qDebug() << "[INFO] videoInitRet succeed." << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        mVideoPlayer->start();
    }
    else
    {
        qDebug() << "[ERROR] videoInitRet FAIL." << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        mVideoPlayer->releaseRunParam();
    }

    if (!audioInitRet)
    {
        qDebug() << "[INFO] mAudioPlayer succeed." << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        mAudioPlayer->start();
    }
    else
    {
        qDebug() << "[ERROR] mAudioPlayer FAIL." << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        mAudioPlayer->releaseRunParam();
    }
    qDebug() << LOG_LEAVE_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
}

/*====================================VideoPlayer=======================================*/
VideoPlayer::VideoPlayer(bool& _mIsPause, double &_mAudioTimeBase, double &_mVideoTimeBase, double &_mTimeDiff, bool &_mNeedStop, QString &_mFilePath, QWidget &_mMainWindow, QThread &_mParent)
    : mTimeDiff(_mTimeDiff),
      mAudioTimeBase(_mAudioTimeBase),
      mVideoTimeBase(_mVideoTimeBase),
      mIsPause(_mIsPause),
      mNeedStop(_mNeedStop),
      mFilePath(_mFilePath),
      mMainWindow(_mMainWindow),
      mParent(_mParent)
{
    mVideoBuf = nullptr;
    mVideoFormatCtx = nullptr;
    mVideoCodecCtx = nullptr;
    mVideoCodec = nullptr;
    mVideoPacket = nullptr;
    mVideoFrame = nullptr;
    mVideoFrameRGB = nullptr;
    mVideoSwsCtx = nullptr;
}

VideoPlayer::~VideoPlayer()
{

}

int VideoPlayer::InitRunParam()
{
    // alloc AVFormatContext
    mVideoFormatCtx = avformat_alloc_context();
    // init AVFormatContext
    int ret = avformat_open_input(&mVideoFormatCtx, mFilePath.toStdString().c_str(), NULL, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avformat_open_input FAIL. file=["
                 << mFilePath << "], err=[" << av_err2str(ret)
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }
    // find video stream info
    ret = avformat_find_stream_info(mVideoFormatCtx, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avformat_find_stream_info FAIL. err=["
                 << QString(av_err2str(ret))
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }
    // find video stream index
    mVideoStreamIndex = av_find_best_stream(mVideoFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    // check whether find stream index
    if (mVideoStreamIndex < 0)
    {
        qDebug() << "[ERROR] avformat_find_stream_info FAIL."
                 << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return -1;
    }
    // alloc video stream decodec context
    mVideoCodecCtx = avcodec_alloc_context3(NULL);
    // set codec parameter
    avcodec_parameters_to_context(mVideoCodecCtx, mVideoFormatCtx->streams[mVideoStreamIndex]->codecpar);
    // find decoder
    mVideoCodec = const_cast<AVCodec*>(avcodec_find_decoder(mVideoCodecCtx->codec_id));
    if (!mVideoCodec)
    {
        qDebug() << "[ERROR] avcodec_find_decoder FAIL."
                 << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return -1;
    }
    // init decodec context parameter
    ret = avcodec_open2(mVideoCodecCtx, mVideoCodec, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avcodec_open2 FAIL. err=["
                 << QString(av_err2str(ret))
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }
    // alloc packet
    mVideoPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    // alloc frame
    mVideoFrame = av_frame_alloc();
    mVideoFrameRGB = av_frame_alloc();
    // init Sws
    mVideoSwsCtx = sws_getContext(mVideoCodecCtx->width, mVideoCodecCtx->height, mVideoCodecCtx->pix_fmt,
                             mVideoCodecCtx->width, mVideoCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    // alloc image buf
    mVideoBuf = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, mVideoCodecCtx->width, mVideoCodecCtx->height, 1));
    av_image_fill_arrays(mVideoFrameRGB->data, mVideoFrameRGB->linesize, mVideoBuf, AV_PIX_FMT_RGB32, mVideoCodecCtx->width, mVideoCodecCtx->height, 1);
    return 0;
}

// release source
void VideoPlayer::releaseRunParam()
{

    if (mVideoFormatCtx)
    {
        avformat_close_input(&mVideoFormatCtx); mVideoFormatCtx = nullptr;
    }
    if (mVideoCodecCtx)
    {
        avcodec_free_context(&mVideoCodecCtx); mVideoCodecCtx = nullptr;
    }
    if (mVideoFrame)
    {
        av_frame_free(&mVideoFrame); mVideoFrame = nullptr;

    }
    if (mVideoFrameRGB)
    {
        //av_freep(mVideoFrameRGB);
        av_frame_free(&mVideoFrameRGB); mVideoFrameRGB = nullptr;
        //qDebug() << "111";
    }
    if (mVideoPacket)
    {
        av_freep(mVideoPacket);
        av_free(mVideoPacket); mVideoPacket = nullptr;
        qDebug() << "111";
    }
    if (mVideoSwsCtx)
    {
        sws_freeContext(mVideoSwsCtx); mVideoSwsCtx = nullptr;
    }
    if (mVideoBuf)
    {
//        av_free(mVideoBuf); mVideoBuf = nullptr;

    }
    qDebug() << "222";
}

void VideoPlayer::run()
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    Player& p = static_cast<Player&>(mParent);
    int ret = 0;
    // time setting
    int64_t startClock;
    int64_t nowClock;
    int64_t wishClock;
    int64_t timeDiff;

    // resize main window size
    emit sigResizeWindows(mVideoCodecCtx->width, mVideoCodecCtx->height);

    startClock = getMiliSecondTimeStamp();  // self clock
    // read video packet in a loop
    while (!mNeedStop)
    {
        if (mIsPause)
        {
            Sleep(PAUSE_SLEEP_TIME);
            startClock += PAUSE_SLEEP_TIME;
            continue;
        }
        // read frame
        ret = av_read_frame(mVideoFormatCtx, mVideoPacket);
        if (ret < 0)
        {
            qDebug() << "[ERROR] av_read_frame FAIL. ret=[" << ret << "], err=["
                     << QString(av_err2str(ret))
                     << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
            goto out;
        }
        if (mVideoPacket->stream_index != mVideoStreamIndex)
        {
            av_packet_unref(mVideoPacket);
            continue;
        }
        // decode frame
        // 1.send packet
        ret = avcodec_send_packet(mVideoCodecCtx, mVideoPacket);
        if (ret == -1 * EAGAIN || ret == -1 * 1094995529)
        {
            continue;
        }
        if (ret < 0)
        {
            qDebug() << "[ERROR] avcodec_send_packet FAIL. ret=[" << ret << "], err=["
                     << QString(av_err2str(ret)).toStdString().c_str()
                     << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
            goto out;
        }
        // 2. decode to frame
        ret = avcodec_receive_frame(mVideoCodecCtx, mVideoFrame);
        if (ret == -1 * EAGAIN || ret == -1 * 1094995529)
        {
            continue;
        }
        if (ret < 0)
        {
            qDebug() << "[ERROR] avcodec_receive_frame FAIL. ret=[" << ret << "], err=["
                     << QString(av_err2str(ret))
                     << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
            goto out;
        }

        // YUV to RGB32
        sws_scale(mVideoSwsCtx, mVideoFrame->data, mVideoFrame->linesize, 0, mVideoCodecCtx->height, mVideoFrameRGB->data, mVideoFrameRGB->linesize);
//            gImgMut.lock();
        gImg = QImage((uchar*)mVideoFrameRGB->data[0], mVideoCodecCtx->width, mVideoCodecCtx->height, QImage::Format_RGB32);
        p.mPixmap = QPixmap::fromImage(gImg);
        //            gImgMut.unlock();
        mMainWindow.update();

        // sync with audio
        if (static_cast<Player&>(mParent).mAudioPlayer->isRunning() && mAudioTimeBase >= 0)
        {
//            mVideoTimeBase = mVideoFrame->pts * av_q2d(mVideoFormatCtx->streams[mVideoStreamIndex]->time_base) * 1000;
            mVideoTimeBase = mVideoPacket->pts * av_q2d(mVideoFormatCtx->streams[mVideoStreamIndex]->time_base) * 1000;
            gTimeDiffMut.lock();
            mTimeDiff = mVideoTimeBase - mAudioTimeBase;
            gTimeDiffMut.unlock();
            if (mTimeDiff > 0)
            {
#ifdef DVE_DEBUG
                qDebug() << "[INFO] *********image too fast, videoTimeBase=[" << mVideoTimeBase << "], audioTimeBase=["
                         << mAudioTimeBase << "]"
                         << getMiliSecondTimeStamp();
#endif
                Sleep(mTimeDiff);
            }
            else
            {
#ifdef DVE_DEBUG
                qDebug() << "[INFO]==========not too fast, videoTimeBase=[" << mVideoTimeBase << "], audioTimeBase=["
                         << mAudioTimeBase << "]"
                         << getMiliSecondTimeStamp();
#endif
            }
        }
        else
        {
            // sync with video stream itself
            nowClock = getMiliSecondTimeStamp();
            wishClock = mVideoPacket->pts * av_q2d(mVideoFormatCtx->streams[mVideoStreamIndex]->time_base) * 1000;
            timeDiff = wishClock - (nowClock - startClock);
            if (timeDiff > 0)
            {
                Sleep(timeDiff);
            }
        }
        av_packet_unref(mVideoPacket);
    }
out:
    releaseRunParam();
    qDebug() << LOG_LEAVE_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    return;
}

/*====================================AudioPlayer=======================================*/
AudioPlayer::AudioPlayer(bool& _mIsPause, double &_mAudioTimeBase, bool &_mNeedStop,
                         QString &_mFilePath, QWidget &_mMainWindow, QThread &_mParent, double &_mAudioVolume)
    : mAudioTimeBase(_mAudioTimeBase),
      mIsPause(_mIsPause),
      mNeedStop(_mNeedStop),
      mFilePath(_mFilePath),
      mMainWindow(_mMainWindow),
      mParent(_mParent),
      mAudioVolume(_mAudioVolume)
{
    mAudioFormat = nullptr;
    mAudioOutput = nullptr;
    mAudioIODevice = nullptr;
    mAudioFormatCtx = nullptr;
    mAudioCodecCtx = nullptr;
    mAudioCodec = nullptr;
    mAudioSwrCtx = nullptr;
    mAudioPacket = nullptr;
    mAudioFrame = nullptr;
    mAudioBuf = nullptr;
}

AudioPlayer::~AudioPlayer()
{

}

void AudioPlayer::InitAudioFormat(QAudioFormat &_audioFormat)
{
    _audioFormat.setSampleRate(mAudioCodecCtx->sample_rate);
    _audioFormat.setChannelCount(mAudioCodecCtx->ch_layout.nb_channels);
    _audioFormat.setSampleSize(8 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
    _audioFormat.setCodec("audio/pcm");
    _audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    _audioFormat.setSampleType(QAudioFormat::SignedInt);
}

void AudioPlayer::releaseRunParam()
{
    if (mAudioFrame)
    {
        av_frame_free(&mAudioFrame); mAudioFrame = nullptr;
    }
    if (mAudioPacket)
    {
        av_packet_free(&mAudioPacket); mAudioPacket = nullptr;
    }
    if (mAudioCodecCtx)
    {
        avcodec_free_context(&mAudioCodecCtx); mAudioCodecCtx = nullptr;
    }
    if (mAudioFormatCtx)
    {
        avformat_free_context(mAudioFormatCtx); mAudioFormatCtx = nullptr;
    }
    if (mAudioBuf)
    {
        av_free(mAudioBuf); mAudioBuf = nullptr;
    }
    if (mAudioFormat)
    {
        delete mAudioFormat; mAudioFormat = nullptr;
    }
    if (mAudioOutput)
    {
        mAudioOutput->stop();
        delete mAudioOutput; mAudioOutput = nullptr;
    }
}

int AudioPlayer::InitRunParam()
{
    int ret = 0;

    mAudioFormatCtx = avformat_alloc_context();
    mAudioPacket = av_packet_alloc();
    mAudioFrame = av_frame_alloc();

    ret = avformat_open_input(&mAudioFormatCtx, mFilePath.toStdString().c_str(), NULL, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avformat_open_input FAIL. file=["
                    << mFilePath << "], err=[" << av_err2str(ret)
                    << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }
    ret = avformat_find_stream_info(mAudioFormatCtx, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avformat_find_stream_info FAIL. err=["
                 << QString(av_err2str(ret))
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }

    mAudioStreamIndex = av_find_best_stream(mAudioFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (mAudioStreamIndex < 0)
    {
        qDebug() << "[ERROR] avformat_find_stream_info FAIL."
                 << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return -1;
    }
    mAudioCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(mAudioCodecCtx, mAudioFormatCtx->streams[mAudioStreamIndex]->codecpar);
    mAudioCodec = const_cast<AVCodec*>(avcodec_find_decoder(mAudioCodecCtx->codec_id));
    if (!mAudioCodec)
    {
        qDebug() << "[ERROR] avcodec_find_decoder FAIL."
                 << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return -1;
    }
    ret = avcodec_open2(mAudioCodecCtx, mAudioCodec, NULL);
    if (ret < 0)
    {
        qDebug() << "[ERROR] avcodec_open2 FAIL. err=["
                 << QString(av_err2str(ret))
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }

    // set audio format
    mAudioFormat = new QAudioFormat();
    InitAudioFormat(*mAudioFormat);
    QAudioDeviceInfo audioDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    if (!audioDeviceInfo.isFormatSupported(*mAudioFormat))
    {
        *mAudioFormat = audioDeviceInfo.nearestFormat(*mAudioFormat);
        qDebug() << "[ERROR] isFormatSupported FAIL, change format to { sample rate=[" << mAudioFormat->sampleRate()
                 << "], channel count=[" << mAudioFormat->channelCount() << "], sample size=[" << mAudioFormat->sampleSize()
                 << "], codec=[" << mAudioFormat->codec() << "], byte order=[" << mAudioFormat->byteOrder()
                 << "], sample type=[" << mAudioFormat->sampleType() << "] }" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    }

    mAudioOutput = new QAudioOutput(*mAudioFormat);
    mAudioOutput->setVolume(mAudioVolume);
    mAudioIODevice = mAudioOutput->start();
    // set decode parameter
    ret = swr_alloc_set_opts2(&mAudioSwrCtx, &mAudioCodecCtx->ch_layout, AV_SAMPLE_FMT_S16, mAudioCodecCtx->sample_rate,
                                  &mAudioCodecCtx->ch_layout, mAudioCodecCtx->sample_fmt, mAudioCodecCtx->sample_rate, 0, nullptr);
    if (ret < 0)
    {
        qDebug() << "[ERROR] swr_alloc_set_opts2 FAIL. err=["
                 << QString(av_err2str(ret))
                 << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
        return ret;
    }
    swr_init(mAudioSwrCtx);
    mAudioBuf = (unsigned char*)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
    return 0;
}

void AudioPlayer::run()
{
    qDebug() << LOG_ENTER_FUNCTION_AND_LINE << getMiliSecondTimeStamp();

    int ret = 0;
    int len = -1;
    int outSize = -1;
    int64_t sleepTime = 0;  // ms

    while (!mNeedStop)
    {
        if (mIsPause)
        {
            Sleep(PAUSE_SLEEP_TIME);
            continue;
        }
        ret = av_read_frame(mAudioFormatCtx, mAudioPacket);
        if (ret < 0)
        {
            qDebug() << "[ERROR] av_read_frame FAIL. ret=[" << ret << "], err=["
                     << QString(av_err2str(ret))
                     << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
            goto out;
        }
        if (mAudioPacket->stream_index != mAudioStreamIndex)
        {
            av_packet_unref(mAudioPacket);
            continue;
        }
        ret = avcodec_send_packet(mAudioCodecCtx, mAudioPacket);
        if (ret == -1 * EAGAIN || ret == -1 * 1094995529)
        {
            av_packet_unref(mAudioPacket);
            continue;
        }
        if (ret < 0)
        {
            qDebug() << "[ERROR] avcodec_send_packet FAIL. ret=[" << ret << "], err=["
                     << QString(av_err2str(ret)).toStdString().c_str()
                     << "]" << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
            goto out;
        }

        while (avcodec_receive_frame(mAudioCodecCtx, mAudioFrame) >= 0)
        {
            if (!av_sample_fmt_is_planar(mAudioCodecCtx->sample_fmt))
            {
                qDebug() << "[ERROR] av_sample_fmt_is_planar FAIL." << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
                continue;
            }
            len = swr_convert(mAudioSwrCtx, &mAudioBuf, MAX_AUDIO_FRAME_SIZE * 2, mAudioFrame->data, mAudioFrame->nb_samples);
            if (len <= 0)
            {
                qDebug() << "[ERROR] swr_convert FAIL." << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
                continue;
            }
            // find size from map
            outSize = av_samples_get_buffer_size(0,mAudioCodecCtx->ch_layout.nb_channels, len, AV_SAMPLE_FMT_S16, 1);
            if (outSize < 0)
            {
                qDebug() << "[ERROR] av_samples_get_buffer_size FAIL. err=["
                         << QString(av_err2str(outSize)).toStdString().c_str() << "]"
                         << LOG_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
                continue;
            }
            sleepTime = (mAudioCodecCtx->sample_rate * 16 * mAudioCodecCtx->ch_layout.nb_channels / 8) / outSize;   // 16 - AV_SAMPLE_FMT_S16
            if (mAudioOutput->bytesFree() < outSize)
            {
                Sleep(sleepTime);
            }
            else
            {

            }
            mAudioIODevice->write((char*)mAudioBuf, outSize);

            gTimeDiffMut.lock();
            mAudioTimeBase = mAudioFrame->pts * av_q2d(mAudioFormatCtx->streams[mAudioStreamIndex]->time_base) * 1000;
            gTimeDiffMut.unlock();
        }
        av_packet_unref(mAudioPacket);
    }
out:
    // release source
    releaseRunParam();
    qDebug() << LOG_LEAVE_FUNCTION_AND_LINE << getMiliSecondTimeStamp();
    return;
}






