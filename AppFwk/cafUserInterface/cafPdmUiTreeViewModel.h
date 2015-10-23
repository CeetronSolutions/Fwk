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

#pragma once

#include "cafUiTreeItem.h"

#include <QAbstractItemModel>
#include <QStringList>

namespace caf
{

class PdmObjectHandle;
class PdmUiItem;
class PdmUiTreeViewEditor;
class PdmUiTreeOrdering;
class PdmUiDragDropInterface;

//==================================================================================================
//
// This class is intended to replace UiTreeModelPdm (cafUiTreeModelPdm)
//
//==================================================================================================
class PdmUiTreeViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    PdmUiTreeViewModel(PdmUiTreeViewEditor* treeViewEditor);

    void                    setPdmItemRoot(PdmUiItem* rootItem);
    void                    updateSubTree(PdmUiItem* subTreeRoot);

    void                    setColumnHeaders(const QStringList& columnHeaders);
    void                    setUiConfigName(const QString& uiConfigName) { m_uiConfigName = uiConfigName; }
 
    // These are supposed to be used from the Editor only, and to implement selection support.
 
    PdmUiItem*              uiItemFromModelIndex(const QModelIndex& index) const;
    QModelIndex             findModelIndex(const PdmUiItem* object) const;

    void                    setDragDropInterface(PdmUiDragDropInterface* dragDropInterface);
    PdmUiDragDropInterface* dragDropInterface();
    
private:
    void                    updateSubTreeRecursive(const QModelIndex& uiSubTreeRootModelIdx, PdmUiTreeOrdering* uiModelSubTreeRoot, PdmUiTreeOrdering* updatedPdmSubTreeRoot);

    PdmUiTreeOrdering*      treeItemFromIndex(const QModelIndex& index) const;
    QModelIndex             findModelIndexRecursive(const QModelIndex& currentIndex, const PdmUiItem * object) const;

    void                    resetTree(PdmUiTreeOrdering* root);
    void                    emitDataChanged(const QModelIndex& index);
    void                    updateEditorsForSubTree(PdmUiTreeOrdering* root);

    PdmUiTreeOrdering*      m_treeOrderingRoot;
    QStringList             m_columnHeaders;
    QString                 m_uiConfigName;

    PdmUiTreeViewEditor*    m_treeViewEditor;

    PdmUiDragDropInterface* m_dragDropInterface;

private:

    // Overrides from QAbstractItemModel

    virtual QModelIndex     index(int row, int column, const QModelIndex &parentIndex = QModelIndex( )) const;
    virtual QModelIndex     parent(const QModelIndex &index) const;

    virtual int             rowCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;
    virtual int             columnCount(const QModelIndex &parentIndex = QModelIndex( ) ) const;

    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant        headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;

    virtual QStringList     mimeTypes() const;
    virtual QMimeData*      mimeData(const QModelIndexList &indexes) const;
    virtual bool            dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::DropActions supportedDropActions() const;
};



} // End of namespace caf
