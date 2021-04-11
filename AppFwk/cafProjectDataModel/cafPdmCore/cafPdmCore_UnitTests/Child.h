#pragma once

#include "cafPdmChildField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj;

class Child : public caf::PdmObjectHandle
{
public:
    Child();
    ~Child() override;

    caf::PdmChildField<TestObj*> m_testObj;
};
