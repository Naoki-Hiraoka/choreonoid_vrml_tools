#ifndef CHOREONOID_VRML_WRITER_H
#define CHOREONOID_VRML_WRITER_H

#include <cnoid/Body>
#include <cnoid/VRMLWriter>

namespace choreonoid_vrml_writer {

  class VRMLWriterImpl2;

  class VRMLWriter2
  {
  public:
    VRMLWriter2(std::ostream& out);
    ~VRMLWriter2();
    void setInFileName(const std::string& ifname); // ここがdefualtと異なる
    void setOutFileName(const std::string& ofname);
    void setIndentSize(int s);
    void setNumOneLineScalarElements(int n);
    void setNumOneLineVectorElements(int n);
    void setNumOneLineFaceElements(int n);
    void writeHeader();
    bool writeNode(cnoid::VRMLNode* node);

  protected:
    VRMLWriterImpl2* impl;
  };

};

#endif
