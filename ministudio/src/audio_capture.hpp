/*
 * audio_capture.hpp
 *
 *  Created on: Feb 3, 2015
 *      Author: Russell
 */

#ifndef AUDIO_CAPTURE_HPP_
#define AUDIO_CAPTURE_HPP_

#include <QtCore/QObject>
#include <QtCore/QMetaType>

#include <bb/cascades/Application>


#include <QBuffer>
#include <buffermgr.hpp>
#include <sys/asoundlib.h>
#include <pthread.h>
#include <bb/cascades/Page>
#include <bb/cascades/QmlDocument>
#include <track.hpp>
#include <audio_timer.hpp>
#include <audio_slider.hpp>


using namespace bb::cascades;


#define AUDIO_RECORDER_ERROR -1
#define AUDIO_RECORDER_SUCCESS 1

#define PCM_SAMPLE_RATE 48000
#define BYTES_PER_SAMPLE 2
#define NUM_VOICES     2

class AudioRecorder : public QObject
{
Q_OBJECT


public :
    AudioRecorder(Track *track, Audiotimer *time, BufferManager *buffer, AudioSlider *slider);
    ~AudioRecorder();

    Q_INVOKABLE int setup();
    Q_INVOKABLE int cleanup();
    Q_INVOKABLE int config();
    Q_INVOKABLE void start();
    Q_INVOKABLE int stop();
    Q_INVOKABLE bool iscapturing() { return m_capture; }

    void setTrackObject(Track *track) { m_track = track; }

    //static void *captureThreadHdlr(void *arg);
    int captureloop();


private:

    snd_pcm_t *m_pcm_capture_handle;
    bool m_capture;
    int m_frameSize;
    pthread_attr_t m_attr_p;
    pthread_t m_capturethread;
    BufferManager *m_recordBuffer;
    Audiotimer *m_timer;
    Track *m_track;
    AudioSlider *m_slider;

};




#endif /* AUDIO_CAPTURE_HPP_ */
