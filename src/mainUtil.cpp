#include<mainUtil.h>
#include<BasicCamera/FreeLookCamera.h>
#include<ArgumentViewer/ArgumentViewer.h>
#include<util.h>
#include<TxtUtils/TxtUtils.h>
#include<CameraPath.h>
#include<geGL/StaticCalls.h>
#include<ShadowMethod.h>
#include<Shading.h>
#include<sstream>
#include<Methods.h>
#include<Vars/Caller.h>

std::string getMethodNameList(vars::Vars&vars){
  vars::Caller caller(vars,__FUNCTION__);
  std::stringstream ss;
  auto const methods = vars.get<Methods>("methods");
  bool first = true;
  for(size_t i=0;i<methods->getNofMethods();++i){
    if(first)first = false;
    else ss << "/";
    ss << methods->getName(i);
  }
  return ss.str();
}

void loadBasicApplicationParameters(vars::Vars&vars,std::shared_ptr<argumentViewer::ArgumentViewer>const&args){
  vars::Caller caller(vars,__FUNCTION__);
  *vars.add<glm::uvec2 >("windowSize"     ) = vector2uvec2(args->getu32v("--window-size", {512, 512}, "window size"));
  *vars.add<glm::vec4  >("lightPosition"  ) = vector2vec4(args->getf32v("--light", {0.f, 1000.f, 0.f, 1.f}, "light position"));
  vars.addString        ("modelName"      ) = args->gets("--model", "/media/windata/ft/prace/models/2tri/2tri.3ds","model file name");
  vars.addBool          ("useShadows"     ) = !args->isPresent("--no-shadows", "turns off shadows");
  vars.addBool          ("verbose"        ) = args->isPresent("--verbose", "toggle verbose mode");

  vars.addString        ("methodName"     ) = args->gets("--method", "","name of shadow method: "+getMethodNameList(vars));
  vars.addSizeT         ("wavefrontSize"  ) = args->getu32("--wavefrontSize", 0,"warp/wavefront size, usually 32 for NVidia and 64 for AMD");
  vars.addSizeT         ("maxMultiplicity") = args->getu32("--maxMultiplicity", 2,"max number of triangles that share the same edge");
  vars.addBool          ("zfail"          ) = args->getu32("--zfail", 1, "shadow volumes zfail 0/1");
  vars.addBool          ("getModelStats"  ) = args->isPresent("--getModelStats","gets models stats - nof triangles, edges, silhouettes, ...");
  auto stats = args->getContext("modelStats","model stats parameters");
  *vars.add<glm::uvec3> ("modelStatsGrid" ) = vector2uvec3(stats->getu32v("grid",{10,10,10},"grid size"));
  vars.addFloat         ("modelStatsScale") = stats->getf32("scale",10.f,"scale factor");
}

void moveCameraWSAD(
    vars::Vars&vars,
    std::map<SDL_Keycode, bool>                          keyDown) {
  vars::Caller caller(vars,__FUNCTION__);
  auto const type = vars.getString("args.camera.type");
  auto const freeCameraSpeed = vars.getFloat("args.camera.freeCameraSpeed");
  if (type != "free") return;
  auto const freeLook = vars.getReinterpret<basicCamera::FreeLookCamera>("cameraTransform");
  for (int a = 0; a < 3; ++a)
    freeLook->move(a, float(keyDown["d s"[a]] - keyDown["acw"[a]]) *
                          freeCameraSpeed);
}

void writeCSVHeaderIfFirstLine(
    std::vector<std::vector<std::string>>&csv,
    std::map<std::string,float>const&measurement){
  if (csv.size() != 0) return;
  std::vector<std::string>line;
  line.push_back("frame");
  for (auto const& x : measurement)
    if (x.first != "") line.push_back(x.first);
  csv.push_back(line);
}

void writeMeasurementIntoCSV(
    vars::Vars&vars,
    std::vector<std::vector<std::string>>&csv,
    std::map<std::string,float>const&measurement,
    size_t idOfMeasurement){
  vars::Caller caller(vars,__FUNCTION__);
  std::vector<std::string> line;
  line.push_back(txtUtils::valueToString(idOfMeasurement));
  for (auto const& x : measurement)
    if (x.first != "")
      line.push_back(txtUtils::valueToString(
          x.second / float(vars.getSizeT("test.framesPerMeasurement"))));
  csv.push_back(line);
}

void setCameraAccordingToKeyFrame(std::shared_ptr<CameraPath>const&cameraPath,vars::Vars&vars,size_t keyFrame){
  vars::Caller caller(vars,__FUNCTION__);
  auto keypoint =
      cameraPath->getKeypoint(float(keyFrame) / float(vars.getSizeT("test.flyLength")));
  auto flc = vars.getReinterpret<basicCamera::FreeLookCamera>("cameraTransform");
  flc->setPosition(keypoint.position);
  flc->setRotation(keypoint.viewVector, keypoint.upVector);
}

void ifMethodExistCreateShadowMask(vars::Vars&vars){
  vars::Caller caller(vars,__FUNCTION__);
  if (!vars.has("shadowMethod"))return;
  auto const cameraProjection = vars.getReinterpret<basicCamera::CameraProjection>("cameraProjection");
  auto const cameraTransform  = vars.getReinterpret<basicCamera::CameraTransform >("cameraTransform" );
  auto       method           = vars.getReinterpret<ShadowMethod>("shadowMethod");
  auto const lightPosition    = *vars.get<glm::vec4>("lightPosition");
  vars.get<ge::gl::Texture>("shadowMask")->clear(0,GL_RED,GL_FLOAT);
  method->create(lightPosition,cameraTransform->getView(),cameraProjection->getProjection());
}

void doShading(vars::Vars&vars){
  vars::Caller caller(vars,__FUNCTION__);
  ge::gl::glDisable(GL_DEPTH_TEST);
  auto const cameraTransform            = vars.getReinterpret<basicCamera::CameraTransform >("cameraTransform" );
  auto       shading                    = vars.get<Shading>("shading");
  auto       lightPosition              = *vars.get<glm::vec4>("lightPosition");
  auto const cameraPositionInViewSpace  = glm::vec4(0, 0, 0, 1);
  auto const viewMatrix                 = cameraTransform->getView();
  auto const viewSpaceToWorldSpace      = glm::inverse(viewMatrix);
  auto       cameraPositionInWorldSpace = glm::vec3( viewSpaceToWorldSpace * cameraPositionInViewSpace);
  shading->draw(lightPosition,cameraPositionInWorldSpace,*vars.get<bool>("useShadows"));
}


