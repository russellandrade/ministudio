/*
 * track.h
 *
 *  Created on: Aug 27, 2014
 *      Author: randrade
 */

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <bb/cascades/Application>
#include <bb/cascades/multimedia/Camera>
#include <bb/cascades/Button>
#include <bb/cascades/Page>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Slider>
#include <bb/cascades/TextField>


#define MAX_NUM_TRACKS   4
#define TRACK_NAME_LENGTH 20 //if you change this make sure to change textbox in main.qml as well


using namespace bb::cascades;

typedef struct _buttonInfo {
    Button *button;
    bool on;
} ButtonInfo;




typedef struct _trackItem {
    int trackNum;
    char trackName[TRACK_NAME_LENGTH];
} TrackItem;




class Track : public QObject
{
Q_OBJECT

public:

    Track(int numTracks = 4);
    ~Track();

    inline int numTracks() { return mNumTracks; }

    Q_INVOKABLE void setTrackId(int trackNum);
    Q_INVOKABLE int getTrackId();
    Q_INVOKABLE void muteTrack(int TrackNum);
    Q_INVOKABLE void soloTrack(int TrackNum);
    Q_INVOKABLE void recordTrack(int TrackNum);

    //active track being recorded
    Q_INVOKABLE int getActiveRecordTrack();
    Q_INVOKABLE int getSequenceNumber(int trackNum);
    Q_INVOKABLE int getCurrentSequenceNumber (int trackNum) { return mSeqNo[trackNum -1]; }


    Q_INVOKABLE inline bool isTrackMuted(int trackNum) { return mMuteButton[trackNum-1].on; }
    Q_INVOKABLE inline bool isTrackSolo(int trackNum) { return mSoloButton[trackNum-1].on; }
    Q_INVOKABLE inline bool isTrackRecord(int trackNum) { return mRecordButton[trackNum-1].on; }

    Q_INVOKABLE inline void lockTrackUI(bool lock) { mlockTrack = lock; }

    Q_INVOKABLE void saveTrackName(int trackNum, QString projectName, QString trackName);
    Q_INVOKABLE void restoreTrackName(QString projectName);

    Q_INVOKABLE void resetTrackLabels();



    void configUI(const bb::cascades::AbstractPane *root);

    static int mNumTracks;

    signals:
    void trackRecordChange(int disabletrack, int enabletrack);
    //trackMuteChange(int)


private :
    int mCurrentTrack;
    bool mlockTrack;
    ButtonInfo mMuteButton[MAX_NUM_TRACKS];
    ButtonInfo mRecordButton[MAX_NUM_TRACKS];
    ButtonInfo mSoloButton[MAX_NUM_TRACKS];
    int mSeqNo[MAX_NUM_TRACKS];
    TextField *mTrackText[MAX_NUM_TRACKS];

};



#endif /* TRACK_H_ */
