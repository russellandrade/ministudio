/*
 * recorderdefines.h
 *
 *  Created on: Dec 20, 2013
 *      Author: randrade
 */

#ifndef RECORDERDEFINES_H_
#define RECORDERDEFINES_H_

#define prlog(_a_)  fprintf(stderr, _a_)

#define SAMPLE_RATE               48000
#define SAMPLES_IN_ONE_MS         (SAMPLE_RATE/1000)
#define NUM_PCM_VOICES                 1
#define PRECONFIG_MAX_TIME   60 * 1000  //60 sec
#define CONVERT_MILLISEC_TO_BYTES(ms) (ms * (SAMPLE_RATE/1000) * 2 * NUM_PCM_VOICES)
#define CONVERT_BYTES_TO_MSEC(bytes) (bytes /(NUM_PCM_VOICES * 2 * SAMPLES_IN_ONE_MS))
#define MILLISEC_PER_MIN          60000
#define MILLISEC_PER_SEC          1000


#define PROJECT_PATH "data/minirecorder"
#define PROJECT_EXTENSION ".prj"
#define TRACK_EXTENSION "*.trk"

#define WAV_PATH "shared/music"


#define MINI_RECORDER_VERSION_MAJOR  1
#define MINI_RECORDER_VERSION_MINOR   0

typedef struct  {
    char *buff; //buffer
    int currpos;  //current position
    int maxpos;  //max position
} TBuffer;

//types of project file items

#define ITEM_TYPE_VERSION  1
#define ITEM_TYPE_TRACK_NAME 2

typedef struct _projectItem
{
    int item_type;
    int item_size;
} ProjectItem;

typedef struct _versionItem
{
    int major_version;
    int minor_version;
} VersionItem;

#endif /* RECORDERDEFINES_H_ */
