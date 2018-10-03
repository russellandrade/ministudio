/*
 * wavencode.cpp
 *
 *  Created on: Dec 23, 2014
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
#include <recorderdefines.h>
#include <wavencode.hpp>



Wavencode::Wavencode(BufferManager *buffermgr)
:mBuff(buffermgr)
{

}

Wavencode::~Wavencode()
{

}


void Wavencode::writeWaveHeader(QFile &file, int sampleRate, int nChannels, long len)
{
    wave_hdr waveHeader;
    int size = len + sizeof(waveHeader) - 8;
    const short bits_per_sample = 16;

    waveHeader.riffTag = { {'R', 'I', 'F', 'F'}, size };
    waveHeader.Wave[0] = 'W';
    waveHeader.Wave[1] = 'A';
    waveHeader.Wave[2] = 'V';
    waveHeader.Wave[3] = 'E';
    waveHeader.wavFormat.formatTag = { { 'f', 'm', 't', ' ' }, (sizeof(waveHeader.wavFormat)-8) };

    waveHeader.wavFormat.audio_format = 1;
    waveHeader.wavFormat.channels = nChannels;
    waveHeader.wavFormat.samples_per_sec = sampleRate;
    waveHeader.wavFormat.avg_bytes_per_sec = nChannels * sampleRate * bits_per_sample/8;
    waveHeader.wavFormat.block_align = bits_per_sample/8 * nChannels;
    waveHeader.wavFormat.bits_per_sample = bits_per_sample;

    waveHeader.wavData = { {'d', 'a', 't', 'a'}, len };

    file.write((const char *)&waveHeader, sizeof(waveHeader));

}

void Wavencode::updateWaveHeader(QFile &file, int dataSize)
{
    wave_hdr waveHeader;

   file.seek(4); // seek to RIFF size
   int size = dataSize + sizeof(waveHeader) - 8;
   file.write((const char *)&size, 4);

   file.seek(sizeof(waveHeader) - 4); // seek to DATA size
   file.write((const char *)&dataSize, 4);
   file.flush();
}

void Wavencode::saveWav()
{

    QString projectName;


    mBuff->getProjectName(projectName);

    QString path = QString(WAV_PATH) + QString("/");
    QDir dir;

    qDebug() <<"Wav path is "<<path;

    dir.mkpath(path);
    dir.cd(path);

    QString fileName = projectName + QString(".wav");


    //see if file already exists.  if it does, delete it

    if ( dir.exists(fileName)) {
        qDebug() << "Deleting file" <<fileName;
        dir.remove(fileName);
    }

    //remix the track prior to saving as wav
    qDebug("Remixing");
    mBuff->remix(projectName, 0);
    TBuffer *tBuff = mBuff->getActivePlayBuffer();
    if (tBuff->maxpos <= 0) {
        qDebug("no bytes to write");
        return;
    }


    qDebug() << "creating file" << fileName;
    QFile audiofile(dir.path() + QString("/") + fileName);

    if ( !audiofile.open(QIODevice::WriteOnly)) {
        return;

    }


    writeWaveHeader(audiofile, SAMPLE_RATE, NUM_PCM_VOICES, tBuff->maxpos);

    qDebug("Writing %d to file", tBuff->maxpos);
    audiofile.write(tBuff->buff, tBuff->maxpos);

    //updateWaveHeader(audiofile, tBuff->maxpos);

    audiofile.close();

}
