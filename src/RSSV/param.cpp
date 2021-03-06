#include <RSSV/param.h>
#include <ArgumentViewer/ArgumentViewer.h>
#include <Vars/Vars.h>

void rssv::loadParams(
    vars::Vars&vars,
    std::shared_ptr<argumentViewer::ArgumentViewer>const&arg){
  auto c = arg->getContext("rssvParam","parameters for ray traced silhouette shadow volumes");

  vars.addUint32("rssv.param.minZBits"                   ) =       c->getu32("minZBits"                   ,10  ,"select number of Z bits - 0 mean max(xBits,yBits)"                               );
  vars.addUint32("rssv.param.propagateWarps"             ) =       c->getu32("propagateWarps"             ,4   ,"number of warps cooperating on propagating data in hierarchy (for AMD 4 is good)");
  vars.addFloat ("rssv.param.bias"                       ) =       c->getf32("bias"                       ,1.f ,"shadow frusta bias"                                                              );
  vars.addUint32("rssv.param.triangleAlignment"          ) =       c->getu32("triangleAlignment"          ,128 ,"alignment of triangles"                                                          );
  vars.addUint32("rssv.param.sfAlignment"                ) =       c->getu32("sfAlignment"                ,128 ,"shadow frusta alignment"                                                         );
  vars.addUint32("rssv.param.sfWGS"                      ) =       c->getu32("sfWGS"                      ,64  ,"shadow frusta work group size"                                                   );
  vars.addBool  ("rssv.param.sfInterleave"               ) = (bool)c->geti32("sfInterleave"               ,1   ,"interleave shadow frusta floats"                                                 );
  vars.addBool  ("rssv.param.triangleInterleave"         ) = (bool)c->geti32("triangleInterleave"         ,1   ,"interleave triangle floats"                                                      );
  vars.addBool  ("rssv.param.morePlanes"                 ) = (bool)c->geti32("morePlanes"                 ,1   ,"additional frustum planes"                                                       );
  vars.addBool  ("rssv.param.ffc"                        ) = (bool)c->geti32("ffc"                        ,0   ,"active front face culling"                                                       );
  vars.addBool  ("rssv.param.noAABB"                     ) = (bool)c->geti32("noAABB"                     ,0   ,"no tight aabb"                                                                   );
  vars.addSizeT ("rssv.param.alignment"                  ) =       c->getu64("alignment"                  ,128 ,"buffer alignment in bytes"                                                       );
  vars.addUint32("rssv.param.extractSilhouettesWGS"      ) =       c->getu32("extractSilhouettesWGS"      ,64  ,"extract silhouettes work groups size"                                            );
  vars.addBool  ("rssv.param.usePadding"                 ) = (bool)c->getu32("usePadding"                 ,1   ,"increase aabb size by half of a pixel"                                           );
  vars.addBool  ("rssv.param.discardBackfacing"          ) = (bool)c->getu32("discardBackfacing"          ,1   ,"discard light backfacing fragments"                                              );
  vars.addBool  ("rssv.param.memoryOptim"                ) = (bool)c->geti32("memoryOptim"                ,1   ,"apply memory optimization"                                                       );
  vars.addUint32("rssv.param.memoryFactor"               ) =       c->getu32("memoryFactor"               ,10  ,"memory optimization - this value is average number of nodes per screen tile"     );
  vars.addBool  ("rssv.param.useBridgePool"              ) = (bool)c->geti32("useBridgePool"              ,0   ,"create buffer containing information about bridges"                              );
  vars.addBool  ("rssv.param.scaledQuantization"         ) = (bool)c->geti32("scaledQuantization"         ,1   ,"fix CPTSV quantization"                                                          );

  vars.addBool  ("rssv.param.performTraverseSilhouettes" ) = (bool)c->geti32("traverseSilhouettes"        ,1   ,"traverse silhouettes"                                                            );
  vars.addBool  ("rssv.param.exactSilhouetteAABB"        ) = (bool)c->geti32("exactSilhouetteAABB"        ,0   ,"perform exact silhouette AABB test"                                              );
  vars.addInt32 ("rssv.param.exactSilhouetteAABBLevel"   ) =       c->geti32("exactSilhouetteAABBLevel"   ,6   ,"perform exact silhouette AABB test up to level"                                  );
  vars.addBool  ("rssv.param.computeSilhouetteBridges"   ) = (bool)c->geti32("computeSilhouetteBridges"   ,1   ,"compute silhouette bridge intersections"                                         );
  vars.addBool  ("rssv.param.computeLastLevelSilhouettes") = (bool)c->geti32("computeLastLevelSilhouettes",1   ,"compute last level of silhouettes"                                               );

  vars.addBool  ("rssv.param.performTraverseTriangles"   ) = (bool)c->geti32("traverseTriangles"          ,1   ,"traverse triangles"                                                              );
  vars.addBool  ("rssv.param.exactTriangleAABB"          ) = (bool)c->geti32("exactTriangleAABB"          ,1   ,"perform exact triangle AABB test"                                                );
  vars.addInt32 ("rssv.param.exactTriangleAABBLevel"     ) =       c->geti32("exactTriangleAABBLevel"     ,2   ,"perform exact triangle AABB test up to level"                                    );
  vars.addBool  ("rssv.param.computeTriangleBridges"     ) = (bool)c->geti32("computeTriangleBridges"     ,1   ,"compute triangle bridge intersections"                                           );
  vars.addBool  ("rssv.param.computeLastLevelTriangles"  ) = (bool)c->geti32("computeLastLevelTriangles"  ,1   ,"compute last level of triangles"                                                 );

  vars.addBool  ("rssv.param.performMerge"               ) = (bool)c->geti32("performMerge"               ,1   ,"perform merge step"                                                              );
  vars.addUint32("rssv.param.persistentWG"               ) =       c->getu32("persistentWG"               ,256 ,"number of persistent work groups"                                                );


  vars.addBool  ("rssv.param.usePersistentThreadsSF"     ) = (bool)c->geti32("usePersistentThreadsSF"     ,1   ,"use persisten threads for computation of shadow frusta"                          );
  vars.addBool  ("rssv.param.computeSilhouettePlanes"    ) = (bool)c->geti32("computeSilhouettePlanes"    ,0   ,"compute collision planes during silhouette extraction"                           );
  vars.addBool  ("rssv.param.orderedSkala"               ) = (bool)c->geti32("orderedSkala"               ,1   ,"ordered clip plane construction"                                                 );
  vars.addBool  ("rssv.param.mergeInMega"                ) = (bool)c->geti32("mergeInMega"                ,0   ,"perform merge stage in mega kernel"                                              );

  //FOR DEBUG
  vars.addBool  ("rssv.param.storeTraverseSilhouettesStat"  );
  vars.addBool  ("rssv.param.storeTraverseTrianglesStat"    );
  vars.addBool  ("rssv.param.storeEdgePlanes"               );
  vars.addBool  ("rssv.param.dumpPointsNotPlanes"           );
  vars.addBool  ("rssv.param.storeBridgesInLocalMemory"     );

}
