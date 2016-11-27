/***
 *
 * GL Shader Tool - Convinient Basic Shader functionality
 *
 * Copyright (c) 2007, Sven Gothel
 * Copyright (c) 2007, MAME Team
 *
 * Dual license: MAME and GPL version 2
 *
 * This module has a dual license, i.e. it is compatible to MAME,
 * but I like to reserve my rights to use this module under
 * the GPL as well.
 *
 * GPL license:
 *      Visit http://www.gnu.org/copyleft/gpl.html
 *
 * MAME license:
 *  Visit http://mamedev.org for licensing and usage restrictions.
 *
 *
 * OpenGL GLSL ARB extentions:
 *
 *  GL_ARB_shader_objects
 *  GL_ARB_shading_language_100
 *  GL_ARB_vertex_shader
 *  GL_ARB_fragment_shader
 *
 */

#include "emu.h"
#include "gl_shader_tool.h"

PFNGLGETOBJECTPARAMETERIVARBPROC pfn_glGetObjectParameterivARB=NULL;
PFNGLGETINFOLOGARBPROC pfn_glGetInfoLogARB=NULL;
PFNGLDELETEOBJECTARBPROC pfn_glDeleteObjectARB=NULL;
PFNGLCREATESHADEROBJECTARBPROC pfn_glCreateShaderObjectARB=NULL;
PFNGLSHADERSOURCEARBPROC pfn_glShaderSourceARB=NULL;
PFNGLCOMPILESHADERARBPROC pfn_glCompileShaderARB=NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC pfn_glCreateProgramObjectARB=NULL;
PFNGLATTACHOBJECTARBPROC pfn_glAttachObjectARB=NULL;
PFNGLLINKPROGRAMARBPROC pfn_glLinkProgramARB=NULL;
PFNGLVALIDATEPROGRAMARBPROC pfn_glValidateProgramARB=NULL;
PFNGLUSEPROGRAMOBJECTARBPROC pfn_glUseProgramObjectARB=NULL;
PFNGLGETUNIFORMLOCATIONARBPROC pfn_glGetUniformLocationARB=NULL;
PFNGLUNIFORM1FARBPROC pfn_glUniform1fARB=NULL;
PFNGLUNIFORM1IARBPROC pfn_glUniform1iARB=NULL;
PFNGLUNIFORM1FVARBPROC pfn_glUniform1fvARB=NULL;
PFNGLUNIFORM2FVARBPROC pfn_glUniform2fvARB=NULL;
PFNGLUNIFORM3FVARBPROC pfn_glUniform3fvARB=NULL;
PFNGLUNIFORM4FVARBPROC pfn_glUniform4fvARB=NULL;
PFNGLUNIFORM1IVARBPROC pfn_glUniform1ivARB=NULL;
PFNGLUNIFORM2IVARBPROC pfn_glUniform2ivARB=NULL;
PFNGLUNIFORM3IVARBPROC pfn_glUniform3ivARB=NULL;
PFNGLUNIFORM4IVARBPROC pfn_glUniform4ivARB=NULL;


