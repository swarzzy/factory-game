#pragma once
#if defined(__gl_h_)
#error gl/gl.h included somewhere
#endif

#include "flux_glcorearb.h"

#include "flux_platform.h"

// NOTE: Extensions
// ARB_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_ARB 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_ARB 0x84FF

// ARB_gl_spirv
#define GL_SHADER_BINARY_FORMAT_SPIR_V_ARB 0x9551
#define GL_SPIR_V_BINARY_ARB 0x9552
extern "C"
{
    typedef void(APIENTRY* PFNGLSPECIALIZESHADERARBPROC)(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);
}

// ARB_spirv_extensions
#define GL_SPIR_V_EXTENSIONS_ARB  0x9553
#define GL_NUM_SPIR_V_EXTENSIONS_ARB 0x9554

// NOTE: It was defined in soko_glcorearb.h
#if defined(APIENTRY_DEFINED)
#undef APIENTRY
#endif

struct OpenGL
{
    union Functions
    {
        struct _Functions
        {
            // 1.0
            PFNGLCULLFACEPROC glCullFace;
            PFNGLFRONTFACEPROC glFrontFace;
            PFNGLHINTPROC glHint;
            PFNGLLINEWIDTHPROC glLineWidth;
            PFNGLPOINTSIZEPROC glPointSize;
            PFNGLPOLYGONMODEPROC glPolygonMode;
            PFNGLSCISSORPROC glScissor;
            PFNGLTEXPARAMETERFPROC glTexParameterf;
            PFNGLTEXPARAMETERFVPROC glTexParameterfv;
            PFNGLTEXPARAMETERIPROC glTexParameteri;
            PFNGLTEXPARAMETERIVPROC glTexParameteriv;
            PFNGLTEXIMAGE1DPROC glTexImage1D;
            PFNGLTEXIMAGE2DPROC glTexImage2D;
            PFNGLDRAWBUFFERPROC glDrawBuffer;
            PFNGLCLEARPROC glClear;
            PFNGLCLEARCOLORPROC glClearColor;
            PFNGLCLEARSTENCILPROC glClearStencil;
            PFNGLCLEARDEPTHPROC glClearDepth;
            PFNGLSTENCILMASKPROC glStencilMask;
            PFNGLCOLORMASKPROC glColorMask;
            PFNGLDEPTHMASKPROC glDepthMask;
            PFNGLDISABLEPROC glDisable;
            PFNGLENABLEPROC glEnable;
            PFNGLFINISHPROC glFinish;
            PFNGLFLUSHPROC glFlush;
            PFNGLBLENDFUNCPROC glBlendFunc;
            PFNGLLOGICOPPROC glLogicOp;
            PFNGLSTENCILFUNCPROC glStencilFunc;
            PFNGLSTENCILOPPROC glStencilOp;
            PFNGLDEPTHFUNCPROC glDepthFunc;
            PFNGLPIXELSTOREFPROC glPixelStoref;
            PFNGLPIXELSTOREIPROC glPixelStorei;
            PFNGLREADBUFFERPROC glReadBuffer;
            PFNGLREADPIXELSPROC glReadPixels;
            PFNGLGETBOOLEANVPROC glGetBooleanv;
            PFNGLGETDOUBLEVPROC glGetDoublev;
            PFNGLGETERRORPROC glGetError;
            PFNGLGETFLOATVPROC glGetFloatv;
            PFNGLGETINTEGERVPROC glGetIntegerv;
            PFNGLGETSTRINGPROC glGetString;
            PFNGLGETTEXIMAGEPROC glGetTexImage;
            PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
            PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
            PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv;
            PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv;
            PFNGLISENABLEDPROC glIsEnabled;
            PFNGLDEPTHRANGEPROC glDepthRange;
            PFNGLVIEWPORTPROC glViewport;
            //  11
            PFNGLDRAWARRAYSPROC glDrawArrays;
            PFNGLDRAWELEMENTSPROC glDrawElements;
            PFNGLGETPOINTERVPROC glGetPointerv;
            PFNGLPOLYGONOFFSETPROC glPolygonOffset;
            PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D;
            PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D;
            PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D;
            PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
            PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D;
            PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
            PFNGLBINDTEXTUREPROC glBindTexture;
            PFNGLDELETETEXTURESPROC glDeleteTextures;
            PFNGLGENTEXTURESPROC glGenTextures;
            PFNGLISTEXTUREPROC glIsTexture;
            //  12
            PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
            PFNGLTEXIMAGE3DPROC glTexImage3D;
            PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
            PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
            //  13
            PFNGLACTIVETEXTUREPROC glActiveTexture;
            PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
            PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
            PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
            PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D;
            PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
            PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
            PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D;
            PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
            //  14
            PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
            PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
            PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
            PFNGLPOINTPARAMETERFPROC glPointParameterf;
            PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
            PFNGLPOINTPARAMETERIPROC glPointParameteri;
            PFNGLPOINTPARAMETERIVPROC glPointParameteriv;
            PFNGLBLENDCOLORPROC glBlendColor;
            PFNGLBLENDEQUATIONPROC glBlendEquation;
            // 15
            PFNGLGENQUERIESPROC glGenQueries;
            PFNGLDELETEQUERIESPROC glDeleteQueries;
            PFNGLISQUERYPROC glIsQuery;
            PFNGLBEGINQUERYPROC glBeginQuery;
            PFNGLENDQUERYPROC glEndQuery;
            PFNGLGETQUERYIVPROC glGetQueryiv;
            PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
            PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
            PFNGLBINDBUFFERPROC glBindBuffer;
            PFNGLDELETEBUFFERSPROC glDeleteBuffers;
            PFNGLGENBUFFERSPROC glGenBuffers;
            PFNGLISBUFFERPROC glIsBuffer;
            PFNGLBUFFERDATAPROC glBufferData;
            PFNGLBUFFERSUBDATAPROC glBufferSubData;
            PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
            PFNGLMAPBUFFERPROC glMapBuffer;
            PFNGLUNMAPBUFFERPROC glUnmapBuffer;
            PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
            PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
            //  20
            PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
            PFNGLDRAWBUFFERSPROC glDrawBuffers;
            PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
            PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
            PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
            PFNGLATTACHSHADERPROC glAttachShader;
            PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
            PFNGLCOMPILESHADERPROC glCompileShader;
            PFNGLCREATEPROGRAMPROC glCreateProgram;
            PFNGLCREATESHADERPROC glCreateShader;
            PFNGLDELETEPROGRAMPROC glDeleteProgram;
            PFNGLDELETESHADERPROC glDeleteShader;
            PFNGLDETACHSHADERPROC glDetachShader;
            PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
            PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
            PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
            PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
            PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
            PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
            PFNGLGETPROGRAMIVPROC glGetProgramiv;
            PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
            PFNGLGETSHADERIVPROC glGetShaderiv;
            PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
            PFNGLGETSHADERSOURCEPROC glGetShaderSource;
            PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
            PFNGLGETUNIFORMFVPROC glGetUniformfv;
            PFNGLGETUNIFORMIVPROC glGetUniformiv;
            PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv;
            PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv;
            PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
            PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv;
            PFNGLISPROGRAMPROC glIsProgram;
            PFNGLISSHADERPROC glIsShader;
            PFNGLLINKPROGRAMPROC glLinkProgram;
            PFNGLSHADERSOURCEPROC glShaderSource;
            PFNGLUSEPROGRAMPROC glUseProgram;
            PFNGLUNIFORM1FPROC glUniform1f;
            PFNGLUNIFORM2FPROC glUniform2f;
            PFNGLUNIFORM3FPROC glUniform3f;
            PFNGLUNIFORM4FPROC glUniform4f;
            PFNGLUNIFORM1IPROC glUniform1i;
            PFNGLUNIFORM2IPROC glUniform2i;
            PFNGLUNIFORM3IPROC glUniform3i;
            PFNGLUNIFORM4IPROC glUniform4i;
            PFNGLUNIFORM1FVPROC glUniform1fv;
            PFNGLUNIFORM2FVPROC glUniform2fv;
            PFNGLUNIFORM3FVPROC glUniform3fv;
            PFNGLUNIFORM4FVPROC glUniform4fv;
            PFNGLUNIFORM1IVPROC glUniform1iv;
            PFNGLUNIFORM2IVPROC glUniform2iv;
            PFNGLUNIFORM3IVPROC glUniform3iv;
            PFNGLUNIFORM4IVPROC glUniform4iv;
            PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
            PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
            PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
            PFNGLVALIDATEPROGRAMPROC glValidateProgram;
            PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d;
            PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv;
            PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
            PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
            PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s;
            PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv;
            PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d;
            PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv;
            PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
            PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
            PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s;
            PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv;
            PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d;
            PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv;
            PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
            PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
            PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s;
            PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv;
            PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv;
            PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv;
            PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv;
            PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub;
            PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv;
            PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv;
            PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv;
            PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv;
            PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d;
            PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv;
            PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
            PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
            PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv;
            PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s;
            PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv;
            PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv;
            PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv;
            PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv;
            PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
            //  21
            PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
            PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
            PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
            PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
            PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
            PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
            //  30
            PFNGLCOLORMASKIPROC glColorMaski;
            PFNGLGETBOOLEANI_VPROC glGetBooleani_v;
            PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
            PFNGLENABLEIPROC glEnablei;
            PFNGLDISABLEIPROC glDisablei;
            PFNGLISENABLEDIPROC glIsEnabledi;
            PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
            PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
            PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
            PFNGLBINDBUFFERBASEPROC glBindBufferBase;
            PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
            PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying;
            PFNGLCLAMPCOLORPROC glClampColor;
            PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender;
            PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender;
            PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
            PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv;
            PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv;
            PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i;
            PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i;
            PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i;
            PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i;
            PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui;
            PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui;
            PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui;
            PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui;
            PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv;
            PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv;
            PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv;
            PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv;
            PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv;
            PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv;
            PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv;
            PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv;
            PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv;
            PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv;
            PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv;
            PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv;
            PFNGLGETUNIFORMUIVPROC glGetUniformuiv;
            PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
            PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation;
            PFNGLUNIFORM1UIPROC glUniform1ui;
            PFNGLUNIFORM2UIPROC glUniform2ui;
            PFNGLUNIFORM3UIPROC glUniform3ui;
            PFNGLUNIFORM4UIPROC glUniform4ui;
            PFNGLUNIFORM1UIVPROC glUniform1uiv;
            PFNGLUNIFORM2UIVPROC glUniform2uiv;
            PFNGLUNIFORM3UIVPROC glUniform3uiv;
            PFNGLUNIFORM4UIVPROC glUniform4uiv;
            PFNGLTEXPARAMETERIIVPROC glTexParameterIiv;
            PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv;
            PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv;
            PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv;
            PFNGLCLEARBUFFERIVPROC glClearBufferiv;
            PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
            PFNGLCLEARBUFFERFVPROC glClearBufferfv;
            PFNGLCLEARBUFFERFIPROC glClearBufferfi;
            PFNGLGETSTRINGIPROC glGetStringi;
            PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
            PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
            PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
            PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
            PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
            PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
            PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
            PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
            PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
            PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
            PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
            PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D;
            PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
            PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D;
            PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
            PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
            PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
            PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
            PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
            PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
            PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
            PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
            PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
            PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
            PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
            PFNGLISVERTEXARRAYPROC glIsVertexArray;
            //  31
            PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
            PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
            PFNGLTEXBUFFERPROC glTexBuffer;
            PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex;
            PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;
            PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
            PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
            PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName;
            PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
            PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
            PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName;
            PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
            //  32
            PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
            PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex;
            PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex;
            PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex;
            PFNGLPROVOKINGVERTEXPROC glProvokingVertex;
            PFNGLFENCESYNCPROC glFenceSync;
            PFNGLISSYNCPROC glIsSync;
            PFNGLDELETESYNCPROC glDeleteSync;
            PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
            PFNGLWAITSYNCPROC glWaitSync;
            PFNGLGETINTEGER64VPROC glGetInteger64v;
            PFNGLGETSYNCIVPROC glGetSynciv;
            PFNGLGETINTEGER64I_VPROC glGetInteger64i_v;
            PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v;
            PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
            PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
            PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample;
            PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv;
            PFNGLSAMPLEMASKIPROC glSampleMaski;
            //  33
            PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed;
            PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex;
            PFNGLGENSAMPLERSPROC glGenSamplers;
            PFNGLDELETESAMPLERSPROC glDeleteSamplers;
            PFNGLISSAMPLERPROC glIsSampler;
            PFNGLBINDSAMPLERPROC glBindSampler;
            PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
            PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv;
            PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf;
            PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv;
            PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv;
            PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv;
            PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv;
            PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv;
            PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv;
            PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv;
            PFNGLQUERYCOUNTERPROC glQueryCounter;
            PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
            PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;
            PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
            PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui;
            PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv;
            PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui;
            PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv;
            PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui;
            PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv;
            PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui;
            PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv;
            // 4.0
            PFNGLMINSAMPLESHADINGPROC glMinSampleShading;
            PFNGLBLENDEQUATIONIPROC glBlendEquationi;
            PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei;
            PFNGLBLENDFUNCIPROC glBlendFunci;
            PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei;
            PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect;
            PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect;
            PFNGLUNIFORM1DPROC glUniform1d;
            PFNGLUNIFORM2DPROC glUniform2d;
            PFNGLUNIFORM3DPROC glUniform3d;
            PFNGLUNIFORM4DPROC glUniform4d;
            PFNGLUNIFORM1DVPROC glUniform1dv;
            PFNGLUNIFORM2DVPROC glUniform2dv;
            PFNGLUNIFORM3DVPROC glUniform3dv;
            PFNGLUNIFORM4DVPROC glUniform4dv;
            PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv;
            PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv;
            PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv;
            PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv;
            PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv;
            PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv;
            PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv;
            PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv;
            PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv;
            PFNGLGETUNIFORMDVPROC glGetUniformdv;
            PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation;
            PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex;
            PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv;
            PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName;
            PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName;
            PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv;
            PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv;
            PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv;
            PFNGLPATCHPARAMETERIPROC glPatchParameteri;
            PFNGLPATCHPARAMETERFVPROC glPatchParameterfv;
            PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
            PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks;
            PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
            PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback;
            PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback;
            PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback;
            PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
            PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream;
            PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed;
            PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed;
            PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv;
            // 4.1
            PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler;
            PFNGLSHADERBINARYPROC glShaderBinary;
            PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat;
            PFNGLDEPTHRANGEFPROC glDepthRangef;
            PFNGLCLEARDEPTHFPROC glClearDepthf;
            PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;
            PFNGLPROGRAMBINARYPROC glProgramBinary;
            PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
            PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages;
            PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram;
            PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
            PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
            PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
            PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
            PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline;
            PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv;
            PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
            PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv;
            PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
            PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv;
            PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d;
            PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv;
            PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui;
            PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv;
            PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i;
            PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv;
            PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
            PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv;
            PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d;
            PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv;
            PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui;
            PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv;
            PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i;
            PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv;
            PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
            PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv;
            PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d;
            PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv;
            PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui;
            PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv;
            PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i;
            PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv;
            PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
            PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv;
            PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d;
            PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv;
            PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui;
            PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv;
            PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv;
            PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv;
            PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;
            PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv;
            PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv;
            PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv;
            PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv;
            PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv;
            PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv;
            PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv;
            PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv;
            PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv;
            PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv;
            PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv;
            PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv;
            PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv;
            PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv;
            PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv;
            PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline;
            PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog;
            PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d;
            PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d;
            PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d;
            PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d;
            PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv;
            PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv;
            PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv;
            PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv;
            PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;
            PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv;
            PFNGLVIEWPORTARRAYVPROC glViewportArrayv;
            PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf;
            PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv;
            PFNGLSCISSORARRAYVPROC glScissorArrayv;
            PFNGLSCISSORINDEXEDPROC glScissorIndexed;
            PFNGLSCISSORINDEXEDVPROC glScissorIndexedv;
            PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv;
            PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed;
            PFNGLGETFLOATI_VPROC glGetFloati_v;
            PFNGLGETDOUBLEI_VPROC glGetDoublei_v;
            // 4.2
            PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glDrawArraysInstancedBaseInstance;
            PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance;
            PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;
            PFNGLGETINTERNALFORMATIVPROC glGetInternalformativ;
            PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC glGetActiveAtomicCounterBufferiv;
            PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
            PFNGLMEMORYBARRIERPROC glMemoryBarrier;
            PFNGLTEXSTORAGE1DPROC glTexStorage1D;
            PFNGLTEXSTORAGE2DPROC glTexStorage2D;
            PFNGLTEXSTORAGE3DPROC glTexStorage3D;
            PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glDrawTransformFeedbackInstanced;
            PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC glDrawTransformFeedbackStreamInstanced;
            // 4.3
            PFNGLCLEARBUFFERDATAPROC glClearBufferData;
            PFNGLCLEARBUFFERSUBDATAPROC glClearBufferSubData;
            PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
            PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect;
            PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData;
            PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
            PFNGLGETFRAMEBUFFERPARAMETERIVPROC glGetFramebufferParameteriv;
            PFNGLGETINTERNALFORMATI64VPROC glGetInternalformati64v;
            PFNGLINVALIDATETEXSUBIMAGEPROC glInvalidateTexSubImage;
            PFNGLINVALIDATETEXIMAGEPROC glInvalidateTexImage;
            PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData;
            PFNGLINVALIDATEBUFFERDATAPROC glInvalidateBufferData;
            PFNGLINVALIDATEFRAMEBUFFERPROC glInvalidateFramebuffer;
            PFNGLINVALIDATESUBFRAMEBUFFERPROC glInvalidateSubFramebuffer;
            PFNGLMULTIDRAWARRAYSINDIRECTPROC glMultiDrawArraysIndirect;
            PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;
            PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceiv;
            PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex;
            PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceName;
            PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv;
            PFNGLGETPROGRAMRESOURCELOCATIONPROC glGetProgramResourceLocation;
            PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex;
            PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
            PFNGLTEXBUFFERRANGEPROC glTexBufferRange;
            PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample;
            PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample;
            PFNGLTEXTUREVIEWPROC glTextureView;
            PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;
            PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
            PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat;
            PFNGLVERTEXATTRIBLFORMATPROC glVertexAttribLFormat;
            PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
            PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor;
            PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
            PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert;
            PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
            PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog;
            PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup;
            PFNGLPOPDEBUGGROUPPROC glPopDebugGroup;
            PFNGLOBJECTLABELPROC glObjectLabel;
            PFNGLGETOBJECTLABELPROC glGetObjectLabel;
            PFNGLOBJECTPTRLABELPROC glObjectPtrLabel;
            PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabel;
            // 4.4
            PFNGLBUFFERSTORAGEPROC glBufferStorage;
            PFNGLCLEARTEXIMAGEPROC glClearTexImage;
            PFNGLCLEARTEXSUBIMAGEPROC glClearTexSubImage;
            PFNGLBINDBUFFERSBASEPROC glBindBuffersBase;
            PFNGLBINDBUFFERSRANGEPROC glBindBuffersRange;
            PFNGLBINDTEXTURESPROC glBindTextures;
            PFNGLBINDSAMPLERSPROC glBindSamplers;
            PFNGLBINDIMAGETEXTURESPROC glBindImageTextures;
            PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers;
            // 4.5
            PFNGLCLIPCONTROLPROC glClipControl;
            PFNGLCREATETRANSFORMFEEDBACKSPROC glCreateTransformFeedbacks;
            PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC glTransformFeedbackBufferBase;
            PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC glTransformFeedbackBufferRange;
            PFNGLGETTRANSFORMFEEDBACKIVPROC glGetTransformFeedbackiv;
            PFNGLGETTRANSFORMFEEDBACKI_VPROC glGetTransformFeedbacki_v;
            PFNGLGETTRANSFORMFEEDBACKI64_VPROC glGetTransformFeedbacki64_v;
            PFNGLCREATEBUFFERSPROC glCreateBuffers;
            PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
            PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
            PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
            PFNGLCOPYNAMEDBUFFERSUBDATAPROC glCopyNamedBufferSubData;
            PFNGLCLEARNAMEDBUFFERDATAPROC glClearNamedBufferData;
            PFNGLCLEARNAMEDBUFFERSUBDATAPROC glClearNamedBufferSubData;
            PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer;
            PFNGLMAPNAMEDBUFFERRANGEPROC glMapNamedBufferRange;
            PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer;
            PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC glFlushMappedNamedBufferRange;
            PFNGLGETNAMEDBUFFERPARAMETERIVPROC glGetNamedBufferParameteriv;
            PFNGLGETNAMEDBUFFERPARAMETERI64VPROC glGetNamedBufferParameteri64v;
            PFNGLGETNAMEDBUFFERPOINTERVPROC glGetNamedBufferPointerv;
            PFNGLGETNAMEDBUFFERSUBDATAPROC glGetNamedBufferSubData;
            PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
            PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC glNamedFramebufferRenderbuffer;
            PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC glNamedFramebufferParameteri;
            PFNGLNAMEDFRAMEBUFFERTEXTUREPROC glNamedFramebufferTexture;
            PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC glNamedFramebufferTextureLayer;
            PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC glNamedFramebufferDrawBuffer;
            PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC glNamedFramebufferDrawBuffers;
            PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC glNamedFramebufferReadBuffer;
            PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC glInvalidateNamedFramebufferData;
            PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC glInvalidateNamedFramebufferSubData;
            PFNGLCLEARNAMEDFRAMEBUFFERIVPROC glClearNamedFramebufferiv;
            PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC glClearNamedFramebufferuiv;
            PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv;
            PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi;
            PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer;
            PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;
            PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC glGetNamedFramebufferParameteriv;
            PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetNamedFramebufferAttachmentParameteriv;
            PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers;
            PFNGLNAMEDRENDERBUFFERSTORAGEPROC glNamedRenderbufferStorage;
            PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC glNamedRenderbufferStorageMultisample;
            PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC glGetNamedRenderbufferParameteriv;
            PFNGLCREATETEXTURESPROC glCreateTextures;
            PFNGLTEXTUREBUFFERPROC glTextureBuffer;
            PFNGLTEXTUREBUFFERRANGEPROC glTextureBufferRange;
            PFNGLTEXTURESTORAGE1DPROC glTextureStorage1D;
            PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
            PFNGLTEXTURESTORAGE3DPROC glTextureStorage3D;
            PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC glTextureStorage2DMultisample;
            PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC glTextureStorage3DMultisample;
            PFNGLTEXTURESUBIMAGE1DPROC glTextureSubImage1D;
            PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
            PFNGLTEXTURESUBIMAGE3DPROC glTextureSubImage3D;
            PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC glCompressedTextureSubImage1D;
            PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC glCompressedTextureSubImage2D;
            PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC glCompressedTextureSubImage3D;
            PFNGLCOPYTEXTURESUBIMAGE1DPROC glCopyTextureSubImage1D;
            PFNGLCOPYTEXTURESUBIMAGE2DPROC glCopyTextureSubImage2D;
            PFNGLCOPYTEXTURESUBIMAGE3DPROC glCopyTextureSubImage3D;
            PFNGLTEXTUREPARAMETERFPROC glTextureParameterf;
            PFNGLTEXTUREPARAMETERFVPROC glTextureParameterfv;
            PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
            PFNGLTEXTUREPARAMETERIIVPROC glTextureParameterIiv;
            PFNGLTEXTUREPARAMETERIUIVPROC glTextureParameterIuiv;
            PFNGLTEXTUREPARAMETERIVPROC glTextureParameteriv;
            PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap;
            PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
            PFNGLGETTEXTUREIMAGEPROC glGetTextureImage;
            PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC glGetCompressedTextureImage;
            PFNGLGETTEXTURELEVELPARAMETERFVPROC glGetTextureLevelParameterfv;
            PFNGLGETTEXTURELEVELPARAMETERIVPROC glGetTextureLevelParameteriv;
            PFNGLGETTEXTUREPARAMETERFVPROC glGetTextureParameterfv;
            PFNGLGETTEXTUREPARAMETERIIVPROC glGetTextureParameterIiv;
            PFNGLGETTEXTUREPARAMETERIUIVPROC glGetTextureParameterIuiv;
            PFNGLGETTEXTUREPARAMETERIVPROC glGetTextureParameteriv;
            PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
            PFNGLDISABLEVERTEXARRAYATTRIBPROC glDisableVertexArrayAttrib;
            PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
            PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer;
            PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
            PFNGLVERTEXARRAYVERTEXBUFFERSPROC glVertexArrayVertexBuffers;
            PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
            PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
            PFNGLVERTEXARRAYATTRIBIFORMATPROC glVertexArrayAttribIFormat;
            PFNGLVERTEXARRAYATTRIBLFORMATPROC glVertexArrayAttribLFormat;
            PFNGLVERTEXARRAYBINDINGDIVISORPROC glVertexArrayBindingDivisor;
            PFNGLGETVERTEXARRAYIVPROC glGetVertexArrayiv;
            PFNGLGETVERTEXARRAYINDEXEDIVPROC glGetVertexArrayIndexediv;
            PFNGLGETVERTEXARRAYINDEXED64IVPROC glGetVertexArrayIndexed64iv;
            PFNGLCREATESAMPLERSPROC glCreateSamplers;
            PFNGLCREATEPROGRAMPIPELINESPROC glCreateProgramPipelines;
            PFNGLCREATEQUERIESPROC glCreateQueries;
            PFNGLGETQUERYBUFFEROBJECTI64VPROC glGetQueryBufferObjecti64v;
            PFNGLGETQUERYBUFFEROBJECTIVPROC glGetQueryBufferObjectiv;
            PFNGLGETQUERYBUFFEROBJECTUI64VPROC glGetQueryBufferObjectui64v;
            PFNGLGETQUERYBUFFEROBJECTUIVPROC glGetQueryBufferObjectuiv;
            PFNGLMEMORYBARRIERBYREGIONPROC glMemoryBarrierByRegion;
            PFNGLGETTEXTURESUBIMAGEPROC glGetTextureSubImage;
            PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC glGetCompressedTextureSubImage;
            PFNGLGETGRAPHICSRESETSTATUSPROC glGetGraphicsResetStatus;
            PFNGLGETNCOMPRESSEDTEXIMAGEPROC glGetnCompressedTexImage;
            PFNGLGETNTEXIMAGEPROC glGetnTexImage;
            PFNGLGETNUNIFORMDVPROC glGetnUniformdv;
            PFNGLGETNUNIFORMFVPROC glGetnUniformfv;
            PFNGLGETNUNIFORMIVPROC glGetnUniformiv;
            PFNGLGETNUNIFORMUIVPROC glGetnUniformuiv;
            PFNGLREADNPIXELSPROC glReadnPixels;
            PFNGLTEXTUREBARRIERPROC glTextureBarrier;
            // 4.6
        } fn;
        void* raw[sizeof(Functions::_Functions) / sizeof(void*)];
    } functions;

