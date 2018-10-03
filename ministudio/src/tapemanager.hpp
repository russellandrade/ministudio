/*
 * tapemanager.hpp
 *
 *  Created on: Sep 3, 2014
 *      Author: Russell
 */

#ifndef TAPEMANAGER_HPP_
#define TAPEMANAGER_HPP_

#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <bb/cascades/CustomControl>

using namespace bb::cascades;

class TapeMgr : public QObject
{
    Q_OBJECT
    /*!
     * The value of the object.
     */
    Q_PROPERTY(bool animate READ isanimate WRITE setanimate NOTIFY animationChanged)

public:
    /*!
     * Constructs an instance of MyCppObject and sets the
     * parent. When the parent object is destroyed,
     * this object is as well.
     */
    TapeMgr();
     ~TapeMgr();


    /*!
     * Gets the current value.
     */
    int isanimate();
    /*!
     * Sets the value.
     */
    void setanimate(bool i);

    Q_SIGNALS:
    /*!
     * Emitted when the value changes.
     */
    void animationChanged(bool i);

private:
    bool m_animate;
};


#endif /* TAPEMANAGER_HPP_ */
