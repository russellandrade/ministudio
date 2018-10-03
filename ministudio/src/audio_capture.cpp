/*
 * audio_capture.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: randrade
 */
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <screen/screen.h>
#include <assert.h>

#include <math.h>
#include <time.h>
#include <screen/screen.h>
#include <pthread.h>

#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <gulliver.h>
#include <bps/bps.h>
#include <bps/audiomixer.h>
#include <audio_capture.hpp>




AudioRecorder::AudioRecorder(Track *track, Audiotimer *time, BufferManager *buffer, AudioSlider *slider)
:m_capture(0),
 m_frameSize(0),
 m_recordBuffer(buffer),
 m_track(track),
 m_timer(time),
 m_slider(slider)
{

    m_pcm_capture_handle = 0;


}

AudioRecorder::~AudioRecorder()
{

}


////////////////////////////////////////////////////////////////////////////////////////////
// setup the capture channel
//
////////////////////////////////////////////////////////////////////////////////////////////

int AudioRecorder::setup() {

	snd_pcm_channel_setup_t pcm_channel_setup;
	int ret;
	snd_pcm_channel_info_t pcm_channel_info;
	snd_mixer_group_t mixer_group;
	snd_pcm_channel_params_t pcm_channel_params;
	//int card = pcm_channel_setup.mixer_card;

	if ((ret = snd_pcm_open_name(&m_pcm_capture_handle, "voice", SND_PCM_OPEN_CAPTURE )) < 0) {
		qDebug() << "snd_pcm_open_name failed:" << snd_strerror (ret);
		return AUDIO_RECORDER_ERROR;
	}

	//disable some plugins

    if ((ret = snd_pcm_plugin_set_disable (m_pcm_capture_handle, PLUGIN_MMAP)) < 0) {

    	qDebug() << "Unable to disable mmap";
        snd_pcm_close(m_pcm_capture_handle);
        return AUDIO_RECORDER_ERROR;
    }

    if ((ret = snd_pcm_plugin_set_enable (m_pcm_capture_handle, PLUGIN_ROUTING)) < 0)
    {
    	qDebug() << "snd_pcm_plugin_set_enable failed: " << snd_strerror(ret);
        snd_pcm_close(m_pcm_capture_handle);
        return AUDIO_RECORDER_ERROR;
    }



	// sample reads the capabilities of the capture
	memset(&pcm_channel_info, 0, sizeof(pcm_channel_info));
	pcm_channel_info.channel = SND_PCM_CHANNEL_CAPTURE;
	if ((ret = snd_pcm_plugin_info(m_pcm_capture_handle, &pcm_channel_info)) < 0) {
		qDebug() << "snd_pcm_plugin_info failed: " << snd_strerror(ret);
		snd_pcm_close(m_pcm_capture_handle);
		return AUDIO_RECORDER_ERROR;
	}

	qDebug () << "CAPTURE Minimum Rate = " << pcm_channel_info.min_rate;

	//inform the device of the format of the data we are capturing
	memset(&pcm_channel_params, 0, sizeof(pcm_channel_params));
	qDebug () << "CAPTURE Minimum fragment size = " << pcm_channel_info.min_fragment_size;

	// Blocking read
	pcm_channel_params.channel = SND_PCM_CHANNEL_CAPTURE;

	pcm_channel_params.mode = SND_PCM_MODE_BLOCK;
	//data format.
	pcm_channel_params.format.interleave = 1;
	pcm_channel_params.format.rate = SAMPLE_RATE;  //high quality optimal sample rate
	pcm_channel_params.format.voices = NUM_PCM_VOICES;  //stereo
	pcm_channel_params.format.format = SND_PCM_SFMT_S16_LE;  //signed 16-bit little endian
	pcm_channel_params.start_mode = SND_PCM_START_DATA;
	// Auto-recover from errors
	pcm_channel_params.stop_mode = SND_PCM_STOP_STOP;  //stop when error occurs
	pcm_channel_params.buf.block.frag_size = pcm_channel_info.max_fragment_size;
	pcm_channel_params.buf.block.frags_max = 3;
	pcm_channel_params.buf.block.frags_min = 1;


	// make the request
	if ((ret = snd_pcm_plugin_params(m_pcm_capture_handle, &pcm_channel_params)) < 0) {
		qDebug() << "ca snd_pcm_plugin_params failed:" << snd_strerror (ret);
		snd_pcm_close(m_pcm_capture_handle);
		return AUDIO_RECORDER_ERROR;
	}

	// Again based on the sample
	memset(&pcm_channel_setup, 0, sizeof(pcm_channel_setup));
	memset(&mixer_group, 0, sizeof(mixer_group));
	pcm_channel_setup.channel = SND_PCM_CHANNEL_CAPTURE;
	pcm_channel_setup.mixer_gid = &mixer_group.gid;
	if ((ret = snd_pcm_plugin_setup(m_pcm_capture_handle, &pcm_channel_setup)) < 0) {
		qDebug() << "snd_pcm_plugin_setup failed:" << snd_strerror (ret);
		snd_pcm_close(m_pcm_capture_handle);
		return AUDIO_RECORDER_ERROR;
	}
	// On the simulator at least, our requested capabilities are accepted.
	//qDebug () <<"CAPTURE Format card = " << snd_pcm_get_format_name (pcm_channel_setup.format.card);
	qDebug () << "CAPTURE Rate" <<  pcm_channel_setup.format.rate;

	if (mixer_group.gid.name[0] == 0) {

		qDebug () << "Mixer Pcm Group not set" << mixer_group.gid.name;
		qDebug () << "*** Input gain controls disabled";
	} else {
		qDebug () << "Mixer Pcm Group" << mixer_group.gid.name;
	}


	m_frameSize = pcm_channel_setup.buf.block.frag_size;
	qDebug () << "CAPTURE frame_size = " << m_frameSize;

	// Sample calls prepare()
	if ((ret = snd_pcm_plugin_prepare(m_pcm_capture_handle, SND_PCM_CHANNEL_CAPTURE))
			< 0) {
		qDebug () << "snd_pcm_plugin_prepare failed" << snd_strerror(ret);
		snd_pcm_close(m_pcm_capture_handle);
		return AUDIO_RECORDER_ERROR;
	}



	return AUDIO_RECORDER_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////
// cleanup audio
//
////////////////////////////////////////////////////////////////////////////////////////////
int AudioRecorder::cleanup()
{
	int error;
	qDebug() << "CAPTURE EXIT BEGIN";
	error = snd_pcm_plugin_flush(m_pcm_capture_handle, SND_PCM_CHANNEL_CAPTURE);

	if ( error != 0) {
		qDebug() << "PCM flush failed " << error;
		return AUDIO_RECORDER_ERROR;
	}

	error =  snd_pcm_close(m_pcm_capture_handle);
	if ( error != 0) {
		qDebug() << "PCM close failed " << error;
		return AUDIO_RECORDER_ERROR;
	}

    return 0;

}

int AudioRecorder::captureloop()
{

	// Re-usable buffer for capture
	char *record_temp_buffer = (char*) malloc(m_frameSize);
	int totalRead = 0;
	// Some diagnostic variables
	int failed = 0;
	snd_pcm_channel_status_t status;
	status.channel = SND_PCM_CHANNEL_CAPTURE;


	qDebug() << "capture a channel";

	if ((!m_recordBuffer) || (!record_temp_buffer)) {
		qDebug() <<"alloc failure";
		if (record_temp_buffer) {
		    free(record_temp_buffer);
		}
		return AUDIO_RECORDER_ERROR;
	}



	int pos = m_slider->getReadPos();
	int trackId = m_track->getActiveRecordTrack();
	m_recordBuffer->setPosition(pos, trackId);
	TBuffer *writebuffer = m_recordBuffer->getActiveRecordBuffer();


	// Loop until stopAudio() flags us
	while (m_capture) {
		// This blocking read appears to take much longer than 20ms on the simulator
		// but it never fails and always returns 160 bytes
		int read = snd_pcm_plugin_read(m_pcm_capture_handle, record_temp_buffer, m_frameSize);

		if (read < 0 || read != m_frameSize) {
			failed++;

			qDebug() << "CAPTURE FAILURE: snd_pcm_plugin_read:" << read <<" requested =" << m_frameSize;
			if (snd_pcm_plugin_status(m_pcm_capture_handle, &status) < 0) {

				qDebug() << "Capture channel status error : " << status.status;
			} else {
				if (status.status == SND_PCM_STATUS_READY
						|| status.status == SND_PCM_STATUS_OVERRUN
						|| status.status == SND_PCM_STATUS_CHANGE
						|| status.status == SND_PCM_STATUS_ERROR) {
					qDebug() << "CAPTURE FAILURE: snd_pcm_plugin_status: = " << status.status;
					if (snd_pcm_plugin_prepare (m_pcm_capture_handle, SND_PCM_CHANNEL_CAPTURE) < 0) {
						qDebug() << "CAPTURE EXIT BEGIN" << status.status;
						break;
					}

				}
			}

		} else {
			totalRead += read;

			if (m_recordBuffer->write(record_temp_buffer, writebuffer, m_frameSize)) {
				m_timer->updateByteCount(writebuffer->currpos);
			}
			else {
			    failed++;
			}
		}
	}

	//save track
    QString projectName;
    m_recordBuffer->getProjectName(projectName);
    //save track to file
    m_recordBuffer->saveTrack(trackId, projectName);


	cleanup();

	// IMPORTANT NB: You only get failed on capture if the play loop has exited hence the circular buffer fills. This is with the simulator
	qDebug() << "CAPTURE EXIT Total Bytes read = " << totalRead << "failed =" << failed;


	free(record_temp_buffer);
	return 0;

}


int AudioRecorder::stop()
{
	fprintf(stderr,"stop capture\n");
	m_capture = false;

    //recording has stopped

    m_timer->stop();
    fprintf(stderr,"updating slider\n");
    m_timer->single();


	return 0;
}


//helper function
static void *captureThreadHdlr(void *arg)
{
	qDebug() << "capture thread handler";
	((AudioRecorder *)arg)->captureloop();
	return (0);
}

void AudioRecorder::start()
{
    int policy;
    struct sched_param param;

    //capture is initiated from a new thread
    m_capture = true;

    pthread_attr_init (&m_attr_p);
    pthread_attr_setdetachstate (&m_attr_p, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched (&m_attr_p, PTHREAD_EXPLICIT_SCHED);
    pthread_getschedparam (pthread_self (), &policy, &param);
    param.sched_priority=12;
    pthread_attr_setschedparam (&m_attr_p, &param);
    pthread_attr_setschedpolicy (&m_attr_p, SCHED_RR);
    pthread_create(&m_capturethread,&m_attr_p, &(captureThreadHdlr), this);
    m_timer->start(TIMER_MODE_RECORD);

}

int AudioRecorder::config()
{
    TBuffer *recBuff = m_recordBuffer->getActiveRecordBuffer();
    QString projectName;
    m_recordBuffer->getProjectName(projectName);

	qDebug() << " setup capture";
	setup();

    //update alignment on slider
    m_slider->setalignment(m_frameSize);

    //configure active buffer
    m_timer->setActiveBuffer(recBuff);

    //need to read out data from original recording into track first
    m_recordBuffer->restoreTrack(m_track->getActiveRecordTrack(), projectName);

	qDebug() << "start capture";


    return 0;

}


