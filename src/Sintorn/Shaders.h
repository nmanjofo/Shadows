#pragma once

#include<iostream>

const std::string writeStencilTextureCompSrc = R".(
//"methods/Sintorn/writestenciltexture.comp";
//DO NOT EDIT ANYTHING BELOW THIS LINE

#ifndef WRITESTENCILTEXTURE_BINDING_FINALSTENCILMASK
  #define WRITESTENCILTEXTURE_BINDING_FINALSTENCILMASK 0
#endif//WRITESTENCILTEXTURE_BINDING_FINALSTENCILMASK

#ifndef WRITESTENCILTEXTURE_BINDING_HSTINPUT
  #define WRITESTENCILTEXTURE_BINDING_HSTINPUT 1
#endif//WRITESTENCILTEXTURE_BINDING_HSTINPUT

#ifndef LOCAL_TILE_SIZE_X
  #define LOCAL_TILE_SIZE_X 8
#endif//LOCAL_TILE_SIZE_X

#ifndef LOCAL_TILE_SIZE_Y
  #define LOCAL_TILE_SIZE_Y 8
#endif//LOCAL_TILE_SIZE_Y

#define SHADOW_VALUE               1
#define WAVEFRONT_SIZE             (LOCAL_TILE_SIZE_X*LOCAL_TILE_SIZE_Y)
#define UINT_BIT_SIZE              32
#define RESULT_LENGTH_IN_UINT      (WAVEFRONT_SIZE/UINT_BIT_SIZE)
#define RESULT_ID                  (INVOCATION_ID_IN_WAVEFRONT/UINT_BIT_SIZE)
#define INVOCATION_ID_IN_WAVEFRONT (gl_LocalInvocationID.y*LOCAL_TILE_SIZE_X+gl_LocalInvocationID.x)

layout(local_size_x=LOCAL_TILE_SIZE_X,local_size_y=LOCAL_TILE_SIZE_Y)in;

layout(r32ui,binding=WRITESTENCILTEXTURE_BINDING_FINALSTENCILMASK)uniform uimage2D FinalStencilMask;
layout(r32ui,binding=WRITESTENCILTEXTURE_BINDING_HSTINPUT        )uniform uimage2D HSTInput;

uniform uvec2 WindowSize;

void main(){
  //are we outside of bounds of window?
  if(any(greaterThanEqual(gl_GlobalInvocationID.xy,WindowSize)))return;
  uint stencilValue=imageLoad(HSTInput,ivec2(gl_WorkGroupID.x*RESULT_LENGTH_IN_UINT+RESULT_ID,gl_WorkGroupID.y)).x;
  if(((stencilValue>>(INVOCATION_ID_IN_WAVEFRONT%UINT_BIT_SIZE))&1u)!=0u&&gl_GlobalInvocationID.y>=8*16*0)
    imageStore(FinalStencilMask,ivec2(gl_GlobalInvocationID),uvec4(SHADOW_VALUE));
}).";






const std::string clearStencilCompSrc = R".(
//"methods/Sintorn/clearstencil.comp";
#version 450 core

#define LIGHT_VALUE 0

layout(local_size_x=8,local_size_y=8)in;

layout(r32ui,binding=0)uniform uimage2D FinalStencilMask;

uniform uvec2 WindowSize;

void main(){
  if(any(greaterThanEqual(gl_GlobalInvocationID.xy,WindowSize)))return;
  imageStore(FinalStencilMask,ivec2(gl_GlobalInvocationID),ivec4(LIGHT_VALUE));
}).";

const std::string rasterizeTextureCompSrc = R".(
//"methods/Sintorn/rasterizetexture.comp";
#define USE_BALLOT

//DO NOT EDIT ANYTHING BELOW THIS COMMENT

#define JOIN__(x,y) x##y
#define JOIN_(x,y) JOIN__(x,y)
#define JOIN(x,y) JOIN_(x,y)

#ifndef NUMBER_OF_LEVELS
  #define NUMBER_OF_LEVELS 4
#endif//NUMBER_OF_LEVELS

#ifndef NUMBER_OF_LEVELS_MINUS_ONE
  #define NUMBER_OF_LEVELS_MINUS_ONE 3
#endif//NUMBER_OF_LEVELS_MINUS_ONE

