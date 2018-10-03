// Default empty project template
import bb.cascades 1.3
import my.library 1.0
import bb.multimedia 1.0
import bb.system 1.0


// creates one page with a label
Page {
    Container {
        
        // The background image
        id: containerID
        background: Color.create ("#00000000")
        topPadding: 20.0
        layout: StackLayout {}
        
        
        DropDown {
            id: projectddown
            objectName: "projectddown"
            title: "Song Name"
            onSelectedIndexChanged: {
                console.log("SelectedIndex was changed to " + selectedIndex)
                if ( selectedIndex >= 0) {
                    _buffermgr.restore(selectedIndex)
                    _audiotrack.restoreTrackName(projectddown.selectedOption.text)
                    recordbutton.setChecked(false)                
                    recordbutton.setImageSourceChecked("asset:///images/record_checked.png")
                }

            }
        
        }
        
        
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }

            horizontalAlignment: HorizontalAlignment.Center
            
            ImageToggleButton {
                id: recordbutton
                objectName: "recordbutton"

                preferredWidth: 90.0
                preferredHeight: 90.0

                onCheckedChanged: {
                    if (_buffermgr.isActiveProject()) { 
                        recordbutton.setImageSourceChecked("asset:///images/record_checked.png")
                        if (_audiorecorder.iscapturing()) {
                            //recordStopSound.play();
                            _audioplayer.stop();
                            _audiorecorder.stop();
                            tape.running = false;
                            _audiotrack.lockTrackUI(false)
                        }
                        else if (_audioplayer.isplaying()) {
                            console.log("audioplayer is playing : reset checked")
                            resetChecked()
                        }
                        else if (checked == false ) {
                            //it's not playing or capturing and checked is set to false so do nothing
                            console.log("checked is false")
                        }
                        else {
                            console.log("starting audio recorder");
                            _audiotrack.lockTrackUI(true)
                            
                            //recordStartSound.play();
                            _audioplayer.config(_audiotrack.getActiveRecordTrack());                            
                            _audiorecorder.config();
                            
                            tape.running = true;
                            _audioplayer.start();
                            _audiorecorder.start();                        
                        
                        } 
                        
                    }
                    else {
                        setImageSourceChecked("asset:///images/record.png")

                        saveDialog.open()
                    
                    }
                }
                imageSourceDefault: "asset:///images/record.png"
                imageSourceChecked: "asset:///images/record_checked.png"
                imageSourceDisabledUnchecked: "asset:///images/record.png"
                imageSourcePressedUnchecked: "asset:///images/record.png"
                imageSourceDisabledChecked: "asset:///images/record.png"
                imageSourcePressedChecked: "asset:///images/record.png"
            
            }
            
            ImageButton {
                id: playbutton
                preferredWidth: 90.0
                preferredHeight: 90.0

                onClicked: {
                    if (_buffermgr.isActiveProject()) {
                        _tapemgr.animationChanged.connect(playbutton.onanimationChanged);
                        if (_audiorecorder.iscapturing()) {
                            //do nothing if audio recorder is capturing
                        
                        }     
                        if (_audioplayer.ispaused()) {
                            _audioplayer.resume();
                        }
                        else  {
                            _audioplayer.play()
                        }
                    }
                
                }
                
                function onanimationChanged(i) {
                    if (i) {
                        if (tape.running == false) {
                            _audiotrack.lockTrackUI(true)
                            tape.running = true;
                            
                        }
                    }
                    else {
                        if (tape.running == true) {
                            tape.running = false;
                            _audiotrack.lockTrackUI(false)
                        }
                    }
                }
                
                pressedImageSource: "asset:///images/play.png"
                disabledImageSource: "asset:///images/play.png"
                defaultImageSource: "asset:///images/play.png"

            }
            
            /*ImageButton {
                id: pausebutton
                preferredWidth: 90.0
                preferredHeight: 90.0
                
                onClicked: {
                    if (_audioplayer.isplaying()) {
                        _audioplayer.pause()
                        
                    }
                }               
                
                    pressedImageSource: "asset:///images/pause.png"
                    disabledImageSource: "asset:///images/pause.png"
                    defaultImageSource: "asset:///images/pause.png"
                    opacity: 1.0
                        
            }*/
                   
            
            ImageButton {
                id: stopbutton
                preferredWidth: 90.0
                preferredHeight: 90.0
                
                onClicked: {
                    if (_audioplayer.isplaying()) {
                        _audioplayer.stop();
                        tape.running = false
                        }                
                }
                
                pressedImageSource: "asset:///images/stop.png"
                disabledImageSource: "asset:///images/stop.png"
                defaultImageSource: "asset:///images/stop.png"
            
            }
            
                    
            ImageButton {
                id: savebutton
                preferredWidth: 90.0
                preferredHeight: 90.0

                onClicked: {
                    if (_buffermgr.isActiveProject() && (!_audioplayer.isplaying() && (!_audiorecorder.iscapturing()))) {                        
                        _wavencode.saveWav();
                        wavToast.show();
                        
                    }
                }
                pressedImageSource: "asset:///images/save.png"
                disabledImageSource: "asset:///images/save.png"
                defaultImageSource: "asset:///images/save.png"
                
            
            }
            
            ImageButton {
                id: deletebutton
                preferredWidth: 90.0
                preferredHeight: 90.0
                
                onClicked: {
                    if ( projectddown.selectedOption) {
                        _audioslider.reset()
                        _buffermgr.deleteProject(projectddown.selectedOption.text)
                        _buffermgr.initProjectList()
                        _buffermgr.populateProjectDropDown()
                    }
                    
                }
                pressedImageSource: "asset:///images/delete.png"
                disabledImageSource: "asset:///images/delete.png"
                defaultImageSource: "asset:///images/delete.png"
            
            
            }
            
            ImageButton {
                id: helpbutton
                objectName: "helpbutton"
                preferredWidth: 90.0
                preferredHeight: 90.0
                onClicked: {
                    pdf.trigger("bb.action.OPEN")
                }
                
                pressedImageSource: "asset:///images/help.png"
                disabledImageSource: "asset:///images/help.png"
                defaultImageSource: "asset:///images/help.png"
            
            
            }
                    
        }
        Slider {
            id : slider
            objectName : "slider"

            horizontalAlignment: HorizontalAlignment.Center

        } // end slider
        
        
        
        Tape {
            id: tape
            objectName: "tape"
            horizontalAlignment: HorizontalAlignment.Center
            //preferredWidth: 400.0
            maxWidth: 500.0
            maxHeight: 500.0
           
            running: false
        }
                
        
        Label {
            id: timeStamp
            objectName: "timeStamp"
            
            
            //textStyle.fontSize: FontSize.XLarge
            textStyle.color: Color.create("#fffa8000")
            horizontalAlignment: HorizontalAlignment.Center

            textFit.mode: LabelTextFitMode.Standard
            textFit.maxFontSizeValue: 10.0

            textFit.minFontSizeValue: 10.0
        }
        
        Container {
            horizontalAlignment: HorizontalAlignment.Center
            
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }

            topMargin: 8.0
            bottomMargin: 8.0
            TextField {
                id : textfield1
                objectName: "textfield1"
                hintText: "Track1"

                preferredWidth: 300.0
                preferredHeight: 4.0
                textFit.minFontSizeValue: 3.0
                textFit.maxFontSizeValue: 3.0
                maximumLength: 15
                
                input {
                    submitKey: SubmitKey.EnterKey;
                    onSubmitted: {
                        //...Perform action on submission...
                        if (projectddown.selectedOptionSet){
                            _audiotrack.saveTrackName(1, projectddown.selectedOption.text, textfield1.text)
                        }
                    }
                }
            }           
            
            
            Button {
                id: mute1
                objectName: "mute1"
                text: "M"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                       _audiotrack.muteTrack(1)
                
                }
            }
            Button {
                id: solo1
                objectName: "solo1"
                text: "S"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.soloTrack(1)
                
                }
            }
            Button {
                id: record1
                objectName: "record1"
                text: "R"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.recordTrack(1)
                
                }
            }
        
        }  //track1 container 
        
        
        Container {
            //translationY: 40.0 //track 2
            horizontalAlignment: HorizontalAlignment.Center
            
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            
            topMargin: 8.0
            bottomMargin: 8.0
            
            TextField {
                id : textfield2
                objectName: "textfield2"
                hintText: "Track2"
                
                preferredWidth: 300.0
                preferredHeight: 4.0
                textFit.minFontSizeValue: 3.0
                textFit.maxFontSizeValue: 3.0
                maximumLength: 15
                
                input {
                    submitKey: SubmitKey.EnterKey;
                    onSubmitted: {                        
                        //...Perform action on submission...
                        if (projectddown.selectedOptionSet){
                            _audiotrack.saveTrackName(2, projectddown.selectedOption.text, textfield2.text)
                        }
                        
                    }
                }
            }            
            
            
            Button {
                id: mute2
                objectName: "mute2"
                text: "M"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.muteTrack(2)
                
                }
            }
            Button {
                id: solo2
                objectName: "solo2"
                text: "S"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.soloTrack(2)
                
                }
            }
            Button {
                id: record2
                objectName: "record2"
                text: "R"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.recordTrack(2)
                
                }
            }
        
        }  //track2 container    
        
        Container {
            
            //translationY: 60.0 //track 3
            horizontalAlignment: HorizontalAlignment.Center
            
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            
            topMargin: 8.0
            bottomMargin: 8.0
            
            
            TextField {
                id : textfield3
                objectName: "textfield3"
                hintText: "Track3"
                
                preferredWidth: 300.0
                preferredHeight: 4.0
                textFit.minFontSizeValue: 3.0
                textFit.maxFontSizeValue: 3.0
                maximumLength: 15
                
                input {
                    submitKey: SubmitKey.EnterKey;
                    onSubmitted: {
                        
                        //...Perform action on submission...
                        if (projectddown.selectedOptionSet){
                            _audiotrack.saveTrackName(3, projectddown.selectedOption.text, textfield3.text)
                        }
                    }
                }
            }           
            
            
            Button {
                id: mute3
                objectName: "mute3"
                text: "M"
                minWidth: 4.0
                preferredWidth: 4.0
                //color : Color.create("#ff0000")
                onClicked: {                    
                    _audiotrack.muteTrack(3)
                
                }
                
            }
            Button {
                id: solo3
                objectName: "solo3"
                text: "S"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.soloTrack(3)
                
                }
            }
            Button {
                id: record3
                objectName: "record3"
                text: "R"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.recordTrack(3)                
                }
            }
        
        }  //track3 container  
        
        Container {
            //translationY: 80.0 //track 4
            horizontalAlignment: HorizontalAlignment.Center
            
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            
            topMargin: 8.0
            bottomMargin: 8.0
            
            TextField {
                id : textfield4
                objectName: "textfield4"
                hintText: "Track4"
                
                preferredWidth: 300.0
                preferredHeight: 4.0
                textFit.minFontSizeValue: 3.0
                textFit.maxFontSizeValue: 3.0
                maximumLength: 15
                
                input {
                    submitKey: SubmitKey.EnterKey;
                    onSubmitted: {                        
                        //...Perform action on submission...
                        if (projectddown.selectedOptionSet){
                            _audiotrack.saveTrackName(4, projectddown.selectedOption.text, textfield4.text)
                        }
                    }
                }
            }           
            
            Button {

                id: mute4
                objectName: "mute4"
                text: "M"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.muteTrack(4)
                    
                }
            }
            Button {
                id: solo4
                objectName: "solo4"
                text: "S"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                    _audiotrack.soloTrack(4)
                
                }
            }
            Button {
                id: record4
                objectName: "record4"
                text: "R"
                minWidth: 4.0
                preferredWidth: 4.0
                onClicked: {
                     _audiotrack.recordTrack(4)
                
                }
            }
        
        }  //track4 container   
        //! [5]
        attachedObjects: [
            SystemSound {
                id: recordStartSound
                sound: SystemSound.RecordingStartEvent
            },
            SystemSound {
                id: recordStopSound
                sound: SystemSound.RecordingStopEvent
            },
            Dialog {

                id: saveDialog
                
                            
                Container {
                    background: Color.LightGray
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Center
                    topPadding: 20.0
                    preferredWidth: 600.0
                    preferredHeight: 400.0
                    Label {
                        text: "Please type a song name or select a previously created song from the dropdown above"
                        textStyle.color: Color.Black
                        multiline: true
                        autoSize.maxLineCount: 10
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Center
                    
                    }
                    TextField {
                         id: fileName
                         textFormat: TextFormat.Plain
                         validator: Validator {
                             mode: ValidationMode.Immediate
                             errorMessage: "Your username must be 10 characters or fewer."
                             
                             onValidate: {
                                 if (_buffermgr.validateName(fileName.text) )
                                     state = ValidationState.Valid;
                                 else
                                     state = ValidationState.Invalid;
                             }
                         }
                        backgroundVisible: false
                        inputMode: TextFieldInputMode.Text
                        opacity: 0.6
                        maximumLength: 20
                        hintText: "Type a song name here"
                    }
                    Container {
                        horizontalAlignment: HorizontalAlignment.Center
                        
                        layout: StackLayout {
                            orientation: LayoutOrientation.LeftToRight
                        }

                        topPadding: 30.0
                        Button {
                          horizontalAlignment: HorizontalAlignment.Left
                          text: "Save"
                           onClicked: {
                               if ((fileName.validator) && (fileName.text != "")) {
                                   _buffermgr.save(fileName.text)
                                   _buffermgr.initProjectList()
                                   _buffermgr.populateProjectDropDown()
                                   _buffermgr.selectProject(fileName.text)
                                   saveDialog.close()
                               }
                               else {
                                   console.log("not valid"); 
                               }
                               
                           }//onclickd
                        color: Color.DarkGray
                        preferredWidth: 150.0
                        preferredHeight: 30.0
                      } //button
                       
                     Button {
                         horizontalAlignment: HorizontalAlignment.Right
                         text: "Cancel"
                         onClicked: {
                             saveDialog.close()
                         
                         }
                         color: Color.DarkGray
                         preferredWidth: 150.0
                         preferredHeight: 30.0
                      }//button
                    }//container buttons
                }  //container dialog
                      
            },//save dialog
            Invocation {
                id: pdf
                query {
                    mimeType: "application/pdf"
                    uri: "file:///" + app.getAssetPath("mini_recorder_help.pdf")
                    invokeActionId: "bb.action.OPEN"
                }
            
            },
            SystemToast {
                id: wavToast
                body: "wav saved!"
               
            }
       ]  //attached objects    
  }    

}