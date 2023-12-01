#include "stdafx.h"
#include "RL.h"

int* defaultShaderLocs;             // Default shader locations pointer to be used on rendering
unsigned int defaultShaderId;       // Default shader program id, supports vertex color and diffuse texture
unsigned int defaultVShaderId;      // Default vertex shader id (used by default shader program)
unsigned int defaultFShaderId;      // Default fragment shader id (used by default shader program)

// Load default shader (just vertex positioning and texture coloring)
// NOTE: This shader program is used for internal buffers
// NOTE: Loaded: RLGL.State.defaultShaderId, RLGL.State.defaultShaderLocs
void rlLoadShaderDefault()
{
    defaultShaderLocs = (int*)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));

    // NOTE: All locations must be reseted to -1 (no location)
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) defaultShaderLocs[i] = -1;

    // Vertex shader directly defined, no external file required
    const char* defaultVShaderCode =
        "#version 330                       \n"
        "in vec3 vertexPosition;            \n"
        "in vec2 vertexTexCoord;            \n"
        "in vec4 vertexColor;               \n"
        "out vec2 fragTexCoord;             \n"
        "out vec4 fragColor;                \n"
        "uniform mat4 mvp;                  \n"
        "void main()                        \n"
        "{                                  \n"
        "    fragTexCoord = vertexTexCoord; \n"
        "    fragColor = vertexColor;       \n"
        "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
        "}                                  \n";

    // Fragment shader directly defined, no external file required
    const char* defaultFShaderCode =
        "#version 330       \n"
        "in vec2 fragTexCoord;              \n"
        "in vec4 fragColor;                 \n"
        "out vec4 finalColor;               \n"
        "uniform sampler2D texture0;        \n"
        "uniform vec4 colDiffuse;           \n"
        "void main()                        \n"
        "{                                  \n"
        "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
        "    finalColor = texelColor*colDiffuse*fragColor;        \n"
        "}                                  \n";

    // NOTE: Compiled vertex/fragment shaders are not deleted,
    // they are kept for re-use as default shaders in case some shader loading fails
    defaultVShaderId = rlCompileShader(defaultVShaderCode, GL_VERTEX_SHADER);     // Compile default vertex shader
    defaultFShaderId = rlCompileShader(defaultFShaderCode, GL_FRAGMENT_SHADER);   // Compile default fragment shader

    defaultShaderId = rlLoadShaderProgram(defaultVShaderId, defaultFShaderId);

    if (defaultShaderId > 0)
    {
        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Default shader loaded successfully", defaultShaderId);

        // Set default shader locations: attributes locations
        defaultShaderLocs[RL_SHADER_LOC_VERTEX_POSITION] = glGetAttribLocation(defaultShaderId, "vertexPosition");
        defaultShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(defaultShaderId, "vertexTexCoord");
        defaultShaderLocs[RL_SHADER_LOC_VERTEX_COLOR] = glGetAttribLocation(defaultShaderId, "vertexColor");

        // Set default shader locations: uniform locations
        defaultShaderLocs[RL_SHADER_LOC_MATRIX_MVP] = glGetUniformLocation(defaultShaderId, "mvp");
        defaultShaderLocs[RL_SHADER_LOC_COLOR_DIFFUSE] = glGetUniformLocation(defaultShaderId, "colDiffuse");
        defaultShaderLocs[RL_SHADER_LOC_MAP_DIFFUSE] = glGetUniformLocation(defaultShaderId, "texture0");
    }
    else TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to load default shader", defaultShaderId);
}

// Get default shader id
unsigned int rlGetShaderIdDefault()
{
    unsigned int id = 0;
    id = defaultShaderId;
    return id;
}

// Compile custom shader and return shader id
unsigned int rlCompileShader(const char* shaderCode, int type)
{
    unsigned int shader = 0;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        switch (type)
        {
        case GL_VERTEX_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile vertex shader code", shader); break;
        case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile fragment shader code", shader); break;
        default: break;
        }

        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char* log = (char*)RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shader, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            RL_FREE(log);
        }
    }
    else
    {
        switch (type)
        {
        case GL_VERTEX_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Vertex shader compiled successfully", shader); break;
        case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Fragment shader compiled successfully", shader); break;
        default: break;
        }
    }

    return shader;
}

