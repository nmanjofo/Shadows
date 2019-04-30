#include <Vars/Vars.h>

#include <Methods.h>
#include <CubeShadowMapping/CubeShadowMapping.h>
#include <CSSV/CSSV.h>
#include <CSSVSOE.h>
#include <RSSV/RSSV.h>
#include <Sintorn/Sintorn.h>
#include <VSSV/VSSV.h>
#include <RayTracing/RayTracing.h>

void initMethods(vars::Vars&vars){
  auto methods = vars.add<Methods>("methods");
  methods->add<CubeShadowMapping>("cubeShadowMapping");
  methods->add<cssv::CSSV       >("cssv"             );
  methods->add<CSSVSOE          >("cssvsoe"          );
  methods->add<Sintorn          >("sintorn"          );
  methods->add<rssv::RSSV       >("rssv"             );
  methods->add<VSSV             >("vssv"             );
  methods->add<RayTracing       >("rayTracing"       );
}