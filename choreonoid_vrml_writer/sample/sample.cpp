#include <iostream>
#include <fstream>

#include <cnoid/VRMLParser>
#include <choreonoid_vrml_writer/choreonoid_vrml_writer.h>

int main(void){
  std::string outFileName = "/tmp/hoge.wrl";
  std::ofstream ofs(outFileName);

  std::string inFileName = "/opt/ros/melodic/share/openhrp3/share/OpenHRP-3.1/sample/model/sample1.wrl";
  cnoid::VRMLParser parser(inFileName);
  choreonoid_vrml_writer::VRMLWriter2 writer(ofs);
  writer.setInFileName(inFileName);
  writer.setOutFileName(outFileName);
  writer.writeHeader();
  cnoid::VRMLNodePtr node = nullptr;
  while(node = parser.readNode()){
    writer.writeNode(node);
  }

  return 0;
}
