
#pragma once

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QStringList>

//==================================================================================================
///
///
//==================================================================================================
class TamComboBox : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    TamComboBox();

    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    void
        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<QString> m_name;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                                       QString                    uiConfigName,
                                       caf::PdmUiEditorAttribute* attribute) override;

private:
    QStringList m_historyItems;
};
