/* audio_player.cpp
 *
 *  Created on: Jan 6, 2014
 *      Author: randrade
 */



#include <pthread.h>

#include <bps/audiomixer.h>
#include <audio_player.hpp>
#include <tapemanager.hpp>
#include <audio/audio_manager_device.h>


AudioPlayer::AudioPlayer(Audiotimer *time, BufferManager *buffer, AudioSlider *slider, TapeMgr *tapemgr)
:
m_pcm_playback_handle(0),
m_play(false),
m_frameSize(0),
m_pause(false),
m_playbackMode(0),
m_timer(time),
m_playerBuffer(buffer),
m_slider(slider),
m_tapemgr(tapemgr)
{

    m_playerthread = 0;

}

AudioPlayer::~AudioPlayer()
{


}


int AudioPlayer::setup()
{
	snd_pcm_channel_setup_t pcm_channel_setup;
	//Enabling Echo canceller
	int ret;
	snd_pcm_channel_info_t pcm_channel_info;
	snd_mixer_group_t pcm_mixer_group;
	snd_pcm_channel_params_t pcm_channel_params;


	if ((ret = snd_pcm_open_name(&m_pcm_playback_handle, "pcmPreferred", SND_PCM_OPEN_PLAYBACK )) < 0) {
		qDebug() << "snd_pcm_open_name failed:" << snd_strerror (ret);
		return AUDIO_PLAYER_ERROR;
	}

	if ((ret = snd_pcm_plugin_set_disable (m_pcm_playback_handle, PLUGIN_DISABLE_MMAP)) < 0) {
	   	qDebug() << "Unable to disable mmap";
	    snd_pcm_close(m_pcm_playback_handle);
	    return AUDIO_PLAYER_ERROR;
	}


    if (( ret = snd_pcm_plugin_set_enable(m_pcm_playback_handle, PLUGIN_ROUTING)) < 0 ) {
    	qDebug() << "snd_pcm_plugin_set_enable failed: " << snd_strerror(ret);
    	snd_pcm_close(m_pcm_playback_handle);
    	return AUDIO_PLAYER_ERROR;
    }


	memset(&pcm_channel_info, 0, sizeof(pcm_channel_info));
	pcm_channel_info.channel = SND_PCM_CHANNEL_PLAYBACK;
	if ((ret = snd_pcm_plugin_info(m_pcm_playback_handle, &pcm_channel_info)) < 0) {
		qDebug() << "snd_pcm_plugin_info failed:" << snd_strerror(ret);
		snd_pcm_close(m_pcm_playback_handle);
		return AUDIO_PLAYER_ERROR;
	}

	qDebug() << "PLAY Minimum Rate = %d\n" << pcm_channel_info.min_rate;
	// Interestingly on the simulator this returns 4096 but in reality always 170 is the result
	qDebug() <<"PLAY Minimum fragment size = " << pcm_channel_info.min_fragment_size;

	memset(&pcm_channel_params, 0, sizeof(pcm_channel_params));

	// Request VoIP compatible capabilities
	// On simulator frag_size is always negotiated to 170


	pcm_channel_params.mode = SND_PCM_MODE_BLOCK;
	pcm_channel_params.channel = SND_PCM_CHANNEL_PLAYBACK;
	pcm_channel_params.stop_mode = SND_PCM_STOP_STOP;
	pcm_channel_params.start_mode = SND_PCM_START_FULL;
	pcm_channel_params.buf.block.frag_size = pcm_channel_info.max_fragment_size;
	// Increasing this internal buffer count delays write failure in the loop
	pcm_channel_params.buf.block.frags_max = PLAYER_MAX_ASYNC_FRAGS;
	pcm_channel_params.buf.block.frags_min = 1;
	pcm_channel_params.format.interleave = 1;
	pcm_channel_params.format.rate = SAMPLE_RATE;
	pcm_channel_params.format.voices = NUM_PCM_VOICES;
	pcm_channel_params.format.format = SND_PCM_SFMT_S16_LE;



	// Make the calls as per the wave sample
	if ((ret = snd_pcm_plugin_params(m_pcm_playback_handle, &pcm_channel_params)) < 0) {
		qDebug() << "pb snd_pcm_plugin_params failed:" << snd_strerror(ret);
		snd_pcm_close(m_pcm_playback_handle);
		return AUDIO_PLAYER_ERROR;
	}



	memset(&pcm_channel_setup, 0, sizeof(pcm_channel_setup));
	memset(&pcm_mixer_group, 0, sizeof(pcm_mixer_group));
	pcm_channel_setup.channel = SND_PCM_CHANNEL_PLAYBACK;
	pcm_channel_setup.mixer_gid = &pcm_mixer_group.gid;
	if ((ret = snd_pcm_plugin_setup(m_pcm_playback_handle, &pcm_channel_setup)) < 0) {
		qDebug() << "snd_pcm_plugin_setup failed:" << snd_strerror(ret);
		snd_pcm_close(m_pcm_playback_handle);
		return AUDIO_PLAYER_ERROR;
	}

	qDebug() << "PLAY frame_size " << pcm_channel_setup.buf.block.frag_size;
	qDebug() << "PLAY Rate %d " << pcm_channel_setup.format.rate;
	m_frameSize = pcm_channel_setup.buf.block.frag_size;

	if (pcm_mixer_group.gid.name[0] == 0) {
		qDebug() << "FATAL Mixer Pcm Group " << pcm_mixer_group.gid.name << "Not Set ";
		snd_pcm_close(m_pcm_playback_handle);
		return AUDIO_PLAYER_ERROR;
	}
	qDebug() << "Mixer Pcm Group" << pcm_mixer_group.gid.name;
	if ((ret = snd_pcm_plugin_prepare(m_pcm_playback_handle, SND_PCM_CHANNEL_PLAYBACK))
			< 0) {
		qDebug() << "snd_pcm_plugin_prepare failed: " << snd_strerror (ret);
		snd_pcm_close(m_pcm_playback_handle);
		return AUDIO_PLAYER_ERROR;
	}


	return AUDIO_PLAYER_SUCCESS;

}

