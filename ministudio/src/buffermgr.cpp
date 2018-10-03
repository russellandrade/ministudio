/*
 * BufferManager.cpp
 *
 *  Created on: Nov 20, 2013
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
#include <buffermgr.hpp>
#include <errno.h>
#include <gulliver.h>
#include <track.hpp>
#include <recorderdefines.h>



BufferManager::BufferManager(const bb::cascades::AbstractPane *root)
: m_frameSize(0),
  m_dropdown(0),
  m_mixedBuffer(0),
  m_track(0),
  m_remix(0)
{

    m_dropdown = root->findChild<DropDown*>("projectddown");

    m_mixedBuffer = new TBuffer;
    //allocate a new data buffer
    if ( m_mixedBuffer) {
        m_mixedBuffer->currpos = 0;
        m_mixedBuffer->maxpos = 0;
        m_mixedBuffer->buff = new char[MAX_SONG_LENGTH];
        if ( !m_mixedBuffer->buff) {
            qDebug() <<"cannot allocate memory for mixed buffer";
        }
    }
    else {
        qDebug() <<"cannot allocate mix buffer";
    }

    for (int i = 0; i < Track::mNumTracks; i++ ) {
        m_trackbuffer[i] = new TBuffer;
        if ( m_trackbuffer[i] ) {
            m_trackbuffer[i]->currpos = 0;
            m_trackbuffer[i]->maxpos = 0;
            m_trackbuffer[i]->buff = 0;

        }
        else {
            qDebug() <<"cannot allocate track buffer";
        }
    }

    current = new char[MAX_SONG_LENGTH];
    if (!current) {
        qDebug() << "cannot allocate buffer";
    }

    //clear all buffers
    clear();

}



BufferManager::~BufferManager()
{
	if ( m_mixedBuffer) {
		qDebug() <<"Deleting data buffer";
		if (m_mixedBuffer->buff) {
		    delete [] m_mixedBuffer->buff;
		}
		delete m_mixedBuffer;
	}

	 for (int i = 0; i < Track::mNumTracks; i++) {
        if (m_trackbuffer[i]) {

            delete m_trackbuffer[i];
        }
    }
	delete [] current;
}

qint64 BufferManager::size()
{

	if ( m_mixedBuffer) {
		return m_mixedBuffer->maxpos;
	}
	else {
		return 0;
	}
}

void BufferManager::setPosition(int pos, int trackId)
{
    if (trackId > Track::mNumTracks) {
        return;
    }

    if ( trackId == 0 ) {
        //position of mixed track
        m_mixedBuffer->currpos = pos;
    }
    else {
        m_trackbuffer[trackId-1]->currpos = pos;
    }

}

qint64 BufferManager::enableTrack(int trackId)
{
    qint64 bytes_read;

    if (trackId > Track::mNumTracks) {
        return 0;
    }
    if (trackId == 0 ) return 0;

    if ( m_trackbuffer[trackId -1] ) {
        m_trackbuffer[trackId-1]->buff = 0;
        m_trackbuffer[trackId-1]->currpos = 0;
        m_trackbuffer[trackId-1]->maxpos = 0;
        QString projectName;
        getProjectName(projectName);
        //reset the track buffer
        memset(current, 0, MAX_SONG_LENGTH);
        bytes_read = restoreTrack(trackId, projectName);

    }

    return bytes_read;

}

qint64 BufferManager::disableTrack(int trackId)
{
    qint64 trackSize;

    if (trackId > Track::mNumTracks) {
        return 0;
    }
    if (trackId == 0 ) return 0;

    if ( m_trackbuffer[trackId -1] ) {
        /*QString projectName;
        getProjectName(projectName);
        //save track to file
        trackSize = saveTrack(trackId, projectName);*/
        trackSize = m_trackbuffer[trackId-1]->maxpos;
        m_trackbuffer[trackId-1]->buff = 0;
        m_trackbuffer[trackId-1]->currpos = 0;
        m_trackbuffer[trackId-1]->maxpos = 0;
        return trackSize;
    }

    return 0;

}


//called by player
//read from mixed buffer
int BufferManager::read(int bytesToRead)
{

	if ( !m_mixedBuffer) {
	    qDebug() << "No data buffer";
	}

	//qDebug("mixed buffer position is %d", m_mixedBuffer->pos);

	//memcpy(src, &m_mixedBuffer->buff[m_mixedBuffer->currpos], bytesToRead);

	//update position
	m_mixedBuffer->currpos += bytesToRead;

	return bytesToRead;
}



