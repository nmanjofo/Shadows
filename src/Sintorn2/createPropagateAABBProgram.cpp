#include <glm/glm.hpp>

#include <Vars/Vars.h>
#include <geGL/geGL.h>

#include <FunctionPrologue.h>
#include <BallotShader.h>

#include <Sintorn2/createPropagateAABBProgram.h>
#include <Sintorn2/propagateAABBShader.h>
#include <Sintorn2/configShader.h>

void sintorn2::createPropagateAABBProgram(vars::Vars&vars){
  FUNCTION_PROLOGUE("sintorn2.method"
      ,"windowSize"
      ,"wavefrontSize"
      ,"sintorn2.param.minZBits"   
      ,"sintorn2.param.tileX"      
      ,"sintorn2.param.tileY"      
      ,"sintorn2.param.memoryOptim"
      );

  auto const wavefrontSize =  vars.getSizeT       ("wavefrontSize"                );
  auto const windowSize    = *vars.get<glm::uvec2>("windowSize"                   );
  auto const minZBits      =  vars.getUint32      ("sintorn2.param.minZBits"      );
  auto const tileX         =  vars.getUint32      ("sintorn2.param.tileX"         );
  auto const tileY         =  vars.getUint32      ("sintorn2.param.tileY"         );
  auto const nofWarps      =  vars.getUint32      ("sintorn2.param.propagateWarps");
  auto const memoryOptim   =  vars.getBool        ("sintorn2.param.memoryOptim"   );

  vars.reCreate<ge::gl::Program>("sintorn2.method.propagateAABBProgram",
      std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER,
        "#version 450\n",
        ge::gl::Shader::define("WARP"        ,(uint32_t)wavefrontSize),
        ge::gl::Shader::define("WINDOW_X"    ,(uint32_t)windowSize.x ),
        ge::gl::Shader::define("WINDOW_Y"    ,(uint32_t)windowSize.y ),
        ge::gl::Shader::define("MIN_Z_BITS"  ,(uint32_t)minZBits     ),
        ge::gl::Shader::define("TILE_X"      ,tileX                  ),
        ge::gl::Shader::define("TILE_Y"      ,tileY                  ),
        ge::gl::Shader::define("NOF_WARPS"   ,(uint32_t)nofWarps     ),
        ge::gl::Shader::define("MEMORY_OPTIM",(int)memoryOptim       ),
        ballotSrc,
        sintorn2::configShader,
        sintorn2::propagateAABBShader
        ));
}
