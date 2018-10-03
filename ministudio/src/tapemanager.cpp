/*
 * tapemanager.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: Russell
 */



#include <tapemanager.hpp>
#include <QtCore/qobject.h>
//#include <QtCore/QDeclarativeItem.h>

TapeMgr::TapeMgr()
{
    m_animate = false;
}

TapeMgr::~TapeMgr()
{

}



int TapeMgr::isanimate()
{
    return m_animate;
}


void TapeMgr::setanimate(bool i)
{
    m_animate = i;
    qDebug() <<"animation changed event";
    emit animationChanged(i);
}



