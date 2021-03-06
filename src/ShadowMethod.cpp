#include <ShadowMethod.h>
#include <Vars/Vars.h>
#include <TimeStamp.h>
#include <cstring>

ShadowMethod::ShadowMethod(vars::Vars&vars):vars(vars){}

/**
 * @brief determine if vertex a is greater than vertex b
 *
 * @param a vertex a
 * @param b vertex b
 *
 * @return return true if a>b
 */
bool greaterVec(glm::vec3 const&a,glm::vec3 const&b){
  return dot(sign(a-b),glm::vec3(4.0f,2.0f,1.0f))>0.f;
}

glm::vec3 toVec3(float const*ptr){assert(ptr!=nullptr);return glm::vec3(ptr[0],ptr[1],ptr[2]);}

/**
 * @brief Compute plane deterministically
 * it sorts vertices so A < B < C and compute 
 * plane using:
 * normal = normalize( (B - A) x (C - A) )
 * plane = (normal, - normal * A)
 * return plane
 *
 * @param A vertex A of triangle
 * @param B vertex B of triangle
 * @param C vertex C of triangle
 *
 * @return plane that is formed using A,B,C
 */
glm::vec4 computePlane(glm::vec3 A,glm::vec3 B,glm::vec3 C){
  bool swapped = false;
  if(greaterVec(A,B)){
    swapped = !swapped;
    swapValues(A,B);
  }
  if(greaterVec(B,C)){
    swapped = !swapped;
    swapValues(B,C);
  }
  if(greaterVec(A,B)){
    swapped = !swapped;
    swapValues(A,B);
  }
  auto n = glm::normalize(glm::cross(B-A,C-A));
  auto p = glm::vec4(n,-glm::dot(n,A));
  if(swapped)return -p;
  return p;
}

void ShadowMethod::ifExistStamp(std::string const&n){
  if(vars.has("timeStamp"))vars.get<TimeStamp>("timeStamp")->stamp(n);
}

void ShadowMethod::drawUser(glm::vec4 const& ,
	glm::mat4 const& ,
	glm::mat4 const& )
{

}

void ShadowMethod::drawDebug(glm::vec4 const& ,
	glm::mat4 const& ,
	glm::mat4 const& )
{

}

bool ShadowMethod::IsConservativeRasterizationSupported() const
{
	int NumberOfExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
	for (int i = 0; i < NumberOfExtensions; i++)
	{
		const char* ccc = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));

		if (std::strcmp(ccc, "GL_NV_conservative_raster") == 0)
		{
			return true;
		}
	}

	return false;
}