// Load custom shader strings and return program id
unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId)
{
    unsigned int program = 0;

    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    // NOTE: Default attribute shader locations must be Bound before linking
    glBindAttribLocation(program, 0, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, 1, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, 2, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, 3, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, 4, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, 5, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);

    // NOTE: If some attrib name is no found on the shader, it locations becomes -1

    glLinkProgram(program);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", program);

        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char* log = (char*)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);
            RL_FREE(log);
        }

        glDeleteProgram(program);

        program = 0;
    }
    else
    {
        // Get the size of compiled shader program (not available on OpenGL ES 2.0)
        // NOTE: If GL_LINK_STATUS is GL_FALSE, program binary length is zero.
        //GLint binarySize = 0;
        //glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binarySize);

        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Program shader loaded successfully", program);
    }
    return program;
}

// Get default shader locs
int* rlGetShaderLocsDefault()
{
    int* locs = NULL;
    locs = defaultShaderLocs;
    return locs;
}

// Load shader from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
unsigned int rlLoadShaderCode(const char* vsCode, const char* fsCode)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = 0;
    unsigned int fragmentShaderId = 0;

    // Compile vertex shader (if provided)
    if (vsCode != NULL) vertexShaderId = rlCompileShader(vsCode, GL_VERTEX_SHADER);
    // In case no vertex shader was provided or compilation failed, we use default vertex shader
    if (vertexShaderId == 0) vertexShaderId = defaultVShaderId;

    // Compile fragment shader (if provided)
    if (fsCode != NULL) fragmentShaderId = rlCompileShader(fsCode, GL_FRAGMENT_SHADER);
    // In case no fragment shader was provided or compilation failed, we use default fragment shader
    if (fragmentShaderId == 0) fragmentShaderId = defaultFShaderId;

    // In case vertex and fragment shader are the default ones, no need to recompile, we can just assign the default shader program id
    if ((vertexShaderId == defaultVShaderId) && (fragmentShaderId == defaultFShaderId)) id = defaultShaderId;
    else
    {
        // One of or both shader are new, we need to compile a new shader program
        id = rlLoadShaderProgram(vertexShaderId, fragmentShaderId);

        // We can detach and delete vertex/fragment shaders (if not default ones)
        // NOTE: We detach shader before deletion to make sure memory is freed
        if (vertexShaderId != defaultVShaderId)
        {
            // WARNING: Shader program linkage could fail and returned id is 0
            if (id > 0) glDetachShader(id, vertexShaderId);
            glDeleteShader(vertexShaderId);
        }
        if (fragmentShaderId != defaultFShaderId)
        {
            // WARNING: Shader program linkage could fail and returned id is 0
            if (id > 0) glDetachShader(id, fragmentShaderId);
            glDeleteShader(fragmentShaderId);
        }

        // In case shader program loading failed, we assign default shader
        if (id == 0)
        {
            // In case shader loading fails, we return the default shader
            TRACELOG(RL_LOG_WARNING, "SHADER: Failed to load custom shader code, using default shader");
            id = defaultShaderId;
        }
        /*
        else
        {
            // Get available shader uniforms
            // NOTE: This information is useful for debug...
            int uniformCount = -1;
            glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

            for (int i = 0; i < uniformCount; i++)
            {
                int namelen = -1;
                int num = -1;
                char name[256] = { 0 };     // Assume no variable names longer than 256
                GLenum type = GL_ZERO;

                // Get the name of the uniforms
                glGetActiveUniform(id, i, sizeof(name) - 1, &namelen, &num, &type, name);

                name[namelen] = 0;
                TRACELOGD("SHADER: [ID %i] Active uniform (%s) set at location: %i", id, name, glGetUniformLocation(id, name));
            }
        }
        */
    }
#endif

    return id;
}

