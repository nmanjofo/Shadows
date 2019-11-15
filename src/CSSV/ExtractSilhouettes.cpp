#include <CSSV/ExtractSilhouettes.h>
#include <FastAdjacency.h>
#include <geGL/StaticCalls.h>
#include <glm/gtc/type_ptr.hpp>
#include <util.h>
#include<CSSV/ExtractSilhouetteShader.h>
#include<SilhouetteShaders.h>

using namespace std;
using namespace ge::gl;
using namespace cssv;

shared_ptr<Buffer>createDIBO(){
  struct DrawArraysIndirectCommand{
    uint32_t nofVertices  = 0;
    uint32_t nofInstances = 0;
    uint32_t firstVertex  = 0;
    uint32_t baseInstance = 0;
  };
  DrawArraysIndirectCommand cmd;
  cmd.nofInstances = 1;
  return make_shared<Buffer>(sizeof(DrawArraysIndirectCommand),&cmd);
}


ExtractSilhouettes::ExtractSilhouettes(vars::Vars&vars,shared_ptr<Adjacency const>const&adj):vars(vars){
  program = make_shared<Program>(
      make_shared<Shader>(GL_COMPUTE_SHADER,
        "#version 450 core\n",
        Shader::define("WORKGROUP_SIZE_X",int32_t(vars.getUint32("cssv.param.computeSidesWGS"))),
        Shader::define("MAX_MULTIPLICITY",int32_t(adj->getMaxMultiplicity()             )),
        Shader::define("LOCAL_ATOMIC"    ,int32_t(vars.getBool("cssv.param.localAtomic"      ))),
        Shader::define("CULL_SIDES"      ,int32_t(vars.getBool("cssv.param.cullSides"        ))),
        Shader::define("USE_PLANES"      ,int32_t(vars.getBool("cssv.param.usePlanes"        ))),
        Shader::define("USE_INTERLEAVING",int32_t(vars.getBool("cssv.param.useInterleaving"  ))),
        silhouetteFunctions,
        computeSrc));
  dibo = createDIBO();
}

void ExtractSilhouettes::compute(glm::vec4 const&lightPosition){
  assert(this                      !=nullptr);
  dibo->clear(GL_R32UI,0,sizeof(uint32_t),GL_RED_INTEGER,GL_UNSIGNED_INT);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,edges->getId());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,sillhouettes->getId());
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,dibo->getId());
  //std::cerr << "edges      : " << program->getBufferBinding("edges"             ) << std::endl;
  //std::cerr << "silhouettes: " << program->getBufferBinding("silhouettes"       ) << std::endl;
  //std::cerr << "dibo       : " << program->getBufferBinding("drawIndirectBuffer") << std::endl;
  program
    ->set1ui    ("numEdge"           ,uint32_t(nofEdges)    )
    ->set4fv    ("lightPosition"     ,glm::value_ptr(lightPosition))
    //->bindBuffer("edges"             ,edges                 )
    //->bindBuffer("silhouettes"       ,sillhouettes          )
    //->bindBuffer("drawIndirectBuffer",dibo                  )
    ->dispatch((GLuint)getDispatchSize(nofEdges,vars.getUint32("cssv.param.computeSidesWGS")));

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glFinish();

}

