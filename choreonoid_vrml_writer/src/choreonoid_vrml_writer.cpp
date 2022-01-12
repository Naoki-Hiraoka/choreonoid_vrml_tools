#include <choreonoid_vrml_writer/choreonoid_vrml_writer.h>

#include <boost/filesystem.hpp>
#include <iostream>
#include <cnoid/src/Util/FileUtil.h>

using namespace std;
using namespace cnoid;
namespace filesystem = boost::filesystem;


namespace choreonoid_vrml_writer {

  struct TIndent {
    TIndent() { size = 2; }
    void setSize(int s) { size = s; }
    void clear() { n = 0; spaces.clear(); }
    TIndent& operator++() {
        n += size;
        updateSpaces();
        return *this;
    }
    TIndent& operator--() {
        n -= size;
        if(n < 0) { n = 0; }
        updateSpaces();
        return *this;
    }
    void updateSpaces(){
        int numTabs = n / 8;
        int numSpaces = n % 8;
        spaces.clear();
        // ここがdefaultと異なる
        //spaces.insert(0, numTabs, '\t');
        //spaces.insert(numTabs, numSpaces, ' ');
        spaces.insert(0, n, ' ');
    }
    std::string spaces;
    int n;
    int size;
  };

  std::ostream& operator<<(std::ostream& out, TIndent& indent)
  {
    return out << indent.spaces;
  }

  const char* boolstr(bool v)
  {
    if(v){
      return "TRUE";
    } else {
      return "FALSE";
    }
  }

  // cnoid::VRMLWriterImpl型を単純に継承してregisterNodeMethodだけ追加で呼びたかったが、cnoid::VRMLWriterNodeMethodがcnoid::VRMLWriterImplクラスのメンバ関数だけを想定しているのでできず、全部作り直すことになった
  typedef void (VRMLWriterImpl2::*VRMLWriterNodeMethod2)(VRMLNodePtr node);

  typedef std::map<std::string, VRMLWriterNodeMethod2> TNodeMethodMap2;
  typedef std::pair<std::string, VRMLWriterNodeMethod2> TNodeMethodPair2;

  TNodeMethodMap2 nodeMethodMap;

}

namespace choreonoid_vrml_writer {

  // defaultと同じ
  std::ostream& operator<<(std::ostream& out, const SFVec2f& v)
  {
    return out << v[0] << " " << v[1];
  }

  std::ostream& operator<<(std::ostream& out, const SFVec3f& v)
  {
    return out << v[0] << " " << v[1] << " " << v[2];
  }

  std::ostream& operator<<(std::ostream& out, const SFColor& v)
  {
    return out << v[0] << " " << v[1] << " " << v[2];
  }

  std::ostream& operator<<(std::ostream& out, const cnoid::SFRotation& v)
  {
    const SFRotation::Vector3& a = v.axis();
    return out << a[0] << " " << a[1] << " " << a[2] << " " << v.angle();
  }


  class VRMLWriterImpl2
  {
  public:
    std::ostream& out;
    std::string ifname; // ここがdefaultと異なる
    std::string ofname;
    TIndent indent;
    typedef std::map<std::string, VRMLNodePtr> NodeMap;
    NodeMap defNodeMap;
    int numOneLineScalarElements;
    int numOneLineVectorElements;
    int numOneLineFaceElements;

    VRMLWriterImpl2(std::ostream& out);
    void registerNodeMethodMap();
    void registerNodeMethod(const std::type_info& t, VRMLWriterNodeMethod2 method);
    VRMLWriterNodeMethod2 getNodeMethod(VRMLNodePtr node);

    template <class MFValues> void writeMFValues(MFValues values, int numColumn);
    void writeMFString(MFString values, int numColumn);
    void writeMFInt32(MFInt32& values, int maxColumns = 10);
    void writeMFInt32SeparatedByMinusValue(MFInt32& values, int maxColumns);
    void writeHeader();
    bool writeNode(VRMLNode* node);
    void writeNodeIter(VRMLNode* node);
    bool beginNode(const char* nodename, VRMLNodePtr node, bool isIndependentNode);
    void endNode();
    void writeGroupNode(VRMLNodePtr node);
    void writeGroupFields(VRMLGroupPtr group);
    void writeTransformNode(VRMLNodePtr node);
    void writeSwitchNode(VRMLNodePtr node);

    std::string reltoabs(std::string& fname); // ここがdefaultと異なる
    std::string abstorel(std::string& fname);
    void writeInlineNode(VRMLNodePtr node);
    void writeShapeNode(VRMLNodePtr node);
    void writeAppearanceNode(VRMLAppearancePtr appearance);
    void writeMaterialNode(VRMLMaterialPtr material);
    void writeBoxNode(VRMLNodePtr node);
    void writeConeNode(VRMLNodePtr node);
    void writeCylinderNode(VRMLNodePtr node);
    void writeSphereNode(VRMLNodePtr node);
    void writeIndexedFaceSetNode(VRMLNodePtr node);
    void writeCoordinateNode(VRMLCoordinatePtr coord);
    void writeNormalNode(VRMLNormalPtr normal);
    void writeColorNode(VRMLColorPtr color);

