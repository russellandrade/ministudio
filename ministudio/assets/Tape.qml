import bb.cascades 1.3


//! [0]
Container {
    property bool running: false
    
    onRunningChanged: {
        if (running)
            animation.play()
        else {
            console.log("stopping animation");
            animation.stop()
        }
    }
    
    layout: DockLayout {}
    
    ImageView {
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
        
        imageSource: "asset:///images/tape_shadow.png"
    }
    
    ImageView {
        id: leftGear
        
        horizontalAlignment: HorizontalAlignment.Center
        verticalAlignment: VerticalAlignment.Center
        
        translationY: 20
       
        
        imageSource: "asset:///images/tape_reel_left.png"
        scaleX: 0.8
        scaleY: 0.8
    }
    

    
    animations: ParallelAnimation {
        id: animation
        
        SequentialAnimation {
            target: leftGear
            
            RotateTransition {
                fromAngleZ: 0
                toAngleZ: 1800
                duration: 15000
                easingCurve: StockCurve.Linear
                repeatCount: AnimationRepeatCount.Forever
            }
        }
    }
    //! [0]
}

