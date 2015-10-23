#pragma once

#include "cafPdmObjectCapability.h"

#include <QString>

class QXmlStreamReader;
class QXmlStreamWriter;


namespace caf
{

class PdmXmlFieldHandle;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmReferenceHelper;


//==================================================================================================
//
// 
//
//==================================================================================================
class PdmXmlObjectHandle : public PdmObjectCapability
{
public:

    PdmXmlObjectHandle(PdmObjectHandle* owner, bool giveOwnership);
    virtual ~PdmXmlObjectHandle() { }

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_XML_HEADER_INIT macro
    virtual QString         classKeyword() = 0;

    /// Convenience methods to serialize/de-serialize this particular object (with children)
    void                    readObjectFromXmlString(const QString& xmlString, PdmObjectFactory* objectFactory);
    QString                 writeObjectToXmlString();
    static PdmObjectHandle* readUnknownObjectFromXmlString(const QString& xmlString, PdmObjectFactory* objectFactory);
    PdmObjectHandle*        copyByXmlSerialization(PdmObjectFactory* objectFactory);

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly. 
    void                    readFields(QXmlStreamReader& inputStream, PdmObjectFactory* objectFactory);
    void                    writeFields(QXmlStreamWriter& outputStream);

    /// Check if a string is a valid Xml element name
    static bool             isValidXmlElementName(const QString& name);

    void                    initAfterReadRecursively()         { initAfterReadRecursively(this->m_owner); };
    void                    setupBeforeSaveRecursively()       { setupBeforeSaveRecursively(this->m_owner); };
    void                    resolveReferencesRecursively()     { resolveReferencesRecursively(this->m_owner); };

protected: // Virtual 
    /// Method gets called from PdmDocument after all objects are read. 
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void            initAfterRead() {};
    /// Method gets called from PdmDocument before saving document. 
    /// Re-implement to make sure your fields have correct data before saving
    virtual void            setupBeforeSave() {};

    /// This method is intended to be used in macros to make compile time errors 
    // if user uses them on wrong type of objects
    bool                    isInheritedFromPdmXmlSerializable() { return true; }

private:
    void                    initAfterReadRecursively(PdmObjectHandle* object);
    void                    setupBeforeSaveRecursively(PdmObjectHandle * object);
    void                    resolveReferencesRecursively(PdmObjectHandle* object);

private:
    friend class PdmObjectHandle ; // Only temporary for void PdmObject::addFieldNoDefault( ) accessing findField

    PdmObjectHandle* m_owner;
};

PdmXmlObjectHandle* xmlObj(PdmObjectHandle* obj);

} // End of namespace caf

#include "cafPdmXmlFieldHandle.h"
