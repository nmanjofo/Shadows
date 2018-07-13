#include<createGBuffer.h>

#include<glm/glm.hpp>
#include<geGL/StaticCalls.h>

#include<Deferred.h>
#include<BasicCamera/Camera.h>
#include<Model.h>

void createGBuffer(vars::Vars&vars){
  auto windowSize = *vars.get<glm::uvec2>("windowSize");
  ge::gl::glViewport(0, 0, windowSize.x, windowSize.y);
  ge::gl::glEnable(GL_DEPTH_TEST);
  vars.get<GBuffer>("gBuffer")->begin();
  ge::gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                  GL_STENCIL_BUFFER_BIT);
  vars.get<ge::gl::Texture>("shadowMask")->clear(0, GL_RED, GL_FLOAT);
  auto const cameraProjection = vars.getReinterpret<basicCamera::CameraProjection>("cameraProjection");
  auto const cameraTransform  = vars.getReinterpret<basicCamera::CameraTransform >("cameraTransform" );
  vars.get<RenderModel>("renderModel")->draw(cameraProjection->getProjection() * cameraTransform->getView());
  vars.get<GBuffer>("gBuffer")->end();
}

