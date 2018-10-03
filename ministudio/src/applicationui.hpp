/*
 * applicationui.hpp
 *
 *  Created on: Feb 3, 2015
 *      Author: Russell
 */

#ifndef APPLICATIONUI_HPP_
#define APPLICATIONUI_HPP_

#include <QObject>
#include <track.hpp>
#include <audio_timer.hpp>
#include <audio_slider.hpp>
#include <buffermgr.hpp>

namespace bb { namespace cascades { class Application; }}



/*!
 * @brief Application pane object
 *
 *Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
public:
    ApplicationUI(bb::cascades::Application *app);
    virtual ~ApplicationUI() {}

    void updateSlider(float value);

    public slots:
    void onRestoreDone(int bytes);
    void onTrackRecordChange(int oldTrack, int newTrack);


    Q_INVOKABLE QString getAssetPath(QString Filename);

private:
    AudioSlider *mSlider;
    BufferManager *mBuffmgr;

};



#endif /* APPLICATIONUI_HPP_ */