    struct
    {
        bool EXT_texture_filter_anisotropic;
        bool ARB_texture_filter_anisotropic;
        bool ARB_framebuffer_sRGB;
        bool ARB_spirv_extensions;
        struct _ARB_gl_spirv
        {
            bool supported;
            PFNGLSPECIALIZESHADERARBPROC glSpecializeShaderARB;
        } ARB_gl_spirv;
    } extensions;

    static const uint32_t FunctionCount = sizeof(Functions::_Functions) / sizeof(void*);

    static const inline  char* FunctionNames[] =
    {
        "glCullFace",
        "glFrontFace",
        "glHint",
        "glLineWidth",
        "glPointSize",
        "glPolygonMode",
        "glScissor",
        "glTexParameterf",
        "glTexParameterfv",
        "glTexParameteri",
        "glTexParameteriv",
        "glTexImage1D",
        "glTexImage2D",
        "glDrawBuffer",
        "glClear",
        "glClearColor",
        "glClearStencil",
        "glClearDepth",
        "glStencilMask",
        "glColorMask",
        "glDepthMask",
        "glDisable",
        "glEnable",
        "glFinish",
        "glFlush",
        "glBlendFunc",
        "glLogicOp",
        "glStencilFunc",
        "glStencilOp",
        "glDepthFunc",
        "glPixelStoref",
        "glPixelStorei",
        "glReadBuffer",
        "glReadPixels",
        "glGetBooleanv",
        "glGetDoublev",
        "glGetError",
        "glGetFloatv",
        "glGetIntegerv",
        "glGetString",
        "glGetTexImage",
        "glGetTexParameterfv",
        "glGetTexParameteriv",
        "glGetTexLevelParameterfv",
        "glGetTexLevelParameteriv",
        "glIsEnabled",
        "glDepthRange",
        "glViewport",
        //  11
        "glDrawArrays",
        "glDrawElements",
        "glGetPointerv",
        "glPolygonOffset",
        "glCopyTexImage1D",
        "glCopyTexImage2D",
        "glCopyTexSubImage1D",
        "glCopyTexSubImage2D",
        "glTexSubImage1D",
        "glTexSubImage2D",
        "glBindTexture",
        "glDeleteTextures",
        "glGenTextures",
        "glIsTexture",
        //  12
        "glDrawRangeElements",
        "glTexImage3D",
        "glTexSubImage3D",
        "glCopyTexSubImage3D",
        //  13
        "glActiveTexture",
        "glSampleCoverage",
        "glCompressedTexImage3D",
        "glCompressedTexImage2D",
        "glCompressedTexImage1D",
        "glCompressedTexSubImage3D",
        "glCompressedTexSubImage2D",
        "glCompressedTexSubImage1D",
        "glGetCompressedTexImage",
        //  14
        "glBlendFuncSeparate",
        "glMultiDrawArrays",
        "glMultiDrawElements",
        "glPointParameterf",
        "glPointParameterfv",
        "glPointParameteri",
        "glPointParameteriv",
        "glBlendColor",
        "glBlendEquation",
        // 15
        "glGenQueries",
        "glDeleteQueries",
        "glIsQuery",
        "glBeginQuery",
        "glEndQuery",
        "glGetQueryiv",
        "glGetQueryObjectiv",
        "glGetQueryObjectuiv",
        "glBindBuffer",
        "glDeleteBuffers",
        "glGenBuffers",
        "glIsBuffer",
        "glBufferData",
        "glBufferSubData",
        "glGetBufferSubData",
        "glMapBuffer",
        "glUnmapBuffer",
        "glGetBufferParameteriv",
        "glGetBufferPointerv",
        //  20
        "glBlendEquationSeparate",
        "glDrawBuffers",
        "glStencilOpSeparate",
        "glStencilFuncSeparate",
        "glStencilMaskSeparate",
        "glAttachShader",
        "glBindAttribLocation",
        "glCompileShader",
        "glCreateProgram",
        "glCreateShader",
        "glDeleteProgram",
        "glDeleteShader",
        "glDetachShader",
        "glDisableVertexAttribArray",
        "glEnableVertexAttribArray",
        "glGetActiveAttrib",
        "glGetActiveUniform",
        "glGetAttachedShaders",
        "glGetAttribLocation",
        "glGetProgramiv",
        "glGetProgramInfoLog",
        "glGetShaderiv",
        "glGetShaderInfoLog",
        "glGetShaderSource",
        "glGetUniformLocation",
        "glGetUniformfv",
        "glGetUniformiv",
        "glGetVertexAttribdv",
        "glGetVertexAttribfv",
        "glGetVertexAttribiv",
        "glGetVertexAttribPointerv",
        "glIsProgram",
        "glIsShader",
        "glLinkProgram",
        "glShaderSource",
        "glUseProgram",
        "glUniform1f",
        "glUniform2f",
        "glUniform3f",
        "glUniform4f",
        "glUniform1i",
        "glUniform2i",
        "glUniform3i",
        "glUniform4i",
        "glUniform1fv",
        "glUniform2fv",
        "glUniform3fv",
        "glUniform4fv",
        "glUniform1iv",
        "glUniform2iv",
        "glUniform3iv",
        "glUniform4iv",
        "glUniformMatrix2fv",
        "glUniformMatrix3fv",
        "glUniformMatrix4fv",
        "glValidateProgram",
        "glVertexAttrib1d",
        "glVertexAttrib1dv",
        "glVertexAttrib1f",
        "glVertexAttrib1fv",
        "glVertexAttrib1s",
        "glVertexAttrib1sv",
        "glVertexAttrib2d",
        "glVertexAttrib2dv",
        "glVertexAttrib2f",
        "glVertexAttrib2fv",
        "glVertexAttrib2s",
        "glVertexAttrib2sv",
        "glVertexAttrib3d",
        "glVertexAttrib3dv",
        "glVertexAttrib3f",
        "glVertexAttrib3fv",
        "glVertexAttrib3s",
        "glVertexAttrib3sv",
        "glVertexAttrib4Nbv",
        "glVertexAttrib4Niv",
        "glVertexAttrib4Nsv",
        "glVertexAttrib4Nub",
        "glVertexAttrib4Nubv",
        "glVertexAttrib4Nuiv",
        "glVertexAttrib4Nusv",
        "glVertexAttrib4bv",
        "glVertexAttrib4d",
        "glVertexAttrib4dv",
        "glVertexAttrib4f",
        "glVertexAttrib4fv",
        "glVertexAttrib4iv",
        "glVertexAttrib4s",
        "glVertexAttrib4sv",
        "glVertexAttrib4ubv",
        "glVertexAttrib4uiv",
        "glVertexAttrib4usv",
        "glVertexAttribPointer",
        //  21
        "glUniformMatrix2x3fv",
        "glUniformMatrix3x2fv",
        "glUniformMatrix2x4fv",
        "glUniformMatrix4x2fv",
        "glUniformMatrix3x4fv",
        "glUniformMatrix4x3fv",
        //  30
        "glColorMaski",
        "VPROC glGetBooleani_v",
        "VPROC glGetIntegeri_v",
        "glEnablei",
        "glDisablei",
        "glIsEnabledi",
        "glBeginTransformFeedback",
        "glEndTransformFeedback",
        "glBindBufferRange",
        "glBindBufferBase",
        "glTransformFeedbackVaryings",
        "glGetTransformFeedbackVarying",
        "glClampColor",
        "glBeginConditionalRender",
        "glEndConditionalRender",
        "glVertexAttribIPointer",
        "glGetVertexAttribIiv",
        "glGetVertexAttribIuiv",
        "glVertexAttribI1i",
        "glVertexAttribI2i",
        "glVertexAttribI3i",
        "glVertexAttribI4i",
        "glVertexAttribI1ui",
        "glVertexAttribI2ui",
        "glVertexAttribI3ui",
        "glVertexAttribI4ui",
        "glVertexAttribI1iv",
        "glVertexAttribI2iv",
        "glVertexAttribI3iv",
        "glVertexAttribI4iv",
        "glVertexAttribI1uiv",
        "glVertexAttribI2uiv",
        "glVertexAttribI3uiv",
        "glVertexAttribI4uiv",
        "glVertexAttribI4bv",
        "glVertexAttribI4sv",
        "glVertexAttribI4ubv",
        "glVertexAttribI4usv",
        "glGetUniformuiv",
        "glBindFragDataLocation",
        "glGetFragDataLocation",
        "glUniform1ui",
        "glUniform2ui",
        "glUniform3ui",
        "glUniform4ui",
        "glUniform1uiv",
        "glUniform2uiv",
        "glUniform3uiv",
        "glUniform4uiv",
        "glTexParameterIiv",
        "glTexParameterIuiv",
        "glGetTexParameterIiv",
        "glGetTexParameterIuiv",
        "glClearBufferiv",
        "glClearBufferuiv",
        "glClearBufferfv",
        "glClearBufferfi",
        "glGetStringi",
        "glIsRenderbuffer",
        "glBindRenderbuffer",
        "glDeleteRenderbuffers",
        "glGenRenderbuffers",
        "glRenderbufferStorage",
        "glGetRenderbufferParameteriv",
        "glIsFramebuffer",
        "glBindFramebuffer",
        "glDeleteFramebuffers",
        "glGenFramebuffers",
        "glCheckFramebufferStatus",
        "glFramebufferTexture1D",
        "glFramebufferTexture2D",
        "glFramebufferTexture3D",
        "glFramebufferRenderbuffer",
        "glGetFramebufferAttachmentParameteriv",
        "glGenerateMipmap",
        "glBlitFramebuffer",
        "glRenderbufferStorageMultisample",
        "glFramebufferTextureLayer",
        "glMapBufferRange",
        "glFlushMappedBufferRange",
        "glBindVertexArray",
        "glDeleteVertexArrays",
        "glGenVertexArrays",
        "glIsVertexArray",
        //  31
        "glDrawArraysInstanced",
        "glDrawElementsInstanced",
        "glTexBuffer",
        "glPrimitiveRestartIndex",
        "glCopyBufferSubData",
        "glGetUniformIndices",
        "glGetActiveUniformsiv",
        "glGetActiveUniformName",
        "glGetUniformBlockIndex",
        "glGetActiveUniformBlockiv",
        "glGetActiveUniformBlockName",
        "glUniformBlockBinding",
        //  32
        "glDrawElementsBaseVertex",
        "glDrawRangeElementsBaseVertex",
        "glDrawElementsInstancedBaseVertex",
        "glMultiDrawElementsBaseVertex",
        "glProvokingVertex",
        "glFenceSync",
        "glIsSync",
        "glDeleteSync",
        "glClientWaitSync",
        "glWaitSync",
        "glGetInteger64v",
        "glGetSynciv",
        "glGetInteger64i_v",
        "glGetBufferParameteri64v",
        "glFramebufferTexture",
        "glTexImage2DMultisample",
        "glTexImage3DMultisample",
        "glGetMultisamplefv",
        "glSampleMaski",
        //  33
        "glBindFragDataLocationIndexed",
        "glGetFragDataIndex",
        "glGenSamplers",
        "glDeleteSamplers",
        "glIsSampler",
        "glBindSampler",
        "glSamplerParameteri",
        "glSamplerParameteriv",
        "glSamplerParameterf",
        "glSamplerParameterfv",
        "glSamplerParameterIiv",
        "glSamplerParameterIuiv",
        "glGetSamplerParameteriv",
        "glGetSamplerParameterIiv",
        "glGetSamplerParameterfv",
        "glGetSamplerParameterIuiv",
        "glQueryCounter",
        "glGetQueryObjecti64v",
        "glGetQueryObjectui64v",
        "glVertexAttribDivisor",
        "glVertexAttribP1ui",
        "glVertexAttribP1uiv",
        "glVertexAttribP2ui",
        "glVertexAttribP2uiv",
        "glVertexAttribP3ui",
        "glVertexAttribP3uiv",
        "glVertexAttribP4ui",
        "glVertexAttribP4uiv",
        // 4.0
        "glMinSampleShading",
        "glBlendEquationi",
        "glBlendEquationSeparatei",
        "glBlendFunci",
        "glBlendFuncSeparatei",
        "glDrawArraysIndirect",
        "glDrawElementsIndirect",
        "glUniform1d",
        "glUniform2d",
        "glUniform3d",
        "glUniform4d",
        "glUniform1dv",
        "glUniform2dv",
        "glUniform3dv",
        "glUniform4dv",
        "glUniformMatrix2dv",
        "glUniformMatrix3dv",
        "glUniformMatrix4dv",
        "glUniformMatrix2x3dv",
        "glUniformMatrix2x4dv",
        "glUniformMatrix3x2dv",
        "glUniformMatrix3x4dv",
        "glUniformMatrix4x2dv",
        "glUniformMatrix4x3dv",
        "glGetUniformdv",
        "glGetSubroutineUniformLocation",
        "glGetSubroutineIndex",
        "glGetActiveSubroutineUniformiv",
        "glGetActiveSubroutineUniformName",
        "glGetActiveSubroutineName",
        "glUniformSubroutinesuiv",
        "glGetUniformSubroutineuiv",
        "glGetProgramStageiv",
        "glPatchParameteri",
        "glPatchParameterfv",
        "glBindTransformFeedback",
        "glDeleteTransformFeedbacks",
        "glGenTransformFeedbacks",
        "glIsTransformFeedback",
        "glPauseTransformFeedback",
        "glResumeTransformFeedback",
        "glDrawTransformFeedback",
        "glDrawTransformFeedbackStream",
        "glBeginQueryIndexed",
        "glEndQueryIndexed",
        "glGetQueryIndexediv",
        // 4.1
        "glReleaseShaderCompiler",
        "glShaderBinary",
        "glGetShaderPrecisionFormat",
        "glDepthRangef",
        "glClearDepthf",
        "glGetProgramBinary",
        "glProgramBinary",
        "glProgramParameteri",
        "glUseProgramStages",
        "glActiveShaderProgram",
        "glCreateShaderProgramv",
        "glBindProgramPipeline",
        "glDeleteProgramPipelines",
        "glGenProgramPipelines",
        "glIsProgramPipeline",
        "glGetProgramPipelineiv",
        "glProgramUniform1i",
        "glProgramUniform1iv",
        "glProgramUniform1f",
        "glProgramUniform1fv",
        "glProgramUniform1d",
        "glProgramUniform1dv",
        "glProgramUniform1ui",
        "glProgramUniform1uiv",
        "glProgramUniform2i",
        "glProgramUniform2iv",
        "glProgramUniform2f",
        "glProgramUniform2fv",
        "glProgramUniform2d",
        "glProgramUniform2dv",
        "glProgramUniform2ui",
        "glProgramUniform2uiv",
        "glProgramUniform3i",
        "glProgramUniform3iv",
        "glProgramUniform3f",
        "glProgramUniform3fv",
        "glProgramUniform3d",
        "glProgramUniform3dv",
        "glProgramUniform3ui",
        "glProgramUniform3uiv",
        "glProgramUniform4i",
        "glProgramUniform4iv",
        "glProgramUniform4f",
        "glProgramUniform4fv",
        "glProgramUniform4d",
        "glProgramUniform4dv",
        "glProgramUniform4ui",
        "glProgramUniform4uiv",
        "glProgramUniformMatrix2fv",
        "glProgramUniformMatrix3fv",
        "glProgramUniformMatrix4fv",
        "glProgramUniformMatrix2dv",
        "glProgramUniformMatrix3dv",
        "glProgramUniformMatrix4dv",
        "glProgramUniformMatrix2x3fv",
        "glProgramUniformMatrix3x2fv",
        "glProgramUniformMatrix2x4fv",
        "glProgramUniformMatrix4x2fv",
        "glProgramUniformMatrix3x4fv",
        "glProgramUniformMatrix4x3fv",
        "glProgramUniformMatrix2x3dv",
        "glProgramUniformMatrix3x2dv",
        "glProgramUniformMatrix2x4dv",
        "glProgramUniformMatrix4x2dv",
        "glProgramUniformMatrix3x4dv",
        "glProgramUniformMatrix4x3dv",
        "glValidateProgramPipeline",
        "glGetProgramPipelineInfoLog",
        "glVertexAttribL1d",
        "glVertexAttribL2d",
        "glVertexAttribL3d",
        "glVertexAttribL4d",
        "glVertexAttribL1dv",
        "glVertexAttribL2dv",
        "glVertexAttribL3dv",
        "glVertexAttribL4dv",
        "glVertexAttribLPointer",
        "glGetVertexAttribLdv",
        "glViewportArrayv",
        "glViewportIndexedf",
        "glViewportIndexedfv",
        "glScissorArrayv",
        "glScissorIndexed",
        "glScissorIndexedv",
        "glDepthRangeArrayv",
        "glDepthRangeIndexed",
        "glGetFloati_v",
        "glGetDoublei_v",
        // 4.2
        "glDrawArraysInstancedBaseInstance",
        "glDrawElementsInstancedBaseInstance",
        "glDrawElementsInstancedBaseVertexBaseInstance",
        "glGetInternalformativ",
        "glGetActiveAtomicCounterBufferiv",
        "glBindImageTexture",
        "glMemoryBarrier",
        "glTexStorage1D",
        "glTexStorage2D",
        "glTexStorage3D",
        "glDrawTransformFeedbackInstanced",
        "glDrawTransformFeedbackStreamInstanced",
        // 4.3
        "glClearBufferData",
        "glClearBufferSubData",
        "glDispatchCompute",
        "glDispatchComputeIndirect",
        "glCopyImageSubData",
        "glFramebufferParameteri",
        "glGetFramebufferParameteriv",
        "glGetInternalformati64v",
        "glInvalidateTexSubImage",
        "glInvalidateTexImage",
        "glInvalidateBufferSubData",
        "glInvalidateBufferData",
        "glInvalidateFramebuffer",
        "glInvalidateSubFramebuffer",
        "glMultiDrawArraysIndirect",
        "glMultiDrawElementsIndirect",
        "glGetProgramInterfaceiv",
        "glGetProgramResourceIndex",
        "glGetProgramResourceName",
        "glGetProgramResourceiv",
        "glGetProgramResourceLocation",
        "glGetProgramResourceLocationIndex",
        "glShaderStorageBlockBinding",
        "glTexBufferRange",
        "glTexStorage2DMultisample",
        "glTexStorage3DMultisample",
        "glTextureView",
        "glBindVertexBuffer",
        "glVertexAttribFormat",
        "glVertexAttribIFormat",
        "glVertexAttribLFormat",
        "glVertexAttribBinding",
        "glVertexBindingDivisor",
        "glDebugMessageControl",
        "glDebugMessageInsert",
        "glDebugMessageCallback",
        "glGetDebugMessageLog",
        "glPushDebugGroup",
        "glPopDebugGroup",
        "glObjectLabel",
        "glGetObjectLabel",
        "glObjectPtrLabel",
        "glGetObjectPtrLabel",
        // 4.4
        "glBufferStorage",
        "glClearTexImage",
        "glClearTexSubImage",
        "glBindBuffersBase",
        "glBindBuffersRange",
        "glBindTextures",
        "glBindSamplers",
        "glBindImageTextures",
        "glBindVertexBuffers",
        // 4.5
        "glClipControl",
        "glCreateTransformFeedbacks",
        "glTransformFeedbackBufferBase",
        "glTransformFeedbackBufferRange",
        "glGetTransformFeedbackiv",
        "glGetTransformFeedbacki_v",
        "glGetTransformFeedbacki64_v",
        "glCreateBuffers",
        "glNamedBufferStorage",
        "glNamedBufferData",
        "glNamedBufferSubData",
        "glCopyNamedBufferSubData",
        "glClearNamedBufferData",
        "glClearNamedBufferSubData",
        "glMapNamedBuffer",
        "glMapNamedBufferRange",
        "glUnmapNamedBuffer",
        "glFlushMappedNamedBufferRange",
        "glGetNamedBufferParameteriv",
        "glGetNamedBufferParameteri64v",
        "glGetNamedBufferPointerv",
        "glGetNamedBufferSubData",
        "glCreateFramebuffers",
        "glNamedFramebufferRenderbuffer",
        "glNamedFramebufferParameteri",
        "glNamedFramebufferTexture",
        "glNamedFramebufferTextureLayer",
        "glNamedFramebufferDrawBuffer",
        "glNamedFramebufferDrawBuffers",
        "glNamedFramebufferReadBuffer",
        "glInvalidateNamedFramebufferData",
        "glInvalidateNamedFramebufferSubData",
        "glClearNamedFramebufferiv",
        "glClearNamedFramebufferuiv",
        "glClearNamedFramebufferfv",
        "glClearNamedFramebufferfi",
        "glBlitNamedFramebuffer",
        "glCheckNamedFramebufferStatus",
        "glGetNamedFramebufferParameteriv",
        "glGetNamedFramebufferAttachmentParameteriv",
        "glCreateRenderbuffers",
        "glNamedRenderbufferStorage",
        "glNamedRenderbufferStorageMultisample",
        "glGetNamedRenderbufferParameteriv",
        "glCreateTextures",
        "glTextureBuffer",
        "glTextureBufferRange",
        "glTextureStorage1D",
        "glTextureStorage2D",
        "glTextureStorage3D",
        "glTextureStorage2DMultisample",
        "glTextureStorage3DMultisample",
        "glTextureSubImage1D",
        "glTextureSubImage2D",
        "glTextureSubImage3D",
        "glCompressedTextureSubImage1D",
        "glCompressedTextureSubImage2D",
        "glCompressedTextureSubImage3D",
        "glCopyTextureSubImage1D",
        "glCopyTextureSubImage2D",
        "glCopyTextureSubImage3D",
        "glTextureParameterf",
        "glTextureParameterfv",
        "glTextureParameteri",
        "glTextureParameterIiv",
        "glTextureParameterIuiv",
        "glTextureParameteriv",
        "glGenerateTextureMipmap",
        "glBindTextureUnit",
        "glGetTextureImage",
        "glGetCompressedTextureImage",
        "glGetTextureLevelParameterfv",
        "glGetTextureLevelParameteriv",
        "glGetTextureParameterfv",
        "glGetTextureParameterIiv",
        "glGetTextureParameterIuiv",
        "glGetTextureParameteriv",
        "glCreateVertexArrays",
        "glDisableVertexArrayAttrib",
        "glEnableVertexArrayAttrib",
        "glVertexArrayElementBuffer",
        "glVertexArrayVertexBuffer",
        "glVertexArrayVertexBuffers",
        "glVertexArrayAttribBinding",
        "glVertexArrayAttribFormat",
        "glVertexArrayAttribIFormat",
        "glVertexArrayAttribLFormat",
        "glVertexArrayBindingDivisor",
        "glGetVertexArrayiv",
        "glGetVertexArrayIndexediv",
        "glGetVertexArrayIndexed64iv",
        "glCreateSamplers",
        "glCreateProgramPipelines",
        "glCreateQueries",
        "glGetQueryBufferObjecti64v",
        "glGetQueryBufferObjectiv",
        "glGetQueryBufferObjectui64v",
        "glGetQueryBufferObjectuiv",
        "glMemoryBarrierByRegion",
        "glGetTextureSubImage",
        "glGetCompressedTextureSubImage",
        "glGetGraphicsResetStatus",
        "glGetnCompressedTexImage",
        "glGetnTexImage",
        "glGetnUniformdv",
        "glGetnUniformfv",
        "glGetnUniformiv",
        "glGetnUniformuiv",
        "glReadnPixels",
        "glTextureBarrier"
    };
};
