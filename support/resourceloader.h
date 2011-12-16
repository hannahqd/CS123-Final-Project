#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QFile>
#include <QGLShaderProgram>
#include "glm.h"

/**
    A simple model struct
 **/
struct Model
{
    GLMmodel *model;
    GLuint idx;
};

/**
   A resource loader with code to handle loading models, skyboxes, and shader programs.

  @author: Justin Ardini (jardini)
   **/
namespace ResourceLoader
{
    // Returns the model
    Model loadObjModel(QString filePath);

    // Returns the skybox ID
    GLuint loadSkybox(GLuint cubeMapID);

    // These return a new QGLShaderProgram.  THIS MUST BE DELETED BY THE CALLER.
    QGLShaderProgram * newVertShaderProgram(const QGLContext *context, QString vertShader);
    QGLShaderProgram * newFragShaderProgram(const QGLContext *context, QString fragShader);
    QGLShaderProgram * newShaderProgram(const QGLContext *context, QString vertShader, QString fragShader);

    // Returns the cubeMap ID
    GLuint loadCubeMap(char* filename);//(QList<QFile *> files);

}

#endif // RESOURCELOADER_H
