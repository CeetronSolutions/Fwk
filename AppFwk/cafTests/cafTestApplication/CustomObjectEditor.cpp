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


#include "CustomObjectEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiDateEditor.h"

#include <QGridLayout>
#include <QWidget>
#include <QDate>
#include <QDateTime>
#include "cafPdmUiDefaultObjectEditor.h"



namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CustomObjectEditor::CustomObjectEditor()
{
    m_columnCount = 2;
    m_rowCount = 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CustomObjectEditor::~CustomObjectEditor()
{
    // If there are field editor present, the usage of this editor has not cleared correctly
    // The intended usage is to call the method setPdmObject(NULL) before closing the dialog
    CAF_ASSERT(m_fieldViews.size() == 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::defineGrid(int rows, int columns)
{
    m_rowCount = rows;
    m_columnCount = columns;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::addWidget(QWidget* w, int row, int column, int rowSpan, int columnSpan, Qt::Alignment /*= 0*/)
{
    CAF_ASSERT(isAreaAvailable(row, column, rowSpan, columnSpan));

    m_customWidgetAreas.push_back(WidgetAndArea(w, CustomObjectEditor::cellIds(row, column, rowSpan, columnSpan)));

    m_layout->addWidget(w, row, column, rowSpan, columnSpan);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::addBlankCell(int row, int column)
{
    m_customWidgetAreas.push_back(WidgetAndArea(nullptr, CustomObjectEditor::cellIds(row, column, 1, 1)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CustomObjectEditor::isAreaAvailable(int row, int column, int rowSpan, int columnSpan) const
{
    auto candidateCells = CustomObjectEditor::cellIds(row, column, rowSpan, columnSpan);
    for (auto candidateCell : candidateCells)
    {
        if (isCellOccupied(candidateCell))
        {
            return false;
        }
    }

    if (row + rowSpan > m_rowCount) return false;
    if (column + columnSpan > m_columnCount) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CustomObjectEditor::isCellOccupied(int cellId) const
{
    for (auto customArea : m_customWidgetAreas)
    {
        for (auto occupiedCell : customArea.m_customWidgetCellIds)
        {
            if (cellId == occupiedCell)
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::removeWidget(QWidget* w)
{
    size_t indexToRemove = 10000;
    for (size_t i = 0; i < m_customWidgetAreas.size(); i++)
    {
        if (w == m_customWidgetAreas[i].m_customWidget)
        {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove < 10000)
    {
        m_layout->removeWidget(w);

        m_customWidgetAreas.erase(m_customWidgetAreas.begin() + indexToRemove);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* CustomObjectEditor::createWidget(QWidget* parent)
{
    m_mainWidget = new QWidget(parent);
    m_layout     = new QGridLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_mainWidget->setLayout(m_layout);
    
    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmUiOrdering config;
    if (pdmObject())
    {
        caf::PdmUiObjectHandle* uiObject = uiObj(pdmObject());
        if (uiObject)
        {
            uiObject->uiOrdering(uiConfigName, config);
        }
    }

    // Set all fieldViews to be unvisited
    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        it->second->setField(NULL);
    }

    // Set all group Boxes to be unvisited
    m_newGroupBoxes.clear();

    const std::vector<PdmUiItem*>& uiItems = config.uiItems();
    
    // TODO: Review that is it not breaking anything to have fields with identical keywords
    //     {
    //         std::set<QString> fieldKeywordNames;
    //         std::set<QString> groupNames;
    // 
    //         recursiveVerifyUniqueNames(uiItems, uiConfigName, &fieldKeywordNames, &groupNames);
    //     }

    recursiveSetupFieldsAndGroupsRoot(uiItems, m_mainWidget, m_layout, uiConfigName);

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector< PdmFieldHandle* > fvhToRemoveFromMap;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        if (it->second->field() == 0)
        {
            PdmUiFieldEditorHandle* fvh = it->second;
            delete fvh;
            fvhToRemoveFromMap.push_back(it->first);               
        }
    }

    for (size_t i = 0; i < fvhToRemoveFromMap.size(); ++i)
    {
        m_fieldViews.erase(fvhToRemoveFromMap[i]);
    }

    // Remove all unmentioned group boxes

    std::map<QString, QPointer<QMinimizePanel> >::iterator itOld;
    std::map<QString, QPointer<QMinimizePanel> >::iterator itNew;

    for (itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld )
    {
        itNew = m_newGroupBoxes.find(itOld->first);
        if (itNew == m_newGroupBoxes.end()) 
        {
            // The old groupBox is not present anymore, get rid of it
            if (!itOld->second.isNull()) delete itOld->second;
        }
    }
    m_groupBoxes = m_newGroupBoxes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::cleanupBeforeSettingPdmObject()
{
    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        PdmUiFieldEditorHandle* fvh = it->second;
        delete fvh;
    }
    m_fieldViews.clear();

    m_newGroupBoxes.clear();

    std::map<QString, QPointer<QMinimizePanel> >::iterator groupIt;
    for (groupIt = m_groupBoxes.begin(); groupIt != m_groupBoxes.end(); ++groupIt)
    {
        if (!groupIt->second.isNull()) groupIt->second->deleteLater();
    }

    m_groupBoxes.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::resetDynamicCellCounter()
{
    m_dynamicCellIndex = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<int, int> CustomObjectEditor::rowAndCell(int cellId) const
{
    int column = cellId % m_columnCount;
    int row = cellId / m_columnCount;

    return std::make_pair(row, column);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int CustomObjectEditor::cellId(int row, int column) const
{
    return row * m_columnCount + column;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int CustomObjectEditor::getNextAvailableCellId()
{
    while (isCellOccupied(m_dynamicCellIndex) && m_dynamicCellIndex < (m_rowCount * m_columnCount))
    {
        m_dynamicCellIndex++;
    }

    if (isCellOccupied(m_dynamicCellIndex))
    {
        return -1;
    }
    else
    {
        return m_dynamicCellIndex++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> CustomObjectEditor::cellIds(int row, int column, int rowSpan, int columnSpan) const
{
    std::vector<int> cells;

    for (auto r = row; r < row + rowSpan; r++)
    {
        for (auto c = column; c < column + columnSpan; c++)
        {
            cells.push_back(cellId(r, c));
        }
    }

    return cells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::recursiveSetupFieldsAndGroupsRoot(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName)
{
    resetDynamicCellCounter();

    QWidget* previousTabOrderWidget = NULL;

    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(uiConfigName)) continue;

        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = group->keyword();
            QMinimizePanel* groupBox = NULL;
            QGridLayout* groupBoxLayout = NULL;

            // Find or create groupBox
            std::map<QString, QPointer<QMinimizePanel> >::iterator it;
            it = m_groupBoxes.find(groupBoxKey);

            if (it == m_groupBoxes.end())
            {
                groupBox = new QMinimizePanel(parent);
                groupBox->setTitle(group->uiName(uiConfigName));
                groupBox->setObjectName(group->keyword());
                groupBoxLayout = new QGridLayout();
                groupBox->contentFrame()->setLayout(groupBoxLayout);
                connect(groupBox, SIGNAL(expandedChanged(bool)), this, SLOT(groupBoxExpandedStateToggled(bool)));

                m_newGroupBoxes[groupBoxKey] = groupBox;
            }
            else
            {
                groupBox = it->second;
                CAF_ASSERT(groupBox);

                groupBoxLayout = dynamic_cast<QGridLayout*>(groupBox->contentFrame()->layout());
                CAF_ASSERT(groupBoxLayout);

                m_newGroupBoxes[groupBoxKey] = groupBox;
            }

            /// Insert the group box at the correct position of the parent layout

            int nextCellId = getNextAvailableCellId();
            std::pair<int, int> rowCol = rowAndCell(nextCellId);
            parentLayout->addWidget(groupBox, rowCol.first, rowCol.second, 1, 1);

            // Set Expanded state
            bool isExpanded = isUiGroupExpanded(group);
            groupBox->setExpanded(isExpanded);

            // Update the title to be able to support dynamic group names
            groupBox->setTitle(group->uiName(uiConfigName));

            recursiveSetupFieldsAndGroups(groupChildren, groupBox->contentFrame(), groupBoxLayout, uiConfigName);
        }
    }

    // TODO : how to handle fields at top level
    //recursiveSetupFieldsAndGroups(uiItems, parent, parentLayout, uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName)
{
    int currentRowIndex = 0;
    QWidget* previousTabOrderWidget = NULL;

    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiHidden(uiConfigName)) continue;

        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = group->keyword();
            QMinimizePanel* groupBox = NULL;
            QGridLayout* groupBoxLayout = NULL;

            // Find or create groupBox
            std::map<QString, QPointer<QMinimizePanel> >::iterator it;
            it = m_groupBoxes.find(groupBoxKey);

            if (it == m_groupBoxes.end())
            {
                groupBox = new QMinimizePanel( parent );
                groupBox->setTitle(group->uiName(uiConfigName));
                groupBox->setObjectName(group->keyword());
                groupBoxLayout = new QGridLayout();
                groupBox->contentFrame()->setLayout(groupBoxLayout);
                connect(groupBox, SIGNAL(expandedChanged(bool)), this, SLOT(groupBoxExpandedStateToggled(bool)));

                m_newGroupBoxes[groupBoxKey] = groupBox;
            }
            else
            {
                groupBox = it->second;
                CAF_ASSERT(groupBox);
                
                groupBoxLayout = dynamic_cast<QGridLayout*>(groupBox->contentFrame()->layout());
                CAF_ASSERT(groupBoxLayout);
                
                m_newGroupBoxes[groupBoxKey] = groupBox;
            }

            /// Insert the group box at the correct position of the parent layout

            parentLayout->addWidget(groupBox, currentRowIndex, 0, 1, 2);

            // Set Expanded state
            bool isExpanded = isUiGroupExpanded(group);
            groupBox->setExpanded(isExpanded);

            // Update the title to be able to support dynamic group names
            groupBox->setTitle(group->uiName(uiConfigName));

            recursiveSetupFieldsAndGroups(groupChildren, groupBox->contentFrame(), groupBoxLayout, uiConfigName);
            currentRowIndex++;
        }
        else
        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItems[i]);
            PdmUiFieldEditorHandle* fieldEditor = NULL;

            // Find or create FieldEditor
            std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>::iterator it;
            it = m_fieldViews.find(field->fieldHandle());

            if (it == m_fieldViews.end())
            {
                fieldEditor = PdmUiFieldEditorHelper::fieldEditorForField(field, uiConfigName);

                if (fieldEditor)
                {
                    m_fieldViews[field->fieldHandle()] = fieldEditor;
                    fieldEditor->createWidgets(parent);
                }
                else
                {
                    // This assert happens if no editor is available for a given field
                    // If the macro for registering the editor is put as the single statement
                    // in a cpp file, a dummy static class must be used to make sure the compile unit
                    // is included
                    //
                    // See cafPdmUiCoreColor3f and cafPdmUiCoreVec3d

                    // This assert will trigger for PdmChildArrayField and PdmChildField
                    // Consider to exclude assert or add editors for these types if the assert is reintroduced
                    //CAF_ASSERT(false);
                }
            }
            else
            {
                fieldEditor = it->second;
            }

            if (fieldEditor)
            {
                fieldEditor->setField(field); 

                // Place the widget(s) into the correct parent and layout
                QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                if (fieldCombinedWidget)
                {
                    fieldCombinedWidget->setParent(parent);
                    parentLayout->addWidget(fieldCombinedWidget, currentRowIndex, 0, 1, 2);
                }
                else
                {

                    PdmUiItemInfo::LabelPosType labelPos = field->uiLabelPosition(uiConfigName);
                    bool labelOnTop = (labelPos == PdmUiItemInfo::TOP);
                    bool editorSpanBoth = labelOnTop;

                    QWidget* fieldEditorWidget = fieldEditor->editorWidget();

                    if (labelPos != PdmUiItemInfo::HIDDEN)
                    {
                        QWidget* fieldLabelWidget  = fieldEditor->labelWidget();
                        if (fieldLabelWidget )
                        {
                            fieldLabelWidget->setParent(parent);

                            // Label widget will span two columns if aligned on top
                            int colSpan = labelOnTop ? 2 : 1;
                            // If the label is on the side, and the editor can expand vertically, allign the label with the top edge of the editor
                            if (!labelOnTop && (fieldEditorWidget->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag))
                                parentLayout->addWidget(fieldLabelWidget, currentRowIndex, 0, 1, colSpan, Qt::AlignTop);
                            else
                                parentLayout->addWidget(fieldLabelWidget, currentRowIndex, 0, 1, colSpan, Qt::AlignVCenter);

                            fieldLabelWidget->show();

                            if (labelOnTop) currentRowIndex++;
                        }
                    }
                    else
                    {
                        QWidget* fieldLabelWidget  = fieldEditor->labelWidget();
                        if (fieldLabelWidget ) fieldLabelWidget->hide();
                        editorSpanBoth = true; // To span both columns when there is no label
                    }


                    if (fieldEditorWidget)
                    {
                        fieldEditorWidget->setParent(parent); // To make sure this widget has the current group box as parent.

                        // Label widget will span two columns if aligned on top
                        int colSpan = editorSpanBoth ? 2 : 1;
                        int colIndex = editorSpanBoth ? 0 : 1;
                        parentLayout->addWidget(fieldEditorWidget, currentRowIndex, colIndex, 1, colSpan, Qt::AlignTop);

                        if (previousTabOrderWidget) QWidget::setTabOrder(previousTabOrderWidget, fieldEditorWidget);
                        
                        previousTabOrderWidget = fieldEditorWidget;
                    }

                }

                fieldEditor->updateUi(uiConfigName);

                currentRowIndex++;
            }

        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CustomObjectEditor::isUiGroupExpanded(const PdmUiGroup* uiGroup)
{
    if (uiGroup->hasForcedExpandedState()) return uiGroup->forcedExpandedState();

    auto kwMapPair = m_objectKeywordGroupUiNameExpandedState.find(pdmObject()->xmlCapability()->classKeyword());
    if ( kwMapPair != m_objectKeywordGroupUiNameExpandedState.end() )
    {
        QString keyword = uiGroup->keyword();

        auto uiNameExpStatePair = kwMapPair->second.find(keyword);
        if ( uiNameExpStatePair != kwMapPair->second.end() )
        {
            return uiNameExpStatePair->second;
        }
    }

    return uiGroup->isExpandedByDefault();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::recursiveVerifyUniqueNames(const std::vector<PdmUiItem*>& uiItems, const QString& uiConfigName, std::set<QString>* fieldKeywordNames, std::set<QString>* groupNames)
{
    for (size_t i = 0; i < uiItems.size(); ++i)
    {
        if (uiItems[i]->isUiGroup())
        {
            PdmUiGroup* group = static_cast<PdmUiGroup*>(uiItems[i]);
            const std::vector<PdmUiItem*>& groupChildren = group->uiItems();

            QString groupBoxKey = group->keyword();

            if (groupNames->find(groupBoxKey) != groupNames->end())
            {
                // It is not supported to have two groups with identical names
                CAF_ASSERT(false);
            }
            else
            {
                groupNames->insert(groupBoxKey);
            }

            recursiveVerifyUniqueNames(groupChildren, uiConfigName, fieldKeywordNames, groupNames);
        }
        else
        {
            PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>(uiItems[i]);

            QString fieldKeyword = field->fieldHandle()->keyword();
            if (fieldKeywordNames->find(fieldKeyword) != fieldKeywordNames->end())
            {
                // It is not supported to have two fields with identical names
                CAF_ASSERT(false);
            }
            else
            {
                fieldKeywordNames->insert(fieldKeyword);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CustomObjectEditor::groupBoxExpandedStateToggled(bool isExpanded)
{
    if (!this->pdmObject()->xmlCapability()) return;

    QString objKeyword = this->pdmObject()->xmlCapability()->classKeyword();
    QMinimizePanel* panel = dynamic_cast<QMinimizePanel*>(this->sender());
    
    if (!panel) return;

    m_objectKeywordGroupUiNameExpandedState[objKeyword][panel->objectName()] = isExpanded;
}

} // end namespace caf
