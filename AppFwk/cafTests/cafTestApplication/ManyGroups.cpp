
#include "ManyGroups.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT(ManyGroups, "LargeObject");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ManyGroups::ManyGroups()
{
    CAF_PDM_InitObject("Many Groups", ":/images/win/filenew.png", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

    CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
    CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
    CAF_PDM_InitField(&m_intField, "IntNumber", 0, "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
    CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

    m_proxyDoubleField.registerSetMethod(this, &ManyGroups::setDoubleMember);
    m_proxyDoubleField.registerGetMethod(this, &ManyGroups::doubleMember);
    CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

    m_proxyDoubleField = 0;
    if (!(m_proxyDoubleField == 3)) { std::cout << "Double is not 3 " << std::endl; }

    CAF_PDM_InitFieldNoDefault(&m_multiSelectList, "SelectedItems", "Multi Select Field", "", "", "");
    m_multiSelectList.xmlCapability()->setIOReadable(false);
    m_multiSelectList.xmlCapability()->setIOWritable(false);
    m_multiSelectList.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    m_multiSelectList.v().push_back("First");
    m_multiSelectList.v().push_back("Second");
    m_multiSelectList.v().push_back("Third");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* ManyGroups::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ManyGroups::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_toggleField)
    {
        std::cout << "Toggle Field changed" << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> ManyGroups::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_multiSelectList)
    {
        QString text;

        text = "First";
        options.push_back(caf::PdmOptionItemInfo(text, text));

        text = "Second";
        options.push_back(caf::PdmOptionItemInfo::createHeader(text, false, QIcon(QString(":/images/win/textbold.png"))));

        {
            text = "Second_a";
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text, true);
            itemInfo.setLevel(1);
            options.push_back(itemInfo);
        }

        {
            text = "Second_b";
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text, false, QIcon(QString(":/images/win/filenew.png")));
            itemInfo.setLevel(1);
            options.push_back(itemInfo);
        }

        int additionalSubItems = 2;
        for (auto i = 0; i < additionalSubItems; i++)
        {
            text = "Second_b_" + QString::number(i);
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
            itemInfo.setLevel(1);
            options.push_back(itemInfo);
        }

        static int s_additionalSubItems = 0;
        if (m_toggleField())
        {
            s_additionalSubItems++;
        }
        for (auto i = 0; i < s_additionalSubItems; i++)
        {
            text = "Second_b_" + QString::number(i);
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
            itemInfo.setLevel(1);
            options.push_back(itemInfo);
        }

        text = "Third";
        options.push_back(caf::PdmOptionItemInfo(text, text));

        text = "Fourth";
        options.push_back(caf::PdmOptionItemInfo(text, text));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ManyGroups::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("First");

        caf::PdmUiGroup* subGroup = group->addNewGroup("First_Content");

        subGroup->add(&m_doubleField);
        subGroup->add(&m_intField);

        caf::PdmUiGroup* subGroup2 = group->addNewGroup("First_Content_2");

    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Second");
        caf::PdmUiGroup* subGroup = group->addNewGroup("Second_Content");
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Third");
        caf::PdmUiGroup* subGroup = group->addNewGroup("Third_Content");
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Fourth");
        caf::PdmUiGroup* subGroup = group->addNewGroup("Fourth_Content");

        subGroup->add(&m_textField);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Fifth");
        caf::PdmUiGroup* subGroup = group->addNewGroup("Fifth_Content");

        subGroup->add(&m_proxyDoubleField);
    }
}