//write to temporary buffer
int BufferManager::write(char* src, TBuffer*writeBuffer, int bytesToWrite)
{


	if ( !writeBuffer) {
	    return 0;
	}

	//you cannot override the buffer
	if (writeBuffer->currpos + bytesToWrite > MAX_SONG_LENGTH) {
	    qDebug("Song length exceeded");
	    return 0;
	}

	memcpy(&writeBuffer->buff[writeBuffer->currpos], src, bytesToWrite);

	//update write position
	writeBuffer->currpos += bytesToWrite;

	writeBuffer->maxpos = (writeBuffer->currpos > writeBuffer->maxpos) ? (writeBuffer->currpos) : (writeBuffer->maxpos);


	return bytesToWrite;
}


TBuffer *BufferManager::getActiveRecordBuffer()
{
    int i = m_track->getActiveRecordTrack() -1;

    //bounds check on i
    if (( i < 0) || (i >= 4)) {
        qDebug("no active record track");
        return 0;
    }

    m_trackbuffer[i]->buff = current;

    return m_trackbuffer[i];
}

TBuffer *BufferManager::getActivePlayBuffer()
{
    //always return the mixed buffer
    return m_mixedBuffer;
}

bool BufferManager::validateName(QString fileName)
{

    int size = fileName.size();


    for (int i = 0; i < size; i++ ) {
        //allow numbers, a-z, A-Z and spaces
        if (((fileName[i] >= 0x30) && (fileName[i] <=0x39)) ||
                ((fileName[i] >= 0x41) && (fileName[i] <=0x5a)) ||
                ((fileName[i] >= 0x61) && (fileName[i] <= 0x7a)) ||
                ((fileName[i] == 0x20 ))) {
            continue;
        }
        else {
            return false;
        }
    }
    return true;
}

