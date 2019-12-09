#include <glm/glm.hpp>

#include <Vars/Vars.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <FunctionPrologue.h>
#include <divRoundUp.h>
#include <BallotShader.h>

#include <Sintorn2/rasterize.h>
#include <Sintorn2/rasterizeShader.h>
#include <Sintorn2/configShader.h>


using namespace ge::gl;
using namespace std;

namespace sintorn2{
void createRasterizeProgram(vars::Vars&vars){
  FUNCTION_PROLOGUE("sintorn2.method"
      ,"wavefrontSize"
      ,"sintorn2.method.nofTriangles"
      ,"sintorn2.param.sfWGS"
      ,"sintorn2.param.triangleAlignment"
      ,"sintorn2.param.sfAlignment"
      ,"sintorn2.param.bias"
      ,"sintorn2.param.sfInterleave"
      ,"sintorn2.param.triangleInterleave"
      );

  auto const wavefrontSize       = vars.getSizeT ("wavefrontSize"                    );
  auto const nofTriangles        = vars.getUint32("sintorn2.method.nofTriangles"     );
  auto const triangleAlignment   = vars.getUint32("sintorn2.param.triangleAlignment" );
  auto const sfAlignment         = vars.getUint32("sintorn2.param.sfAlignment"       );
  auto const sfInterleave        = vars.getInt32 ("sintorn2.param.sfInterleave"      );

  vars.reCreate<ge::gl::Program>("sintorn2.method.rasterizeProgram",
      std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER,
        "#version 450\n",
        Shader::define("WARP"               ,(uint32_t)wavefrontSize     ),
        Shader::define("NOF_TRIANGLES"      ,(uint32_t)nofTriangles      ),
        Shader::define("TRIANGLE_ALIGNMENT" ,(uint32_t)triangleAlignment ),
        Shader::define("SF_ALIGNMENT"       ,(uint32_t)sfAlignment       ),
        Shader::define("SF_INTERLEAVE"      ,(int)     sfInterleave      ),
        ballotSrc,
        sintorn2::configShader,
        sintorn2::rasterizeShader
        ));

}

void createJobCounter(vars::Vars&vars){
  FUNCTION_PROLOGUE("sintorn2.method");
  vars.reCreate<Buffer>("sintorn2.method.jobCounter;",sizeof(uint32_t));
}

}


void sintorn2::rasterize(vars::Vars&vars){
  //createRasterizeProgram(vars);
  //createJobCounter(vars);

  //auto jobCounter = vars.get<Buffer> ("sintorn2.method.jobCounter"      );
  //auto prg        = vars.get<Program>("sintorn2.method.rasterizeProgram");


  //jobCounter->clear(GL_R32UI,GL_RED_INTEGER,GL_UNSIGNED_INT);

  //jobCounter->bindBase(GL_SHADER_STORAGE_BUFFER,5);


  //prg->use();

}