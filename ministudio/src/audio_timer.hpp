/*
 * timer.hpp
 *
 *  Created on: Sep 14, 2014
 *      Author: Russell
 */

#ifndef AUDIOTIMER_HPP_
#define AUDIOTIMER_HPP_


#include <bb/cascades/Page>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Application>
#include <QtCore/QTimer>
#include <bb/cascades/Label>
#include <track.hpp>
#include <audio_slider.hpp>
#include <recorderdefines.h>


using namespace bb::cascades;


#define TIMER_INTERVAL 100   //capture ticks in millisec

typedef enum  {
    TIMER_MODE_RECORD,
    TIMER_MODE_PLAY
} timermode_t;

class Audiotimer : public QObject {
Q_OBJECT

public :

    Audiotimer(const bb::cascades::AbstractPane *root, AudioSlider *slider);
    ~Audiotimer();

    Q_INVOKABLE void start(timermode_t mode);
    Q_INVOKABLE void stop();
    //Q_INVOKABLE void reset();
    Q_INVOKABLE void single();
    Q_INVOKABLE void updateByteCount(int bytes);

    void setActiveBuffer(TBuffer *buffer);
    timermode_t getMode();

private slots:
    void processtimeout();
    void cleanup();



private :
    QTimer *mTime;
    Label *mTimeLabel;
    TBuffer *mActiveBuffer;
    AudioSlider *mSlider;
    timermode_t mMode;
    int mPos;
};



#endif /* TIMER_HPP_ */
