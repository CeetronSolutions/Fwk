//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

// Needed for moc file
#include "cvfBase.h"
#include "cvfObject.h"

#include "cvfuSnippetFactory.h"

#include <QtGui/QWidget>

class QDockWidget;
class QLabel;
class QLineEdit;
class QComboBox;
class QLineEdit;
class QPushButton;



//==================================================================================================
//
// QSRRunPanel
//
//==================================================================================================
class QSRRunPanel : public QWidget
{
	Q_OBJECT

public:
	QSRRunPanel(QDockWidget* parent);

private:
    void    updateAvailableSnippets();
    void    updateCurrSnippetInfoWidgets();
    void    executeCurrentSnippet();

private slots:
    void    slotCategoryComboCurrentIndexChanged();
    void    slotCurrSnippetEditReturnPressed();
    void    slotNextButtonClicked();
    void    slotPrevButtonClicked();

private:
    std::vector<cvfu::SnippetInfo>  m_availableSnippets;
    int                             m_currSnippetIndex;
    QComboBox*                      m_categoryFilterCombo;
    QLabel*		                    m_infoLabel;
    QLineEdit*		                m_currSnippetEdit;
    QPushButton*		            m_nextButton;
    QPushButton*		            m_prevButton;
};




