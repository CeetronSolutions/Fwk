//##################################################################################################
//
//   Custom Visualization Core library
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

#include "cafPdmUiFieldEditorHandle.h"

#include <QAbstractItemModel>

class QLabel;
class QTreeView;
class QAbstractItemModel;
class QCheckBox;
class QLineEdit;
class QSortFilterProxyModel;
class QModelIndex;
class QItemSelection;

namespace caf
{
class PdmUiTreeSelectionQModel;

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTreeSelectionEditorAttribute : public PdmUiEditorAttribute
{
public:
    bool showTextFilter;
    bool showToggleAllCheckbox;

    caf::PdmUiFieldHandle* highLightField;

public:
    PdmUiTreeSelectionEditorAttribute()
    {
        showTextFilter = true;
        showToggleAllCheckbox = true;

        highLightField = nullptr;
    }
};

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTreeSelectionEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTreeSelectionEditor();
    virtual ~PdmUiTreeSelectionEditor();

protected:
    virtual void        configureAndUpdateUi(const QString& uiConfigName);
    virtual QWidget*    createEditorWidget(QWidget* parent);
    virtual QWidget*    createLabelWidget(QWidget* parent);

private slots:
    void                customMenuRequested(const QPoint& pos);

    void                slotSetSelectedOn();
    void                slotSetSelectedOff();
    void                slotSetSubItemsOn();
    void                slotSetSubItemsOff();

    void                slotToggleAll();

    void                slotTextFilterChanged();

    void                slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous);

private:
    void                checkAllItems();
    void                unCheckAllItems();

    QModelIndexList     allVisibleSourceModelIndices() const;
    void                recursiveAppendVisibleSourceModelIndices(const QModelIndex& parent,
                                                                 QModelIndexList* sourceModelIndices) const;

private:
    QPointer<QTreeView>         m_treeView;
    QPointer<QLabel>            m_label;
    QPointer<QCheckBox>         m_toggleAllCheckBox;
    QPointer<QLineEdit>         m_textFilterLineEdit;

    PdmUiTreeSelectionQModel*   m_model;
    QSortFilterProxyModel*      m_proxyModel;

    PdmUiTreeSelectionEditorAttribute m_attributes;
};

} // end namespace caf