int gl_shader_loadExtention(PFNGLGETPROCADDRESSOS GetProcAddress)
{
	pfn_glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) GetProcAddress("glGetObjectParameterivARB");
	pfn_glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC) GetProcAddress ("glGetInfoLogARB");
	pfn_glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC) GetProcAddress ("glDeleteObjectARB");
	pfn_glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC) GetProcAddress ("glCreateShaderObjectARB");
	pfn_glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC) GetProcAddress ("glShaderSourceARB");
	pfn_glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC) GetProcAddress ("glCompileShaderARB");
	pfn_glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC) GetProcAddress ("glCreateProgramObjectARB");
	pfn_glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC) GetProcAddress ("glAttachObjectARB");
	pfn_glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC) GetProcAddress ("glLinkProgramARB");
	pfn_glValidateProgramARB      = (PFNGLVALIDATEPROGRAMARBPROC) GetProcAddress ("glValidateProgramARB");
	pfn_glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC) GetProcAddress ("glUseProgramObjectARB");
	pfn_glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC) GetProcAddress ("glGetUniformLocationARB");
	pfn_glUniform1fARB            = (PFNGLUNIFORM1FARBPROC) GetProcAddress ("glUniform1fARB");
	pfn_glUniform1iARB            = (PFNGLUNIFORM1IARBPROC) GetProcAddress ("glUniform1iARB");
	pfn_glUniform1fvARB           = (PFNGLUNIFORM1FVARBPROC) GetProcAddress ("glUniform1fvARB");
	pfn_glUniform2fvARB           = (PFNGLUNIFORM2FVARBPROC) GetProcAddress ("glUniform2fvARB");
	pfn_glUniform3fvARB           = (PFNGLUNIFORM3FVARBPROC) GetProcAddress ("glUniform3fvARB");
	pfn_glUniform4fvARB           = (PFNGLUNIFORM4FVARBPROC) GetProcAddress ("glUniform4fvARB");
	pfn_glUniform1ivARB           = (PFNGLUNIFORM1IVARBPROC) GetProcAddress ("glUniform1ivARB");
	pfn_glUniform2ivARB           = (PFNGLUNIFORM2IVARBPROC) GetProcAddress ("glUniform2ivARB");
	pfn_glUniform3ivARB           = (PFNGLUNIFORM3IVARBPROC) GetProcAddress ("glUniform3ivARB");
	pfn_glUniform4ivARB           = (PFNGLUNIFORM4IVARBPROC) GetProcAddress ("glUniform4ivARB");

	if ( pfn_glGetObjectParameterivARB && pfn_glGetInfoLogARB && pfn_glDeleteObjectARB && pfn_glCreateShaderObjectARB &&
	     pfn_glShaderSourceARB && pfn_glCompileShaderARB && pfn_glCreateProgramObjectARB && pfn_glAttachObjectARB &&
	     pfn_glLinkProgramARB && pfn_glValidateProgramARB && pfn_glUseProgramObjectARB && pfn_glGetUniformLocationARB &&
	     pfn_glGetUniformLocationARB && pfn_glUniform1fARB && pfn_glUniform1iARB &&
	     pfn_glUniform1fvARB && pfn_glUniform2fvARB && pfn_glUniform3fvARB && pfn_glUniform4fvARB &&
	     pfn_glUniform1ivARB && pfn_glUniform2ivARB && pfn_glUniform3ivARB && pfn_glUniform4ivARB
	   )
	{
		return 0;
	}

	mame_printf_error("OpenGL: missing ARB shader function: ");
	if (!pfn_glGetObjectParameterivARB) mame_printf_error("glGetObjectParameterivARB, ");
	if (!pfn_glGetInfoLogARB) mame_printf_error("glGetInfoLogARB, ");
	if (!pfn_glDeleteObjectARB) mame_printf_error("glDeleteObjectARB, ");
	if (!pfn_glCreateShaderObjectARB) mame_printf_error("glCreateShaderObjectARB, ");
	if (!pfn_glShaderSourceARB) mame_printf_error("glShaderSourceARB, ");
	if (!pfn_glCompileShaderARB) mame_printf_error("glCompileShaderARB, ");
	if (!pfn_glCreateProgramObjectARB) mame_printf_error("glCreateProgramObjectARB, ");
	if (!pfn_glAttachObjectARB) mame_printf_error("glAttachObjectARB, ");
	if (!pfn_glLinkProgramARB) mame_printf_error("glLinkProgramARB, ");
	if (!pfn_glValidateProgramARB) mame_printf_error("glValidateProgramARB, ");
	if (!pfn_glUseProgramObjectARB) mame_printf_error("glUseProgramObjectARB, ");
	if (!pfn_glGetUniformLocationARB) mame_printf_error("glGetUniformLocationARB, ");
	if (!pfn_glUniform1fARB) mame_printf_error("glUniform1fARB, ");
	if (!pfn_glUniform1iARB) mame_printf_error("glUniform1iARB");
	if (!pfn_glUniform1fvARB) mame_printf_error("glUniform1fvARB, ");
	if (!pfn_glUniform2fvARB) mame_printf_error("glUniform2fvARB, ");
	if (!pfn_glUniform3fvARB) mame_printf_error("glUniform3fvARB, ");
	if (!pfn_glUniform4fvARB) mame_printf_error("glUniform4fvARB, ");
	if (!pfn_glUniform1ivARB) mame_printf_error("glUniform1ivARB");
	if (!pfn_glUniform2ivARB) mame_printf_error("glUniform2ivARB");
	if (!pfn_glUniform3ivARB) mame_printf_error("glUniform3ivARB");
	if (!pfn_glUniform4ivARB) mame_printf_error("glUniform4ivARB");
	mame_printf_error("\n");

	return -1;
}

