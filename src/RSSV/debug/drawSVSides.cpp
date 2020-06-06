#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Vars/Vars.h>
#include <imguiVars/addVarsLimits.h>
#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <FunctionPrologue.h>
#include <FastAdjacency.h>

#include <RSSV/getEdgePlanesShader.h>

using namespace ge::gl;
using namespace std;

namespace rssv::debug{

void prepareDrawSVSides(vars::Vars&vars){
  FUNCTION_PROLOGUE("rssv.method.debug"
      ,"wavefrontSize"                        
      );

  auto const alignedNofEdges =  vars.getUint32  ("rssv.method.alignedNofEdges"  );

  std::string const vsSrc = R".(
  #version 450

  flat out uint vId;
  void main(){
    vId = gl_VertexID;
  }

  ).";

  std::string const fsSrc = R".(

  layout(location=0)out vec4 fColor;
  void main(){
    fColor = vec4(0,1,1,.5);
  }
  ).";

  std::string const gsSrc = R".(

layout(binding=0)buffer EdgeBuffer       {float edgeBuffer       [];};
layout(binding=1)buffer MultBuffer       {uint  multBuffer       [];};
layout(binding=2)buffer SilhouetteCounter{uint  silhouetteCounter[];};

layout(points)in;
layout(triangle_strip,max_vertices=4)out;

flat in uint vId[];

uniform mat4 view;
uniform mat4 proj;

uniform mat4 debugView;
uniform mat4 debugProj;

uniform vec4 light;
uniform int selectedEdge = -1;

void main(){
  uint thread = vId[0];

  if(thread >= silhouetteCounter[0])return;
  if(selectedEdge >= 0 && int(thread) != selectedEdge)return;

  uint res = multBuffer[thread];
  uint edge = res & 0x1fffffffu;
  int  mult = int(res) >> 29;

  vec4 P[4];
  P[0][0] = edgeBuffer[edge+0*ALIGNED_NOF_EDGES];
  P[0][1] = edgeBuffer[edge+1*ALIGNED_NOF_EDGES];
  P[0][2] = edgeBuffer[edge+2*ALIGNED_NOF_EDGES];
  P[0][3] = 1;
  P[1][0] = edgeBuffer[edge+3*ALIGNED_NOF_EDGES];
  P[1][1] = edgeBuffer[edge+4*ALIGNED_NOF_EDGES];
  P[1][2] = edgeBuffer[edge+5*ALIGNED_NOF_EDGES];
  P[1][3] = 1;

  float len = 10;
  P[2] = P[0]+len*(P[0]-light);
  P[3] = P[1]+len*(P[1]-light);

  mat4 M = proj*view;

  gl_Position = M * P[0];EmitVertex();
  gl_Position = M * P[1];EmitVertex();
  gl_Position = M * P[2];EmitVertex();
  gl_Position = M * P[3];EmitVertex();
  EndPrimitive();
}

  ).";


  auto vs = make_shared<Shader>(GL_VERTEX_SHADER,vsSrc);
  auto gs = make_shared<Shader>(GL_GEOMETRY_SHADER,
      "#version 450\n",
      Shader::define("ALIGNED_NOF_EDGES",alignedNofEdges),
      getEdgePlanesShader,
      gsSrc);
  auto fs = make_shared<Shader>(GL_FRAGMENT_SHADER,
      "#version 450\n",
      fsSrc);

  vars.reCreate<Program>(
      "rssv.method.debug.drawSVSidesProgram",
      vs,
      gs,
      fs
      );

}

void drawSVSides(vars::Vars&vars){
  prepareDrawSVSides(vars);
  auto const view              = *vars.get<glm::mat4>  ("rssv.method.debug.viewMatrix"            );
  auto const proj              = *vars.get<glm::mat4>  ("rssv.method.debug.projectionMatrix"      );

  auto const debugLight        = *vars.get<glm::vec4 >("rssv.method.debug.dump.lightPosition"     );

  auto const adj               =  vars.get<Adjacency>  ("adjacency"                               );
  auto const vao               =  vars.get<VertexArray>("rssv.method.debug.vao"                   );
  auto const prg               =  vars.get<Program>    ("rssv.method.debug.drawSVSidesProgram"    );
  auto const edges             =  vars.get<Buffer>     ("rssv.method.edgeBuffer"                  );
  auto const multBuffer        =  vars.get<Buffer>     ("rssv.method.debug.dump.multBuffer"       );
  auto const silhouetteCounter =  vars.get<Buffer>     ("rssv.method.debug.dump.silhouetteCounter");
  auto nofEdges = adj->getNofEdges();

  edges->bindBase(GL_SHADER_STORAGE_BUFFER,0);
  multBuffer->bindBase(GL_SHADER_STORAGE_BUFFER,1);
  silhouetteCounter->bindBase(GL_SHADER_STORAGE_BUFFER,2);

  vao->bind();
  prg->use();
  prg
    ->setMatrix4fv("view"         ,glm::value_ptr(view      ))
    ->setMatrix4fv("proj"         ,glm::value_ptr(proj      ))
    ->set4fv      ("light"        ,glm::value_ptr(debugLight))
    ;
  prg->set1i("selectedEdge",vars.addOrGetInt32("rssv.param.selectedEdge",-1));


  ge::gl::glEnable(GL_DEPTH_TEST);
  ge::gl::glEnable(GL_BLEND);
  ge::gl::glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glDrawArrays(GL_POINTS,0,nofEdges);
  ge::gl::glDisable(GL_BLEND);
  ge::gl::glDisable(GL_DEPTH_TEST);

  vao->unbind();

}

}