#ifndef RASTERIZETEXTURE_BINDING_SHADOWFRUSTA
  #define RASTERIZETEXTURE_BINDING_SHADOWFRUSTA 0
#endif//RASTERIZETEXTURE_BINDING_SHADOWFRUSTA

#ifndef RASTERIZETEXTURE_BINDING_FINALSTENCILMASK
  #define RASTERIZETEXTURE_BINDING_FINALSTENCILMASK 0
#endif//RASTERIZETEXTURE_BINDING_FINALSTENCILMASK

#ifndef RASTERIZETEXTURE_BINDING_HST
  #define RASTERIZETEXTURE_BINDING_HST 1
#endif//RASTERIZETEXTURE_BINDING_HST

#ifndef RASTERIZETEXTURE_BINDING_HDT
  #define RASTERIZETEXTURE_BINDING_HDT 5
#endif//RASTERIZETEXTURE_BINDING_HDT

#ifndef RASTERIZETEXTURE_BINDING_TRIANGLE_ID
  #define RASTERIZETEXTURE_BINDING_TRIANGLE_ID 9
#endif//RASTERIZETEXTURE_BINDING_TRIANGLE_ID

#ifndef WAVEFRONT_SIZE
  #define WAVEFRONT_SIZE 64
#endif//WAVEFRONT_SIZE

#ifndef SHADOWFRUSTUMS_PER_WORKGROUP
  #define SHADOWFRUSTUMS_PER_WORKGROUP 1
#endif//SHADOWFRUSTUMS_PER_WORKGROUP

#define NOF_COMPONENTS_OF_VEC4   4
#define UINT_BIT_SIZE           32

#if NUMBER_OF_LEVELS > 8
  #error Too many levels
#endif

#if WAVEFRONT_SIZE % UINT_BIT_SIZE != 0
  #error WAVEFRONT_SIZE is incommensurable by\
  UINT_BIT_SIZE
#endif

#ifdef USE_BALLOT
  #if WAVEFRONT_SIZE == 64
    #extension GL_AMD_gcn_shader       : enable
    #extension GL_AMD_gpu_shader_int64 : enable
    #define BALLOT(x) ballotAMD(x)
    #define TRANSFORM_WAVEFRONT_RESULT_TO_UINTS(result) unpackUint2x32(result)
    #define UINT_RESULT_ARRAY uvec2
    #define GET_UINT_FROM_UINT_ARRAY(array,i) array[i]
  #else
    #extension GL_NV_shader_thread_group : enable
    #define BALLOT(x) ballotThreadNV(x)
    #define TRANSFORM_WAVEFRONT_RESULT_TO_UINTS(result) result
    #define UINT_RESULT_ARRAY uint
    #define GET_UINT_FROM_UINT_ARRAY(array,i) array
  #endif
#endif



#define RESULT_LENGTH_IN_UINT         (WAVEFRONT_SIZE/UINT_BIT_SIZE)

#define       VEC4_PER_SHADOWFRUSTUM  6
#define NOF_PLANES_PER_SF             4
#define     FLOATS_PER_SHADOWFRUSTUM  (VEC4_PER_SHADOWFRUSTUM*NOF_COMPONENTS_OF_VEC4)

#define     WORKGROUP_ID_IN_DISPATCH  (uint(gl_WorkGroupID.x + triangleOffset))
#define    INVOCATION_ID_IN_WAVEFRONT (uint(gl_LocalInvocationID.x))
#define SHADOWFRUSTUM_ID_IN_WORKGROUP (uint(gl_LocalInvocationID.y))
#define SHADOWFRUSTUM_ID_IN_DISPATCH  (WORKGROUP_ID_IN_DISPATCH*SHADOWFRUSTUMS_PER_WORKGROUP + SHADOWFRUSTUM_ID_IN_WORKGROUP)

#define TRIVIAL_ACCEPT                0x00000004u
#define TRIVIAL_REJECT                0x00000002u
#define INTERSECTS                    0x00000001u
#define SHADOW_VALUE                  1u

#define MAX_DEPTH                     1.f

#define LEFT_DOWN_CORNER_COORD        uvec2( 0)
#define LEFT_DOWN_CORNER_CLIP_COORD    vec2(-1)

