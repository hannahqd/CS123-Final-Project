#include "glwidget.h"

#include <iostream>
#include <QFileDialog>
#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QMouseEvent>
#include <QTime>
#include <QTimer>
#include <QWheelEvent>
#include "glm.h"

using std::cout;
using std::endl;

extern "C"
{
    extern void APIENTRY glActiveTexture(GLenum);
}

static const int MAX_FPS = 120;

/**
  Constructor.  Initialize all member variables here.
 **/
GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
    m_timer(this), m_prevTime(0), m_prevFps(0.f), m_fps(0.f),
    m_font("Deja Vu Sans Mono", 8, 4)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_camera.center = Vector3(0.f, 0.f, 0.f);
    m_camera.up = Vector3(0.f, 1.f, 0.f);
    m_camera.zoom = 3.5f;
    m_camera.theta = M_PI * 1.5f, m_camera.phi = 0.2f;
    m_camera.fovy = 60.f;

    m_exp = 1.0;
    m_isHDR = true;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
  Destructor.  Delete any 'new'ed objects here.
 **/
GLWidget::~GLWidget()
{
    foreach (QGLShaderProgram *sp, m_shaderPrograms)
        delete sp;
    foreach (QGLFramebufferObject *fbo, m_framebufferObjects)
        delete fbo;
    glDeleteLists(m_skybox, 1);
    const_cast<QGLContext *>(context())->deleteTexture(m_cubeMap);
    glmDelete(m_dragon.model);
}

/**
  Initialize the OpenGL state and start the drawing loop.
 **/
void GLWidget::initializeGL()
{
    // Set up OpenGL
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDisable(GL_DITHER);

   // glDisable(GL_LIGHTING);
    // Enable color materials with ambient and diffuse lighting terms
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set up global (ambient) lighting
    GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    // Set up GL_LIGHT0 with a position and lighting properties
    GLfloat ambientLight0[] = {0.25f, 0.1625f, 0.05f, 1.0f};
    GLfloat diffuseLight0[] = { 2.0f, 1.0f, 0.4, 1.0f };
    //GLfloat specularLight0[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position0[] = { 10.0f, 10.0f, 50.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight0);
    glLightfv(GL_LIGHT0, GL_POSITION, position0);

 /*   // Set up GL_LIGHT1 with a position and lighting properties
    GLfloat ambientLight1[] = {0.1f, 0.2f, 0.5f, 1.0f};
    GLfloat diffuseLight1[] = { 1.0f, 0.0f, 1.0, 1.0f };
    GLfloat specularLight1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position1[] = { 4.0f, -5.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight1);
    glLightfv(GL_LIGHT1, GL_POSITION, position1);
*/


    // Set up material properties
    GLfloat shiny = 25;
    GLfloat ambientMat[] = {0.0f, 0.0f, 0.0f, 0.0f};
    GLfloat diffuseMat[] = { 0.0f, 0.0f, 0.0, 0.0f };
    GLfloat specularMat[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientMat);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseMat);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMat);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shiny);

    glShadeModel(GL_FLAT);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Load resources, including creating shader programs and framebuffer objects
    initializeResources();

    // Start the drawing timer
    m_timer.start(1000.0f / MAX_FPS);
}

/**
  Initialize all resources.
  This includes models, textures, call lists, shader programs, and framebuffer objects.
 **/
void GLWidget::initializeResources()
{
    cout << "Using OpenGL Version " << glGetString(GL_VERSION) << endl << endl;
    // Ideally we would now check to make sure all the OGL functions we use are supported
    // by the video card.  But that's a pain to do so we're not going to.
    cout << "--- Loading Resources ---" << endl;

    m_dragon = ResourceLoader::loadObjModel("/course/cs123/data/mesh/sphere.obj");
    cout << "Loaded dragon..." << endl;

    char* cube_map = "../final/textures/stpeters_cross.hdr";
    loadCubeMap(cube_map);
    cout << "Loaded cube map..." << endl;

    m_skybox = ResourceLoader::loadSkybox(m_cubeMap);
    cout << "Loaded skybox..." << endl;
    createShaderPrograms();
    cout << "Loaded shader programs..." << endl;

    createFramebufferObjects(width(), height());
    cout << "Loaded framebuffer objects..." << endl;

    cout << " --- Finish Loading Resources ---" << endl;
}

/**
  Load a cube map for the skybox
 **/
void GLWidget::loadCubeMap(char* filename)
{
    m_cubeMap = ResourceLoader::loadCubeMap(filename);
}