int gl_check_error(GLSLCheckMode m, const char *file, const int line)
{
        GLenum glerr = glGetError();

        if (GL_NO_ERROR != glerr)
        {
                if ( CHECK_VERBOSE <= m )
                {
			mame_printf_warning( "%s:%d: GL Error: %d 0x%X\n", file, line, (int)glerr, (unsigned int)glerr);
                }
        }
	return (GL_NO_ERROR != glerr)? glerr : 0;
}

int gl_shader_check_error(GLhandleARB obj, GLenum obj_query, GLSLCheckMode m, const char *file, const int line)
{
        GLsizei length;
        GLcharARB buffer[255];
        GLenum glerr;
	GLint  param;
	int    res=0;

        glerr = gl_check_error(m, file, line);

	if(!obj)
		return glerr;

	if(obj_query != GL_OBJECT_TYPE_ARB            &&
	   obj_query != GL_OBJECT_DELETE_STATUS_ARB   &&
	   obj_query != GL_OBJECT_COMPILE_STATUS_ARB  &&
	   obj_query != GL_OBJECT_LINK_STATUS_ARB     &&
	   obj_query != GL_OBJECT_VALIDATE_STATUS_ARB
	  )
	{
		mame_printf_warning("%s:%d: GL Error: gl_shader_check_error unsupported object query 0x%X\n", file, line, (unsigned int)obj_query);
		return -1;
	}

	pfn_glGetObjectParameterivARB(obj, obj_query, &param);

	switch(obj_query)
	{
	  case GL_OBJECT_TYPE_ARB:
		if( param!=GL_PROGRAM_OBJECT_ARB && param!=GL_SHADER_OBJECT_ARB )
		{
			if ( CHECK_VERBOSE <= m )
				mame_printf_warning("%s:%d: GL Error: object type 0x%X generation failed\n", file, line, (unsigned int)obj);
			res=-1;
		} else if ( CHECK_ALWAYS_VERBOSE <= m )
		{
			if(param==GL_PROGRAM_OBJECT_ARB)
				mame_printf_warning("%s:%d: GL Error: object type 0x%X is PROGRAM, successful\n", file, line, (unsigned int)obj);
			else
				mame_printf_warning("%s:%d: GL Info: object type 0x%X is SHADER, successful\n", file, line, (unsigned int)obj);
		}
		break;
	  case GL_OBJECT_DELETE_STATUS_ARB:
		if(param!=1)
		{
			if ( CHECK_ALWAYS_VERBOSE <= m )
				mame_printf_warning("%s:%d: GL Info: object 0x%X not yet marked for deletion\n", file, line, (unsigned int)obj);
		} else if ( CHECK_ALWAYS_VERBOSE <= m )
		{
			mame_printf_warning("%s:%d: GL Info: object 0x%X deletion successful\n", file, line, (unsigned int)obj);
		}
		break;
	  case GL_OBJECT_COMPILE_STATUS_ARB:
		if(param!=1)
		{
			if ( CHECK_VERBOSE <= m )
				mame_printf_warning("%s:%d: GL Error: object 0x%X compilation failed\n", file, line, (unsigned int)obj);
			res=-1;
		} else if ( CHECK_ALWAYS_VERBOSE <= m )
		{
			mame_printf_warning("%s:%d: GL Info: object 0x%X compiled successful\n", file, line, (unsigned int)obj);
		}
		break;
	  case GL_OBJECT_LINK_STATUS_ARB:
		if(param!=1)
		{
			if ( CHECK_VERBOSE <= m )
				mame_printf_warning("%s:%d: GL Error: object 0x%X linking failed\n", file, line, (unsigned int)obj);
			res=-1;
		} else if ( CHECK_ALWAYS_VERBOSE <= m )
		{
			mame_printf_warning("%s:%d: GL Info: object 0x%X linked successful\n", file, line, (unsigned int)obj);
		}
		break;
	  case GL_OBJECT_VALIDATE_STATUS_ARB:
		if(param!=1)
		{
			if ( CHECK_VERBOSE <= m )
				mame_printf_warning("%s:%d: GL Error: object 0x%X validation failed\n", file, line, (unsigned int)obj);
			res=-1;
		} else if ( CHECK_ALWAYS_VERBOSE <= m )
		{
			mame_printf_warning("%s:%d: GL Info: object 0x%X validation successful\n", file, line, (unsigned int)obj);
		}
		break;
	 }

	if ( res<0 || CHECK_ALWAYS_VERBOSE <= m )
	{
		length=0;
		pfn_glGetInfoLogARB(obj, sizeof(buffer), &length, buffer);
		if(length>0)
			mame_printf_warning("%s:%d glInfoLog: %s\n", file, line, buffer);
	}

	(void) glGetError(); // ;-)


	return res;
}