void BufferManager::getProjectName(QString &projectName)
{
    if (m_dropdown) {
        projectName = m_content.at(m_dropdown->selectedIndex());
        qDebug() <<"Get project Name returns" << projectName;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//  save to secondary storage
//
///////////////////////////////////////////////////////////////////////////////////////////
int BufferManager::save(QString projectName)
{

    QDir dir;
    ProjectItem projectitem;
    VersionItem vitem;

    qDebug() << "Project name is "<< projectName;

    if (projectName.isEmpty()) {
        qDebug("project name is empty");
        return BUFFER_MANAGER_FAILURE;
    }


    QString path(PROJECT_PATH + QString("/") + projectName);


    dir.mkpath(path);
    dir.cd(path);

    //see if project file exists.  if not create one
    if (!dir.exists(projectName + PROJECT_EXTENSION)) {
        QFile projectFile(path + QString("/") + projectName + PROJECT_EXTENSION);
        qDebug("creating project file");
        if ( !projectFile.open(QIODevice::WriteOnly))
        {
            return BUFFER_MANAGER_FAILURE;
        }

        projectitem.item_type = ITEM_TYPE_VERSION;
        projectitem.item_size = sizeof (VersionItem);
        vitem.major_version = MINI_RECORDER_VERSION_MAJOR;
        vitem.minor_version = MINI_RECORDER_VERSION_MINOR;

        projectFile.write((const char *)(&projectitem), sizeof(ProjectItem));
        projectFile.write((const char *)(&vitem), sizeof(VersionItem));

        //processing is done, now close file
       projectFile.close();

    }
    else {
        //project file exists so we have a problem

    }

#if 0
    qDebug() << "max tracks " <<Track::mNumTracks;

    for ( i = 1; i <= Track::mNumTracks; i++ ) {

        saveTrack(i, projectName);
    }
#endif

    return BUFFER_MANAGER_SUCCESS;
}



qint64 BufferManager::saveTrack(int trackId, QString projectName)
{
    int index = trackId-1;
    if (( !m_trackbuffer[index]) || (!m_trackbuffer[index]->buff)) {
        return BUFFER_MANAGER_FAILURE;
    }

    QString path = QString(PROJECT_PATH) + QString("/") + projectName;
    QDir dir;

    qDebug() <<"Path is "<<path;

    dir.mkpath(path);
    dir.cd(path);


    QString fileName = "trackSeq_" + QString::number(trackId) + "_1";

    //see if file already exists.  if it does, delete it

    if ( dir.exists(fileName)) {
        qDebug() << "Deleting file" <<fileName;
        dir.remove(fileName);
    }

    qDebug() <<"Dir path is " <<dir.path();

    QFile audiofile(dir.path() + QString("/") + fileName);

    qDebug() << "creating file" <<fileName;


    if ( !audiofile.open(QIODevice::WriteOnly))
    {
        return BUFFER_MANAGER_FAILURE;
    }


    qint64 fileSize = audiofile.write(m_trackbuffer[index]->buff, m_trackbuffer[index]->maxpos);

    qDebug() << "writing out" << fileSize << "to file" << fileName;

    //processing is done, now close file
    audiofile.close();

    return fileSize;

}

qint64 BufferManager::getTrackSize(int trackId, QString projectName)
{

    QString pathName(PROJECT_PATH + QString("/") + projectName + QString("/"));
    QString fileName = "trackSeq_" + QString::number(trackId) + "_" + QString::number(1);

    qDebug() << "restoring" << pathName<< fileName;

    QFile audiofile(pathName + fileName);

    if ( !audiofile.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file" << fileName;
        audiofile.close();
        //not finding a file track is not an error condition
        //all it means is that we didn't record on that track previously

        return 0;  //0 bytes read
    }

    qint64 fileSize = audiofile.size();
    audiofile.close();
    return fileSize;

}

qint64 BufferManager::restoreTrack(int trackId, QString projectName)
{
    QString pathName(PROJECT_PATH + QString("/") + projectName + QString("/"));
    QString fileName = "trackSeq_" + QString::number(trackId) + "_" + QString::number(1);

    qDebug() << "restoring" << pathName<< fileName;

    QFile audiofile(pathName + fileName);

    if ( !audiofile.open(QIODevice::ReadOnly))
    {
        qDebug() << "cannot find file" << fileName;
        audiofile.close();
        //not finding a file track is not an error condition
        //all it means is that we didn't record on that track previously

        return 0;  //0 bytes read
    }

    qint64 fileSize = audiofile.size();

    if (fileSize > MAX_SONG_LENGTH) {
        qDebug() <<"Cannot restore : file size of "<< fileSize << " is longer than song length";
        audiofile.close();
        return BUFFER_MANAGER_FAILURE;
    }

    qDebug() << "reading out" << fileSize << "from file" << fileName;

    audiofile.read(current, fileSize);

    m_trackbuffer[trackId-1]->buff = current;
    m_trackbuffer[trackId-1]->currpos = fileSize;
    m_trackbuffer[trackId-1]->maxpos = fileSize;

    //processing is done, now close file
    audiofile.close();

    return fileSize;

}

int BufferManager::restore(int index)
{

    QString fileName;
    int i;
    int size = 0, maxSize = 0;

    if ((index < 0) || (index >= m_content.size())) {
        return BUFFER_MANAGER_FAILURE;
    }

    //restore is called once when a new song is loaded
    //clear all content
    clear();


    //check first track to see if it's muted or not
    for ( i = 1; i <= Track::mNumTracks; i++ ) {
        if ( m_track->isTrackRecord(i)) {
            size = restoreTrack(i, m_content.at(index));
        }
        else {
            size = getTrackSize(i, m_content.at(index));
        }
        maxSize = (size > maxSize ) ? size : maxSize;
    }


    if ( size >= 0){
        emit restoreDone(maxSize);
    }

    return BUFFER_MANAGER_SUCCESS;

}


void BufferManager::combine(char *dst, char *src, int size)
{
    qint16 temp1, temp2;
    qint32 tmp_result;
    int overflow_count_pos = 0, overflow_count_neg = 0;

    for (int i = 0; i < size; i+= 2 ) {

        temp1 =  (qint16) ((dst[i] & 0xff) | (dst[i+1] << 8));
        temp2 =  (qint16) ((src[i] & 0xff) | (src[i+1] << 8));


        tmp_result = (qint32)(temp1+temp2);


        //clamp
        if (tmp_result > 32767) {
            overflow_count_pos ++;
            tmp_result = 32767;
        }
        if (tmp_result < -32767) {
            overflow_count_neg ++;
            tmp_result = -32767;
        }

        dst[i] = tmp_result & 0x00ff;
        dst[i+1] = (tmp_result & 0xff00) >> 8;

    }
    qDebug("Overflow count [pos, neg] : %d %d", overflow_count_pos, overflow_count_neg);

}

int BufferManager::remix(QString prjName, int recordTrack)
{
    int mixCount = 0;
    int soloTrack = 0;
    int i;
    qint64 size;

    //clear all tracks
    clear();

    //only one track can be solo, so if we only have one solo track,
    //then just copy that into mixed buffer
    for ( i = 1; i <= Track::mNumTracks; i++ ) {
        if ( m_track->isTrackSolo(i)) {
            soloTrack = i;
            qDebug("Solo track is %d", soloTrack);
        }
    }

    if ( soloTrack ) {
        if ( recordTrack == soloTrack ) {
            qDebug("record track is solo track");
            return 0;
        }
        size = restoreTrack(soloTrack, prjName);
        if (size > 0 ) {
            qDebug() << "Memcpy bytes of data" << size;
            memcpy(m_mixedBuffer->buff, current, size);
            m_mixedBuffer->currpos = size;
            m_mixedBuffer->maxpos = size;
            mixCount++;

        }
    }
    else {
        for ( i = 1; i <= Track::mNumTracks; i++ ) {
            // if the track is muted or the track is being recorded then no remix is necessary
            if (( !m_track->isTrackMuted(i)) && (i != recordTrack)) {
                qDebug("remixing track %d", i);
                size = restoreTrack(i, prjName);
                if (size >  0) {
                    if ( mixCount == 0) {
                        memcpy(m_mixedBuffer->buff, current, size);
                    }
                    else {
                        combine(m_mixedBuffer->buff, current, size);
                    }
                    mixCount++;
                }
                if ( size > m_mixedBuffer->maxpos ) {
                    m_mixedBuffer->currpos = size;
                    m_mixedBuffer->maxpos = size;
                }
            }
        }
    }

    return mixCount;

}


void BufferManager::clear()
{
    qDebug("Clearing buffers");
    if (m_mixedBuffer) {
        memset(m_mixedBuffer->buff, 0, MAX_SONG_LENGTH);
        m_mixedBuffer->currpos = 0;
        m_mixedBuffer->maxpos = 0;
    }

    if (current) {
        memset(current, 0, MAX_SONG_LENGTH);
    }


    //clear all individual buffers

    for (int i = 0; i < Track::mNumTracks; i++ ){
        if ( m_trackbuffer[i] ) {
            //memset(m_trackbuffer[i]->buff, 0, MAX_SONG_LENGTH);
            m_trackbuffer[i]->buff = 0;
            m_trackbuffer[i]->currpos = 0;
            m_trackbuffer[i]->maxpos = 0;
        }
    }

}

void BufferManager::populateProjectDropDown()
{

    if (!m_content.isEmpty()) {

        if(!m_dropdown) {
            qDebug( "WARNING: could not find child element ");
        } else {
            m_dropdown->removeAll();
            for (int i = 0; i < m_content.size(); i++) {
                if ((m_content.at(i)) == "Song Name") {
                    m_dropdown->add(Option::create().text(m_content.at(i)).selected(true));
                } else {
                    m_dropdown->add(Option::create().text(m_content.at(i)));
                }
            }
        }
    } else {
      qDebug("List is empty" );
      if (m_dropdown) {
          //removing all elements from dropdown
          qDebug("removing all elements from dropdown");
          m_dropdown->removeAll();
      }
    }
}



void BufferManager::initProjectList()
{
    QString filepath;
    m_content.clear();
    QDir dir(PROJECT_PATH);


    if (dir.isReadable()) {
        QFileInfoList entries = dir.entryInfoList();
        for (QList<QFileInfo>::iterator it = entries.begin(); it != entries.end(); it++) {
            QFileInfo &fileInfo = *it;
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
                continue;
            }
            //only want to read directories
            if (fileInfo.isDir() && fileInfo.isReadable()) {
                QDir prj_dir;
                prj_dir.cd(fileInfo.path() + QString("/") + fileInfo.fileName());

                if (prj_dir.exists(fileInfo.fileName() + PROJECT_EXTENSION)) {
                    m_content.append(fileInfo.fileName());
                }
            }

        }
    }
}

bool BufferManager::isActiveProject()
{
    if (m_content.isEmpty()) {
        return false;
    }
    else {
        if(!m_dropdown) {
            return false;
        }
        else {
            if ((m_dropdown->selectedIndex() == DropDown::SelectedIndexNone) ||
                    (m_dropdown->selectedIndex() >= m_content.size())) {
                return false;
            }
            else {
                return true;
            }
        }

    }

}

void BufferManager::selectProject(QString text)
{
    if (!m_content.isEmpty()) {

        for (int i = 0; i < m_content.size(); i++) {
            if ((m_content.at(i)) == text) {
               m_dropdown->setSelectedIndex(i);
            }
        }
    }

}

void BufferManager::deleteProject(QString projectName)
{
    QDir dir;

    qDebug() << "Deleting Project  "<< projectName;

    //first clear out buffers
    clear();

    QString path(PROJECT_PATH + QString("/") + projectName);

    dir.mkpath(path);
    dir.cd(path);

    //see if project file exists.  otherwise nothing to delete
    if ((dir.exists()) && (dir.isReadable())) {
        QFileInfoList entries = dir.entryInfoList();
        for (QList<QFileInfo>::iterator it = entries.begin(); it != entries.end(); it++) {
            QFileInfo &fileInfo = *it;
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
                continue;
            }

            //only want to delete files
            if (!fileInfo.isDir()) {
                qDebug() << "Removing " <<fileInfo.fileName();
                dir.remove(fileInfo.fileName());
            }
            else {
                //do nothing
            }
        }
        //delete directory
        dir.cdUp();

        if (dir.rmdir(projectName)) {
            qDebug() <<"Removing " <<projectName;
        }
        else {
            qDebug() <<"cannot remove" <<projectName;
        }

        //finally reset track labels
        m_track->resetTrackLabels();
    }

    return;

}


