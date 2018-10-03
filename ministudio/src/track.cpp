/*
 * track.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: randrade
 */

#include <track.hpp>
#include <bb/cascades/Color>
#include <recorderdefines.h>

#define COLOR_RED      0Xff0000
#define COLOR_ORANGE   0Xfffa8000
#define COLOR_BLUE     0Xff004bff

#define BUTTON_COLOR   COLOR_BLUE


Track::~Track()
{
    //destructor
}

Track::Track(int numTracks)
:    mCurrentTrack(1),
     mlockTrack(false)
{
    //initialize all buttons
    for (int i = 0; i < numTracks; i++ ) {
        mMuteButton[i].button = 0;
        mMuteButton[i].on = 0;
        mRecordButton[i].button = 0;
        mRecordButton[i].on = 0;
        mSoloButton[i].button = 0;
        mSoloButton[i].on = 0;
        mSeqNo[i] = 0;
    }

    Track::mNumTracks = numTracks;


}

void Track::setTrackId(int trackNum)
{
    mCurrentTrack = trackNum;
}


int Track::getTrackId()
{
    return mCurrentTrack;
}

void Track::muteTrack(int trackNum)
{
    //if tracks are locked just return
    if (mlockTrack) return;

    if (mMuteButton[trackNum-1].on) {
        mMuteButton[trackNum-1].button->resetColor();
        mMuteButton[trackNum-1].on = false;

    }
    else {
        mMuteButton[trackNum-1].button->setColor(Color::fromARGB(BUTTON_COLOR));
        mMuteButton[trackNum-1].on = true;

        //if the track was on solo, then unsolo it
        if (mSoloButton[trackNum-1].on) {
            mSoloButton[trackNum-1].button->resetColor();
            mSoloButton[trackNum-1].on = false;

        }
    }

}

void Track::soloTrack(int trackNum)
{
    if (mlockTrack) return;

    if (mSoloButton[trackNum-1].on) {
        mSoloButton[trackNum-1].button->resetColor();
        mSoloButton[trackNum-1].on = false;

    }
    else {
        mSoloButton[trackNum-1].button->setColor(Color::fromARGB(BUTTON_COLOR));
        mSoloButton[trackNum-1].on = true;

        // if the track was muted then unmute it
        if (mMuteButton[trackNum-1].on) {
            mMuteButton[trackNum-1].button->resetColor();
            mMuteButton[trackNum-1].on = false;
        }

        //only allow one track for soloing so reset other tracks
        for (int i = 0; i < mNumTracks; i++ ) {
            if ((i != trackNum-1) && (mSoloButton[i].on)) {
                mSoloButton[i].button->resetColor();
                mSoloButton[i].on = false;
            }
        }
    }

}

void Track::recordTrack(int trackNum)
{
    int disableTrack = 0;
    int enableTrack = 0;

    if (mlockTrack) return;

    if (mRecordButton[trackNum-1].on) {
        //mRecordButton[trackNum-1].button->resetColor();
        //mRecordButton[trackNum-1].on = false;
        //disableTrack = trackNum;
        qDebug("At least one track needs to be in record mode");
        return;

    }
    else {
        //only allow one track for recording so reset other tracks
        //first we should disable, then enable
        for (int i = 0; i < mNumTracks; i++ ) {
            if ((i != trackNum-1) && (mRecordButton[i].on)) {
                mRecordButton[i].button->resetColor();
                mRecordButton[i].on = false;
                disableTrack = i+1;
            }
        }
        mRecordButton[trackNum-1].button->setColor(Color::fromARGB(BUTTON_COLOR));
        mRecordButton[trackNum-1].on = true;
        enableTrack = trackNum;
    }

    emit trackRecordChange(disableTrack, enableTrack);

}


void Track::configUI(const bb::cascades::AbstractPane *root)
{

    mMuteButton[0].button = root->findChild<Button *>("mute1");
    mMuteButton[1].button = root->findChild<Button *>("mute2");
    mMuteButton[2].button = root->findChild<Button *>("mute3");
    mMuteButton[3].button = root->findChild<Button *>("mute4");

    mRecordButton[0].button = root->findChild<Button *>("record1");
    mRecordButton[1].button = root->findChild<Button *>("record2");
    mRecordButton[2].button = root->findChild<Button *>("record3");
    mRecordButton[3].button = root->findChild<Button *>("record4");

    mSoloButton[0].button = root->findChild<Button *>("solo1");
    mSoloButton[1].button = root->findChild<Button *>("solo2");
    mSoloButton[2].button = root->findChild<Button *>("solo3");
    mSoloButton[3].button = root->findChild<Button *>("solo4");

    mTrackText[0] = root->findChild<TextField *>("textfield1");
    mTrackText[1] = root->findChild<TextField *>("textfield2");
    mTrackText[2] = root->findChild<TextField *>("textfield3");
    mTrackText[3] = root->findChild<TextField *>("textfield4");


    //mute all tracks
    soloTrack(1);
    recordTrack(1);

}