#ifdef  USE_UNIFORM_TILE_DIVISIBILITY
  #if NUMBER_OF_LEVELS > 7
    #define TILE_DIVISIBILITY7 TileDivisibility[7]
  #endif
  #if NUMBER_OF_LEVELS > 6
    #define TILE_DIVISIBILITY6 TileDivisibility[6]
  #endif
  #if NUMBER_OF_LEVELS > 5
    #define TILE_DIVISIBILITY5 TileDivisibility[5]
  #endif
  #if NUMBER_OF_LEVELS > 4
    #define TILE_DIVISIBILITY4 TileDivisibility[4]
  #endif
  #if NUMBER_OF_LEVELS > 3
    #define TILE_DIVISIBILITY3 TileDivisibility[3]
  #endif
  #if NUMBER_OF_LEVELS > 2
    #define TILE_DIVISIBILITY2 TileDivisibility[2]
  #endif
  #if NUMBER_OF_LEVELS > 1
    #define TILE_DIVISIBILITY1 TileDivisibility[1]
  #endif
  #if NUMBER_OF_LEVELS > 0
    #define TILE_DIVISIBILITY0 TileDivisibility[0]
  #endif
#endif//USE_UNIFORM_TILE_DIVISIBILITY

#ifdef  USE_UNIFORM_TILE_SIZE_IN_CLIP_SPACE
  #if NUMBER_OF_LEVELS > 7
    #define TILE_SIZE_IN_CLIP_SPACE7 TileSizeInClipSpace[7]
  #endif
  #if NUMBER_OF_LEVELS > 6
    #define TILE_SIZE_IN_CLIP_SPACE6 TileSizeInClipSpace[6]
  #endif
  #if NUMBER_OF_LEVELS > 5
    #define TILE_SIZE_IN_CLIP_SPACE5 TileSizeInClipSpace[5]
  #endif
  #if NUMBER_OF_LEVELS > 4
    #define TILE_SIZE_IN_CLIP_SPACE4 TileSizeInClipSpace[4]
  #endif
  #if NUMBER_OF_LEVELS > 3
    #define TILE_SIZE_IN_CLIP_SPACE3 TileSizeInClipSpace[3]
  #endif
  #if NUMBER_OF_LEVELS > 2
    #define TILE_SIZE_IN_CLIP_SPACE2 TileSizeInClipSpace[2]
  #endif
  #if NUMBER_OF_LEVELS > 1
    #define TILE_SIZE_IN_CLIP_SPACE1 TileSizeInClipSpace[1]
  #endif
  #if NUMBER_OF_LEVELS > 0
    #define TILE_SIZE_IN_CLIP_SPACE0 TileSizeInClipSpace[0]
  #endif
#endif//USE_UNIFORM_TILE_SIZE_IN_CLIP_SPACE

layout(local_size_x=WAVEFRONT_SIZE,local_size_y=SHADOWFRUSTUMS_PER_WORKGROUP)in;

layout(r32ui ,binding=RASTERIZETEXTURE_BINDING_FINALSTENCILMASK)writeonly uniform   uimage2D FinalStencilMask;
layout(r32ui ,binding=RASTERIZETEXTURE_BINDING_HST             )          uniform   uimage2D HST[NUMBER_OF_LEVELS];
layout(       binding=RASTERIZETEXTURE_BINDING_HDT             )          uniform  sampler2D HDT[NUMBER_OF_LEVELS];
layout(       binding=RASTERIZETEXTURE_BINDING_TRIANGLE_ID     )          uniform usampler2D triangleID;
layout(std430,binding=RASTERIZETEXTURE_BINDING_SHADOWFRUSTA    ) readonly buffer SFData{float ShadowFrusta[];};

shared vec4 SharedShadowFrusta[SHADOWFRUSTUMS_PER_WORKGROUP][VEC4_PER_SHADOWFRUSTUM];

/*
  Tile size in different units.
  TileDivisibility returns number of subtiles.
  TileSizeInClipSpace returns x,y size of AABB in clip-space
*/

#ifdef  USE_UNIFORM_TILE_DIVISIBILITY
uniform uvec2 TileDivisibility    [NUMBER_OF_LEVELS];
#endif//USE_UNIFORM_TILE_DIVISIBILITY

