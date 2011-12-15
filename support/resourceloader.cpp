#include "resourceloader.h"

#include <QGLShaderProgram>
#include <QList>
#include <QString>
#include <stdio.h>
#include "glm.h"
#include "rgbe/rgbe.h"
#include <iostream>
/**
  Loads the cube map into video memory.

  @param files: a list of files containing the cube map images (should be length
  six) in order
  @return the assigned OpenGL id to the cube map
**/
GLuint ResourceLoader::loadCubeMap(QList<QFile *> files)
{
    //Q_ASSERT(files.length() == 1);//For LDR: 6);

    // Generate an ID
    GLuint id;
    glGenTextures(1, &id);

    // Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    // Load and build mipmaps for each image
//    for (int i = 0; i < 6; ++i)
//    {
//        QImage image, texture;
//        image.load(files[0]->fileName());
//        image = image.mirrored(false, true);
//        texture = QGLWidget::convertToGLFormat(image);
//        texture = texture.scaledToWidth(1024, Qt::SmoothTransformation);
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 3, 3, texture.width(), texture.height(), 0, GL_RGBA,GL_UNSIGNED_BYTE, texture.bits());
//        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
//    }

    int height;
    int width;
    FILE *hdrfile = fopen("../final/textures/uffizi_cross.hdr", "rb");
    RGBE_ReadHeader(hdrfile, &width, &height, NULL);
    //TODO: may want to make this data accessible elsewhere?? also memory delete...
    float* hdrpix = new float[3*width*height];
    RGBE_ReadPixels_RLE(hdrfile, hdrpix, width, height);
    fclose(hdrfile);    
