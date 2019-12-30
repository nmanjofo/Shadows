#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Vars/Vars.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>
#include <imguiDormon/imgui.h>

#include <Deferred.h>
#include <FunctionPrologue.h>
#include <divRoundUp.h>

#include <RSSV/mortonShader.h>
#include <RSSV/debug/drawDebug.h>
#include <RSSV/debug/dumpData.h>
#include <RSSV/debug/drawSamples.h>
#include <RSSV/debug/drawNodePool.h>
#include <RSSV/debug/drawTraverse.h>
#include <RSSV/debug/drawSF.h>
#include <RSSV/configShader.h>
#include <RSSV/config.h>

using namespace ge::gl;
using namespace std;

namespace rssv::debug{

void prepareCommon(vars::Vars&vars){
  FUNCTION_PROLOGUE("rssv.method.debug");
  vars.reCreate<VertexArray>("rssv.method.debug.vao");
}

void blitDepth(vars::Vars&vars){
  auto windowSize = *vars.get<glm::uvec2>("windowSize");
  auto gBuffer = vars.get<GBuffer>("gBuffer");
  glBlitNamedFramebuffer(
      gBuffer->fbo->getId(),
      0,
      0,0,windowSize.x,windowSize.y,
      0,0,windowSize.x,windowSize.y,
      GL_DEPTH_BUFFER_BIT,
      GL_NEAREST);
}

}

void rssv::drawDebug(vars::Vars&vars){
  debug::prepareCommon(vars);

  auto&levelsToDraw       = vars.addOrGetUint32("rssv.method.debug.levelsToDraw",0);
  auto&drawTightAABB      = vars.addOrGetBool  ("rssv.method.debug.drawTightAABB");
  auto&wireframe          = vars.addOrGetBool  ("rssv.method.debug.wireframe",true);

  auto&drawSamples        = vars.addOrGetBool("rssv.method.debug.drawSamples"     );
  auto&drawNodePool       = vars.addOrGetBool("rssv.method.debug.drawNodePool"    );
  auto&drawTraverse       = vars.addOrGetBool("rssv.method.debug.drawTraverse"    );
  auto&drawShadowFrusta   = vars.addOrGetBool("rssv.method.debug.drawShadowFrusta");

  auto&taToDraw = vars.addOrGetUint32("rssv.method.debug.taToDraw",0);
  auto&trToDraw = vars.addOrGetUint32("rssv.method.debug.trToDraw",0);
  auto&inToDraw = vars.addOrGetUint32("rssv.method.debug.inToDraw",0);

  if(ImGui::BeginMainMenuBar()){
    if(ImGui::BeginMenu("dump")){
      if(ImGui::MenuItem("copyData"))
        rssv::debug::dumpData(vars);

      if(ImGui::MenuItem("drawSamples")){
        drawSamples = !drawSamples;
        vars.updateTicks("rssv.method.debug.drawSamples");
      }

      if(ImGui::MenuItem("drawNodePool")){
        drawNodePool = !drawNodePool;
        vars.updateTicks("rssv.method.debug.drawNodePool");
      }

      if(ImGui::MenuItem("drawTraverse")){
        drawTraverse = !drawTraverse;
        vars.updateTicks("rssv.method.debug.drawTraverse");
      }

      if(ImGui::MenuItem("drawTightAABB")){
        drawTightAABB = !drawTightAABB;
        vars.updateTicks("rssv.method.debug.drawTightAABB");
      }

      if(ImGui::MenuItem("wireframe")){
        wireframe = !wireframe;
        vars.updateTicks("rssv.method.debug.wireframe");
      }

      if(ImGui::MenuItem("drawShadowFrusta")){
        drawShadowFrusta = !drawShadowFrusta;
        vars.updateTicks("rssv.method.debug.drawShadowFrusta");
      }

      if(drawNodePool){
        if(vars.has("rssv.method.debug.dump.config")){
          auto const cfg = *vars.get<Config>        ("rssv.method.debug.dump.config"    );
          for(uint32_t i=0;i<cfg.nofLevels;++i){
            std::stringstream ss;
            ss << "level" << i;
            if(ImGui::MenuItem(ss.str().c_str())){
              levelsToDraw ^= 1<<i;
            }
          }
        }
      }

      if(drawTraverse){
        if(vars.has("rssv.method.debug.dump.config")){
          auto const cfg = *vars.get<Config>        ("rssv.method.debug.dump.config"    );
          for(uint32_t i=0;i<cfg.nofLevels;++i){
            std::stringstream ss;
            ss << "trivialAccept" << i;
            if(ImGui::MenuItem(ss.str().c_str())){
              taToDraw ^= 1<<i;
            }
          }
          for(uint32_t i=0;i<cfg.nofLevels;++i){
            std::stringstream ss;
            ss << "trivialReject" << i;
            if(ImGui::MenuItem(ss.str().c_str())){
              trToDraw ^= 1<<i;
            }
          }
          for(uint32_t i=0;i<cfg.nofLevels;++i){
            std::stringstream ss;
            ss << "intersect" << i;
            if(ImGui::MenuItem(ss.str().c_str())){
              inToDraw ^= 1<<i;
            }
          }
        }
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  if(drawSamples)
    debug::drawSamples(vars);

  if(drawNodePool)
    debug::drawNodePool(vars);

  if(drawTraverse)
    debug::drawTraverse(vars);

  if(drawShadowFrusta)
    debug::drawSF(vars);


}
