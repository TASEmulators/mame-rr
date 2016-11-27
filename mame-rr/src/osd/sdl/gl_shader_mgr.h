
#ifndef GL_SHADER_MGR_H
#define GL_SHADER_MGR_H

// #define GLSL_SOURCE_ON_DISK 1

typedef enum {
	GLSL_SHADER_TYPE_IDX16,
	GLSL_SHADER_TYPE_RGB32_LUT,
	GLSL_SHADER_TYPE_RGB32_DIRECT,
	GLSL_SHADER_TYPE_NUMBER
} GLSL_SHADER_TYPE;

typedef enum {
	GLSL_SHADER_FEAT_PLAIN,
	GLSL_SHADER_FEAT_BILINEAR,
	GLSL_SHADER_FEAT_INT_NUMBER,
	GLSL_SHADER_FEAT_CUSTOM = GLSL_SHADER_FEAT_INT_NUMBER,
	GLSL_SHADER_FEAT_MAX_NUMBER,
} GLSL_SHADER_FEATURE;

// old code passed sdl_info * to functions here
// however the parameter was not used
// changed interface to more generic one.
typedef struct _glsl_shader_info glsl_shader_info;
struct _glsl_shader_info
{
	int dummy; // avoid compiler breakage
};


/**
 * returns pointer if ok, otherwise NULL
 */
glsl_shader_info *glsl_shader_init(void);

/**
 * returns 0 if ok, otherwise an error value
 */
int glsl_shader_free(glsl_shader_info *shinfo);

/**
 * returns the GLSL program if ok/available, otherwise 0
 */
GLhandleARB glsl_shader_get_program_mamebm(int glslShaderType, int glslShaderFeature, int idx);

const char * glsl_shader_get_filter_name_mamebm(int glslShaderFeature);

int glsl_shader_add_mamebm(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx);

GLhandleARB glsl_shader_get_program_scrn(int idx);
int glsl_shader_add_scrn(glsl_shader_info *shinfo, const char * custShaderPrefix, int idx);

#endif // GL_SHADER_MGR_H
