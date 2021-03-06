#include <geGL/StaticCalls.h>
#include <geGL/geGL.h>

#include <glm/glm.hpp>

#include <Simple3DApp/Application.h>
#include <CameraParam.h>
#include <mainUtil.h>
#include <util.h>

#include <DrawPrimitive.h>
#include <Model.h>
#include <RSSV/RSSV.h>
//#include <RSSV/Tiles.h>
#include <Shading.h>
#include <ShadowMethod.h>
#include <Sintorn/Sintorn.h>
#include <Sintorn/Param.h>
#include <Vars/Vars.h>
#include <Vars/Caller.h>
#include <modelStats.h>
#include <selectMethod.h>
#include <createShadowMask.h>
#include <createMethod.h>
#include <stopAppAfterMaxFrame.h>
#include <takeAScreenShot.h>
#include <initMethods.h>
#include <parseArguments.h>
#include <measureFly.h>
#include <drawScene.h>
#include <createGeometryBuffer.h>
#include <saveGBufferAsPointCloud.h>
#include <drawPointCloud.h>
#include <TimeStamp.h>
#include <imguiVars/imguiVars.h>
#include <imguiVars/addVarsLimits.h>

#include <FunctionPrologue.h>
#include <Methods.h>
#include <Timer.h>

#define ___ std::cerr << __FILE__ << ": " << __LINE__ << std::endl

class Shadows : public simple3DApp::Application {
 public:
  Shadows(int argc, char* argv[]) : Application(argc, argv) {}
  virtual void draw() override;

  vars::Vars vars;

  virtual void                init() override;
  virtual void                deinit() override;
  void                        initWavefrontSize();
  virtual void                mouseMove(SDL_Event const& event) override;
  std::map<SDL_Keycode, bool> keyDown;
  virtual void                key(SDL_Event const& e, bool down) override;
  virtual void                resize(uint32_t x,uint32_t y) override;
};

void Shadows::initWavefrontSize() {
  FUNCTION_CALLER();
  vars.getSizeT("wavefrontSize") = getWavefrontSize(vars.getSizeT("wavefrontSize"));
}

void Shadows::init() {
  FUNCTION_CALLER();
  SDL_GL_SetSwapInterval(0); //disable vsync

  vars.add<sdl2cpp::MainLoop*>("mainLoop",&*mainLoop);
  vars.add<sdl2cpp::Window  *>("window"  ,&*window  );
  vars.addUint32("argc",argc);
  vars.add<char**>("argv",argv);
  hide(vars,"argc");
  hide(vars,"argv");

  initMethods(vars);
  parseArguments(vars);

  if(vars.getBool("notResizable")){
    SDL_SetWindowResizable(window->getWindow(),SDL_FALSE);
  }

  if(vars.getBool("getModelStats")){
    getModelStats(vars);
    exit(0);
  }

  auto windowSize = *vars.get<glm::uvec2>("windowSize");
  window->setSize(windowSize.x, windowSize.y);
  SDL_SetWindowPosition(window->getWindow(), 0, 30);

  ge::gl::glEnable(GL_DEPTH_TEST);
  ge::gl::glDepthFunc(GL_LEQUAL);
  ge::gl::glDisable(GL_CULL_FACE);
  ge::gl::glClearColor(0, 0, 0, 1);

  initWavefrontSize();

  if (vars.getString("test.name") == "fly" || vars.getString("test.name") == "grid")
    vars.getString("args.camera.type") = "free";


  createView      (vars);
  createProjection(vars);

  createGeometryBuffer(vars);
  createShadowMask(vars);
  vars.add<Model          >("model"      ,vars.getString("modelName"));
  vars.add<RenderModel    >("renderModel",vars.get<Model>("model"));
  vars.add<Shading        >("shading"    ,vars);

  createMethod(vars);

  bool isTest = vars.getString("test.name") == "fly";
  if (vars.getBool("verbose") || (vars.has("shadowMethod") && isTest))
    vars.add<TimeStamp>("timeStamp");

  vars.add<DrawPrimitive>("drawPrimitive",windowSize);

}