int gl_round_to_pow2(int v)
{
	if (v & (v - 1))
	{
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v++;
	}
	return v;
}

int gl_texture_check_size(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
                          GLint border, GLenum format, GLenum type,
			  GLint *avail_width, GLint *avail_height,
			  int verbose)
{
    int err=1;
    GLenum texTargetProxy = (target==GL_TEXTURE_RECTANGLE_ARB)?GL_PROXY_TEXTURE_RECTANGLE_ARB:GL_PROXY_TEXTURE_2D;

    if ( !avail_width || !avail_height)
	return -1;

    *avail_width  = 0;
    *avail_height = 0;

    GL_CHECK_ERROR_QUIET();

    /* Test the max texture size */
    while(err && width>=1 && height>=1 /* && width>=64 && height>=64 */)
    {
	glTexImage2D (texTargetProxy, level,
		    internalFormat,
		    width, height,
		    border, format, type, NULL);
	if ( 0!=(err=GL_CHECK_ERROR_NORMAL() )) return err;

	glGetTexLevelParameteriv( texTargetProxy, level, GL_TEXTURE_WIDTH,  avail_width);
	glGetTexLevelParameteriv( texTargetProxy, level, GL_TEXTURE_HEIGHT, avail_height);
	if ( 0!=(err=GL_CHECK_ERROR_NORMAL() )) return err;

	if ( (*avail_width) != width || (*avail_height) != height )
	{
		err=1;

		if(verbose)
		{
			mame_printf_warning("gl_texture_size_check: "
			  "TexImage2D(0x%X, %d, 0x%X, %d, %d, %d, 0x%X, 0x%X): returned size does not match: %dx%d\n",
				(unsigned int)target, (int)level, (int)internalFormat,
				(int)width, (int)height, (int)border, (unsigned int)format, (unsigned int)type,
                                (int)*avail_width, (int)*avail_height);
		}

		if ( (*avail_width)  == width   )
			height /= 2;
		else if ( (*avail_height) == height )
			width /= 2;
		else if (width > height)
			  width /= 2;
		else
			  height /= 2;
		if(verbose)
		{
			fprintf (stderr, "gl_texture_size_check: trying [%dx%d] !\n", (int)height, (int)width);
		}
	} else {
		err=0;
	}
    }
    if(!err)
    {
	    *avail_width  = width;
	    *avail_height = height;
    }
    return err;
}

static int delete_shader_tool( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader, int externalcall)
{
	int res=0;

	GL_CHECK_ERROR_QUIET();
	if(program!=NULL && *program!=0)
	{
		pfn_glDeleteObjectARB(*program);
		if(externalcall)
		{
			res |= GL_SHADER_CHECK(*program, GL_OBJECT_DELETE_STATUS_ARB);
		}
		*program=0;
	}
	if(vertex_shader!=NULL && *vertex_shader!=0)
	{
		pfn_glDeleteObjectARB(*vertex_shader);
		if(externalcall)
		{
			res |= GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_DELETE_STATUS_ARB);
		}
		*vertex_shader=0;
	}
	if(fragment_shader!=NULL && *fragment_shader!=0)
	{
		pfn_glDeleteObjectARB(*fragment_shader);
		if(externalcall)
		{
			res |= GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_DELETE_STATUS_ARB);
		}
		*fragment_shader=0;
	}

        return res;
}

