#ifndef PLAYER_H__
#define PLAYER_H__

#include <QString>
#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QVector>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioOutput>

extern "C"
{
#include "ffmpeg/include/libavformat/avformat.h"
#include "ffmpeg/include/libavcodec/avcodec.h"
#include "ffmpeg/include/libavutil/avutil.h"
#include "ffmpeg/include/libavutil/imgutils.h"
#include "ffmpeg/include/libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libpostproc/postprocess.h"
}

extern QMutex gImgMut;
extern QImage gImg;

class VideoPlayer : public QThread
{
    Q_OBJECT
public:
    VideoPlayer(bool& _mIsPause, double &_mAudioTimeBase, double &_mVideoTimeBase, double &_mTimeDiff, bool &_mNeedStop, QString &_mFilePath, QWidget &_mMainWindow, QThread &_mParent);
    ~VideoPlayer();

    void run() override;
    int InitRunParam();
    void releaseRunParam();

    double &mTimeDiff;
    double &mAudioTimeBase;
    double &mVideoTimeBase;
    bool &mIsPause;
    bool &mNeedStop;
    QString &mFilePath;
    QWidget &mMainWindow;
    QThread &mParent;

    /*************************************************** run param ************************************************************/
    int mVideoStreamIndex;
    unsigned char* mVideoBuf;
    AVFormatContext *mVideoFormatCtx;
    AVCodecContext *mVideoCodecCtx;
    AVCodec *mVideoCodec;
    AVPacket *mVideoPacket;
    AVFrame *mVideoFrame, *mVideoFrameRGB;
    struct SwsContext *mVideoSwsCtx;

signals:
    void sigResizeWindows(int, int);

};

class AudioPlayer : public QThread
{
public:
    AudioPlayer(bool& _mIsPause, double &_mAudioTimeBase, bool &_mNeedStop,
                QString &_mFilePath, QWidget &_mMainWindow, QThread &_mParent, double &_mAudioVolume);
    ~AudioPlayer();

    void run() override;
    void InitAudioFormat(QAudioFormat &_audioFormat);
    int InitRunParam();
    void releaseRunParam();

    double &mAudioTimeBase;
    bool &mIsPause;
    bool &mNeedStop;
    QString &mFilePath;
    QWidget &mMainWindow;
    QThread &mParent;
    double &mAudioVolume;

    /*************************************************** run param ************************************************************/
    QAudioFormat *mAudioFormat;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIODevice;
    AVFormatContext *mAudioFormatCtx;
    AVCodecContext *mAudioCodecCtx;
    AVCodec *mAudioCodec;
    SwrContext *mAudioSwrCtx;
    AVPacket *mAudioPacket;
    AVFrame *mAudioFrame;
    unsigned char* mAudioBuf;
    int mAudioStreamIndex;
};

class Player : public QThread
{
    Q_OBJECT
public:
    Player(QWidget &_mMainWindow);
    virtual ~Player();

    void Start(const QString _filePath);
    void run() override;

    VideoPlayer *mVideoPlayer = nullptr;
    AudioPlayer *mAudioPlayer = nullptr;

    QString mFilePath;
    bool mIsPause;
    bool mNeedStop;
    double mVideoTimeBase;
    double mAudioTimeBase;
    double mTimeDiff;
    QWidget &mMainWindow;
    double mAudioVolume;

    // =========================================================video run param==================================================================
//    int mVideoStreamIndex;
//    unsigned char* mVideoBuf;
//    AVFormatContext *mVideoFormatCtx;
//    AVCodecContext *mVideoCodecCtx;
//    AVCodec *mVideoCodec;
//    AVPacket *mVideoPacket;
//    AVFrame *mVideoFrame, *mVideoFrameRGB;
//    struct SwsContext *mVideoSwsCtx;

    // =========================================================audio run param==================================================================
//    QAudioOutput *mAudioOutput;
//    QIODevice *mAudioIODevice;
//    AVFormatContext *mAudioFormatCtx;
//    AVCodecContext *mAudioCodecCtx;
//    AVCodec *mAudioCodec;
//    SwrContext *mAudioSwrCtx;
//    AVPacket *mAudioPacket;
//    AVFrame *mAudioFrame;
//    unsigned char* mAudioBuf;
};



#endif  // PLAYER_H__
