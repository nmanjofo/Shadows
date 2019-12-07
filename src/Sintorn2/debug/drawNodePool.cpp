#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Vars/Vars.h>
#include <imguiVars/addVarsLimits.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <Deferred.h>
#include <FunctionPrologue.h>
#include <divRoundUp.h>
#include <requiredBits.h>

#include <Sintorn2/debug/drawNodePool.h>

#include <Sintorn2/mortonShader.h>
#include <Sintorn2/quantizeZShader.h>
#include <Sintorn2/depthToZShader.h>
#include <Sintorn2/config.h>

using namespace ge::gl;
using namespace std;

namespace sintorn2::debug{

void prepareDrawNodePool(vars::Vars&vars){
  FUNCTION_PROLOGUE("sintorn2.method.debug"
      "wavefrontSize"                        ,
      "sintorn2.method.debug.dump.config"    ,
      "sintorn2.method.debug.dump.near"      ,
      "sintorn2.method.debug.dump.far"       ,
      "sintorn2.method.debug.dump.fovy"      ,
      );

  auto const cfg            = *vars.get<Config>        ("sintorn2.method.debug.dump.config"    );
  auto const nnear          =  vars.getFloat           ("sintorn2.method.debug.dump.near"      );
  auto const ffar           =  vars.getFloat           ("sintorn2.method.debug.dump.far"       );
  auto const fovy           =  vars.getFloat           ("sintorn2.method.debug.dump.fovy"      );

  auto const wavefrontSize  =  vars.getSizeT           ("wavefrontSize"                        );


  std::string const vsSrc = R".(
  #version 450

  flat out uint vId;
  void main(){
    vId = gl_VertexID;
  }

  ).";

  std::string const fsSrc = R".(
  #version 450

  uniform uint levelToDraw = 0;

  const vec4 colors[6] = {
    vec4(1,1,1,1),
    vec4(1,0,0,1),
    vec4(0,1,0,1),
    vec4(0,0,1,1),
    vec4(1,1,0,1),
    vec4(1,0,1,1),
  };
  layout(location=0)out vec4 fColor;
  void main(){
    fColor = vec4(colors[levelToDraw]);
  }
  ).";

  std::string const gsSrc = R".(
#ifndef WARP
#define WARP 64
#endif//WARP
#line 72
#ifndef WINDOW_X
#define WINDOW_X 512
#endif//WINDOW_X

#ifndef WINDOW_Y
#define WINDOW_Y 512
#endif//WINDOW_Y

#ifndef TILE_X
#define TILE_X 8
#endif//TILE_X

#ifndef TILE_Y
#define TILE_Y 8
#endif//TILE_Y

#ifndef MIN_Z_BITS
#define MIN_Z_BITS 9
#endif//MIN_Z_BITS

#ifndef NEAR
#define NEAR 0.01f
#endif//NEAR

#ifndef FAR
#define FAR 1000.f
#endif//FAR

#ifndef FOVY
#define FOVY 1.5707963267948966f
#endif//FOVY

uint divRoundUp(uint x,uint y){
  return uint(x/y) + uint((x%y)>0);
}

layout(points)in;
layout(line_strip,max_vertices=28)out;

flat in uint vId[];

layout(binding=0)buffer NodePool{uint nodePool[];};
layout(binding=1)buffer AABBPool{float aabbPool[];};

uniform mat4 view;
uniform mat4 proj;

uniform mat4 nodeView;
uniform mat4 nodeProj;

#line 122
uniform uint levelToDraw = 0;

uniform uint drawTightAABB = 0;

