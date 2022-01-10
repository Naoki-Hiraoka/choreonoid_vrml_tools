#include <choreonoid_vrml_editor/choreonoid_vrml_editor.h>

namespace choreonoid_vrml_editor {
  cnoid::VRMLNodePtr findNodeDef(cnoid::VRMLNodePtr node, const std::string& defName){
    if(node->defName == defName) return node;

    if(node->isCategoryOf(cnoid::PROTO_INSTANCE_NODE)) {
      cnoid::VRMLProtoInstancePtr instance = cnoid::static_pointer_cast<cnoid::VRMLProtoInstance>(node);
      for(cnoid::VRMLProtoFieldMap::iterator it = instance->fields.begin(); it != instance->fields.end(); it++){
        if(it->second.which() == cnoid::SFNODE){
          cnoid::VRMLNodePtr child = boost::get<cnoid::SFNode>(it->second);
          if(child){
            cnoid::VRMLNodePtr result = findNodeDef(child, defName);
            if(result) return result;
          }
        }
        if(it->second.which() == cnoid::MFNODE){
          cnoid::MFNode& children = boost::get<cnoid::MFNode>(it->second);
          for(int i=0;i<children.size();i++){
            if(children[i]){
              cnoid::VRMLNodePtr result = findNodeDef(children[i], defName);
              if(result) return result;
            }
          }
        }
      }
    }
    return nullptr;
  }

  cnoid::VRMLProtoPtr createSurfaceProto(){
    cnoid::VRMLProtoPtr proto = new cnoid::VRMLProto("Surface");
    proto->fields["bboxCenter"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["bboxSize"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["visual"] = cnoid::MFNode();
    proto->fields["collision"] = cnoid::MFNode();
    proto->fields["addChildren"] = cnoid::MFNode();
    proto->fields["removeChildren"] = cnoid::MFNode();
    return proto;
  }


  cnoid::VRMLProtoPtr createJointProto(){
    cnoid::VRMLProtoPtr proto = new cnoid::VRMLProto("Joint");
    proto->fields["center"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["children"] = cnoid::MFNode();
    proto->fields["llimit"] = cnoid::MFFloat();
    proto->fields["lvlimit"] = cnoid::MFFloat();
    proto->fields["limitOrientation"] = cnoid::SFRotation(0,Eigen::Vector3d::UnitZ());
    proto->fields["name"] = cnoid::SFString();
    proto->fields["rotation"] = cnoid::SFRotation(0,Eigen::Vector3d::UnitZ());
    proto->fields["scale"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["scaleOrientation"] = cnoid::SFRotation(0,Eigen::Vector3d::UnitZ());
    proto->fields["stiffness"] = cnoid::MFFloat(3,0);
    proto->fields["translation"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["ulimit"] = cnoid::MFFloat();
    proto->fields["uvlimit"] = cnoid::MFFloat();
    proto->fields["climit"] = cnoid::MFFloat();
    proto->fields["jointType"] = cnoid::SFString();
    proto->fields["jointId"] = cnoid::SFInt32(-1);
    proto->fields["jointAxis"] = cnoid::SFVec3f::UnitZ().eval();
    proto->fields["gearRatio"] = cnoid::SFFloat(1);
    proto->fields["rotorInertia"] = cnoid::SFFloat(0);
    proto->fields["rotorResistor"] = cnoid::SFFloat(0);
    proto->fields["torqueConst"] = cnoid::SFFloat(1);
    proto->fields["encoderPulse"] = cnoid::SFFloat(1);
    return proto;
  }

  cnoid::VRMLProtoPtr createSegmentProto(){
    cnoid::VRMLProtoPtr proto = new cnoid::VRMLProto("Segment");
    proto->fields["bboxCenter"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["bboxSize"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["centerOfMass"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["children"] = cnoid::MFNode();
    proto->fields["coord"] = cnoid::SFNode(nullptr);
    proto->fields["displacers"] = cnoid::MFNode();
    proto->fields["mass"] = cnoid::SFFloat(0);
    proto->fields["momentsOfInertia"] = cnoid::MFFloat(9,0);
    proto->fields["name"] = cnoid::SFString();
    proto->fields["addChildren"] = cnoid::MFNode();
    proto->fields["removeChildren"] = cnoid::MFNode();
    return proto;
  }
  cnoid::VRMLProtoPtr createForceSensorProto(){
    cnoid::VRMLProtoPtr proto = new cnoid::VRMLProto("ForceSensor");
    proto->fields["maxForce"] = (cnoid::SFVec3f::Ones() * -1).eval();
    proto->fields["maxTorque"] = (cnoid::SFVec3f::Ones() * -1).eval();
    proto->fields["translation"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["rotation"] = cnoid::SFRotation(0,Eigen::Vector3d::UnitZ());
    proto->fields["sensorId"] = cnoid::SFInt32(-1);
    return proto;
  }

  cnoid::VRMLProtoPtr createRangeSensorProto(){
    cnoid::VRMLProtoPtr proto = new cnoid::VRMLProto("RangeSensor");
    proto->fields["translation"] = cnoid::SFVec3f::Zero().eval();
    proto->fields["rotation"] = cnoid::SFRotation(0,Eigen::Vector3d::UnitZ());
    proto->fields["children"] = cnoid::MFNode();
    proto->fields["sensorId"] = cnoid::SFInt32(-1);
    proto->fields["scanAngle"] = cnoid::SFFloat(3.14159);
    proto->fields["scanStep"] = cnoid::SFFloat(0.1);
    proto->fields["scanRate"] = cnoid::SFFloat(10.0);
    proto->fields["minDistance"] = cnoid::SFFloat(0.01);
    proto->fields["maxDistance"] = cnoid::SFFloat(10.0);

    return proto;
  }

}