int AudioPlayer::reset() {
	snd_pcm_channel_status_t pcm_status;
	pcm_status.channel = SND_PCM_CHANNEL_PLAYBACK;
	if (snd_pcm_plugin_status(m_pcm_playback_handle, &pcm_status) < 0) {
		qDebug() << "FATAL Playback channel status error " << pcm_status.status;
		return AUDIO_PLAYER_ERROR;
	}


	if (pcm_status.status == SND_PCM_STATUS_READY
			|| pcm_status.status == SND_PCM_STATUS_UNDERRUN
			|| pcm_status.status == SND_PCM_STATUS_CHANGE
			|| pcm_status.status == SND_PCM_STATUS_ERROR) {
		qDebug () << "PLAYBACK FAILURE:snd_pcm_plugin_status: =" << pcm_status.status;
		if (snd_pcm_plugin_prepare (m_pcm_playback_handle, SND_PCM_CHANNEL_PLAYBACK) < 0) {
			qDebug() << "FATAL Playback channel prepare error" << pcm_status.status;
			return AUDIO_PLAYER_ERROR;
		}
	}

	return AUDIO_PLAYER_SUCCESS;
}

int AudioPlayer::cleanup()
{
	int error;
	qDebug() << "PLAYER CLEANUP BEGIN";
	error = snd_pcm_plugin_flush(m_pcm_playback_handle, SND_PCM_CHANNEL_PLAYBACK);

	if ( error != 0) {
		qDebug() << "PCM flush failed " << error;
		return AUDIO_PLAYER_ERROR;
	}

	error =  snd_pcm_close(m_pcm_playback_handle);
	if ( error != 0) {
		qDebug() << "PCM close failed " << error;
		return AUDIO_PLAYER_ERROR;
	}

	return 0;

}


int AudioPlayer::playloop()
{
	int bytes_read = 0;
	int failed = 0;
	int written;
	int bytes_to_write;
	int bytesinBuffer = m_playerBuffer->size();
	TBuffer *playbuff = m_playerBuffer->getActivePlayBuffer();


	if ( !playbuff) return 0;

	qDebug() << "playloop buffer has size " << bytesinBuffer;


	int pos = m_slider->getReadPos();

	m_playerBuffer->setPosition(pos, 0);

	// play until we have read out the buffer or the stop button was hit
	while ((m_play) && (pos < bytesinBuffer)) {

#if PAUSE_CODE

	    //if we paused the player, wait until condition is set
	    pthread_mutex_lock(&mutex);
	    while (m_pause) {
	        m_timer->stop();
	        pthread_cond_wait(&cond, &mutex);
	        m_timer->start(TIMER_MODE_PLAY);
	    }
	    pthread_mutex_unlock( &mutex );
#endif
	    // in async mode send multiples of framesize bytes to the audio driver to avoid jitter when recording
	    if ( m_playbackMode ) {
	        bytes_to_write = bytesinBuffer-pos;
	        if (bytes_to_write > m_frameSize * PLAYER_MAX_ASYNC_FRAGS) {
	            bytes_to_write = m_frameSize * PLAYER_MAX_ASYNC_FRAGS;

	        }
	    }
	    else {

	        //in symc mode send framesize bytes
	        bytes_to_write = m_frameSize;
	        m_timer->updateByteCount(pos);

	    }



	    // Read the circular buffer
	    // returns true only if there is data to satisfy frame_size
	    bytes_read = m_playerBuffer->read(  bytes_to_write);

	    if (bytes_read == bytes_to_write) {
	        written = snd_pcm_plugin_write(m_pcm_playback_handle, &playbuff->buff[pos],
	                bytes_to_write);
	        if (written < 0 || written != bytes_to_write) {
	            qDebug() << "PLAY RESET";
	            //need to prepare channel for transfer
	            reset();
	        }else{

	            pos += written;
	        }

	    } else {
	        //
	        qDebug() << "bytes_read != bytes to write";

	        failed++;
	        break;

	    }
	}

	qDebug () << "PLAY EXIT BEGIN";

	//in record mode, player does not set animation and time so no need to do anything

	if ( !m_playbackMode ) {
	    m_timer->updateByteCount(pos);
	    m_tapemgr->setanimate(false);
	    m_timer->stop();
	}



	cleanup();

	qDebug() << "Play Exit :  Total Bytes played " << pos;


	m_play = false;

	return AUDIO_PLAYER_SUCCESS;

}