void main(){
  const uint warpBits        = uint(ceil(log2(float(WARP))));
  const uint clustersX       = uint(WINDOW_X/TILE_X) + uint(WINDOW_X%TILE_X != 0u);
  const uint clustersY       = uint(WINDOW_Y/TILE_Y) + uint(WINDOW_Y%TILE_Y != 0u);
  const uint xBits           = uint(ceil(log2(float(clustersX))));
  const uint yBits           = uint(ceil(log2(float(clustersY))));
  const uint zBits           = MIN_Z_BITS>0?MIN_Z_BITS:max(max(xBits,yBits),MIN_Z_BITS);
  const uint clustersZ       = uint(1u << zBits);
  const uint allBits         = xBits + yBits + zBits;
  const uint nofLevels       = uint(allBits/warpBits) + uint(allBits%warpBits != 0u);
  const uint uintsPerWarp    = uint(WARP/32u);

  const uint warpMask        = uint(WARP - 1u);
  const uint floatsPerAABB   = 6u;


  const uint nodesPerLevel[6] = {
    1u << uint(max(int(allBits) - int((nofLevels-1u)*warpBits),0)),
    1u << uint(max(int(allBits) - int((nofLevels-2u)*warpBits),0)),
    1u << uint(max(int(allBits) - int((nofLevels-3u)*warpBits),0)),
    1u << uint(max(int(allBits) - int((nofLevels-4u)*warpBits),0)),
    1u << uint(max(int(allBits) - int((nofLevels-5u)*warpBits),0)),
    1u << uint(max(int(allBits) - int((nofLevels-6u)*warpBits),0)),
  };

  const uint nodeLevelSizeInUints[6] = {
    max(nodesPerLevel[0] >> warpBits,1u) * uintsPerWarp,
    max(nodesPerLevel[1] >> warpBits,1u) * uintsPerWarp,
    max(nodesPerLevel[2] >> warpBits,1u) * uintsPerWarp,
    max(nodesPerLevel[3] >> warpBits,1u) * uintsPerWarp,
    max(nodesPerLevel[4] >> warpBits,1u) * uintsPerWarp,
    max(nodesPerLevel[5] >> warpBits,1u) * uintsPerWarp,
  };

  const uint nodeLevelOffsetInUints[6] = {
    0,
    0 + nodeLevelSizeInUints[0],
    0 + nodeLevelSizeInUints[0] + nodeLevelSizeInUints[1],
    0 + nodeLevelSizeInUints[0] + nodeLevelSizeInUints[1] + nodeLevelSizeInUints[2],
    0 + nodeLevelSizeInUints[0] + nodeLevelSizeInUints[1] + nodeLevelSizeInUints[2] + nodeLevelSizeInUints[3],
    0 + nodeLevelSizeInUints[0] + nodeLevelSizeInUints[1] + nodeLevelSizeInUints[2] + nodeLevelSizeInUints[3] + nodeLevelSizeInUints[4],
  };

  const uint aabbLevelSizeInFloats[6] = {
    nodesPerLevel[0] * floatsPerAABB,
    nodesPerLevel[1] * floatsPerAABB,
    nodesPerLevel[2] * floatsPerAABB,
    nodesPerLevel[3] * floatsPerAABB,
    nodesPerLevel[4] * floatsPerAABB,
    nodesPerLevel[5] * floatsPerAABB,
  };

  const uint aabbLevelOffsetInFloats[6] = {
    0,
    0 + aabbLevelSizeInFloats[0],
    0 + aabbLevelSizeInFloats[0] + aabbLevelSizeInFloats[1],
    0 + aabbLevelSizeInFloats[0] + aabbLevelSizeInFloats[1] + aabbLevelSizeInFloats[2],
    0 + aabbLevelSizeInFloats[0] + aabbLevelSizeInFloats[1] + aabbLevelSizeInFloats[2] + aabbLevelSizeInFloats[3],
    0 + aabbLevelSizeInFloats[0] + aabbLevelSizeInFloats[1] + aabbLevelSizeInFloats[2] + aabbLevelSizeInFloats[3] + aabbLevelSizeInFloats[4],
  };

  uint gId = vId[0];
#line 157
  uint bitsToDiv = warpBits*(nofLevels-1-levelToDraw);
  uint xBitsToDiv = divRoundUp(bitsToDiv , 3u);
  uint yBitsToDiv = divRoundUp(uint(max(int(bitsToDiv)-1,0)) , 3u);
  uint zBitsToDiv = divRoundUp(uint(max(int(bitsToDiv)-2,0)) , 3u);

#line 163
  uint clusX = divRoundUp(clustersX,1u<<xBitsToDiv);
  uint clusY = divRoundUp(clustersY,1u<<yBitsToDiv);

  uint x = gId % clusX;
  uint y = (gId / clusX) % clusY;
  uint z = (gId / (clusX * clusY));

  uint mor = morton(uvec3(x<<xBitsToDiv,y<<yBitsToDiv,z<<zBitsToDiv));

  uint bit  = (mor >> (warpBits*(nofLevels-1-levelToDraw))) & warpMask;
  uint node = (mor >> (warpBits*(nofLevels  -levelToDraw)));


  uint doesNodeExist = nodePool[nodeLevelOffsetInUints[clamp(levelToDraw,0u,5u)]+node*uintsPerWarp+uint(bit>31u)]&(1u<<(bit&0x1fu));

  if(doesNodeExist == 0)return;

  uint aabbNode = (mor >> (warpBits*(nofLevels-1-levelToDraw)));
  float mminX = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+0];
  float mmaxX = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+1];
  float mminY = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+2];
  float mmaxY = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+3];
  float mminZ = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+4];
  float mmaxZ = aabbPool[aabbLevelOffsetInFloats[clamp(levelToDraw,0u,5u)]+aabbNode*floatsPerAABB+5];

  float startZ = clusterToZ(z<<zBitsToDiv);
  float endZ   = clusterToZ((z+1)<<zBitsToDiv);

  float startX = -1.f + 2.f * float((x<<xBitsToDiv)*TILE_X) / float(WINDOW_X);
  float startY = -1.f + 2.f * float((y<<yBitsToDiv)*TILE_Y) / float(WINDOW_Y);

  float endX = clamp(-1.f + 2.f * float(((x+1)<<xBitsToDiv)*TILE_X) / float(WINDOW_X),-1.f,1.f);
  float endY = clamp(-1.f + 2.f * float(((y+1)<<yBitsToDiv)*TILE_Y) / float(WINDOW_Y),-1.f,1.f);

