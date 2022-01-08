#include <iostream>
#include <fstream>

#include <cnoid/VRMLParser>
#include <choreonoid_vrml_editor/choreonoid_vrml_editor.h>

int main(void){
  std::string inFileName = "/opt/ros/melodic/share/openhrp3/share/OpenHRP-3.1/sample/model/sample1.wrl";
  cnoid::VRMLParser parser(inFileName);
  cnoid::VRMLNodePtr node;
  while(node = parser.readNode()){
    if(node->isCategoryOf(cnoid::PROTO_INSTANCE_NODE)) break;
  }

  cnoid::VRMLNodePtr n = choreonoid_vrml_editor::findNodeDef(node, "LLEG_HIP_R");
  if(n) std::cout << "node found" << std::endl;
  else std::cout << "node NOT found" << std::endl;

  return 0;
}