#ifdef  USE_UNIFORM_TILE_SIZE_IN_CLIP_SPACE
uniform  vec2 TileSizeInClipSpace [NUMBER_OF_LEVELS];
#endif//USE_UNIFORM_TILE_SIZE_IN_CLIP_SPACE

uniform uint  NumberOfTriangles;
uniform uint  triangleOffset = 0;

vec2 TrivialRejectCorner2D(vec2 Normal){
  return vec2((ivec2(sign(Normal))+1)/2);
}

bool SampleInsideEdge(vec2 Sample,vec3 Edge){
  return dot(vec3(Sample,1),Edge)<=0;
}

vec3 TrivialRejectCorner3D(vec3 Normal){
  return vec3((ivec3(sign(Normal))+1)/2);
}

uint TrivialRejectAcceptAABB(vec3 TileCorner,vec3 TileSize){
  if(TileCorner.z>=MAX_DEPTH)return TRIVIAL_REJECT;//no direct lighting
  bool accept = true;
  for(int i=0;i<NOF_PLANES_PER_SF;++i){//loop over planes
    vec3 tr=TrivialRejectCorner3D(SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][i].xyz);
    if(dot(vec4(TileCorner+tr*TileSize,1),SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][i])<0)
      return TRIVIAL_REJECT;//trivial reject corner is behind a plane
    tr=1-tr;//compute trivial accept
    accept = accept && (dot(vec4(TileCorner+tr*TileSize,1),SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][i])>0);
  }
  if(accept)return TRIVIAL_ACCEPT;
  return INTERSECTS;
}

#define getLocalCoords(LEVEL)\
  uvec2(INVOCATION_ID_IN_WAVEFRONT % JOIN(TILE_DIVISIBILITY,LEVEL).x , INVOCATION_ID_IN_WAVEFRONT/JOIN(TILE_DIVISIBILITY,LEVEL).x)

#define TEST_SHADOW_FRUSTUM_LAST(LEVEL)\
void JOIN(TestShadowFrustumHDB,LEVEL)(uvec2 coord,vec2 clipCoord){\
  uvec2 localCoord             = getLocalCoords(LEVEL);\
  uvec2 globalCoord            = coord * JOIN(TILE_DIVISIBILITY,LEVEL) + localCoord;\
  vec2  globalClipCoord        = clipCoord + JOIN(TILE_SIZE_IN_CLIP_SPACE,LEVEL) * localCoord;\
  if(texelFetch(triangleID,ivec2(globalCoord),0).x == SHADOWFRUSTUM_ID_IN_DISPATCH)return;\
  vec4  SampleCoordInClipSpace = vec4(\
    globalClipCoord + JOIN(TILE_SIZE_IN_CLIP_SPACE,LEVEL)*.5,\
    texelFetch(HDT[LEVEL],ivec2(globalCoord),0).x,1);\
  if(SampleCoordInClipSpace.z >= 1)return;\
  bool inside = true;\
  for(int p = 0; p < NOF_PLANES_PER_SF; ++p)\
    inside=inside && dot(SampleCoordInClipSpace,SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][p])>=0;\
  if(inside)\
    imageStore(FinalStencilMask,ivec2(globalCoord),uvec4(SHADOW_VALUE));\
}

