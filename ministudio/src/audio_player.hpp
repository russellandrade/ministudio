/*
 * audio_player.hpp
 *
 *  Created on: Jan 6, 2014
 *      Author: randrade
 */

#ifndef AUDIO_PLAYER_HPP_
#define AUDIO_PLAYER_HPP_

#include <QtCore/QObject>
#include <QtCore/QMetaType>

#include <bb/cascades/Application>
#include <bb/cascades/multimedia/Camera>
#include <bb/cascades/Button>
#include <QBuffer>
#include <buffermgr.hpp>
#include <sys/asoundlib.h>
#include <pthread.h>
#include <audio_slider.hpp>
#include <audio_timer.hpp>
#include <tapemanager.hpp>


using namespace bb::cascades;


#define AUDIO_PLAYER_ERROR -1
#define AUDIO_PLAYER_SUCCESS 1

#define PLAYER_MAX_ASYNC_FRAGS  400


class AudioPlayer : public QObject
{
Q_OBJECT


public :
	AudioPlayer(Audiotimer *time, BufferManager *buffer, AudioSlider *slider, TapeMgr *tapemgr);
	~AudioPlayer();

	Q_INVOKABLE int setup();

	Q_INVOKABLE int config(int activeRecordTrack);
	Q_INVOKABLE void start();
	Q_INVOKABLE int play();
	Q_INVOKABLE int pause();
	Q_INVOKABLE int stop();
	Q_INVOKABLE int reset();
	Q_INVOKABLE int cleanup();
	Q_INVOKABLE int resume();
	Q_INVOKABLE bool isplaying() {return m_play; }
	Q_INVOKABLE bool ispaused() {return m_pause; }

	int playloop();

	void setPause(bool pause);



private:

	snd_pcm_t *m_pcm_playback_handle;
	bool m_play;
	int m_frameSize;
	bool m_pause;
	int m_playbackMode;
	pthread_attr_t m_attr_p;
	pthread_t m_playerthread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
    Audiotimer *m_timer;
    BufferManager *m_playerBuffer;
	AudioSlider *m_slider;
	TapeMgr *m_tapemgr;

};



#endif /* AUDIO_PLAYER_HPP_ */