    // ここがdefaultと違う
    void writeProtoNode(VRMLNodePtr node);
    void writeProtoInstanceNode(VRMLNodePtr node);
  };

  VRMLWriter2::VRMLWriter2(std::ostream& out)
  {
    impl = new VRMLWriterImpl2(out);
  }


  VRMLWriterImpl2::VRMLWriterImpl2(std::ostream& out)
    : out(out), ofname()
  {
    if(nodeMethodMap.empty()){
        registerNodeMethodMap();
    }
    numOneLineScalarElements = 10;
    numOneLineVectorElements = 1;
    numOneLineFaceElements = 1;
  }


  VRMLWriter2::~VRMLWriter2()
  {
    delete impl;
  }


  template <class MFValues> void VRMLWriterImpl2::writeMFValues(MFValues values, int numColumn)
  {
    int col = 0;
    int n = values.size();
    int row = 0;
    if(n > 0){
        row = (n - 1) / numColumn + 1;
    }
    ++indent;
    if(row <= 1){
        out << "[ ";
    } else {
        out << "[\n";
        out << indent;
    }
    for(int i=0; i < n; i++){
        if(col >= numColumn){
            col = 0;
            out << "\n";
            out << indent;
            ++row;
        }
        out << values[i] << " ";
        col++;
    }
    --indent;
    if(row <= 1){
        out << "]\n";
    } else {
        out << "\n" << indent << "]\n";
    }
  }

  void VRMLWriterImpl2::writeMFString(MFString values, int numColumn)
  {
    int col = 0;
    int n = values.size();
    int row = 0;
    if(n > 0){
        row = (n - 1) / numColumn + 1;
    }
    ++indent;
    if(row <= 1){
        out << "[ ";
    } else {
        out << "[\n";
        out << indent;
    }
    for(int i=0; i < n; i++){
        if(col >= numColumn){
            col = 0;
            out << "\n";
            out << indent;
            ++row;
        }
        out << "\"" << values[i] << "\" ";
        col++;
    }
    --indent;
    if(row <= 1){
        out << "]\n";
    } else {
        out << "\n" << indent << "]\n";
    }
  }


  void VRMLWriterImpl2::writeMFInt32(MFInt32& values, int maxColumns)
  {
    out << ++indent << "[\n";
    ++indent;

    out << indent;
    int col = 0;
    int n = values.size();
    for(int i=0; i < n; i++){
        out << values[i] << " ";
        ++col;
        if(col == maxColumns){
            col = 0;
            out << "\n";
            if(i < n-1){
                out << indent;
            }
        }
    }
    if(col < maxColumns){
        out << "\n";
    }
  
    out << --indent << "]\n";
    --indent;
  }


  void VRMLWriterImpl2::writeMFInt32SeparatedByMinusValue(MFInt32& values, int maxColumns)
  {
    out << "[\n";
    
    ++indent;

    out << indent;
    int col = 0;
    int n = values.size();
    for(int i=0; i < n; i++){
        out << values[i] << " ";
        if(values[i] < 0){
            ++col;
            if(col == maxColumns){
                col = 0;
                out << "\n";
                if(i < n-1){
                    out << indent;
                }
            }
        }
    }
    if(col < maxColumns){
        out << "\n";
    }
  
    out << --indent << "]\n";
  }


  void VRMLWriterImpl2::registerNodeMethod(const std::type_info& t, VRMLWriterNodeMethod2 method) {
    nodeMethodMap.insert(TNodeMethodPair2(t.name(), method));
  }


  VRMLWriterNodeMethod2 VRMLWriterImpl2::getNodeMethod(VRMLNodePtr node) {
    TNodeMethodMap2::iterator p = nodeMethodMap.find(typeid(*node).name());
    return (p != nodeMethodMap.end()) ? p->second : 0;
  }


  void VRMLWriterImpl2::registerNodeMethodMap()
  {
    registerNodeMethod(typeid(VRMLGroup),          &VRMLWriterImpl2::writeGroupNode);
    registerNodeMethod(typeid(VRMLTransform),      &VRMLWriterImpl2::writeTransformNode);
    registerNodeMethod(typeid(VRMLSwitch),         &VRMLWriterImpl2::writeSwitchNode);
    registerNodeMethod(typeid(VRMLInline),         &VRMLWriterImpl2::writeInlineNode);
    registerNodeMethod(typeid(VRMLShape),          &VRMLWriterImpl2::writeShapeNode);
    registerNodeMethod(typeid(VRMLIndexedFaceSet), &VRMLWriterImpl2::writeIndexedFaceSetNode);
    registerNodeMethod(typeid(VRMLBox),            &VRMLWriterImpl2::writeBoxNode);
    registerNodeMethod(typeid(VRMLCone),           &VRMLWriterImpl2::writeConeNode);
    registerNodeMethod(typeid(VRMLCylinder),       &VRMLWriterImpl2::writeCylinderNode);
    registerNodeMethod(typeid(VRMLSphere),         &VRMLWriterImpl2::writeSphereNode);

    // ここがdefaultと違う
    registerNodeMethod(typeid(VRMLProto),          &VRMLWriterImpl2::writeProtoNode);
    registerNodeMethod(typeid(VRMLProtoInstance),  &VRMLWriterImpl2::writeProtoInstanceNode);
  }