#ifdef USE_BALLOT
  #define TEST_SHADOW_FRUSTUM(LEVEL,NEXT_LEVEL)\
  void JOIN(TestShadowFrustumHDB,LEVEL)(\
      uvec2    coord,\
      vec2 ClipCoord){\
    uvec2 localCoord      = getLocalCoords(LEVEL);\
    uvec2 globalCoord     = coord * TILE_DIVISIBILITY ## LEVEL + localCoord;\
    vec2  globalClipCoord = ClipCoord + TILE_SIZE_IN_CLIP_SPACE ## LEVEL * localCoord;\
    vec2  ZMinMax         = texelFetch(HDT[LEVEL],ivec2(globalCoord),0).xy;\
    vec3  AABBCorner      = vec3(globalClipCoord,ZMinMax.x);\
    vec3  AABBSize        = vec3(TILE_SIZE_IN_CLIP_SPACE ## LEVEL,ZMinMax.y-ZMinMax.x);\
    uint  Result          = TrivialRejectAcceptAABB(AABBCorner,AABBSize);\
    if(Result==INTERSECTS){\
      for(uint i = 0; i < uint(SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF].w); ++i){\
        vec2 tr = globalClipCoord+TrivialRejectCorner2D(SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF+i].xy)*TILE_SIZE_IN_CLIP_SPACE ## LEVEL;\
        if(SampleInsideEdge(tr,SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF+i].xyz))Result=TRIVIAL_REJECT;\
      }\
    }\
    UINT_RESULT_ARRAY AcceptBallot     = TRANSFORM_WAVEFRONT_RESULT_TO_UINTS(BALLOT(Result==TRIVIAL_ACCEPT));\
    UINT_RESULT_ARRAY IntersectsBallot = TRANSFORM_WAVEFRONT_RESULT_TO_UINTS(BALLOT(Result==INTERSECTS    ));\
    if(INVOCATION_ID_IN_WAVEFRONT<RESULT_LENGTH_IN_UINT){\
      ivec2 hstGlobalCoord=ivec2(coord.x*RESULT_LENGTH_IN_UINT+INVOCATION_ID_IN_WAVEFRONT,coord.y);\
      imageAtomicOr(HST[LEVEL],hstGlobalCoord,GET_UINT_FROM_UINT_ARRAY(AcceptBallot,INVOCATION_ID_IN_WAVEFRONT));\
    }\
    \
    for(int i=0;i<RESULT_LENGTH_IN_UINT;++i){\
      uint UnresolvedIntersects = GET_UINT_FROM_UINT_ARRAY(IntersectsBallot,i);\
      while(UnresolvedIntersects!=0u){\
        int   CurrentBit         = findMSB(UnresolvedIntersects);\
        int   CurrentTile        = CurrentBit+i*UINT_BIT_SIZE;\
        uvec2 CurrentLocalCoord  = uvec2(CurrentTile%TILE_DIVISIBILITY ## LEVEL.x,CurrentTile/TILE_DIVISIBILITY ## LEVEL.x);\
        uvec2 CurrentGlobalCoord = coord * TILE_DIVISIBILITY ## LEVEL + CurrentLocalCoord;\
        vec2  CurrentClipCoord   = ClipCoord+TILE_SIZE_IN_CLIP_SPACE ## LEVEL*CurrentLocalCoord;\
        JOIN(TestShadowFrustumHDB,NEXT_LEVEL) (CurrentGlobalCoord,CurrentClipCoord);\
        UnresolvedIntersects    &= ~(1u<<CurrentBit);\
      }\
    }\
  }