int gl_compile_shader_source( GLhandleARB *shader, GLenum type, const char * shader_source, int verbose )
{
        int err = 0;

	if(shader==NULL || shader_source==NULL)
	{
		if(shader==NULL)
			mame_printf_warning("error: gl_compile_shader_source: NULL shader passed\n");
		if(shader_source==NULL)
			mame_printf_warning("error: gl_compile_shader_source: NULL shader source passed\n");
		return -1;
	}
        *shader = pfn_glCreateShaderObjectARB(type);
	err=GL_SHADER_CHECK(*shader, GL_OBJECT_TYPE_ARB);
        if(err) goto errout;

        pfn_glShaderSourceARB(*shader, 1, (const GLcharARB **)&shader_source, NULL);

        pfn_glCompileShaderARB(*shader);
        err=GL_SHADER_CHECK(*shader, GL_OBJECT_COMPILE_STATUS_ARB);
        if(err) goto errout;

	if(verbose)
		mame_printf_warning( "<%s>\n", shader_source);

	return 0;

errout:
	if(*shader!=0)
	{
		pfn_glDeleteObjectARB(*shader);
	}
	mame_printf_warning("failed to process shader: <%s>\n", shader_source);
        return err;
}

int gl_compile_shader_file( GLhandleARB *shader, GLenum type, const char * shader_file, int verbose )
{
    int err = 0, i, c;
	FILE * file = NULL;
	const int buffer_len=8192;
	GLcharARB *buffer=NULL;

	if(shader==NULL || shader_file==NULL)
	{
		if(shader==NULL)
			mame_printf_warning("error: gl_compile_shader_source: NULL shader passed\n");
		if(shader_file==NULL)
			mame_printf_warning("error: gl_compile_shader_source: NULL shader file passed\n");
		return -1;
	}

	file = fopen(shader_file, "r");
	if(!file)
	{
		mame_printf_warning("cannot open shader_file: %s\n", shader_file);
		return -1;
	}

	buffer = (GLcharARB *) malloc(buffer_len);
	memset(buffer, 0, buffer_len);

        /* Load Shader Sources */
	i=0;
	while( i<buffer_len-1 && EOF!=(c=fgetc(file)) )
		buffer[i++]=(char)c;
	fclose(file);
	buffer[i]='\0';

	err=gl_compile_shader_source(shader, type, buffer, verbose);
        if(err) goto errout;

	free(buffer);

	if(verbose)
		mame_printf_warning("shader file: %s\n", shader_file);

	return 0;

errout:
	free(buffer);
	mame_printf_warning("failed to process shader_file: %s\n", shader_file);
        return err;
}


int gl_compile_shader_files( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader,
                             const char * vertex_shader_file,
                             const char * fragment_shader_file,
			     int verbose
                           )
{
	int err;

	if (!program)
	{
		err=-1;
		mame_printf_warning("no program ptr passed\n");
		goto errout;
	}
	if (!vertex_shader)
	{
		err=-1;
		mame_printf_warning("no vertex_shader ptr passed\n");
		goto errout;
	}
	if (!fragment_shader)
	{
		err=-1;
		mame_printf_warning("no fragment_shader ptr passed\n");
		goto errout;
	}

        *program = pfn_glCreateProgramObjectARB();
        err=GL_SHADER_CHECK(*program, GL_OBJECT_TYPE_ARB);
        if(err) goto errout;

	if(!vertex_shader_file)
	{
		if (!(*vertex_shader))
		{
			err=-1;
			mame_printf_warning("no vertex_shader_file, nor vertex_shader id passed\n");
			goto errout;
		}
		err=GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_TYPE_ARB);
		if(err) goto errout;
	} else {
		err = gl_compile_shader_file(vertex_shader, GL_VERTEX_SHADER_ARB, vertex_shader_file, verbose);
		if(err) return err;
	}
        pfn_glAttachObjectARB(*program, *vertex_shader);

	if(!fragment_shader_file)
	{
		if (!(*fragment_shader))
		{
			err=-1;
			mame_printf_warning("no fragment_shader_file, nor fragment_shader id passed\n");
			goto errout;
		}
		err=GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_TYPE_ARB);
		if(err) goto errout;
	} else {
		err = gl_compile_shader_file(fragment_shader, GL_FRAGMENT_SHADER_ARB, fragment_shader_file, verbose);
		if(err) return err;
	}
        pfn_glAttachObjectARB(*program, *fragment_shader);

        pfn_glLinkProgramARB(*program);
        err=GL_SHADER_CHECK(*program, GL_OBJECT_LINK_STATUS_ARB);
        if(err) goto errout;

        pfn_glValidateProgramARB(*program);
        err=GL_SHADER_CHECK(*program, GL_OBJECT_VALIDATE_STATUS_ARB);
        if(err) goto errout;

        return 0;