  // ここがdefualtと異なる
  void VRMLWriter2::setInFileName(const std::string& ifname)
  {
    impl->ifname = ifname;
  }

  void VRMLWriter2::setOutFileName(const std::string& ofname)
  {
    impl->ofname = ofname;
  }

  void VRMLWriter2::setIndentSize(int s)
  {
    impl->indent.setSize(s);
  }


  void VRMLWriter2::setNumOneLineScalarElements(int n)
  {
    impl->numOneLineScalarElements = n;
  }


  void VRMLWriter2::setNumOneLineVectorElements(int n)
  {
    impl->numOneLineVectorElements = n;
  }


  void VRMLWriter2::setNumOneLineFaceElements(int n)
  {
    impl->numOneLineFaceElements = n;
  }


  void VRMLWriter2::writeHeader()
  {
    impl->writeHeader();
  }


  void VRMLWriterImpl2::writeHeader()
  {
    out << "#VRML V2.0 utf8\n";
  }


  bool VRMLWriter2::writeNode(VRMLNode* node)
  {
    return impl->writeNode(node);
  }


  bool VRMLWriterImpl2::writeNode(VRMLNode* node)
  {
    indent.clear();
    defNodeMap.clear();
    out << "\n";
    writeNodeIter(node);
    defNodeMap.clear();
    return true;
  }


  void VRMLWriterImpl2::writeNodeIter(VRMLNode* node)
  {
    VRMLWriterNodeMethod2 method = getNodeMethod(node);
    if(method){
        (this->*method)(node);
    } else {
        cout << "cannot find writer for " << typeid(*node).name() << " node." << endl;
    }
  }


  bool VRMLWriterImpl2::beginNode(const char* nodename, VRMLNodePtr node, bool isIndependentNode)
  {
    if(isIndependentNode){
        out << indent;
    }
    
    const string& defName = node->defName;
    if(defName.empty()){
        out << nodename << " {\n";
        
    } else {
        auto p = defNodeMap.find(defName);
        if(p != defNodeMap.end() && node == p->second){
            out << "USE " << defName << "\n";
            return false;
        } else {
            out << "DEF " << node->defName << " " << nodename << " {\n";
            defNodeMap[defName] = node;
        }
    }

    ++indent;
    return true;
  }


  void VRMLWriterImpl2::endNode()
  {
    out << --indent << "}\n";
  }


  void VRMLWriterImpl2::writeGroupNode(VRMLNodePtr node)
  {
    VRMLGroupPtr group = static_pointer_cast<VRMLGroup>(node);

    if(beginNode("Group", group, true)){
        writeGroupFields(group);
        endNode();
    }
  }


  void VRMLWriterImpl2::writeGroupFields(VRMLGroupPtr group)
  {
    if(group->bboxSize[0] >= 0){
        out << indent << "bboxCenter " << group->bboxCenter << "\n";
        out << indent << "bboxSize " << group->bboxSize << "\n";
    }

    if(!group->children.empty()){
        out << indent << "children [\n";
        ++indent;
        for(size_t i=0; i < group->children.size(); i++){
            writeNodeIter(group->children[i]);
        }
        out << --indent << "]\n";
    }
  }


  void VRMLWriterImpl2::writeTransformNode(VRMLNodePtr node)
  {
    VRMLTransformPtr trans = static_pointer_cast<VRMLTransform>(node);

    if(beginNode("Transform", trans, true)){

        if (trans->center != SFVec3f::Zero()){
            out << indent << "center " << trans->center << "\n";
        }
        if (trans->rotation.angle() != 0){
            out << indent << "rotation " << trans->rotation << "\n";
        }
        if (trans->scale != SFVec3f(1,1,1)){
            out << indent << "scale " << trans->scale << "\n";
        }
        if (trans->scaleOrientation.angle() != 0){
            out << indent << "scaleOrientation " << trans->scaleOrientation << "\n";
        }
        if (trans->translation != SFVec3f::Zero()){
            out << indent << "translation " << trans->translation << "\n";
        }
        
        writeGroupFields(trans);

        endNode();
    }
  }


  void VRMLWriterImpl2::writeSwitchNode(VRMLNodePtr node)
  {
    VRMLSwitchPtr switc = static_pointer_cast<VRMLSwitch>(node);

    if(beginNode("Switch", switc, true)){

        out << indent << "whichChoice " << switc->whichChoice << "\n";

        if(!switc->choice.empty()){
            out << indent << "choice [\n";
            ++indent;
            for(size_t i=0; i < switc->choice.size(); ++i){
                writeNodeIter(switc->choice[i]);
            }
            out << --indent << "]\n";
        }
        
        endNode();
    }
  }