#else//USE_BALLOT
  #define NUM_ITERATION (int(ceil(log(UINT_BIT_SIZE)/log(2))))
  shared uint ResAcc2 [SHADOWFRUSTUMS_PER_WORKGROUP][WAVEFRONT_SIZE];
  shared uint ResInt2 [SHADOWFRUSTUMS_PER_WORKGROUP][WAVEFRONT_SIZE];
  #define TEST_SHADOW_FRUSTUM(LEVEL,NEXT_LEVEL)\
  shared uint JOIN(ResIntersects,LEVEL) [SHADOWFRUSTUMS_PER_WORKGROUP][RESULT_LENGTH_IN_UINT];\
  void JOIN(TestShadowFrustumHDB,LEVEL)(\
      uvec2    Coord,\
      vec2 ClipCoord){\
    if(INVOCATION_ID_IN_WAVEFRONT<RESULT_LENGTH_IN_UINT)\
      ResIntersects ## LEVEL [SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT]=0u;\
    barrier();\
    uvec2 LocalCoord      = getLocalCoords(LEVEL);\
    uvec2 GlobalCoord     = Coord*TILE_DIVISIBILITY ## LEVEL+LocalCoord;\
    vec2  GlobalClipCoord = ClipCoord+TILE_SIZE_IN_CLIP_SPACE ## LEVEL * LocalCoord;\
    vec2  ZMinMax         = texelFetch(HDT[LEVEL],ivec2(GlobalCoord),0).xy;\
    vec3  AABBCorner      = vec3(GlobalClipCoord,ZMinMax.x);\
    vec3  AABBSize        = vec3(TILE_SIZE_IN_CLIP_SPACE ## LEVEL,ZMinMax.y-ZMinMax.x);\
    uint  Result          = TrivialRejectAcceptAABB(AABBCorner,AABBSize);\
    if(Result==INTERSECTS){\
      for(uint i=0;i<uint(SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF].w);++i){\
        vec2 tr=GlobalClipCoord+TrivialRejectCorner2D(SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF+i].xy)*TILE_SIZE_IN_CLIP_SPACE ## LEVEL;\
        if(SampleInsideEdge(tr,SharedShadowFrusta[SHADOWFRUSTUM_ID_IN_WORKGROUP][NOF_PLANES_PER_SF+i].xyz))Result=TRIVIAL_REJECT;\
      }\
    }\
    ResAcc2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT]=uint(Result==TRIVIAL_ACCEPT)<<(INVOCATION_ID_IN_WAVEFRONT % UINT_BIT_SIZE);\
    ResInt2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT]=uint(Result==INTERSECTS    )<<(INVOCATION_ID_IN_WAVEFRONT % UINT_BIT_SIZE);\
    for(int l=0;l<NUM_ITERATION;++l){\
      if(INVOCATION_ID_IN_WAVEFRONT<(WAVEFRONT_SIZE>>(l+1))){\
        ResAcc2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT<<(l+1)]|=ResAcc2[SHADOWFRUSTUM_ID_IN_WORKGROUP][(INVOCATION_ID_IN_WAVEFRONT<<(l+1))+(1<<l)];\
        ResInt2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT<<(l+1)]|=ResInt2[SHADOWFRUSTUM_ID_IN_WORKGROUP][(INVOCATION_ID_IN_WAVEFRONT<<(l+1))+(1<<l)];\
      }\
    }\
    if(INVOCATION_ID_IN_WAVEFRONT<RESULT_LENGTH_IN_UINT){\
      ivec2 hstGlobalCoord=ivec2(Coord.x*RESULT_LENGTH_IN_UINT+INVOCATION_ID_IN_WAVEFRONT,Coord.y);\
      imageAtomicOr(HST[LEVEL],hstGlobalCoord,                                           ResAcc2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT * UINT_BIT_SIZE]);\
      ResIntersects ## LEVEL [SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT]=ResInt2[SHADOWFRUSTUM_ID_IN_WORKGROUP][INVOCATION_ID_IN_WAVEFRONT * UINT_BIT_SIZE];\
    }\
    for(int i=0;i<RESULT_LENGTH_IN_UINT;++i){\
      uint UnresolvedIntersects=ResIntersects##LEVEL[SHADOWFRUSTUM_ID_IN_WORKGROUP][i];\
      while(UnresolvedIntersects!=0u){\
        int   CurrentBit         = findMSB(UnresolvedIntersects);\
        int   CurrentTile        = CurrentBit+i*UINT_BIT_SIZE;\
        uvec2 CurrentLocalCoord  = uvec2(CurrentTile%TILE_DIVISIBILITY ## LEVEL.x,CurrentTile/TILE_DIVISIBILITY ## LEVEL.x);\
        uvec2 CurrentGlobalCoord = Coord*TILE_DIVISIBILITY ## LEVEL+CurrentLocalCoord;\
        vec2  CurrentClipCoord   = ClipCoord+TILE_SIZE_IN_CLIP_SPACE ## LEVEL*CurrentLocalCoord;\
        JOIN(TestShadowFrustumHDB,NEXT_LEVEL) (CurrentGlobalCoord,CurrentClipCoord);\
        UnresolvedIntersects    &= ~(1u<<CurrentBit);\
      }\
    }\
  }
#endif//USE_BALLOT

TEST_SHADOW_FRUSTUM_LAST(NUMBER_OF_LEVELS_MINUS_ONE)
//TEST_SHADOW_FRUSTUM_LAST(3)
//TEST_SHADOW_FRUSTUM(2,3)
//TEST_SHADOW_FRUSTUM(1,2)
//TEST_SHADOW_FRUSTUM(0,1)

#if NUMBER_OF_LEVELS > 7
TEST_SHADOW_FRUSTUM(6,7)
#endif

#if NUMBER_OF_LEVELS > 6
TEST_SHADOW_FRUSTUM(5,6)
#endif

#if NUMBER_OF_LEVELS > 5
TEST_SHADOW_FRUSTUM(4,5)
#endif

#if NUMBER_OF_LEVELS > 4
TEST_SHADOW_FRUSTUM(3,4)
#endif