#ifdef FAR_IS_INFINITE
  float e = -1.f;
  float f = -2.f * NEAR;
#else
  float e = -(FAR + NEAR) / (FAR - NEAR);
  float f = -2.f * NEAR * FAR / (FAR - NEAR);
#endif

  if(drawTightAABB != 0){
    startX = mminX;
    endX   = mmaxX;

    startY = mminY;
    endY   = mmaxY;

    startZ = depthToZ(mminZ);
    endZ   = depthToZ(mmaxZ);
  }


  mat4 M = proj*view*inverse(nodeView)*inverse(nodeProj);
  gl_Position = M*vec4(startX*(-startZ),startY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-startZ),startY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-startZ),  endY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(startX*(-startZ),  endY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(startX*(-startZ),startY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  EndPrimitive();
#line 208
  gl_Position = M*vec4(startX*(-  endZ),startY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-  endZ),startY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-  endZ),  endY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  gl_Position = M*vec4(startX*(-  endZ),  endY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  gl_Position = M*vec4(startX*(-  endZ),startY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  EndPrimitive();

  gl_Position = M*vec4(startX*(-startZ),startY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(startX*(-  endZ),startY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  EndPrimitive();
  gl_Position = M*vec4(  endX*(-startZ),startY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-  endZ),startY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  EndPrimitive();
  gl_Position = M*vec4(  endX*(-startZ),  endY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(  endX*(-  endZ),  endY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  EndPrimitive();
  gl_Position = M*vec4(startX*(-startZ),  endY*(-startZ),e*startZ+f,(-startZ));EmitVertex();
  gl_Position = M*vec4(startX*(-  endZ),  endY*(-  endZ),e*  endZ+f,(-  endZ));EmitVertex();
  EndPrimitive();
}

  ).";

  auto vs = make_shared<Shader>(GL_VERTEX_SHADER,vsSrc);
  auto gs = make_shared<Shader>(GL_GEOMETRY_SHADER,
      "#version 450\n",
      ge::gl::Shader::define("WARP"      ,(uint32_t)wavefrontSize),
      ge::gl::Shader::define("WINDOW_X"  ,(uint32_t)cfg.windowX  ),
      ge::gl::Shader::define("WINDOW_Y"  ,(uint32_t)cfg.windowY  ),
      ge::gl::Shader::define("MIN_Z_BITS",(uint32_t)cfg.minZBits ),
      ge::gl::Shader::define("NEAR"      ,nnear                  ),
      glm::isinf(ffar)?ge::gl::Shader::define("FAR_IS_INFINITE"):ge::gl::Shader::define("FAR",ffar),
      ge::gl::Shader::define("FOVY"      ,fovy                   ),
      ge::gl::Shader::define("TILE_X"    ,cfg.tileX              ),
      ge::gl::Shader::define("TILE_Y"    ,cfg.tileY              ),

      sintorn2::mortonShader,
      sintorn2::depthToZShader,
      sintorn2::quantizeZShader,
      gsSrc);
  auto fs = make_shared<Shader>(GL_FRAGMENT_SHADER,fsSrc);

  vars.reCreate<Program>(
      "sintorn2.method.debug.drawNodePoolProgram",
      vs,
      gs,
      fs
      );

}

void drawNodePool(vars::Vars&vars){
  prepareDrawNodePool(vars);

  auto const cfg            = *vars.get<Config>        ("sintorn2.method.debug.dump.config"          );

  auto const nodeView       = *vars.get<glm::mat4>     ("sintorn2.method.debug.dump.viewMatrix"      );
  auto const nodeProj       = *vars.get<glm::mat4>     ("sintorn2.method.debug.dump.projectionMatrix");
  auto const nodePool       =  vars.get<Buffer>        ("sintorn2.method.debug.dump.nodePool"        );
  auto const aabbPool       =  vars.get<Buffer>        ("sintorn2.method.debug.dump.aabbPool"        );

  auto const view           = *vars.get<glm::mat4>     ("sintorn2.method.debug.viewMatrix"           );
  auto const proj           = *vars.get<glm::mat4>     ("sintorn2.method.debug.projectionMatrix"     );
  auto const levelsToDraw   =  vars.getUint32          ("sintorn2.method.debug.levelsToDraw"         );
  auto const drawTightAABB  =  vars.getBool            ("sintorn2.method.debug.drawTightAABB"        );

  auto vao = vars.get<VertexArray>("sintorn2.method.debug.vao");

  auto prg = vars.get<Program>("sintorn2.method.debug.drawNodePoolProgram");

  vao->bind();
  nodePool->bindBase(GL_SHADER_STORAGE_BUFFER,0);
  aabbPool->bindBase(GL_SHADER_STORAGE_BUFFER,1);
  prg->use();
  prg
    ->setMatrix4fv("nodeView"   ,glm::value_ptr(nodeView))
    ->setMatrix4fv("nodeProj"   ,glm::value_ptr(nodeProj))
    ->setMatrix4fv("view"       ,glm::value_ptr(view    ))
    ->setMatrix4fv("proj"       ,glm::value_ptr(proj    ))
    ->set1ui      ("drawTightAABB",(uint32_t)drawTightAABB)
    ;


  for(uint32_t l=0;l<cfg.nofLevels;++l){
    if((levelsToDraw&(1u<<l)) == 0)continue;
    prg->set1ui      ("levelToDraw",l);
    glDrawArrays(GL_POINTS,0,cfg.nofNodesPerLevel[l]);
  }

  vao->unbind();

}

}
