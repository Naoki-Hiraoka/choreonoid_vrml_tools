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
}