#if NUMBER_OF_LEVELS > 3
TEST_SHADOW_FRUSTUM(2,3)
#endif

#if NUMBER_OF_LEVELS > 2
TEST_SHADOW_FRUSTUM(1,2)
#endif

#if NUMBER_OF_LEVELS > 1
TEST_SHADOW_FRUSTUM(0,1)
#endif


void main(){
  if(SHADOWFRUSTUM_ID_IN_DISPATCH>=NumberOfTriangles)return;
  if(INVOCATION_ID_IN_WAVEFRONT<FLOATS_PER_SHADOWFRUSTUM){
    SharedShadowFrusta
      [SHADOWFRUSTUM_ID_IN_WORKGROUP]
      [INVOCATION_ID_IN_WAVEFRONT/NOF_COMPONENTS_OF_VEC4]
      [INVOCATION_ID_IN_WAVEFRONT%NOF_COMPONENTS_OF_VEC4]=
        ShadowFrusta
          [SHADOWFRUSTUM_ID_IN_DISPATCH*FLOATS_PER_SHADOWFRUSTUM
            +INVOCATION_ID_IN_WAVEFRONT];
  }
  TestShadowFrustumHDB0(
    LEFT_DOWN_CORNER_COORD,
    LEFT_DOWN_CORNER_CLIP_COORD);
}).";




const std::string blitCompSrc = R".(
#version 450 core
layout(local_size_x=8,local_size_y=8)in;

uniform uvec2 windowSize = uvec2(512,512);

layout(r32ui,binding=0)readonly  uniform uimage2D finalStencilMask;
layout(r32f ,binding=1)writeonly uniform  image2D shadowMask      ;

void main(){
  if(any(greaterThanEqual(gl_GlobalInvocationID.xy,windowSize)))return;
  uint S=imageLoad(finalStencilMask,ivec2(gl_GlobalInvocationID.xy)).x;
  imageStore(shadowMask,ivec2(gl_GlobalInvocationID.xy),vec4(1-S));
}
).";


const std::string drawHSTVertSrc = R".(
#version 450 core
void main(){
  gl_Position = vec4(-1+2*(gl_VertexID%2),-1+2*(gl_VertexID/2),0,1);
}
).";

const std::string drawHSTFragSrc = R".(
#version 450 core
#define UINT_BIT_SIZE 32

#ifndef WAVEFRONT_SIZE
#define WAVEFRONT_SIZE 64
#endif//WAVEFRONT_SIZE

#define RESULT_LENGTH_IN_UINT         (WAVEFRONT_SIZE/UINT_BIT_SIZE)

layout(location=0)out vec4 fColor;
layout(r32ui,binding=0)readonly uniform uimage2D HST;

uvec2 Coord=uvec2(gl_FragCoord.xy);

uniform uvec2 windowSize = uvec2(512,512);

void main(){
  Coord = uvec2(vec2(Coord)*imageSize(HST)*vec2(0.5,1)/windowSize);
  uvec2 cc=(Coord)%uvec2(8);
  uint invocationId=cc.y*8+cc.x;
  uint stencilValue=imageLoad(HST,ivec2((Coord.x/8)*RESULT_LENGTH_IN_UINT+(invocationId/UINT_BIT_SIZE),(Coord.y/8))).x;
  uint shadow=(stencilValue>>(invocationId%UINT_BIT_SIZE))&1u;
  fColor=vec4(1-shadow,1,0,1);
  return;
}
).";

const std::string drawFinalStencilMaskFragSrc = R".(
#version 450 core
#define UINT_BIT_SIZE 32

#ifndef WAVEFRONT_SIZE
#define WAVEFRONT_SIZE 64
#endif//WAVEFRONT_SIZE

#define RESULT_LENGTH_IN_UINT         (WAVEFRONT_SIZE/UINT_BIT_SIZE)

layout(location=0)out vec4 fColor;
layout(r32ui,binding=0)readonly uniform uimage2D FinalStencilMask;

uvec2 Coord=uvec2(gl_FragCoord.xy);

void main(){
  fColor=vec4(1,0,0,1);
  uint S=imageLoad(FinalStencilMask,ivec2(Coord)).x;
  fColor=vec4(1-S,1,0,1);
}
).";
