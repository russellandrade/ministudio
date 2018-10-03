/*
 * audio_slider.hpp
 *
 *  Created on: Sep 15, 2014
 *      Author: randrade
 */

#ifndef AUDIO_SLIDER_HPP_
#define AUDIO_SLIDER_HPP_


#include <bb/cascades/Page>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Application>
#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <bb/cascades/Slider>
#include <bb/cascades/Label>

using namespace bb::cascades;

class AudioSlider : public QObject {
Q_OBJECT

public:
    AudioSlider(const bb::cascades::AbstractPane *root);
    ~AudioSlider();

    Q_INVOKABLE void setMaxValue(int bytes);
    Q_INVOKABLE void setValue(int bytes);
    Q_INVOKABLE void reset();
    int getReadPos();
    void getTimeComponents(int total_ms, int &ms, int &sec, int &min);
    void setalignment(int align);
    int getMaxPos();



private slots:
    void updateOffset(float value);



private:
    Slider *mSlider;
    Label *mTimeLabel;
    int m_align;
    float m_totalms;
};


#endif /* AUDIO_SLIDER_HPP_ */