errout:
        delete_shader_tool(program, vertex_shader, fragment_shader, 0);
        return err;

}

int gl_compile_shader_sources( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader,
                               const GLcharARB * vertex_shader_source,
                               const GLcharARB * fragment_shader_source
                             )
{
        int err = 0;

	if (!program)
	{
		err=-1;
		mame_printf_warning("no program ptr passed\n");
		goto errout;
	}
	if (!vertex_shader)
	{
		err=-1;
		mame_printf_warning("no vertex_shader ptr passed\n");
		goto errout;
	}
	if (!fragment_shader)
	{
		err=-1;
		mame_printf_warning("no fragment_shader ptr passed\n");
		goto errout;
	}

        *program = pfn_glCreateProgramObjectARB();
        err=GL_SHADER_CHECK(*program, GL_OBJECT_TYPE_ARB);
        if(err) goto errout;

	if(!vertex_shader_source)
	{
		if (!(*vertex_shader))
		{
			err=-1;
			mame_printf_warning("no vertex_shader_source nor a vertex_shader id passed\n");
			goto errout;
		}
		err=GL_SHADER_CHECK(*vertex_shader, GL_OBJECT_TYPE_ARB);
		if(err) goto errout;
	}
	if(!fragment_shader_source)
	{
		if (!(*fragment_shader))
		{
			err=-1;
			mame_printf_warning("no fragment_shader_source nor a fragment_shader id passed\n");
			goto errout;
		}
		err=GL_SHADER_CHECK(*fragment_shader, GL_OBJECT_TYPE_ARB);
		if(err) goto errout;
	}

	if(vertex_shader_source)
	{
		err=gl_compile_shader_source(vertex_shader, GL_VERTEX_SHADER_ARB, vertex_shader_source, 0);
		if(err) goto errout;
	}
        pfn_glAttachObjectARB(*program, *vertex_shader);

	if(fragment_shader_source)
	{
		err=gl_compile_shader_source(fragment_shader, GL_FRAGMENT_SHADER_ARB, fragment_shader_source, 0);
		if(err) goto errout;
	}
        pfn_glAttachObjectARB(*program, *fragment_shader);

        pfn_glLinkProgramARB(*program);
        err=GL_SHADER_CHECK(*program, GL_OBJECT_LINK_STATUS_ARB);
        if(err) {
		mame_printf_warning("failed to link program\n");
		mame_printf_warning("vertex shader: <%s>\n", vertex_shader_source);
		mame_printf_warning("fragment shader: <%s>\n", fragment_shader_source);
		goto errout;
	}

        pfn_glValidateProgramARB(*program);
        err=GL_SHADER_CHECK(*program, GL_OBJECT_VALIDATE_STATUS_ARB);
        if(err) {
		mame_printf_warning("failed to validate program\n");
		mame_printf_warning("vertex shader: <%s>\n", vertex_shader_source);
		mame_printf_warning("fragment shader: <%s>\n", fragment_shader_source);
		goto errout;
	}

        return 0;

errout:
        delete_shader_tool(program, vertex_shader, fragment_shader, 0);
        return err;

}


int gl_delete_shader( GLhandleARB *program, GLhandleARB *vertex_shader, GLhandleARB *fragment_shader )
{
	return delete_shader_tool(program, vertex_shader, fragment_shader, 1);
}


