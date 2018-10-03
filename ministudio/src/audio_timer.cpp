/*
 * Audiotimer.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: Russell
 */


#include <audio_timer.hpp>
#include <recorderdefines.h>


Audiotimer::Audiotimer(const bb::cascades::AbstractPane *root, AudioSlider *slider)
: mTime(0),
  mTimeLabel(0),
  mActiveBuffer(0),
  mSlider(slider),
  mMode(TIMER_MODE_RECORD)
{
    if (!mTime ) {
        mTime = new QTimer(this);
        connect(mTime, SIGNAL(timeout()), this, SLOT(processtimeout()));
    }

    mTimeLabel = root->findChild<Label *>("timeStamp");
    mTimeLabel->setText("00:00:000");

}

Audiotimer::~Audiotimer()
{
    //delete timer
    if (mTime) {
        delete mTime;
        mTime = 0;
    }

}

void Audiotimer::start(timermode_t mode)
{

    if (mTime) {
        //mpos should be reset to slider read position
        //player and recorder will update read position accordingly
        mPos =   mSlider->getReadPos();
        mTime->start(TIMER_INTERVAL);

        mMode = mode;
    }


}

void Audiotimer::stop()
{
    if (mTime) {
        mTime->stop();
    }

}

void Audiotimer::single()
{

    mTime->singleShot(100, this, SLOT(cleanup()));

}

timermode_t Audiotimer::getMode()
{
    return mMode;
}

void Audiotimer::updateByteCount(int bytes)
{
    mPos = bytes;

}

void Audiotimer::processtimeout()
{
    int bytes_read;
    int ms, min, sec;
    QString time;
    int currmax;

    switch (mMode) {

        case TIMER_MODE_RECORD :

            if (mActiveBuffer) {
                bytes_read = mActiveBuffer->currpos;
            }
            else {
                bytes_read = 0;
            }

            if (bytes_read < 0) {
                qDebug("Bytes read is %d", bytes_read);
            }

            ms = CONVERT_BYTES_TO_MSEC(bytes_read);
            if ( ms < 0 ) {
                qDebug ("negative ms %d", ms);
            }

            min = ms / MILLISEC_PER_MIN;
            ms = ms % MILLISEC_PER_MIN;
            sec = ms / MILLISEC_PER_SEC;
            ms = ms % MILLISEC_PER_SEC;

            time = QString::number(min) + ":" + QString::number(sec) + ":" + QString::number(ms);

            mTimeLabel->setText(time);
            currmax = mSlider->getMaxPos();
            if ( mPos <= currmax ) {
                mSlider->setValue(mPos);
            }
            break;

        case TIMER_MODE_PLAY:
            //qDebug("setting slider value %d", mPos);
            mSlider->setValue(mPos);

            break;
    }


}


void Audiotimer::setActiveBuffer(TBuffer *buffer)
{
    mActiveBuffer = buffer;
}

void Audiotimer::cleanup()
{
    int currmax = mSlider->getMaxPos();
    qDebug("cleaning up timer");
    //update the slider
    if (mActiveBuffer->maxpos > currmax) {
        mSlider->setMaxValue(mActiveBuffer->maxpos);
    }
    mSlider->setValue(mActiveBuffer->currpos);
}



