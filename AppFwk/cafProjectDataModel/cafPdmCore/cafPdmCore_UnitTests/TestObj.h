#pragma once

#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj : public caf::PdmObjectHandle
{
public:
    TestObj();
    ~TestObj() override;

    caf::PdmDataValueField<double> m_position;
};
