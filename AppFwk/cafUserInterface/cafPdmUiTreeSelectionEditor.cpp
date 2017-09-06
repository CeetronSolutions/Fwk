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


#include "cafPdmUiTreeSelectionEditor.h"

#include "cafAssert.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeSelectionQModel.h"

#include <QTreeView>
#include <QLabel>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiTreeSelectionEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::PdmUiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::~PdmUiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    bool optionsOnly = true;
    QList<PdmOptionItemInfo> options = field()->valueOptions(&optionsOnly);

    caf::PdmUiTreeSelectionQModel* model = new caf::PdmUiTreeSelectionQModel(m_treeView);
    m_treeView->setModel(model);

    connect(model, SIGNAL(signalSelectionStateForIndexHasChanged(int, bool)), this, SLOT(slotSetSelectionStateForIndex(int, bool)));

    model->setOptions(this, options);

    // TODO: Try to merge expanded state with newly generated tree
    //m_treeView->expandAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectionStateForIndex(int index, bool isSelected)
{
    unsigned int unsignedValue = static_cast<unsigned int>(index);

    QVariant fieldValue = field()->uiValue();
    QList<QVariant> valuesSelectedInField = fieldValue.toList();

    if (!isSelected)
    {
        valuesSelectedInField.removeAll(QVariant(unsignedValue));
    }
    else
    {
        bool isIndexPresent = false;
        for (QVariant v : valuesSelectedInField)
        {
            unsigned int indexInField = v.toUInt();
            if (indexInField == unsignedValue)
            {
                isIndexPresent = true;
            }
        }

        if (!isIndexPresent)
        {
            valuesSelectedInField.push_back(QVariant(unsignedValue));
        }
    }

    this->setValueToField(valuesSelectedInField);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createEditorWidget(QWidget * parent)
{
    m_treeView = new QTreeView(parent);

    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}


} // end namespace caf