/**
  Create shader programs.
 **/
void GLWidget::createShaderPrograms()
{
    const QGLContext *ctx = context();
    m_shaderPrograms["reflect"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/reflect.vert",
                                                                   "../final/shaders/reflect.frag");
    m_shaderPrograms["shadow"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/shadow.vert",
                                                                   "../final/shaders/shadow.frag");
    m_shaderPrograms["refract"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/refract.vert",
                                                                   "../final/shaders/refract.frag");
    m_shaderPrograms["basic"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/basic.vert",
                                                                   "../final/shaders/basic.frag");

    m_shaderPrograms["brightpass"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/brightpass.frag");
    m_shaderPrograms["blur"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/blur.frag");

    m_shaderPrograms["bilat"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/bilat.frag");

    m_shaderPrograms["tonemap"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/tonemap.frag");
}

/**
  Allocate framebuffer objects.

  @param width: the viewport width
  @param height: the viewport height
 **/
void GLWidget::createFramebufferObjects(int width, int height)
{
    // Allocate the main framebuffer object for rendering the scene to
    // This needs a depth attachment
    m_framebufferObjects["fbo_0"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);
    m_framebufferObjects["fbo_0"]->format().setSamples(16);
    // Allocate the secondary framebuffer obejcts for rendering textures to (post process effects)
    // These do not require depth attachments
    m_framebufferObjects["fbo_1"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);
    //
    m_framebufferObjects["fbo_2"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);

    m_framebufferObjects["fbo_3"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);

    m_framebufferObjects["fbo_4"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);
}

/**
  Called to switch to an orthogonal OpenGL camera.
  Useful for rending a textured quad across the whole screen.

  @param width: the viewport width
  @param height: the viewport height
**/
void GLWidget::applyOrthogonalCamera(float width, float height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  Called to switch to a perspective OpenGL camera.

  @param width: the viewport width
  @param height: the viewport height
**/
void GLWidget::applyPerspectiveCamera(float width, float height)
{
    float ratio = ((float) width) / height;
    Vector3 dir(-Vector3::fromAngles(m_camera.theta, m_camera.phi));
    Vector3 eye(m_camera.center - dir * m_camera.zoom);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(m_camera.fovy, ratio, 0.1f, 1000.f);
    gluLookAt(eye.x, eye.y, eye.z, eye.x + dir.x, eye.y + dir.y, eye.z + dir.z,
              m_camera.up.x, m_camera.up.y, m_camera.up.z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  Draws the scene to a buffer which is rendered to the screen when this function exits.
 **/
void GLWidget::paintGL()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the fps
    int time = m_clock.elapsed();
    m_fps = 1000.f / (time - m_prevTime);
    m_prevTime = time;
    int width = this->width();
    int height = this->height();

    if(!m_isHDR)
    {
        applyPerspectiveCamera(width, height);
        renderScene();
    }
    else
    {
        // Render the scene to a framebuffer
        m_framebufferObjects["fbo_0"]->bind();
        applyPerspectiveCamera(width, height);
        renderScene();
        m_framebufferObjects["fbo_0"]->release();

        // Copy the rendered scene into framebuffer 1
        m_framebufferObjects["fbo_0"]->blitFramebuffer(m_framebufferObjects["fbo_1"],
                                                       QRect(0, 0, width, height), m_framebufferObjects["fbo_0"],
                                                       QRect(0, 0, width, height), GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // TODO: Add drawing code here
        applyOrthogonalCamera(width, height);
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
        renderTexturedQuad(width, height, true);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_framebufferObjects["fbo_2"]->bind();
        m_shaderPrograms["tonemap"]->bind();
        m_shaderPrograms["tonemap"]->setUniformValue("exposure", m_exp);
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
        renderTexturedQuad(width, height, false);
        m_shaderPrograms["tonemap"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_framebufferObjects["fbo_2"]->release();

        // TODO: Uncomment this section in step 2 of the lab

        float scales[] = {4.f,8.f,16.f,32.f};
        for (int i = 0; i < 4; ++i)
        {
            // Render the blurred brightpass filter result to fbo 1
            renderBlur(width / scales[i], height / scales[i]);

            // Bind the image from fbo to a texture
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            // Enable alpha blending and render the texture to the screen
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glTranslatef(0.f, (scales[i] - 1) * -height, 0.f);
            renderTexturedQuad(width * scales[i], height * scales[i], false);
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }


    paintText();
}

/**
  Renders the scene.  May be called multiple times by paintGL() if necessary.
**/
void GLWidget::renderScene() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Enable cube maps and draw the skybox
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMap);
    glCallList(m_skybox);

    // Enable culling (back) faces for rendering the dragon
    glEnable(GL_CULL_FACE);

    // Render the dragon with the refraction shader bound
    glActiveTexture(GL_TEXTURE0);
    m_shaderPrograms["refract"]->bind();
    m_shaderPrograms["refract"]->setUniformValue("CubeMap", GL_TEXTURE0);
    glPushMatrix();
    glTranslatef(-1.25f, 0.f, 0.f);
    glCallList(m_dragon.idx);
    glPopMatrix();
    m_shaderPrograms["refract"]->release();

   // Vector3 eta = Vector3(0.75, 0.77, 0.8);
    // Render the dragon with the reflection shader bound
    m_shaderPrograms["reflect"]->bind();
    m_shaderPrograms["reflect"]->setUniformValue("envMap", GL_TEXTURE0);

    m_shaderPrograms["reflect"]->setUniformValue("eta1D", 0.9f);
    m_shaderPrograms["reflect"]->setUniformValue("etaR", 0.91f);
    m_shaderPrograms["reflect"]->setUniformValue("etaG", 0.93f);
    m_shaderPrograms["reflect"]->setUniformValue("etaB", 0.96f);

    m_shaderPrograms["reflect"]->setUniformValue("r0", 0.4f);
    glPushMatrix();
    glTranslatef(1.25f,0.f,0.f);
    glCallList(m_dragon.idx);
    glPopMatrix();
    m_shaderPrograms["reflect"]->release();

    // Disable culling, depth testing and cube maps
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);
}

void GLWidget::renderShadowScene() {
    // Enable depth testing

  //  m_framebufferObjects["fbo_3"]->bind();
    glPushMatrix();
    glTranslatef(0.f, -2.f, 10.f);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable culling (back) faces for rendering the dragon
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);

    // Render the first model
//    glActiveTexture(GL_TEXTURE0);
//    m_shaderPrograms["refract"]->bind();
//    m_shaderPrograms["refract"]->setUniformValue("CubeMap", GL_TEXTURE0);
    glPushMatrix();
    glTranslatef(-1.25f, 0.f, 0.f);
    glCallList(m_dragon.idx);
    glPopMatrix();
//    m_shaderPrograms["refract"]->release();

    // Render the second model
    glPushMatrix();
    glTranslatef(1.25f,0.f,0.f);
    glCallList(m_dragon.idx);
    glPopMatrix();

//    // Disable culling, depth testing and cube maps
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
//    m_framebufferObjects["fbo_3"]->release();

    glPopMatrix();

    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
}

/**
  Run a gaussian blur on the texture stored in fbo 2 and
  put the result in fbo 1.  The blur should have a radius of 2.

  @param width: the viewport width
  @param height: the viewport height
**/
void GLWidget::renderBlur(int width, int height)
{
    int radius = 3;
    int dim = radius * 2 + 1;
    GLfloat kernel[dim * dim];
    GLfloat offsets[dim * dim * 2];
    createBlurKernel(radius, width, height, &kernel[0], &offsets[0]);
    // TODO: Finish filling this in
    m_shaderPrograms["blur"]->setUniformValueArray('offsets', offsets, (dim*dim), 2);
    m_shaderPrograms["blur"]->setUniformValueArray('kernel', kernel, (dim*dim), 1);
    m_shaderPrograms["blur"]->setUniformValue('arraySize', dim*dim);
    m_framebufferObjects["fbo_1"]->bind();
    m_shaderPrograms["blur"]->bind();
    glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_2"]->texture());
    renderTexturedQuad(width, height, true);
    m_shaderPrograms["blur"]->release();
    glBindTexture(GL_TEXTURE_2D, 0);
    m_framebufferObjects["fbo_1"]->release();

}

/**
  Called when the mouse is dragged.  Rotates the camera based on mouse movement.
**/
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    Vector2 pos(event->x(), event->y());
    if (event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton)
    {
        m_camera.mouseMove(pos - m_prevMousePos);
    }
    m_prevMousePos = pos;
}

/**
  Record a mouse press position.
 **/
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_prevMousePos.x = event->x();
    m_prevMousePos.y = event->y();
}

/**
  Called when the mouse wheel is turned.  Zooms the camera in and out.
**/
void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() == Qt::Vertical)
    {
        m_camera.mouseWheel(event->delta());
    }
}

/**
  Called when the GLWidget is resized.
 **/
void GLWidget::resizeGL(int width, int height)
{
    // Resize the viewport
    glViewport(0, 0, width, height);

    // Reallocate the framebuffers with the new window dimensions
    foreach (QGLFramebufferObject *fbo, m_framebufferObjects)
    {
        const QString &key = m_framebufferObjects.key(fbo);
        QGLFramebufferObjectFormat format = fbo->format();
        delete fbo;
        m_framebufferObjects[key] = new QGLFramebufferObject(width, height, format);
    }
}

/**
  Draws a textured quad. The texture most be bound and unbound
  before and after calling this method - this method assumes that the texture
  has been bound before hand.

  @param w: the width of the quad to draw
  @param h: the height of the quad to draw
  @param flip: flip the texture vertically
**/
void GLWidget::renderTexturedQuad(int width, int height, bool flip) {
    // Clamp value to edge of texture when texture index is out of bounds
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Draw the  quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, flip ? 1.0f : 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, flip ? 1.0f : 0.0f);
    glVertex2f(width, 0.0f);
    glTexCoord2f(1.0f, flip ? 0.0f : 1.0f);
    glVertex2f(width, height);
    glTexCoord2f(0.0f, flip ? 0.0f : 1.0f);
    glVertex2f(0.0f, height);
    glEnd();
}