  // ここがdefaultと異なる
  std::string VRMLWriterImpl2::reltoabs(std::string& fname)
  {
    filesystem::path from(fname);
    if(checkAbsolute(from)) return fname;

    filesystem::path parentPath(ifname);
    filesystem::path to = parentPath.parent_path() / from;
    to.normalize();
    return getAbsolutePathString(to);
  }

  /**
   * create relative path from absolute path
   * http://stackoverflow.com/questions/10167382/boostfilesystem-get-relative-path
   **/
  std::string VRMLWriterImpl2::abstorel(std::string& fname)
  {
    filesystem::path from(ofname);
    filesystem::path to(fname);
    filesystem::path::const_iterator fromIter = from.begin();
    filesystem::path::const_iterator toIter = to.begin();
    
    while(fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter)) {
        ++toIter;
        ++fromIter;
    }
    
    if (fromIter != from.end()) ++fromIter;
    
    filesystem::path finalPath;
    while(fromIter != from.end()) {
        finalPath /= "..";
        ++fromIter;
    }
    while(toIter != to.end()) {
        finalPath /= *toIter;
        ++toIter;
    }
    return finalPath.string();
  }


  void VRMLWriterImpl2::writeInlineNode(VRMLNodePtr node)
  {
    VRMLInlinePtr vinline = static_pointer_cast<VRMLInline>(node);

    if(beginNode("Inline", vinline, true)){

        int n = vinline->urls.size();
        if (n == 1) {
            std::string abspath = reltoabs(vinline->urls[0]);// ここがdefaultと異なる
            out << indent << "url \"" << abstorel(abspath) << "\"\n";
        } else {
            out << indent << "urls [\n";
            for(int i=0; i < n; i++){
                std::string abspath = reltoabs(vinline->urls[1]);// ここがdefaultと異なる
                out << indent << "   \"" << abstorel(abspath) << "\"\n";
            }
            out << indent << "]\n";
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeShapeNode(VRMLNodePtr node)
  {
    VRMLShapePtr shape = static_pointer_cast<VRMLShape>(node);

    if(beginNode("Shape", shape, true)){

        if(shape->appearance){
            out << indent << "appearance ";
            writeAppearanceNode(shape->appearance);
        }
        if(shape->geometry){
            VRMLWriterNodeMethod2 method = getNodeMethod(shape->geometry);
            if(method){
                out << indent << "geometry ";
                (this->*method)(shape->geometry);
            }
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeAppearanceNode(VRMLAppearancePtr appearance)
  {
    if(beginNode("Appearance", appearance, false)){

        if(appearance->material){
            out << indent << "material ";
            writeMaterialNode(appearance->material);
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeMaterialNode(VRMLMaterialPtr material)
  {
    if(beginNode("Material", material, false)){

        if (material->ambientIntensity != 0.2){
            out << indent << "ambientIntensity " << material->ambientIntensity << "\n";
        }
        out << indent << "diffuseColor " << material->diffuseColor << "\n";
        if (material->emissiveColor != SFColor::Zero()){
            out << indent << "emissiveColor " << material->emissiveColor << "\n";
        }
        if (material->shininess != 0.2){
            out << indent << "shininess " << material->shininess << "\n";
        }
        if (material->specularColor != SFColor::Zero()){
            out << indent << "specularColor " << material->specularColor << "\n";
        }
        if (material->transparency != 0){
            out << indent << "transparency " << material->transparency << "\n";
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeBoxNode(VRMLNodePtr node)
  {
    VRMLBoxPtr box = static_pointer_cast<VRMLBox>(node);

    if(beginNode("Box", box, false)){
        out << indent << "size " << box->size << "\n";
        endNode();
    }
  }


  void VRMLWriterImpl2::writeConeNode(VRMLNodePtr node)
  {
    VRMLConePtr cone = static_pointer_cast<VRMLCone>(node);

    if(beginNode("Cone", cone, false)){
        out << indent << "bottomRadius " << cone->bottomRadius << "\n";
        out << indent << "height " << cone->height << "\n";
        if(!cone->side){
            out << indent << "side " << boolstr(cone->side) << "\n";
        }
        if(!cone->bottom){
            out << indent << "bottom " << boolstr(cone->bottom) << "\n";
        }
        endNode();
    }
  }


  void VRMLWriterImpl2::writeCylinderNode(VRMLNodePtr node)
  {
    VRMLCylinderPtr cylinder = static_pointer_cast<VRMLCylinder>(node);

    if(beginNode("Cylinder", cylinder, false)){

        out << indent << "radius " << cylinder->radius << "\n";
        out << indent << "height " << cylinder->height << "\n";
        if (!cylinder->top){
            out << indent << "top " << boolstr(cylinder->top) << "\n";
        }
        if (!cylinder->bottom){
            out << indent << "bottom " << boolstr(cylinder->bottom) << "\n";
        }
        if (!cylinder->side){
            out << indent << "side " << boolstr(cylinder->side) << "\n";
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeSphereNode(VRMLNodePtr node)
  {
    VRMLSpherePtr sphere = static_pointer_cast<VRMLSphere>(node);

    if(beginNode("Sphere", sphere, false)){
        out << indent << "radius " << sphere->radius << "\n";
        endNode();
    }
  }


  void VRMLWriterImpl2::writeIndexedFaceSetNode(VRMLNodePtr node)
  {
    VRMLIndexedFaceSetPtr faceset = static_pointer_cast<VRMLIndexedFaceSet>(node);

    if(beginNode("IndexedFaceSet", faceset, false)){

        if(faceset->coord){
            out << indent << "coord ";
            writeCoordinateNode(faceset->coord);
        }
        if(!faceset->coordIndex.empty()){
            out << indent << "coordIndex ";
            writeMFInt32SeparatedByMinusValue(faceset->coordIndex, numOneLineFaceElements);
        }
        
        bool hasNormals = false;
        if(faceset->normal){
            out << indent << "normal ";
            writeNormalNode(faceset->normal);
            
            if(!faceset->normalIndex.empty()){
                out << indent << "normalIndex ";
                if(faceset->normalPerVertex){
                    writeMFInt32SeparatedByMinusValue(faceset->normalIndex, numOneLineFaceElements);
                } else {
                    writeMFInt32(faceset->normalIndex, numOneLineScalarElements);
                }
            }
            hasNormals = true;
        }
        
        bool hasColors = false;
        if(faceset->color){
            out << indent << "color ";
            writeColorNode(faceset->color);
            
            if(!faceset->colorIndex.empty()){
                out << indent << "colorIndex ";
                if(faceset->colorPerVertex){
                    writeMFInt32SeparatedByMinusValue(faceset->colorIndex, numOneLineFaceElements);
                } else {
                    writeMFInt32(faceset->colorIndex, numOneLineScalarElements);
                }
            }
            hasColors = true;
        }

        if(!faceset->ccw){
            out << indent << "ccw " << boolstr(faceset->ccw) << "\n";
        }
        if(!faceset->convex){
            out << indent << "convex " << boolstr(faceset->convex) << "\n";
        }
        if(!faceset->solid){
            out << indent << "solid " << boolstr(faceset->solid) << "\n";
        }
        if(faceset->creaseAngle > 0.0){
            out << indent << "creaseAngle " << faceset->creaseAngle << "\n";
        }
        if(hasNormals && !faceset->normalPerVertex){
            out << indent << "normalPerVertex " << boolstr(faceset->normalPerVertex) << "\n";
        }
        if(hasColors && !faceset->colorPerVertex){
            out << indent << "colorPerVertex " << boolstr(faceset->colorPerVertex) << "\n";
        }
        
        endNode();
    }
  }


  void VRMLWriterImpl2::writeCoordinateNode(VRMLCoordinatePtr coord)
  {
    if(beginNode("Coordinate", coord, false)){
        if(!coord->point.empty()){
            out << indent << "point ";
            writeMFValues(coord->point, numOneLineVectorElements);
        }
        endNode();
    }
  }


  void VRMLWriterImpl2::writeNormalNode(VRMLNormalPtr normal)
  {
    if(beginNode("Normal", normal, false)){
        if(!normal->vector.empty()){
            out << indent << "vector ";
            writeMFValues(normal->vector, numOneLineVectorElements);
        }
        endNode();
    }
  }


  void VRMLWriterImpl2::writeColorNode(VRMLColorPtr color)
  {
    if(beginNode("Color", color, false)){
        if(!color->color.empty()){
            out << indent << "color ";
            writeMFValues(color->color, numOneLineVectorElements);
        }
        endNode();
    }
  }



  // ここがdefaultと違う
  void VRMLWriterImpl2::writeProtoNode(VRMLNodePtr node)
  {
    VRMLProtoPtr proto = static_pointer_cast<VRMLProto>(node);

    // protoから直接情報を得て書き込みたいが、VRMLProtoクラスはもとのVRMLを復元するには情報が足りないので、書き下すしかない
    if (proto->protoName == "Humanoid"){
      out <<
"PROTO Humanoid [\n\
  field         SFVec3f     bboxCenter        0 0 0\n\
  field         SFVec3f     bboxSize          -1 -1 -1\n\
  exposedField  SFVec3f     center            0 0 0\n\
  exposedField  MFNode      humanoidBody      [ ]\n\
  exposedField  MFString    info              [ ]\n\
  exposedField  MFNode      joints            [ ]\n\
  exposedField  SFString    name              \"\"\n\
  exposedField  SFRotation  rotation          0 0 1 0\n\
  exposedField  SFVec3f     scale             1 1 1\n\
  exposedField  SFRotation  scaleOrientation  0 0 1 0\n\
  exposedField  MFNode      segments          [ ]\n\
  exposedField  MFNode      sites             [ ]\n\
  exposedField  SFVec3f     translation       0 0 0\n\
  exposedField  SFString    version           \"1.1\"\n\
  exposedField  MFNode      viewpoints        [ ]\n\
]\n\
{\n\
  Transform {\n\
    bboxCenter       IS bboxCenter\n\
    bboxSize         IS bboxSize\n\
    center           IS center\n\
    rotation         IS rotation\n\
    scale            IS scale\n\
    scaleOrientation IS scaleOrientation\n\
    translation      IS translation\n\
    children [\n\
      Group {\n\
        children IS viewpoints\n\
      }\n\
      Group {\n\
        children IS humanoidBody\n\
      }\n\
    ]\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "Joint"){
      // dhは本来ないが,HRP2にはある
      out <<
"PROTO Joint [\n\
  exposedField     SFVec3f      center              0 0 0\n\
  exposedField     MFNode       children            []\n\
  exposedField     MFFloat      llimit              []\n\
  exposedField     MFFloat      lvlimit             []\n\
  exposedField     SFRotation   limitOrientation    0 0 1 0\n\
  exposedField     SFString     name                \"\"\n\
  exposedField     SFRotation   rotation            0 0 1 0\n\
  exposedField     SFVec3f      scale               1 1 1\n\
  exposedField     SFRotation   scaleOrientation    0 0 1 0\n\
  exposedField     MFFloat      stiffness           [ 0 0 0 ]\n\
  exposedField     SFVec3f      translation         0 0 0\n\
  exposedField     MFFloat      ulimit              []\n\
  exposedField     MFFloat      uvlimit             []\n\
  exposedField     MFFloat      climit              []\n\
  exposedField     MFFloat      dh                  [ 0 0 0 0 ]\n\
  exposedField     SFString     jointType           \"\"\n\
  exposedField     SFInt32      jointId             -1\n\
  exposedField     SFVec3f      jointAxis           0 0 1\n\
\n\
  exposedField     SFFloat      gearRatio           1\n\
  exposedField     SFFloat      rotorInertia        0\n\
  exposedField     SFFloat      rotorResistor       0\n\
  exposedField     SFFloat      torqueConst         1\n\
  exposedField     SFFloat      encoderPulse        1\n\
]\n\
{\n\
  Transform {\n\
    center           IS center\n\
    children         IS children\n\
    rotation         IS rotation\n\
    scale            IS scale\n\
    scaleOrientation IS scaleOrientation\n\
    translation      IS translation\n\
  }\n\
}" << std::endl;
      return;
    }

    if (proto->protoName == "Segment"){
      out <<
"PROTO Segment [\n\
  field         SFVec3f   bboxCenter        0 0 0\n\
  field         SFVec3f   bboxSize          -1 -1 -1\n\
  exposedField  SFVec3f   centerOfMass      0 0 0\n\
  exposedField  MFNode    children          [ ]\n\
  exposedField  SFNode    coord             NULL\n\
  exposedField  MFNode    displacers        [ ]\n\
  exposedField  SFFloat   mass              0\n\
  exposedField  MFFloat   momentsOfInertia  [ 0 0 0 0 0 0 0 0 0 ]\n\
  exposedField  SFString  name              \"\"\n\
  eventIn       MFNode    addChildren\n\
  eventIn       MFNode    removeChildren\n\
]\n\
{\n\
  Group {\n\
    addChildren    IS addChildren\n\
    bboxCenter     IS bboxCenter\n\
    bboxSize       IS bboxSize\n\
    children       IS children\n\
    removeChildren IS removeChildren\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "ExtraJoint"){
      out <<
"PROTO ExtraJoint [\n\
  exposedField SFString link1Name       \"\"\n\
  exposedField SFString link2Name       \"\"\n\
  exposedField SFVec3f  link1LocalPos   0 0 0\n\
  exposedField SFVec3f  link2LocalPos   0 0 0\n\
  exposedField SFString jointType       \"xyz\"\n\
  exposedField SFVec3f  jointAxis       1 0 0\n\
]\n\
{\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "AccelerationSensor"){
      out <<
"PROTO AccelerationSensor [\n\
  exposedField SFVec3f    maxAcceleration -1 -1 -1\n\
  exposedField SFVec3f    translation     0 0 0\n\
  exposedField SFRotation rotation        0 0 1 0\n\
  exposedField SFInt32    sensorId        -1\n\
]\n\
{\n\
  Transform {\n\
    translation IS translation\n\
    rotation    IS rotation\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "Gyro"){
      out <<
"PROTO Gyro [\n\
  exposedField SFVec3f    maxAngularVelocity -1 -1 -1\n\
  exposedField SFVec3f    translation        0 0 0\n\
  exposedField SFRotation rotation           0 0 1 0\n\
  exposedField SFInt32    sensorId           -1\n\
]\n\
{\n\
  Transform {\n\
    translation IS translation\n\
    rotation    IS rotation\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "VisionSensor"){
      // childrenは本来無いが、samplerobotのwrlにはあるので
      out <<
"PROTO VisionSensor\n\
[\n\
  exposedField  SFVec3f     translation       0 0 0\n\
  exposedField  SFRotation  rotation          0 0 1 0\n\
  exposedField  MFNode      children          [ ]\n\
  exposedField  SFFloat     fieldOfView       0.785398\n\
  field         SFString    name              \"\"\n\
  exposedField  SFFloat     frontClipDistance 0.01\n\
  exposedField  SFFloat     backClipDistance  10.0\n\
  exposedField  SFString    type              \"NONE\"\n\
  exposedField  SFInt32     sensorId          -1\n\
  exposedField  SFInt32     width             320\n\
  exposedField  SFInt32     height            240\n\
  exposedField  SFFloat     frameRate         30\n\
]\n\
{\n\
  Transform\n\
  {\n\
    translation IS translation\n\
    rotation    IS rotation\n\
    children    IS children\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "ForceSensor"){
      out <<
"PROTO ForceSensor [\n\
  exposedField SFVec3f maxForce -1 -1 -1\n\
  exposedField SFVec3f maxTorque -1 -1 -1\n\
  exposedField SFVec3f translation 0 0 0\n\
  exposedField SFRotation rotation 0 0 1 0\n\
  exposedField SFInt32 sensorId -1\n\
]\n\
{\n\
  Transform {\n\
translation IS translation\n\
    rotation IS rotation\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "RangeSensor"){
      out <<
"PROTO RangeSensor [\n\
   exposedField SFVec3f    translation       0 0 0\n\
   exposedField SFRotation rotation          0 0 1 0\n\
   exposedField MFNode     children          [ ]\n\
   exposedField SFInt32    sensorId          -1\n\
   exposedField SFFloat    scanAngle         3.14159 #[rad]\n\
   exposedField SFFloat    scanStep          0.1     #[rad]\n\
   exposedField SFFloat    scanRate          10      #[Hz]\n\
   exposedField SFFloat    minDistance        0.01\n\
   exposedField SFFloat    maxDistance      10\n\
]\n\
{\n\
   Transform {\n\
     rotation         IS rotation\n\
     translation      IS translation\n\
     children         IS children\n\
   }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "SpotLightDevice"){
      out <<
"PROTO SpotLightDevice [\n\
  exposedField SFVec3f attenuation       1 0 0     # [0,)\n\
  exposedField SFFloat beamWidth         1.570796  # (0,/2]\n\
  exposedField SFColor color             1 1 1     # [0,1]\n\
  exposedField SFFloat cutOffAngle       0.785398  # (0,/2]\n\
  exposedField SFVec3f direction         0 0 -1    # (-,)\n\
  exposedField SFFloat intensity         1         # [0,1]\n\
  exposedField SFBool  on                TRUE\n\
  exposedField SFVec3f    translation 0 0 0\n\
  exposedField SFRotation rotation    0 0 1 0\n\
]\n\
{\n\
  Transform {\n\
    translation      IS translation\n\
    rotation         IS rotation\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "Surface"){
      out <<
"PROTO Surface [\n\
  field   SFVec3f bboxCenter 0 0 0\n\
  field   SFVec3f bboxSize   -1 -1 -1\n\
  field   MFNode  visual     [ ]\n\
  field   MFNode  collision  [ ]\n\
  eventIn MFNode  addChildren\n\
  eventIn MFNode  removeChildren\n\
]\n\
{\n\
  Group {\n\
    addChildren    IS addChildren\n\
    bboxCenter     IS bboxCenter\n\
    bboxSize       IS bboxSize\n\
    children       IS visual\n\
    removeChildren IS removeChildren\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "PressureSensor"){
      out <<
"PROTO PressureSensor [\n\
  exposedField SFFloat maxPressure -1\n\
  exposedField SFVec3f translation 0 0 0\n\
  exposedField SFRotation rotation 0 0 1 0\n\
  exposedField SFInt32 sensorId -1\n\
]\n\
{\n\
  Transform {\n\
    translation IS translation\n\
    rotation IS rotation\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "PhotoInterrupter"){
      out <<
"PROTO PhotoInterrupter [\n\
  exposedField SFVec3f transmitter 0 0 0\n\
  exposedField SFVec3f receiver 0 0 0\n\
  exposedField SFInt32 sensorId -1\n\
]\n\
{\n\
  Transform{\n\
    children [\n\
      Transform{\n\
        translation IS transmitter\n\
      }\n\
      Transform{\n\
        translation IS receiver\n\
      }\n\
    ]\n\
  }\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "CylinderSensorZ"){
      out <<
"PROTO CylinderSensorZ [\n\
    exposedField    SFFloat    maxAngle	      -1\n\
    exposedField    SFFloat    minAngle        0\n\
    exposedField    MFNode     children        [ ]\n\
]\n\
{\n\
  Transform{\n\
    rotation 1 0 0 1.5708\n\
    children [\n\
      DEF SensorY CylinderSensor{\n\
	maxAngle IS maxAngle\n\
	minAngle IS minAngle\n\
      }\n\
      DEF AxisY Transform{\n\
        children [\n\
          Transform{\n\
            rotation 1 0 0 -1.5708\n\
            children IS children\n\
          }\n\
        ]\n\
      }\n\
    ]\n\
  }\n\
  ROUTE SensorY.rotation_changed TO AxisY.set_rotation\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "CylinderSensorY"){
      out <<
"PROTO CylinderSensorY [\n\
    exposedField    SFFloat    maxAngle	      -1\n\
    exposedField    SFFloat    minAngle        0\n\
    exposedField    MFNode     children        [ ]\n\
]\n\
{\n\
  Transform{\n\
    rotation 0 1 0 1.5708\n\
    children [\n\
      DEF SensorY CylinderSensor{\n\
	maxAngle IS maxAngle\n\
	minAngle IS minAngle\n\
      }\n\
      DEF AxisX Transform{\n\
        children [\n\
          Transform{\n\
            rotation 0 1 0 -1.5708\n\
            children IS children\n\
          }\n\
        ]\n\
      }\n\
    ]\n\
  }\n\
  ROUTE SensorX.rotation_changed TO AxisX.set_rotation\n\
}"<< std::endl;
      return;
    }

    if (proto->protoName == "CylinderSensorX"){
      out <<
"PROTO CylinderSensorX [\n\
    exposedField    SFFloat    maxAngle	      -1\n\
    exposedField    SFFloat    minAngle        0\n\
    exposedField    MFNode     children        [ ]\n\
]\n\
{\n\
  Transform{\n\
    rotation 0 0 1 -1.5708\n\
    children [\n\
      DEF SensorZ CylinderSensor{\n\
	maxAngle IS maxAngle\n\
	minAngle IS minAngle\n\
      }\n\
      DEF AxisZ Transform{\n\
        children [\n\
          Transform{\n\
            rotation 0 0 1 1.5708\n\
            children IS children\n\
          }\n\
        ]\n\
      }\n\
    ]\n\
  }\n\
  ROUTE SensorZ.rotation_changed TO AxisZ.set_rotation\n\
}"<< std::endl;
      return;
    }

    cout << "cannot find writer for " << proto->protoName << " proto node." << endl;
  }

  // ここがdefaultと違う
  void VRMLWriterImpl2::writeProtoInstanceNode(VRMLNodePtr node)
  {
    VRMLProtoInstancePtr instance = static_pointer_cast<VRMLProtoInstance>(node);
    if(beginNode(instance->proto->protoName.c_str(), node, true)){
      for(cnoid::VRMLProtoFieldMap::iterator it = instance->fields.begin(); it != instance->fields.end(); it++){
        out << indent << it->first << " ";
        switch(it->second.which()){
        case SFBOOL:
          out << boolstr(get<SFBool>(it->second)) << std::endl;
          break;
        case SFINT32:
          out << get<SFInt32>(it->second) << std::endl;
          break;
        case SFFLOAT:
          out << get<SFFloat>(it->second) << std::endl;
          break;
        case SFVEC2F:
          out << get<SFVec2f>(it->second) << std::endl;
          break;
        case SFVEC3F:
          out << get<SFVec3f>(it->second) << std::endl;
          break;
        case SFROTATION:
          out << get<SFRotation>(it->second) << std::endl;
          break;
        case SFCOLOR:
          out << get<SFColor>(it->second) << std::endl;
          break;
        case SFTIME:
          cout << "cannot write SFTIME." << endl;
          break;
        case SFSTRING:
          out << "\"" << get<SFString>(it->second) << "\"" << std::endl;
          break;
        case SFNODE:
          if(get<SFNode>(it->second)) writeNodeIter(get<SFNode>(it->second));
          else out << "NULL" << std::endl;
          break;
        case SFIMAGE:
          cout << "cannot write SFIMAGE." << endl;
          break;
        case MFINT32:
          writeMFInt32(get<MFInt32>(it->second));
          break;
        case MFFLOAT:
          writeMFValues(get<MFFloat>(it->second), 10);
          break;
        case MFVEC2F:
          writeMFValues(get<MFVec2f>(it->second), 10);
          break;
        case MFVEC3F:
          writeMFValues(get<MFVec3f>(it->second), 10);
          break;
        case MFROTATION:
          writeMFValues(get<MFRotation>(it->second), 10);
          break;
        case MFCOLOR:
          writeMFValues(get<MFColor>(it->second), 10);
          break;
        case MFTIME:
          cout << "cannot write MFTIME." << endl;
          break;
        case MFSTRING:
          writeMFString(get<MFString>(it->second), 1);
          break;
        case MFNODE:
          out << "[" << std::endl;
          ++indent;
          for(int i=0;i<get<MFNode>(it->second).size();i++){
            writeNodeIter(get<MFNode>(it->second)[i]);
          }
          out << --indent << "]" << std::endl;
          break;
        default:
          cout << "cannot write " << it->second.which() << endl;
          break;
        }
      }

      endNode();
    }
  }
}
