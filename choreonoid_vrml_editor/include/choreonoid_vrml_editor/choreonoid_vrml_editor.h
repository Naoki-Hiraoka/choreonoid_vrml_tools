#ifndef CHOREONOID_VRML_EDITOR_H
#define CHOREONOID_VRML_EDITOR_H

#include <cnoid/VRML>

namespace choreonoid_vrml_editor {

  cnoid::VRMLNodePtr findNodeDef(cnoid::VRMLNodePtr node, const std::string& defName);

  cnoid::VRMLProtoPtr createSurfaceProto();
  cnoid::VRMLProtoPtr createJointProto();
  cnoid::VRMLProtoPtr createSegmentProto();
  cnoid::VRMLProtoPtr createForceSensorProto();
  cnoid::VRMLProtoPtr createRangeSensorProto();
  cnoid::VRMLProtoPtr createVisionSensorProto();
};

#endif
