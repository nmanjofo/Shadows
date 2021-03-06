#include <Sintorn2/Sintorn2.h>
#include <Deferred.h>
#include <FunctionPrologue.h>
#include <divRoundUp.h>
#include <requiredBits.h>
#include <startStop.h>
#include <sstream>
#include <algorithm>
#include <BallotShader.h>

#include <Sintorn2/buildHierarchy.h>
#include <Sintorn2/computeShadowFrusta.h>
#include <Sintorn2/rasterize.h>
#include <Sintorn2/merge.h>
#include <Sintorn2/debug/drawDebug.h>

Sintorn2::Sintorn2(vars::Vars& vars) : ShadowMethod(vars) {}

Sintorn2::~Sintorn2() {vars.erase("cssv.method");}



void Sintorn2::create(glm::vec4 const& lightPosition,
                      glm::mat4 const& viewMatrix,
                      glm::mat4 const& projectionMatrix)
{
  FUNCTION_CALLER();
  *vars.addOrGet<glm::vec4>("sintorn2.method.lightPosition"   ) = lightPosition   ;
  *vars.addOrGet<glm::mat4>("sintorn2.method.viewMatrix"      ) = viewMatrix      ;
  *vars.addOrGet<glm::mat4>("sintorn2.method.projectionMatrix") = projectionMatrix;

  //glFinish();
  ifExistStamp("");
  sintorn2::computeShadowFrusta(vars);
  ifExistStamp("computeShadowFrusta");
  sintorn2::buildHierarchy(vars);
  ifExistStamp("buildHierarchy");
  sintorn2::rasterize(vars);
  ifExistStamp("rasterize");
  sintorn2::merge(vars);
  ifExistStamp("merge");

}

void Sintorn2::drawDebug(glm::vec4 const& lightPosition,
                      glm::mat4 const& viewMatrix,
                      glm::mat4 const& projectionMatrix)
{
  FUNCTION_CALLER();
  *vars.addOrGet<glm::vec4>("sintorn2.method.debug.lightPosition"   ) = lightPosition   ;
  *vars.addOrGet<glm::mat4>("sintorn2.method.debug.viewMatrix"      ) = viewMatrix      ;
  *vars.addOrGet<glm::mat4>("sintorn2.method.debug.projectionMatrix") = projectionMatrix;
  sintorn2::drawDebug(vars);
}
