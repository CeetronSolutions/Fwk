#include "TapCvfSpecialization.h"
/*

#include "MainWindow.h"
#include "WidgetLayoutTest.h"

#include <QDockWidget>
#include <QTreeView>
#include <QAction>
#include <QMenuBar>
#include <QUndoView>


#include "cafAppEnum.h"
#include "cafAppExecCommandManager.h"
#include "cafCommandFeaturesCore.h"
#include "cafCommandFeatureManager.h"
#include "cafPdmDocument.h"
#include "cafPdmObject.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTableView.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeView.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"
#include "cafUiTreeModelPdm.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
*/

CAF_PDM_SOURCE_INIT(TapCvfSpecialization, "TapCvfSpecialization");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TapCvfSpecialization::TapCvfSpecialization()
{
    CAF_PDM_InitObject("Test Object for type specializations", "", "", "");

/*
    CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
    CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
    CAF_PDM_InitField(&m_intField, "IntNumber", 0, "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
    CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

    m_proxyDoubleField.registerSetMethod(this, &SmallDemoPdmObject::setDoubleMember);
    m_proxyDoubleField.registerGetMethod(this, &SmallDemoPdmObject::doubleMember);
    CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

    m_proxyDoubleField = 0;
    if (!(m_proxyDoubleField == 3)) { std::cout << "Double is not 3 " << std::endl; }
*/
}