void Shadows::deinit(){
  storeCamera(vars);
}


void Shadows::draw() {
  FUNCTION_CALLER();
  auto timer = Timer<float>();
  createGeometryBuffer(vars);
  createShadowMask(vars);
  createProjection(vars);
  createMethod(vars);
  
  stopAppAfterMaxFrame(vars);

  ge::gl::glClearColor(1,1,1,1);
  ge::gl::glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  assert(this != nullptr);

  if (vars.getString("test.name") == "fly") {
    measureFly(vars);
    return;
  }

  moveCameraWSAD(vars, keyDown);

  if(vars.addOrGetBool("drawPointCloud")){
    drawPointCloud(vars);
  }else
    drawScene(vars);

  if (vars.getString("methodName") == "sintorn") {
    //std::cerr << "asd" << std::endl;
    auto sintorn = vars.getReinterpret<Sintorn>("shadowMethod");
    auto dp = vars.get<DrawPrimitive>("drawPrimitive");
    auto drawTex = [&](char s,int i){if (keyDown[s]) dp->drawTexture(vars.getVector<std::shared_ptr<ge::gl::Texture>>("sintorn.HDT")[i]);};
    for(int i=0;i<4;++i)drawTex("hjkl"[i],i);
    if (keyDown['v']) sintorn->drawHST(0);
    if (keyDown['b']) sintorn->drawHST(1);
    if (keyDown['n']) sintorn->drawHST(2);
    if (keyDown['m']) sintorn->drawHST(3);
    if (keyDown[',']) sintorn->drawFinalStencilMask();
  }
  //if (vars.getString("methodName") == "rssv") {
  //  auto rssv = vars.getReinterpret<rssv::RSSV>("shadowMethod");
  //  auto dp = vars.get<DrawPrimitive>("drawPrimitive");
  //  auto drawTex = [&](char s,int i){if (keyDown[s]) dp->drawTexture(rssv->_HDT[i]);};
  //  for(int i=0;i<4;++i)drawTex("hjkl"[i],i);
  //}



  drawImguiVars(vars);



  selectMethod(vars);

  if(ImGui::Button("screenshot"))
    takeAScreenShot(vars);

  if(ImGui::Button("storePointCloud"))
    saveGBufferAsPointCloud(vars);


  swap();

  auto time = timer.elapsedFromStart();
  auto&t = vars.addOrGetFloat("frameTime");
  auto&fps = vars.addOrGetFloat("fps");
  t = time*1000;
  fps = 1.f/time;
}

int main(int argc, char* argv[]) {
  Shadows app{argc, argv};
  app.start();
  return EXIT_SUCCESS;
}

void Shadows::key(SDL_Event const& event, bool DOWN) {
  keyDown[event.key.keysym.sym] = DOWN;

  if (DOWN && event.key.keysym.sym == 'p')
  {
	  printCameraPosition(vars);
  }

  if (DOWN && event.key.keysym.sym == SDLK_ESCAPE)
  {
	  mainLoop->removeWindow(window->getId());
  }

  if (DOWN && event.key.keysym.sym == '.') {
    auto n = vars.getNofVars();
    for(size_t i=0;i<n;++i)
      std::cerr << vars.getVarName(i) << std::endl;
  }

  if (DOWN && event.key.keysym.sym == SDLK_o)
  {
	  updateLightPosViewUpFromCamera(vars);
  }

  if (DOWN && event.key.keysym.sym == SDLK_u)
  {
      *vars.get<bool>("showShadowMask") = !(*vars.get<bool>("showShadowMask"));
  }
}

void Shadows::resize(uint32_t x,uint32_t y){
  FUNCTION_CALLER();

  auto windowSize = vars.get<glm::uvec2>("windowSize");
  windowSize->x = x;
  windowSize->y = y;
  vars.updateTicks("windowSize");
  ge::gl::glViewport(0,0,x,y);
}

void Shadows::mouseMove(SDL_Event const& event) {
  mouseMoveCamera(vars, event);
}

