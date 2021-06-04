//=====================================================================================================================
/**
 * @file     classifierssetupwidget.h
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Lorenz Esch <lesch@mgh.harvard.edu>;
 *           Viktor Klueber <Viktor.Klueber@tu-ilmenau.de>
 * @since    0.1.0
 * @date     February, 2013
 *
 * @section  LICENSE
 *
 * Copyright (C) 2013, Christoph Dinh, Lorenz Esch, Viktor Klueber. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @brief    Contains the declaration of the classifierssetupwidget class.
 *
 */
//=====================================================================================================================
#ifndef CLASSIFIERSSETUPWIDGET_H
#define CLASSIFIERSSETUPWIDGET_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
//#include <QWidget>
#include <QtWidgets>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================
namespace Ui {
    class ClassifiersSetupWidget;
}

//=====================================================================================================================
// DEFINE NAMESPACE CLASSIFIERSPLUGIN
//=====================================================================================================================
namespace CLASSIFIERSPLUGIN
{

//=====================================================================================================================
// CLASSIFIERSPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================
class Classifiers;

//=====================================================================================================================
/**
 * DECLARE CLASSClassifiersSetupWidget
 *
 * @brief The ClassifiersSetupWidget class provides the Classifiers configuration window.
 */
class ClassifiersSetupWidget : public QWidget
{
    Q_OBJECT

public:
    //=================================================================================================================
    /**
     * Constructs a ClassifiersSetupWidget which is a child of parent.
     *
     * @param [in] toolbox a pointer to the corresponding Classifiers.
     * @param [in] parent pointer to parent widget;
     *              If parent is 0, the new ClassifiersSetupWidget becomes a window.
     *              If parent is another widget, ClassifiersSetupWidget becomes a child window inside parent.
     *              ClassifiersSetupWidget is deleted when its parent is deleted.
     */
    ClassifiersSetupWidget(Classifiers *pClassifiers = 0, QWidget *parent = 0);

    //=================================================================================================================
    /**
     * Destroys the ClassifiersSetupWidget.
     * All ClassifiersSetupWidget's children are deleted first.
     * The application exits if ClassifiersSetupWidget is the main widget.
     */
    ~ClassifiersSetupWidget();

private:
    Classifiers*               m_pClassifiers;	/**< Holds a pointer to corresponding Classifiers.*/

    Ui::ClassifiersSetupWidget*  m_pUi;              /**< Holds the user interface for the ClassifiersSetupWidget.*/
};
} // NAMESPACE

#endif // CLASSIFIERSSETUPWIDGET_H