/**
  Creates a gaussian blur kernel with the specified radius.  The kernel values
  and offsets are stored.

  @param radius: The radius of the kernel to create.
  @param width: The width of the image.
  @param height: The height of the image.
  @param kernel: The array to write the kernel values to.
  @param offsets: The array to write the offset values to.
**/
void GLWidget::createBlurKernel(int radius, int width, int height,
                                                    GLfloat* kernel, GLfloat* offsets)
{
    int size = radius * 2 + 1;
    float sigma = radius / 3.0f;
    float twoSigmaSigma = 2.0f * sigma * sigma;
    float rootSigma = sqrt(twoSigmaSigma * M_PI);
    float total = 0.0f;
    float xOff = 1.0f / width, yOff = 1.0f / height;
    int offsetIndex = 0;
    for (int y = -radius, idx = 0; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x,++idx)
        {
            float d = x * x + y * y;
            kernel[idx] = exp(-d / twoSigmaSigma) / rootSigma;
            total += kernel[idx];
            offsets[offsetIndex++] = x * xOff;
            offsets[offsetIndex++] = y * yOff;
        }
    }
    for (int i = 0; i < size * size; ++i)
    {
        kernel[i] /= total;
    }
}

/**
  Handles any key press from the keyboard
 **/
void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_S:
        {
            QImage qi = grabFrameBuffer(false);
            QString filter;
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("PNG Image (*.png)"), &filter);
            qi.save(QFileInfo(fileName).absoluteDir().absolutePath() + "/" + QFileInfo(fileName).baseName() + ".png", "PNG", 100);
        }
        break;

        case Qt::Key_O:
        {
            QString filter;
            QString fn = QFileDialog::getOpenFileName(this, tr("Open Skybox Image"), "", tr("HDR Image (*.hdr)"), &filter);
            QByteArray ba = fn.toLocal8Bit();
            char * fileName = ba.data();
            loadCubeMap(fileName);
            cout << "Loaded new cube map..." << endl;
        }
        break;
        case Qt::Key_E:
        {
            m_exp += 0.1;
            paintGL();
        }
        break;
        case Qt::Key_D:
        {
            m_exp -= 0.1;
            paintGL();
        }
        break;
        case Qt::Key_H:
        {
            m_isHDR = true;
            paintGL();
        }
        break;
        case Qt::Key_L:
        {
            m_isHDR = false;
            paintGL();
        }
        break;
    }
}

/**
  Draws text for the FPS and screenshot prompt
 **/
void GLWidget::paintText()
{
    glColor3f(1.f, 1.f, 1.f);

    // Combine the previous and current framerate
    if (m_fps >= 0 && m_fps < 1000)
    {
       m_prevFps *= 0.95f;
       m_prevFps += m_fps * 0.05f;
    }

    // QGLWidget's renderText takes xy coordinates, a string, and a font
    renderText(10, 20, "FPS: " + QString::number((int) (m_prevFps)), m_font);
    renderText(10, 35, "S: Save screenshot", m_font);
    renderText(10, 50, "O: Open new texture", m_font);
    renderText(10, 65, "E: Increase exposure", m_font);
    renderText(10, 80, "H: HDR scene", m_font);
    renderText(10, 95, "L: LDR scene", m_font);

}