// Get shader location uniform
int rlGetLocationUniform(unsigned int shaderId, const char* uniformName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(shaderId, uniformName);

    //if (location == -1) TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to find shader uniform: %s", shaderId, uniformName);
    //else TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Shader uniform (%s) set at location: %i", shaderId, uniformName, location);
#endif
    return location;
}

// Get shader location attribute
int rlGetLocationAttrib(unsigned int shaderId, const char* attribName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetAttribLocation(shaderId, attribName);

    //if (location == -1) TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to find shader attribute: %s", shaderId, attribName);
    //else TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Shader attribute (%s) set at location: %i", shaderId, attribName, location);
#endif
    return location;
}

// Load shader from code strings and bind default locations
RLShader LoadShaderFromMemory(const char* vsCode, const char* fsCode)
{
    RLShader shader = { 0 };

    shader.id = rlLoadShaderCode(vsCode, fsCode);

    // After shader loading, we TRY to set default location names
    if (shader.id > 0)
    {
        // Default shader attribute locations have been binded before linking:
        //          vertex position location    = 0
        //          vertex texcoord location    = 1
        //          vertex normal location      = 2
        //          vertex color location       = 3
        //          vertex tangent location     = 4
        //          vertex texcoord2 location   = 5

        // NOTE: If any location is not found, loc point becomes -1

        shader.locs = (int*)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));

        // All locations reset to -1 (no location)
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

        // Get handles to GLSL input attribute locations
        shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
        shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
        shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
        shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);

        // Get handles to GLSL uniform locations (vertex shader)
        shader.locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
        shader.locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
        shader.locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
        shader.locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
        shader.locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);

        // Get handles to GLSL uniform locations (fragment shader)
        shader.locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
        shader.locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
        shader.locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
        shader.locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    }

    return shader;
}

// Get shader uniform location
int GetShaderLocation(RLShader shader, const char* uniformName)
{
    return rlGetLocationUniform(shader.id, uniformName);
}

// Get shader attribute location
int GetShaderLocationAttrib(RLShader shader, const char* attribName)
{
    return rlGetLocationAttrib(shader.id, attribName);
}

// Enable shader program
void rlEnableShader(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(id);
#endif
}

// Set shader value uniform
void rlSetUniform(int locIndex, const void* value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (uniformType)
    {
    case RL_SHADER_UNIFORM_FLOAT: glUniform1fv(locIndex, count, (float*)value); break;
    case RL_SHADER_UNIFORM_VEC2: glUniform2fv(locIndex, count, (float*)value); break;
    case RL_SHADER_UNIFORM_VEC3: glUniform3fv(locIndex, count, (float*)value); break;
    case RL_SHADER_UNIFORM_VEC4: glUniform4fv(locIndex, count, (float*)value); break;
    case RL_SHADER_UNIFORM_INT: glUniform1iv(locIndex, count, (int*)value); break;
    case RL_SHADER_UNIFORM_IVEC2: glUniform2iv(locIndex, count, (int*)value); break;
    case RL_SHADER_UNIFORM_IVEC3: glUniform3iv(locIndex, count, (int*)value); break;
    case RL_SHADER_UNIFORM_IVEC4: glUniform4iv(locIndex, count, (int*)value); break;
    case RL_SHADER_UNIFORM_SAMPLER2D: glUniform1iv(locIndex, count, (int*)value); break;
    default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set uniform value, data type not recognized");
    }
#endif
}


// Set shader value uniform matrix
void rlSetUniformMatrix(int locIndex, Matrix mat)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    float matfloat[16] = {
        mat.m0, mat.m1, mat.m2, mat.m3,
        mat.m4, mat.m5, mat.m6, mat.m7,
        mat.m8, mat.m9, mat.m10, mat.m11,
        mat.m12, mat.m13, mat.m14, mat.m15
    };
    glUniformMatrix4fv(locIndex, 1, false, matfloat);
#endif
}

// Set vertex attribute divisor
void rlSetVertexAttributeDivisor(unsigned int index, int divisor)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribDivisor(index, divisor);
#endif
}

// Disable shader program
void rlDisableShader(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(0);
#endif
}