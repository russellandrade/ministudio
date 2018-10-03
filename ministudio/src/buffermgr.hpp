/*
 * BufferManager.h
 *
 *  Created on: Nov 20, 2013
 *      Author: randrade
 */

#ifndef BUFFERMGR_HPP_
#define BUFFERMGR_HPP_

#include <bb/cascades/Application>
#include <bb/cascades/multimedia/Camera>
#include <bb/cascades/Button>
#include <bb/cascades/DropDown>
#include <QBuffer>
#include <QByteArray>
#include <QString>
#include <track.hpp>
#include <audio_timer.hpp>
#include <recorderdefines.h>

using namespace bb::cascades;

#define NUM_FRAMES  20
#define MAX_SONG_LENGTH_IN_MIN  5
#define MAX_SONG_LENGTH_IN_SEC (60 * MAX_SONG_LENGTH_IN_MIN)
#define MAX_SONG_LENGTH (SAMPLE_RATE*MAX_SONG_LENGTH_IN_SEC*2 * NUM_PCM_VOICES)

#define BUFFER_MANAGER_SUCCESS  0
#define BUFFER_MANAGER_FAILURE -1



class BufferManager : public QObject {
Q_OBJECT
public :
	BufferManager(const bb::cascades::AbstractPane *root);
	~BufferManager();
	int write(char* src, TBuffer*writeBuffer, int bytesToWrite);
	int read(int bytestoRead);
	Q_INVOKABLE qint64 size();
	Q_INVOKABLE void setPosition(int pos, int trackId);
	Q_INVOKABLE int save(QString projectName);
	Q_INVOKABLE bool validateName(QString projectName);
	Q_INVOKABLE int restore(int index);
	Q_INVOKABLE bool isActiveProject();
	Q_INVOKABLE void selectProject(QString text);
	Q_INVOKABLE void deleteProject(QString projectName);
	void clear();
	int combine();
	TBuffer *getActiveRecordBuffer();
	TBuffer *getActivePlayBuffer();
	Q_INVOKABLE void populateProjectDropDown();
	Q_INVOKABLE void initProjectList();
	qint64 saveTrack(int trackId, QString projectName);
	qint64 restoreTrack(int trackId, QString projectName);
	qint64 getTrackSize(int trackId, QString projectName);
	void getProjectName(QString &projectName);
	qint64 enableTrack(int trackId);
	qint64 disableTrack(int trackId);
	void setTrack(Track *track) {m_track = track; }
	void combine(char *dst, char *src, int size);
	//void remix(bool remix) { m_remix = remix; }
	int remix(QString prjName, int recordTrack);
	bool needsremix() { return m_remix; }




	signals:
	    void restoreDone(int size);
private :

	int m_frameSize;
    QList<QString> m_content;
    QDeclarativePropertyMap* m_targetMap;

    DropDown *m_dropdown;

	TBuffer *m_mixedBuffer;
	//buffers for each track
	TBuffer *m_trackbuffer[MAX_NUM_TRACKS];
	char *current;
	Track *m_track;
	bool m_remix;

};

#endif

