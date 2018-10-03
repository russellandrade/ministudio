/*
  * audio_slider.cpp
 *
 *  Created on: Sep 15, 2014
 *      Author: randrade
 */

#include <audio_slider.hpp>
#include <recorderdefines.h>



AudioSlider::AudioSlider(const bb::cascades::AbstractPane *root)
:m_align(1),
 m_totalms(0)
{
    //float toVal = PRECONFIG_MAX_TIME;
    mSlider = root->findChild<Slider *>("slider");
    mTimeLabel = root->findChild<Label *>("timeStamp");

    //mSlider->setRange(0.0f, 1.0f);
    mSlider->setFromValue(0.0f);
    mSlider->setToValue(0.0f);
    mSlider->setValue(0.0f);

    connect(mSlider, SIGNAL(valueChanged(float)), this, SLOT(updateOffset(float)));

}

AudioSlider::~AudioSlider()
{

}

void AudioSlider::reset()
{
    mSlider->setFromValue(0.0f);
    mSlider->setToValue(0.0f);
    mSlider->setValue(0.0f);
    mTimeLabel->setText("00:00:000");
}

void AudioSlider::setMaxValue(int bytes)
{
    float millisec = (float)CONVERT_BYTES_TO_MSEC(bytes);
    qDebug("setting max value to %f", millisec);

    mSlider->setToValue(millisec);
    m_totalms = millisec;

}


void AudioSlider::setValue(int bytes)
{
    float millisec = (float)CONVERT_BYTES_TO_MSEC(bytes);
    //qDebug("trying to set value to %f", millisec);


    mSlider->setValue(millisec);
    //qDebug("max value is %f ms", millisec);

}



void AudioSlider::getTimeComponents(int total_ms, int &ms, int &sec, int &min)
{
    min = total_ms/MILLISEC_PER_MIN;
    sec = (total_ms % MILLISEC_PER_MIN)/MILLISEC_PER_SEC;
    ms = (total_ms % MILLISEC_PER_MIN) % MILLISEC_PER_SEC;

}

void AudioSlider::updateOffset(float value)
{
    int min, sec, ms;
    //calculate bytes offset from value


    int byte_offset = getReadPos();

    //qDebug("update offset %d bytes", byte_offset);

    int total_ms = CONVERT_BYTES_TO_MSEC(byte_offset);
    getTimeComponents(total_ms, ms, sec, min);
    QString time = QString::number(min) + ":" + QString::number(sec) + ":" + QString::number(ms);

    mTimeLabel->setText(time);
    //qDebug() <<"value is "<<value;
}

void AudioSlider::setalignment(int align)
{
    if ( align == 0 ) return;
    m_align = align;
}

int AudioSlider::getReadPos()
{
    int readPos = (int) CONVERT_MILLISEC_TO_BYTES(mSlider->value());

    readPos = (readPos/m_align) * m_align;

    //qDebug("read pos is %d", readPos);

    return readPos;

}

int AudioSlider::getMaxPos()
{
    int maxPos = (int) CONVERT_MILLISEC_TO_BYTES(mSlider->toValue());

    return maxPos;

}
