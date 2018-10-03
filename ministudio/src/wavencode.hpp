
 /*
 * wavencode.hpp
 *
 *  Created on: Dec 23, 2014
 *      Author: randrade
 */

#ifndef WAVENCODE_HPP_
#define WAVENCODE_HPP_

#include <buffermgr.hpp>

typedef struct
{
    char    tag[4];
    long    length;
}
riff_tag;

typedef struct
{
    riff_tag formatTag;
    short   audio_format;
    short   channels;
    long    samples_per_sec;
    long    avg_bytes_per_sec;
    short   block_align;
    short   bits_per_sample;
}
wav_format;

typedef struct
{
    riff_tag riffTag;
    char    Wave[4];
    wav_format wavFormat;
    riff_tag wavData;
}
wave_hdr;

class Wavencode : public QObject {
Q_OBJECT

public:
    Wavencode(BufferManager *buffermgr);
    ~Wavencode();

    Q_INVOKABLE void saveWav();


private:
    void writeWaveHeader(QFile &file, int sampleRate, int nChannels, long len);
    void updateWaveHeader(QFile &file, int dataSize);

    BufferManager *mBuff;


};



#endif /* WAVENCODE_HPP_ */