//    std::cout<<"check first pixels (should be black): "<<hdrpix[0]<<" "<<hdrpix[1]<<" "<<hdrpix[2]<<std::endl;
//    std::cout<<"check first row middle pixels (should be hdr color): "<<hdrpix[1152]<<" "<<hdrpix[1153]<<" "<<hdrpix[1154]<<std::endl;

    //break up cross file into 6 cube faces - specific to Debevec cross box layout for hdr images
    int face_width = int(float(width)/3.0);
    int face_height = int(float(height)/4.0);
    std::cout<<"face height and width: "<<face_height<<", "<<face_width<<std::endl;
    float* posx = new float[3*face_width*face_height];
    int ind = 0;
    for(int y = face_height; y < 2*face_height; y++)
    {
        for(int x = face_width-1; x >= 0 ; x--)
        {
            posx[ind] = hdrpix[3*(y*width+x)];
            posx[ind+1] = hdrpix[3*(y*width+x)+1];
            posx[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }
//    std::cout<<"size and ind: "<<3*face_width*face_height<<", "<<ind<<std::endl;
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, posx);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, posx);

    float* negx = new float[3*face_width*face_height];
    ind = 0;
    for(int y = face_height; y < 2*face_height; y++)
    {
        for(int x = 3*face_width-1; x >= 2*face_width ; x--)
        {
            negx[ind] = hdrpix[3*(y*width+x)];
            negx[ind+1] = hdrpix[3*(y*width+x)+1];
            negx[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, negx);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, negx);

    float* posy = new float[3*face_width*face_height];
    ind = 0;
    for(int y = 0; y < face_height; y++)
    {
        for(int x = 2*face_width-1; x >= face_width ; x--)
        {
            posy[ind] = hdrpix[3*(y*width+x)];
            posy[ind+1] = hdrpix[3*(y*width+x)+1];
            posy[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, posy);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, posy);

    float* negy = new float[3*face_width*face_height];
    ind = 0;
    for(int y = 2*face_height; y < 3*face_height; y++)
    {
        for(int x = 2*face_width-1; x >= face_width ; x--)
        {
            negy[ind] = hdrpix[3*(y*width+x)];
            negy[ind+1] = hdrpix[3*(y*width+x)+1];
            negy[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, negy);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, negy);

    float* posz = new float[3*face_width*face_height];
    ind = 0;
    for(int y = face_height; y < 2*face_height; y++)
    {
        for(int x = 2*face_width-1; x >= face_width ; x--)
        {
            posz[ind] = hdrpix[3*(y*width+x)];
            posz[ind+1] = hdrpix[3*(y*width+x)+1];
            posz[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, posz);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, posz);

    float* negz = new float[3*face_width*face_height];
    ind = 0;
    for(int y = 4*face_height-1; y >= 3*face_height; y--)
    {
        for(int x = face_width; x < 2*face_width ; x++)
        {
            negz[ind] = hdrpix[3*(y*width+x)];
            negz[ind+1] = hdrpix[3*(y*width+x)+1];
            negz[ind+2] = hdrpix[3*(y*width+x)+2];
            ind = ind+3;
        }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, 3, GL_RGBA16F, face_width, face_height, 0, GL_RGB,GL_FLOAT, negz);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, GL_RGBA16F, face_width, face_height, GL_RGB,GL_FLOAT, negz);


    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Set filter when pixel occupies more than one texture element
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    // Set filter when pixel smaller than one texture element
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    delete hdrpix;
    return id;
}

/**
    Loads an OBJ models from a file
  **/
Model ResourceLoader::loadObjModel(QString filePath)
{
    Model m;
    m.model = glmReadOBJ(filePath.toStdString().c_str());
    glmUnitize(m.model);
    m.idx = glmList(m.model, GLM_SMOOTH);
    return m;
}


/**
    Creates a call list for a skybox
  **/
GLuint ResourceLoader::loadSkybox()
{
    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);

    // Be glad we wrote this for you...ugh.
    glBegin(GL_QUADS);
    float extent = 100.f;
    glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( extent, -extent, -extent);
    glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-extent, -extent, -extent);
    glTexCoord3f(-1.0f,  1.0f, -1.0f); glVertex3f(-extent,  extent, -extent);
    glTexCoord3f( 1.0f,  1.0f, -1.0f); glVertex3f( extent,  extent, -extent);
    glTexCoord3f( 1.0f, -1.0f,  1.0f); glVertex3f( extent, -extent,  extent);
    glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( extent, -extent, -extent);
    glTexCoord3f( 1.0f,  1.0f, -1.0f); glVertex3f( extent,  extent, -extent);
    glTexCoord3f( 1.0f,  1.0f,  1.0f); glVertex3f( extent,  extent,  extent);
    glTexCoord3f(-1.0f, -1.0f,  1.0f); glVertex3f(-extent, -extent,  extent);
    glTexCoord3f( 1.0f, -1.0f,  1.0f); glVertex3f( extent, -extent,  extent);
    glTexCoord3f( 1.0f,  1.0f,  1.0f); glVertex3f( extent,  extent,  extent);
    glTexCoord3f(-1.0f,  1.0f,  1.0f); glVertex3f(-extent,  extent,  extent);
    glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-extent, -extent, -extent);
    glTexCoord3f(-1.0f, -1.0f,  1.0f); glVertex3f(-extent, -extent,  extent);
    glTexCoord3f(-1.0f,  1.0f,  1.0f); glVertex3f(-extent,  extent,  extent);
    glTexCoord3f(-1.0f,  1.0f, -1.0f); glVertex3f(-extent,  extent, -extent);
    glTexCoord3f(-1.0f,  1.0f, -1.0f); glVertex3f(-extent,  extent, -extent);
    glTexCoord3f(-1.0f,  1.0f,  1.0f); glVertex3f(-extent,  extent,  extent);
    glTexCoord3f( 1.0f,  1.0f,  1.0f); glVertex3f( extent,  extent,  extent);
    glTexCoord3f( 1.0f,  1.0f, -1.0f); glVertex3f( extent,  extent, -extent);
    glTexCoord3f(-1.0f, -1.0f, -1.0f); glVertex3f(-extent, -extent, -extent);
    glTexCoord3f(-1.0f, -1.0f,  1.0f); glVertex3f(-extent, -extent,  extent);
    glTexCoord3f( 1.0f, -1.0f,  1.0f); glVertex3f( extent, -extent,  extent);
    glTexCoord3f( 1.0f, -1.0f, -1.0f); glVertex3f( extent, -extent, -extent);
    glEnd();
    glEndList();

    return id;
}

/**
    Creates a new shader program from a vert shader only
  **/
QGLShaderProgram * ResourceLoader::newVertShaderProgram(const QGLContext *context, QString vertShader)
{
    QGLShaderProgram *program = new QGLShaderProgram(context);
    program->addShaderFromSourceFile(QGLShader::Vertex, vertShader);
    program->link();
    return program;
}

/**
    Creates a new shader program from a frag shader only
  **/
QGLShaderProgram * ResourceLoader::newFragShaderProgram(const QGLContext *context, QString fragShader)
{
    QGLShaderProgram *program = new QGLShaderProgram(context);
    program->addShaderFromSourceFile(QGLShader::Fragment, fragShader);
    program->link();
    return program;
}

/**
    Creates a shader program from both vert and frag shaders
  **/
QGLShaderProgram * ResourceLoader::newShaderProgram(const QGLContext *context, QString vertShader, QString fragShader)
{
    QGLShaderProgram *program = new QGLShaderProgram(context);
    program->addShaderFromSourceFile(QGLShader::Vertex, vertShader);
    program->addShaderFromSourceFile(QGLShader::Fragment, fragShader);
    program->link();
    return program;
}
