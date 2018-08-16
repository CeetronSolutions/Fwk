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

#include "cafPdmUiItem.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiObjectEditorHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo(const QString&  anOptionUiText,
                                     const QVariant& aValue,
                                     bool            isReadOnly /* = false */,
                                     QIcon           anIcon /* = QIcon()*/)
    : m_optionUiText(anOptionUiText)
    , m_value(aValue)
    , m_isReadOnly(isReadOnly)
    , m_icon(anIcon)
    , m_level(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo(const QString&        anOptionUiText,
                                     caf::PdmObjectHandle* obj,
                                     bool                  isReadOnly /*= false*/,
                                     QIcon                 anIcon /*= QIcon()*/)
    : m_optionUiText(anOptionUiText)
    , m_isReadOnly(isReadOnly)
    , m_icon(anIcon)
    , m_level(0)
{
    m_value = QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(obj));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo
    PdmOptionItemInfo::createHeader(const QString& anOptionUiText, bool isReadOnly /*= false*/, QIcon anIcon /*= QIcon()*/)
{
    PdmOptionItemInfo header(anOptionUiText, QVariant(), isReadOnly, anIcon);

    return header;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmOptionItemInfo::setLevel(int level)
{
    m_level = level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmOptionItemInfo::optionUiText() const
{
    return m_optionUiText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QVariant PdmOptionItemInfo::value() const
{
    return m_value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isReadOnly() const
{
    return m_isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isHeading() const
{
    return !m_value.isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon PdmOptionItemInfo::icon() const
{
    return m_icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmOptionItemInfo::level() const
{
    return m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList PdmOptionItemInfo::extractUiTexts(const QList<PdmOptionItemInfo>& optionList)
{
    QStringList texts;

    for (const auto& option : optionList)
    {
        texts.push_back(option.optionUiText());
    }

    return texts;
}

//==================================================================================================
/// PdmUiItem
//==================================================================================================

bool PdmUiItem::sm_showExtraDebugText = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiName(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_uiName.isNull())) return conInfo->m_uiName;
    if (defInfo && !(defInfo->m_uiName.isNull())) return defInfo->m_uiName;
    if (sttInfo && !(sttInfo->m_uiName.isNull())) return sttInfo->m_uiName;

    return QString("");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiName(const QString& uiName, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_uiName = uiName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon PdmUiItem::uiIcon(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_icon.isNull())) return conInfo->m_icon;
    if (defInfo && !(defInfo->m_icon.isNull())) return defInfo->m_icon;
    if (sttInfo && !(sttInfo->m_icon.isNull())) return sttInfo->m_icon;

    return QIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiIcon(const QIcon& uiIcon, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_icon = uiIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiToolTip(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    QString text;

    if (conInfo && !(conInfo->m_toolTip.isNull()))
    {
        text = conInfo->m_toolTip;
        if (PdmUiItem::showExtraDebugText() && !conInfo->m_extraDebugText.isEmpty())
        {
            text += QString(" (%1)").arg(conInfo->m_extraDebugText);
        }
    }

    if (text.isEmpty() && defInfo && !(defInfo->m_toolTip.isNull()))
    {
        text = defInfo->m_toolTip;
        if (PdmUiItem::showExtraDebugText() && !defInfo->m_extraDebugText.isEmpty())
        {
            text += QString(" (%1)").arg(defInfo->m_extraDebugText);
        }
    }

    if (text.isEmpty() && sttInfo && !(sttInfo->m_toolTip.isNull()))
    {
        text = sttInfo->m_toolTip;
        if (PdmUiItem::showExtraDebugText() && !sttInfo->m_extraDebugText.isEmpty())
        {
            text += QString(" (%1)").arg(sttInfo->m_extraDebugText);
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiToolTip(const QString& uiToolTip, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_toolTip = uiToolTip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiWhatsThis(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_whatsThis.isNull())) return conInfo->m_whatsThis;
    if (defInfo && !(defInfo->m_whatsThis.isNull())) return defInfo->m_whatsThis;
    if (sttInfo && !(sttInfo->m_whatsThis.isNull())) return sttInfo->m_whatsThis;

    return QString("");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiWhatsThis(const QString& uiWhatsThis, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_whatsThis = uiWhatsThis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiHidden(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isHidden == -1)) return conInfo->m_isHidden;
    if (defInfo && !(defInfo->m_isHidden == -1)) return defInfo->m_isHidden;
    if (sttInfo && !(sttInfo->m_isHidden == -1)) return sttInfo->m_isHidden;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiHidden(bool isHidden, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiTreeHidden(const QString& uiConfigName) const
{
    // TODO: Must be separated from uiHidden when childField object embedding is implemented

    return isUiHidden(uiConfigName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiTreeHidden(bool isHidden, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiTreeChildrenHidden(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isTreeChildrenHidden == -1)) return conInfo->m_isTreeChildrenHidden;
    if (defInfo && !(defInfo->m_isTreeChildrenHidden == -1)) return defInfo->m_isTreeChildrenHidden;
    if (sttInfo && !(sttInfo->m_isTreeChildrenHidden == -1)) return sttInfo->m_isTreeChildrenHidden;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiTreeChildrenHidden(bool isTreeChildrenHidden, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_isTreeChildrenHidden = isTreeChildrenHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiReadOnly(const QString& uiConfigName /*= ""*/) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isReadOnly == -1)) return conInfo->m_isReadOnly;
    if (defInfo && !(defInfo->m_isReadOnly == -1)) return defInfo->m_isReadOnly;
    if (sttInfo && !(sttInfo->m_isReadOnly == -1)) return sttInfo->m_isReadOnly;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiReadOnly(bool isReadOnly, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_isReadOnly = isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiItem::uiEditorTypeName(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_editorTypeName.isEmpty())) return conInfo->m_editorTypeName;
    if (defInfo && !(defInfo->m_editorTypeName.isEmpty())) return defInfo->m_editorTypeName;
    if (sttInfo && !(sttInfo->m_editorTypeName.isEmpty())) return sttInfo->m_editorTypeName;

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiEditorTypeName(const QString& editorTypeName, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_editorTypeName = editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiGroup() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItemInfo::LabelPosType PdmUiItem::uiLabelPosition(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo) return conInfo->m_labelAlignment;
    if (defInfo) return defInfo->m_labelAlignment;
    if (sttInfo) return sttInfo->m_labelAlignment;

    return PdmUiItemInfo::LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiLabelPosition(PdmUiItemInfo::LabelPosType alignment, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_labelAlignment = alignment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isCustomContextMenuEnabled(const QString& uiConfigName /*= ""*/) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && (conInfo->m_isCustomContextMenuEnabled != -1)) return conInfo->m_isCustomContextMenuEnabled;
    if (defInfo && (defInfo->m_isCustomContextMenuEnabled != -1)) return defInfo->m_isCustomContextMenuEnabled;
    if (sttInfo && (sttInfo->m_isCustomContextMenuEnabled != -1)) return sttInfo->m_isCustomContextMenuEnabled;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setCustomContextMenuEnabled(bool enableCustomContextMenu, const QString& uiConfigName /*= ""*/)
{
    m_configItemInfos[uiConfigName].m_isCustomContextMenuEnabled = enableCustomContextMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::configInfo(const QString& uiConfigName) const
{
    if (uiConfigName == "" || uiConfigName.isNull()) return nullptr;

    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find(uiConfigName);

    if (it != m_configItemInfos.end()) return &(it->second);

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::defaultInfo() const
{
    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find("");

    if (it != m_configItemInfos.end()) return &(it->second);

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateConnectedEditors() const
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for (it = m_editors.begin(); it != m_editors.end(); ++it)
    {
        (*it)->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateAllRequiredEditors() const
{
    updateConnectedEditors();

    PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem::~PdmUiItem()
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for (it = m_editors.begin(); it != m_editors.end(); ++it)
    {
        (*it)->m_pdmItem = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem::PdmUiItem()
    : m_staticItemInfo(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateUiIconFromState(bool isActive, const QString& uiConfigName)
{
    static const QString iconStorageConfigNamePostfix = "_Internally_StoredNormalIcon";
    const PdmUiItemInfo* conInfo                      = configInfo(uiConfigName + iconStorageConfigNamePostfix);
    QIcon                normalIcon;

    if (conInfo)
    {
        normalIcon = conInfo->m_icon;
    }
    else
    {
        normalIcon = this->uiIcon(uiConfigName);
    }

    this->setUiIcon(normalIcon, uiConfigName + iconStorageConfigNamePostfix);

    if (isActive)
    {
        this->setUiIcon(normalIcon, uiConfigName);
        m_configItemInfos.erase(uiConfigName + iconStorageConfigNamePostfix);
    }
    else
    {
        QIcon disabledIcon(normalIcon.pixmap(16, 16, QIcon::Disabled));
        this->setUiIcon(disabledIcon, uiConfigName);
        this->setUiIcon(normalIcon, uiConfigName + iconStorageConfigNamePostfix);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<PdmUiEditorHandle*> PdmUiItem::connectedEditors() const
{
    std::vector<PdmUiEditorHandle*> editors;
    for (auto e : m_editors)
    {
        editors.push_back(e);
    }

    return editors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::showExtraDebugText()
{
    return sm_showExtraDebugText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::enableExtraDebugText(bool enable)
{
    sm_showExtraDebugText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiItemInfo(PdmUiItemInfo* itemInfo)
{
    m_staticItemInfo = itemInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::removeFieldEditor(PdmUiEditorHandle* fieldView)
{
    m_editors.erase(fieldView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::addFieldEditor(PdmUiEditorHandle* fieldView)
{
    m_editors.insert(fieldView);
}

} // End of namespace caf