int Track::getActiveRecordTrack()
{
    for (int i = 0; i < mNumTracks; i++ ) {
        if (mRecordButton[i].on) {
            return i+1;
        }
    }

    return -1;
}

int Track::getSequenceNumber(int trackNum)
{
    if (trackNum > mNumTracks) return -1;

    mSeqNo[trackNum-1]++;

    return mSeqNo[trackNum-1];
}

void Track::saveTrackName(int trackNum, QString projectName, QString trackName)
{
    QDir dir;
    ProjectItem projectitem;
    TrackItem trackitem;
    qDebug() <<"track Num is "<< trackNum;
    qDebug() <<"project Name is " << projectName;
    qDebug() <<"save track name called" << trackName;

    QString path(PROJECT_PATH + QString("/") + projectName);


    dir.mkpath(path);
    dir.cd(path);

    QFile projectFile(path + QString("/") + projectName + PROJECT_EXTENSION);

    //see if project file exists.  if not create one
    if (dir.exists(projectName + PROJECT_EXTENSION)) {
        qDebug("creating project file");
        if ( !projectFile.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            qDebug("cannot open project file in append mode");
            return;
        }
    }
    else {
        qDebug("creating project file");
        if ( !projectFile.open(QIODevice::WriteOnly))
        {
            qDebug("cannot open project file in write mode");
            return;
        }

    }

    projectitem.item_type = ITEM_TYPE_TRACK_NAME;
    projectitem.item_size = sizeof (TrackItem);

    trackitem.trackNum = trackNum;
    strcpy(trackitem.trackName, trackName.toUtf8().data());


    projectFile.write((const char *)(&projectitem), sizeof(ProjectItem));
    projectFile.write((const char *)(&trackitem), sizeof(TrackItem));

    //processing is done, now close file
    projectFile.close();

}

void Track::restoreTrackName(QString projectName)
{
    QDir dir;
    ProjectItem projectitem;
    TrackItem trackitem;

    QString path(PROJECT_PATH + QString("/") + projectName);


    dir.mkpath(path);
    dir.cd(path);

    qDebug("Restoring track name");

    resetTrackLabels();

    //see if project file exists.  if not create one
    if (dir.exists(projectName + PROJECT_EXTENSION)) {

        QFile projectFile(path + QString("/") + projectName + PROJECT_EXTENSION);
        qDebug("opening project file");
        if ( !projectFile.open(QIODevice::ReadOnly))
        {
            qDebug("cannot open project file");
            return;
        }

       //scan for track project types
        while (!projectFile.atEnd()) {
            projectFile.read((char *)(&projectitem), sizeof(ProjectItem));
            if (projectitem.item_type == ITEM_TYPE_TRACK_NAME ) {
                projectFile.read((char *)(&trackitem), sizeof(TrackItem));


                qDebug()<< "Item type is " <<projectitem.item_type;
                qDebug()<< "Item size is " <<projectitem.item_size;
                qDebug()<< "Track num is " <<trackitem.trackNum;
                qDebug("Track name is %s", trackitem.trackName);
                QString str(trackitem.trackName);
                if (( trackitem.trackNum > 0) && (trackitem.trackNum <= MAX_NUM_TRACKS)) {
                    mTrackText[trackitem.trackNum-1]->setText(str);
                }

            }
            else {
                qint64 pos = projectFile.pos();
                projectFile.seek(pos + projectitem.item_size);
            }
        }




        //processing is done, now close file
        projectFile.close();

    }
    else {
        qDebug("Error no track names exist");
        return;
    }

}

void Track::resetTrackLabels()
{
    qDebug("Resetting track labels");

    if ( mTrackText[0] ) {
        mTrackText[0]->resetText();
    }

    if ( mTrackText[1] ) {
        mTrackText[1]->resetText();
    }

    if ( mTrackText[2] ) {
        mTrackText[2]->resetText();
    }

    if ( mTrackText[3] ) {
        mTrackText[3]->resetText();
    }


}


