 // Default empty project template
#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <audio_capture.hpp>
#include <audio_player.hpp>
#include <tapemanager.hpp>
#include <track.hpp>
#include <wavencode.hpp>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeReply>
#include <bb/system/InvokeRequest>
#include <bb/system/InvokeTargetReply>
//#include <bb/cascades/ImageButton>
//#include <bb/cascades/ImageToggleButton>




using namespace bb::cascades;
using namespace bb::system;

BufferManager *g_audioBuffer;


int Track::mNumTracks = 4;

ApplicationUI::ApplicationUI(bb::cascades::Application *app)
: QObject(app)
{

    qmlRegisterType<TapeMgr>("my.library", 1, 0, "TapeMgr");

    // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    qml->setContextProperty("app", this);

    // create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();



    //create 4 tracks
    Track * audiotrack = new Track(4);
    qml->setContextProperty("_audiotrack", audiotrack);
    audiotrack->configUI(root);

    //slider
    mSlider = new AudioSlider(root);
    qml->setContextProperty("_audioslider", mSlider);


    //create timer class
    Audiotimer *timer = new Audiotimer(root, mSlider);
    qml->setContextProperty("_timer", timer);


    //tape manager
    TapeMgr *tapemgr = new TapeMgr();
    qml->setContextProperty("_tapemgr", tapemgr);


    //buffers
    mBuffmgr = new BufferManager(root);
    qml->setContextProperty("_buffermgr", mBuffmgr);
    mBuffmgr->setTrack(audiotrack);
    mBuffmgr->initProjectList();
    mBuffmgr->populateProjectDropDown();


    //creating audio recorder class
    AudioRecorder *mRecorder = new AudioRecorder(audiotrack, timer, mBuffmgr, mSlider);
    qml->setContextProperty("_audiorecorder", mRecorder);

    //creating audio player class
    AudioPlayer *mPlayer = new AudioPlayer(timer, mBuffmgr, mSlider, tapemgr);
    qml->setContextProperty("_audioplayer", mPlayer);

    connect(mBuffmgr, SIGNAL(restoreDone(int)), this, SLOT(onRestoreDone(int)));
    connect(audiotrack, SIGNAL(trackRecordChange(int, int)), this, SLOT(onTrackRecordChange(int, int)));

    Wavencode *wavencode = new Wavencode(mBuffmgr);
    qml->setContextProperty("_wavencode", wavencode);


    // set created root object as a scene
    app->setScene(root);
}

void ApplicationUI::onRestoreDone(int bytes)
{
    qDebug()<<"restore done : setting slider value, maxvalue to "<<bytes;
    mSlider->setMaxValue(bytes);
    mSlider->setValue(bytes);

}

void ApplicationUI::onTrackRecordChange(int oldTrack, int newTrack)
{
    int  bytes_read = 0;

    if (!mBuffmgr->isActiveProject()) return;

    qDebug("Track record change %d %d", oldTrack, newTrack);

    //save the old track first, then restore the new track
    if (oldTrack != 0) {
        mBuffmgr->disableTrack(oldTrack);
    }


    if ( newTrack != 0 ) {
        bytes_read = mBuffmgr->enableTrack(newTrack);
        //get the current read value of the slider
        //update maxpos
        TBuffer *recordbuff = mBuffmgr->getActiveRecordBuffer();
        int sliderPos = mSlider->getMaxPos();
        recordbuff->maxpos = (sliderPos > bytes_read) ? (sliderPos): bytes_read;
        qDebug("New max pos is %d", recordbuff->maxpos);
    }
}



QString ApplicationUI::getAssetPath(QString assetName) {
   return QDir::currentPath() + "/app/native/assets/help/" + assetName;
}





