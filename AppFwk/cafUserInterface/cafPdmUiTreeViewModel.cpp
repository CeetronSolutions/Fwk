//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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


#include "cafPdmUiTreeViewModel.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDragDropHandle.h"
#include "cafPdmUiTreeItemEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewEditor.h"

#include <QTreeView>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewModel::PdmUiTreeViewModel(PdmUiTreeViewEditor* treeViewEditor)
{
    m_treeOrderingRoot = NULL;
    m_dragDropHandle = NULL;

    m_treeViewEditor = treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// Will populate the tree with the contents of the Pdm data structure rooted at rootItem.
/// Will not show the rootItem itself, only the children and downwards 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setPdmItemRoot(PdmUiItem* rootItem)
{
    // Check if we are already watching this root
    if (m_treeOrderingRoot && m_treeOrderingRoot->activeItem() == rootItem)
    {
        this->updateSubTree(rootItem);
        return;
    }

    PdmUiTreeOrdering* newRoot = NULL;
    PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*> (rootItem);

    if (field)
    {
        newRoot = new PdmUiTreeOrdering(field->fieldHandle());
        PdmUiObjectHandle::expandUiTree(newRoot, m_uiConfigName);
    }
    else
    {
        PdmUiObjectHandle * obj = dynamic_cast<PdmUiObjectHandle*> (rootItem);
        if (obj)
        {
            newRoot = obj->uiTreeOrdering(m_uiConfigName);
        }
    }

    assert( newRoot || rootItem == NULL ); // Only fields, objects or NULL is allowed.

    //if (newRoot) newRoot->debugDump(0);

    this->resetTree(newRoot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::resetTree(PdmUiTreeOrdering* newRoot)
{
    beginResetModel();
    
    if (m_treeOrderingRoot)
    {
        delete m_treeOrderingRoot;
    }

    m_treeOrderingRoot = newRoot;

    updateEditorsForSubTree(m_treeOrderingRoot);

    endResetModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setColumnHeaders(const QStringList& columnHeaders)
{
    m_columnHeaders = columnHeaders;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::emitDataChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root PdmUiItem
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateSubTree(PdmUiItem* pdmRoot)
{
    // Build the new "Correct" Tree

    PdmUiTreeOrdering* newTreeRootTmp = NULL;
    PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*> (pdmRoot);
    if (field)
    {
        newTreeRootTmp = new PdmUiTreeOrdering(field->fieldHandle());
    }
    else
    {
        PdmUiObjectHandle* obj = dynamic_cast<PdmUiObjectHandle*> (pdmRoot);
        if (obj)
        {
            newTreeRootTmp = new PdmUiTreeOrdering(obj->objectHandle());
        }
    }

    PdmUiObjectHandle::expandUiTree(newTreeRootTmp, m_uiConfigName);

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "New Stuff: " << std::endl ;
    newTreeRootTmp->debugDump(0);
#endif

    // Find the corresponding entry for "root" in the existing Ui tree

    QModelIndex existingSubTreeRootModIdx = findModelIndex(pdmRoot);

    PdmUiTreeOrdering* existingSubTreeRoot = NULL;
    if (existingSubTreeRootModIdx.isValid())
    {
        existingSubTreeRoot = treeItemFromIndex(existingSubTreeRootModIdx);
    }
    else
    {
        existingSubTreeRoot = m_treeOrderingRoot;
    }

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Old :"<< std::endl ;
    existingSubTreeRoot->debugDump(0);
#endif

    updateSubTreeRecursive(existingSubTreeRootModIdx, existingSubTreeRoot, newTreeRootTmp);

    delete newTreeRootTmp;

    updateEditorsForSubTree(existingSubTreeRoot);

	// Notify Qt that the toggle/name/icon etc might have been changed
	emitDataChanged(existingSubTreeRootModIdx);

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Result :"<< std::endl ;
    existingSubTreeRoot->debugDump(0);
#endif	
}

class RecursiveUpdateData
{
public:
    RecursiveUpdateData(QModelIndex mi, PdmUiTreeOrdering* existingChild, PdmUiTreeOrdering* sourceChild)
        : m_modelIndex(mi),
        m_existingChild(existingChild),
        m_sourceChild(sourceChild)
    {
    };

    QModelIndex         m_modelIndex;
    PdmUiTreeOrdering*  m_existingChild;
    PdmUiTreeOrdering*  m_sourceChild;
};

//--------------------------------------------------------------------------------------------------
/// Makes the existingSubTreeRoot tree become identical to the tree in sourceSubTreeRoot, 
/// calling begin..() end..() to make the UI update accordingly.
/// This assumes that all the items have a pointer an unique PdmObject 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateSubTreeRecursive(const QModelIndex& existingSubTreeRootModIdx, PdmUiTreeOrdering* existingSubTreeRoot, PdmUiTreeOrdering* sourceSubTreeRoot)
{
    // Build map for source items
    std::map<caf::PdmUiItem*, int> sourceTreeMap;
    for (int i = 0; i < sourceSubTreeRoot->childCount() ; ++i)
    {
        PdmUiTreeOrdering* child = sourceSubTreeRoot->child(i);

        if (child && child->activeItem())
        {
            sourceTreeMap[child->activeItem()] = i;
        }
    }

    // Detect items to be deleted from existing tree
    std::vector<int> indicesToRemoveFromExisting;
    for (int i = 0; i < existingSubTreeRoot->childCount() ; ++i)
    {
        PdmUiTreeOrdering* child = existingSubTreeRoot->child(i);

        std::map<caf::PdmUiItem*, int>::iterator it = sourceTreeMap.find(child->activeItem());
        if (it == sourceTreeMap.end())
        {
            indicesToRemoveFromExisting.push_back(i);
        }
    }

    // Delete items with largest index first from existing
    for (std::vector<int>::reverse_iterator it = indicesToRemoveFromExisting.rbegin(); it != indicesToRemoveFromExisting.rend(); it++)
    {
        this->beginRemoveRows(existingSubTreeRootModIdx, *it, *it);
        existingSubTreeRoot->removeChildren(*it, 1);
        this->endRemoveRows();
    }

    // Build map for existing items without the deleted items
    std::map<caf::PdmUiItem*, int> existingTreeMap;
    for (int i = 0; i < existingSubTreeRoot->childCount() ; ++i)
    {
        PdmUiTreeOrdering* child = existingSubTreeRoot->child(i);

        if (child && child->activeItem())
        {
            existingTreeMap[child->activeItem()] = i;
        }
    }

    // Check if there are any changes between existing and source
    // If no changes, update the subtree recursively
    bool anyChanges = false;
    if (existingSubTreeRoot->childCount() == sourceSubTreeRoot->childCount())
    {
        for (int i = 0; i < existingSubTreeRoot->childCount(); ++i)
        {
            if (existingSubTreeRoot->child(i)->activeItem() != sourceSubTreeRoot->child(i)->activeItem())
            {
                anyChanges = true;
                break;
            }
        }
    }
    else
    {
        anyChanges = true;
    }

    if (!anyChanges)
    {
        // No changes to list of children at this level, call update on all children
        for (int i = 0; i < existingSubTreeRoot->childCount(); ++i)
        {
            updateSubTreeRecursive( index(i, 0, existingSubTreeRootModIdx), existingSubTreeRoot->child(i), sourceSubTreeRoot->child(i));
        }
    }
    else
    {
        std::vector<RecursiveUpdateData> recursiveUpdateData;
        std::vector<PdmUiTreeOrdering*> newMergedOrdering;

        emit layoutAboutToBeChanged();
        {
            // Detect items to be moved from source to existing
            // Merge items from existing and source into newMergedOrdering using order in sourceSubTreeRoot
            std::vector<int> indicesToRemoveFromSource;
            for (int i = 0; i < sourceSubTreeRoot->childCount() ; ++i)
            {
                PdmUiTreeOrdering* sourceChild = sourceSubTreeRoot->child(i);
                std::map<caf::PdmUiItem*, int>::iterator it = existingTreeMap.find(sourceChild->activeItem());
                if (it != existingTreeMap.end())
                {
                    newMergedOrdering.push_back(existingSubTreeRoot->child(it->second));

                    recursiveUpdateData.push_back(RecursiveUpdateData(index(static_cast<int>(newMergedOrdering.size() - 1), 0, existingSubTreeRootModIdx), existingSubTreeRoot->child(it->second), sourceChild));
                }
                else
                {
                    newMergedOrdering.push_back(sourceChild);

                    indicesToRemoveFromSource.push_back(i);
                }
            }

            // Delete new items from source because they have been moved into newMergedOrdering
            for (std::vector<int>::reverse_iterator it = indicesToRemoveFromSource.rbegin(); it != indicesToRemoveFromSource.rend(); it++)
            {
                // Use the removeChildrenNoDelete() to remove the pointer from the list without deleting the pointer
                sourceSubTreeRoot->removeChildrenNoDelete(*it, 1);
            }

            // Delete all items from existingSubTreeRoot, as the complete list is present in newMergedOrdering
            existingSubTreeRoot->removeChildrenNoDelete(0, existingSubTreeRoot->childCount());

            // First, reorder all items in existing tree, as this operation is valid when later emitting the signal layoutChanged()
            // Insert of new items before issuing this signal causes the tree items below the inserted item to collapse
            for (size_t i = 0; i < newMergedOrdering.size(); i++)
            {
                if (existingTreeMap.find(newMergedOrdering[i]->activeItem()) != existingTreeMap.end())
                {
                    existingSubTreeRoot->appendChild(newMergedOrdering[i]);
                }
            }
        }

        emit layoutChanged();

        // Insert new items into existingSubTreeRoot
        for (size_t i = 0; i < newMergedOrdering.size(); i++)
        {
            if (existingTreeMap.find(newMergedOrdering[i]->activeItem()) == existingTreeMap.end())
            {
                this->beginInsertRows(existingSubTreeRootModIdx, static_cast<int>(i), static_cast<int>(i));
                existingSubTreeRoot->insertChild(static_cast<int>(i), newMergedOrdering[i]);
                this->endInsertRows();
            }
        }

        for (size_t i = 0; i < recursiveUpdateData.size(); i++)
        {
            updateSubTreeRecursive(recursiveUpdateData[i].m_modelIndex, recursiveUpdateData[i].m_existingChild, recursiveUpdateData[i].m_sourceChild);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateEditorsForSubTree(PdmUiTreeOrdering* root)
{
    if (!root) return;

    if (!root->editor())
    {
        PdmUiTreeItemEditor* treeItemEditor = new PdmUiTreeItemEditor(root->activeItem());
        root->setEditor(treeItemEditor);
        assert(root->editor());
    }

    PdmUiTreeItemEditor* treeItemEditor = dynamic_cast<PdmUiTreeItemEditor*>(root->editor());
    if (treeItemEditor)
    {
        treeItemEditor->setTreeViewEditor(m_treeViewEditor);
    }

    for (int i = 0; i < root->childCount(); ++i)
    {
        updateEditorsForSubTree(root->child(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* caf::PdmUiTreeViewModel::treeItemFromIndex(const QModelIndex& index) const 
{
    if (!index.isValid())
    {
        return m_treeOrderingRoot;
    }
    
    assert(index.internalPointer());

    PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>(index.internalPointer());

    return treeItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::findModelIndex( const PdmUiItem * object) const
{
    QModelIndex foundIndex;
    int numRows = rowCount(QModelIndex());
    int r = 0;
    while (r < numRows && !foundIndex.isValid())
    {
        foundIndex = findModelIndexRecursive(index(r, 0, QModelIndex()), object);
        ++r;
    }
    return foundIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::findModelIndexRecursive(const QModelIndex& currentIndex, const PdmUiItem * pdmItem) const
{
    if (currentIndex.internalPointer())
    {
        PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>(currentIndex.internalPointer());
        if (treeItem->activeItem() == pdmItem) return currentIndex;
    }

   int row;
   for (row = 0; row < rowCount(currentIndex); ++row)
   {
       QModelIndex foundIndex = findModelIndexRecursive(index(row, 0, currentIndex), pdmItem);
       if (foundIndex.isValid()) return foundIndex;
   }
   return QModelIndex();
}



//--------------------------------------------------------------------------------------------------
/// An invalid parent index is implicitly meaning the root item, and not "above" root, since
/// we are not showing the root item itself
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::index(int row, int column, const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return QModelIndex();

    PdmUiTreeOrdering* parentItem = NULL;

    if (!parentIndex.isValid())
        parentItem = m_treeOrderingRoot;
    else
        parentItem = static_cast<PdmUiTreeOrdering*>(parentIndex.internalPointer());

    assert(parentItem);

    if (parentItem->childCount() <= row)
    {
        return QModelIndex();
    }

    PdmUiTreeOrdering* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid()) return QModelIndex();

    PdmUiTreeOrdering* childItem = static_cast<PdmUiTreeOrdering*>(childIndex.internalPointer());
    if (!childItem) return QModelIndex();

    PdmUiTreeOrdering* parentItem = childItem->parent();
    if (!parentItem) return QModelIndex();

    if (parentItem == m_treeOrderingRoot) return QModelIndex();

    return createIndex(parentItem->indexInParent(), 0, parentItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::rowCount(const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return 0;

    if (parentIndex.column() > 0)
        return 0;

    PdmUiTreeOrdering* parentItem = this->treeItemFromIndex(parentIndex);

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::columnCount(const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return 0;

    return 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::data(const QModelIndex &index, int role ) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    PdmUiTreeOrdering* uitreeOrdering = static_cast<PdmUiTreeOrdering*>(index.internalPointer());
    if (!uitreeOrdering)
    {
        return QVariant();
    }

    bool isObjRep = uitreeOrdering->isRepresentingObject();
    bool isFieldRep = uitreeOrdering->isRepresentingField();
    bool isDisplayOnly = uitreeOrdering->isDisplayItemOnly();


    if (role == Qt::DisplayRole && !uitreeOrdering->isValid())
    {
        QString str;

#ifdef DEBUG
        str = "Invalid uiordering";
#endif

        return QVariant(str);
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (isObjRep)
        {
            PdmUiObjectHandle* pdmUiObject = uiObj(uitreeOrdering->object());
            if (pdmUiObject)
            {
                QVariant v;
                if (pdmUiObject->userDescriptionField())
                {
					caf::PdmUiFieldHandle* uiFieldHandle = pdmUiObject->userDescriptionField()->uiCapability();
                    if (uiFieldHandle)
                    {
                        v = uiFieldHandle->uiValue();
                    }
                }
                else
                {
                    v = pdmUiObject->uiName();
                }

                QString txt = v.toString();

                if (m_treeViewEditor->isAppendOfClassNameToUiItemTextEnabled())
                {
                    PdmObjectHandle* pdmObjHandle = pdmUiObject->objectHandle();
                    if (pdmObjHandle)
                    {
                        txt += " - ";
                        txt += typeid(*pdmObjHandle).name();
                    }
                }

                return txt;
            }
            else
            {
                return QVariant();
            }
        }
       
        if (uitreeOrdering->activeItem())
        {
            return uitreeOrdering->activeItem()->uiName();
        }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (uitreeOrdering->activeItem())
        {
            return uitreeOrdering->activeItem()->uiIcon();
        }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        if (uitreeOrdering->activeItem())
        {
             return uitreeOrdering->activeItem()->uiToolTip();
        }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::WhatsThisRole)
    {
        if (uitreeOrdering->activeItem())
        {
            return uitreeOrdering->activeItem()->uiWhatsThis();
        }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (isObjRep)
        {
            PdmUiObjectHandle* pdmUiObj = uiObj(uitreeOrdering->object());
            if (pdmUiObj && pdmUiObj->objectToggleField())
            {
				caf::PdmUiFieldHandle* uiFieldHandle = pdmUiObj->objectToggleField()->uiCapability();
                if (uiFieldHandle)
                {
                    bool isToggledOn = uiFieldHandle->uiValue().toBool();
                    if (isToggledOn)
                    {
                        return Qt::Checked;
                    }
                    else
                    {
                        return Qt::Unchecked;
                    }
                }
                else
                {
                    return QVariant();
                }
            }
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
    {
        return false;
    }

    PdmUiTreeOrdering* treeItem = PdmUiTreeViewModel::treeItemFromIndex(index);
    assert(treeItem);

    if (!treeItem->isRepresentingObject()) return false;

    PdmUiObjectHandle* uiObject = uiObj(treeItem->object());
    if (uiObject)
    {
        if (role == Qt::EditRole && uiObject->userDescriptionField())
        {
			PdmUiFieldHandle* userDescriptionUiField = uiObject->userDescriptionField()->uiCapability();
            if (userDescriptionUiField)
            {
                userDescriptionUiField->setValueFromUi(value);
            }

            return true;
        }
        else if (role == Qt::CheckStateRole && uiObject->objectToggleField())
        {
            bool toggleOn = (value == Qt::Checked);

			PdmUiFieldHandle* toggleUiField = uiObject->objectToggleField()->uiCapability();
            if (toggleUiField)
            {
                toggleUiField->setValueFromUi(value);
            }

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Enable edit of this item if we have a editable user description field for a pdmObject
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags PdmUiTreeViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index);

    PdmUiTreeOrdering* treeItem = treeItemFromIndex(index);
    assert(treeItem);

    if (treeItem->isRepresentingObject())
    {
        PdmUiObjectHandle* pdmUiObject = uiObj(treeItem->object());
        if (pdmUiObject)
        {
			if (pdmUiObject->userDescriptionField() && !pdmUiObject->userDescriptionField()->uiCapability()->isUiReadOnly())
        {
            flagMask = flagMask | Qt::ItemIsEditable;
        }

            if (pdmUiObject->objectToggleField())
        {
            flagMask = flagMask | Qt::ItemIsUserCheckable;
        }
    }
    }

    if (treeItem->isValid())
    {
        if (treeItem->activeItem()->isUiReadOnly())
        {
            flagMask = flagMask & (~Qt::ItemIsEnabled);
        }
    }

    if (m_dragDropHandle)
    {
        Qt::ItemFlags dragDropFlags = m_dragDropHandle->flags(index);
        flagMask |= dragDropFlags;
    }

    return flagMask;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (section < m_columnHeaders.size())
    {
        return m_columnHeaders[section];
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeViewModel::uiItemFromModelIndex(const QModelIndex& index) const
{
    PdmUiTreeOrdering* treeItem = this->treeItemFromIndex(index);
    if (treeItem)
    {
        return treeItem->activeItem();
    }
    
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setDragDropHandle(PdmUiDragDropHandle* dragDropHandle)
{
    m_dragDropHandle = dragDropHandle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::endDrag()
{
    if (m_dragDropHandle)
    {
        m_dragDropHandle->endDrag();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList PdmUiTreeViewModel::mimeTypes() const
{
    if (m_dragDropHandle)
    {
        return m_dragDropHandle->mimeTypes();
    }
    else
    {
        return QAbstractItemModel::mimeTypes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMimeData * PdmUiTreeViewModel::mimeData(const QModelIndexList &indexes) const
{
    if (m_dragDropHandle)
    {
        return m_dragDropHandle->mimeData(indexes);
    }
    else
    {
        return QAbstractItemModel::mimeData(indexes);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (m_dragDropHandle)
    {
        return m_dragDropHandle->dropMimeData(data, action, row, column, parent);
    }
    else
    {
        return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::DropActions PdmUiTreeViewModel::supportedDropActions() const
{
    if (m_dragDropHandle)
    {
        return m_dragDropHandle->supportedDropActions();
    }
    else
    {
        return QAbstractItemModel::supportedDropActions();
    }
}


} // end namespace caf
