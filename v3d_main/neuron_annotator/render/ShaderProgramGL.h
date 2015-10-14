/*
 * ShaderProgramGlSl.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef SHADERPROGRAMGLSL_H_
#define SHADERPROGRAMGLSL_H_

#include "../../3drenderer/GLee_r.h"
#include "boost/shared_ptr.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * Either a vertex shader or fragment shader.
 * Part of a ShaderProgram.
 */
class ShaderPhase
{
public:
    ShaderPhase(int shaderType /* = GL_VERTEX_SHADER */
            , std::string shaderSource)
        : shaderType(shaderType)
        , shaderId(0)
        , isInitialized(false)
        , shaderSource(shaderSource)
    {}

    virtual ~ShaderPhase()
    {
        if (shaderId != 0) {
            glDeleteShader(shaderId);
            shaderId = 0;
        }
    }

    GLint id() const {return shaderId;}

    /*
     * Construct shader phase,
     * but only after an OpenGL context is enabled.
     */
    virtual bool initGL()
    {
        if (isInitialized)
            return true;
        if (shaderId == 0)
            shaderId = glCreateShader(shaderType);
        if (shaderId == 0)
            return false;
        const char* sourceArray = shaderSource.c_str();
        int sizeArray = shaderSource.length();
        glShaderSource(shaderId, 1,
                &sourceArray,
                &sizeArray);
        glCompileShader(shaderId);
        const int maxLogLength = 1000;
        char log[maxLogLength];
        int logLength;
        glGetShaderInfoLog(shaderId, maxLogLength,
                &logLength, log);
        if (logLength > 0) {
            // TODO - possibly an error
            return false;
        }
        isInitialized = true;
        return true;
    }

    void loadSourceFromFile(std::string fileName)
    {
        std::ifstream f(fileName.c_str());
        loadSourceFromFile(f);
    }

    void loadSourceFromFile(std::istream& is)
    {
        // http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << is.rdbuf();
        setSourceCode(buffer.str());
    }

    void setSourceCode(std::string source)
    {
        if (source != shaderSource) {
            shaderSource = source;
            isInitialized = false;
        }
    }

protected:
    int shaderType;
    GLint shaderId;
    bool isInitialized;
    std::string shaderSource;
};

class ShaderProgramGL
{
public:
    friend class Activator;

    /**
     * Allocate an Activator on the stack to use a shader program
     */
    class Activator
    {
    public:
        Activator(ShaderProgramGL& shader)
            : shader(shader)
            , previousProgramId(0)
        {
            glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgramId);
            glUseProgram(shader.programId);
        }

        virtual ~Activator()
        {
            glUseProgram(previousProgramId);
        }

    protected:
        ShaderProgramGL& shader;
        GLint previousProgramId;
    };

public:

    ShaderProgramGL()
        : programId(0)
        , isInitialized(false)
    {}

    virtual ~ShaderProgramGL()
    {
        for (ShaderList::iterator s = shaders.begin(); s != shaders.end(); ++s)
            glDetachShader(programId, (*s)->id());
    }

    /*
     * Creates a stack-allocated object that uses this shader program
     * while the object is in scope.
     * OpenGL context must be active when calling getActivator().
     */
    virtual Activator getActivator()
    {
        if (! isInitialized)
            initGL();
        return Activator(*this);
    }

    virtual bool initGL()
    {
        if (isInitialized)
            return true;
        bool result = true;
        for (ShaderList::iterator s = shaders.begin(); s != shaders.end(); ++s)
        {
            ShaderPhase& shader = **s;
            if (shader.initGL()) {
                glAttachShader(programId, shader.id());
            }
            else
                result = false;
        }
        if (result)
        {
            glLinkProgram(programId);
            isInitialized = true;
        }
        return result;
    }

protected:
    bool isInitialized;
    GLint programId;
    typedef boost::shared_ptr<ShaderPhase> ShaderPtr;
    typedef std::vector<ShaderPtr> ShaderList;
    ShaderList shaders;
};

#endif /* SHADERPROGRAMGLSL_H_ */