static void *playerThreadHdlr(void *arg)
{
	qDebug() << "player thread handler";
	((AudioPlayer *)arg)->playloop();
	return (0);
}

void AudioPlayer::setPause(bool pause)
{
    //initialize mutex and conditional variable
    mutex = PTHREAD_MUTEX_INITIALIZER;
    cond = PTHREAD_COND_INITIALIZER;

    pthread_mutex_lock( &mutex );
    m_pause = pause;
    pthread_mutex_unlock( &mutex );

}
void AudioPlayer::start()
{
    //play is initiated from a new thread
    int policy;
    struct sched_param param;

    if (!m_play) return;

    qDebug() << "start playing";

    pthread_attr_init (&m_attr_p);
    pthread_attr_setdetachstate (&m_attr_p, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched (&m_attr_p, PTHREAD_EXPLICIT_SCHED);
    pthread_getschedparam (pthread_self (), &policy, &param);
    int min = sched_get_priority_min(SCHED_FIFO);
    int max = sched_get_priority_max(SCHED_FIFO);
    qDebug("schedul priority min = %d, max = %d", min, max);

    param.sched_priority=12;
    pthread_attr_setschedparam (&m_attr_p, &param);
    pthread_attr_setschedpolicy (&m_attr_p, SCHED_RR);

    pthread_create(&m_playerthread, &m_attr_p, &(playerThreadHdlr), this);

}

int AudioPlayer::config(int activeRecordTrack)
{

    if (m_play) return 0;


    qDebug() << " Setting up playing, active record track = "<< activeRecordTrack;


    QString projectName;
    m_playerBuffer->getProjectName(projectName);

#if 0
    //only do a remix if a headset is connected
    audio_manager_is_device_connected(AUDIO_DEVICE_HEADSET, &connected);

    if ( !connected ) {
        qDebug() <<"headset not connected";
        m_play = false;
        return 0;
    }
#endif

    m_play = true;

    setPause(false);

    int mixCount = m_playerBuffer->remix(projectName, activeRecordTrack);

    if ( mixCount ) {
        m_playbackMode = 1;
        setup();
    }
    else {
        m_play = false;
        m_playbackMode = 0;
    }

    return mixCount;

}

int AudioPlayer::play()
{
    // if audio is already playing, then do nothing
    if (m_play) return 0;

    m_play = true;
    m_playbackMode = 0; //sync mode

    setPause(false);

    QString projectName;
    m_playerBuffer->getProjectName(projectName);


    m_playerBuffer->remix(projectName, 0);

	setup();


	m_slider->setalignment(m_frameSize);
	m_timer->setActiveBuffer(m_playerBuffer->getActivePlayBuffer());



	start();
	m_tapemgr->setanimate(true);
	m_timer->start(TIMER_MODE_PLAY);

    return AUDIO_PLAYER_SUCCESS;
}

int AudioPlayer::pause()
{
    if ((!m_play) || (m_pause)) {
        return 0;
    }


    qDebug() << "pause playback";
    setPause(true);

    m_tapemgr->setanimate(false);

    return AUDIO_PLAYER_SUCCESS;
}

int AudioPlayer::stop()
{

    // if it is paused, first unpause
    if (m_pause) {
        resume();
    }
    fprintf(stderr,"stop playback\n");

    m_play = false;

    if ( m_playbackMode ) {
        if ( snd_pcm_playback_drain(m_pcm_playback_handle) < 0) {
            qDebug() << "cannot drain pcm channel " ;
            return AUDIO_PLAYER_ERROR;
        }

        //the playloop function will exit it's loop and finish the cleanup
    }
    else {
        m_tapemgr->setanimate(false);
    }


    return AUDIO_PLAYER_SUCCESS;
}

int AudioPlayer::resume()
{
    qDebug() <<"resuming play";

    //first check if player is paused
    if (!m_pause) {
        return AUDIO_PLAYER_ERROR;
    }


    setPause(false);

    m_play = true;
    m_tapemgr->setanimate(true);

    return AUDIO_PLAYER_SUCCESS;
}
