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
#include <math.h>

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

    m_exp = 0.50;
    m_isHDR = true;
    m_isBilat = false;
    m_isEdges = false;
    m_increment = 0.0;
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
    glmDelete(m_sphere.model);
    glmDelete(m_model1.model);
    glmDelete(m_model2.model);
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

    glDisable(GL_LIGHTING);
    // Enable color materials with ambient and diffuse lighting terms
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

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


    m_dragon = ResourceLoader::loadObjModel("/course/cs123/data/mesh/dragon.obj");
    cout << "Loaded dragon..." << endl;

    m_sphere = ResourceLoader::loadObjModel("/course/cs123/data/mesh/sphere.obj");
    cout << "Loaded sphere..." << endl;

    m_elephant = ResourceLoader::loadObjModel("/home/gen/courses/Courses_Fall2011/cs123/final/models/elephal.obj");
    cout << "Loaded elephant..." << endl;

    m_model1 = ResourceLoader::loadObjModel("/course/cs123/data/mesh/objAnotexture.obj");
    cout << "Loaded polygon-a-mawhatsit..." << endl;

    m_model2 = ResourceLoader::loadObjModel("/course/cs123/data/mesh/piano.obj");
    cout << "Loaded piano..." << endl;


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
    m_shaderPrograms["refractFres"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/refractFres.vert",
                                                                   "../final/shaders/refractFres.frag");
    m_shaderPrograms["basic"] = ResourceLoader::newShaderProgram(ctx, "../final/shaders/basic.vert",
                                                                   "../final/shaders/basic.frag");

    m_shaderPrograms["brightpass"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/brightpass.frag");
    m_shaderPrograms["blur"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/blur.frag");

    m_shaderPrograms["bilat"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/bilat.frag");
    m_shaderPrograms["bilat_high"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/bilat_high.frag");

    m_shaderPrograms["tonemap"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/tonemap.frag");
    m_shaderPrograms["color"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/color.frag");
    m_shaderPrograms["combine"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/combine.frag");
    m_shaderPrograms["tester"] = ResourceLoader::newFragShaderProgram(ctx, "../final/shaders/tester.frag");
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

    m_framebufferObjects["fbo_3"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);

    m_framebufferObjects["fbo_4"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);
    m_framebufferObjects["fbo_5"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D, GL_RGB16F_ARB);
    m_framebufferObjects["fbo_6"] = new QGLFramebufferObject(width, height, QGLFramebufferObject::NoAttachment,
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


float gauss(float coeff, float sigsqX2, int x){
    float gaussian = (coeff)*(pow(M_E, ((-(x * x))/sigsqX2)));
    return gaussian;
}

void computeKernel(float* kern, int rad){
    int kernLen = (2 * rad) + 1;
    float sigma = (rad/3.0);
    float sr2pi = (sqrt(2 * M_PI));
    float sigXsr2pi = (sigma * sr2pi);
    float sigsqX2 = (2 * (sigma * sigma));
    float coeff = (1/sigXsr2pi);
    for(int i = 0; i < kernLen; i++){
      int x = rad - i;
      kern[i] = gauss(coeff, sigsqX2, x);
  }
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

        applyOrthogonalCamera(width, height);

        //render with global tone mapping
        if(!m_isBilat)
        {


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

            float scales[] = {4.f,8.f,16.f,32.f};
            for (int i = 0; i < 1; ++i)//4; ++i)
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
        //render with bilateral filter mapping
        else
        {

        m_framebufferObjects["fbo_3"]->bind();
        m_shaderPrograms["bilat"]->bind();
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
        renderTexturedQuad(width, height, false);
        m_shaderPrograms["bilat"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_framebufferObjects["fbo_3"]->release();

        m_framebufferObjects["fbo_2"]->bind();
        m_shaderPrograms["tonemap"]->bind();
        m_shaderPrograms["tonemap"]->setUniformValue("exposure", m_exp);
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_3"]->texture());
        renderTexturedQuad(width, height, true);
        m_shaderPrograms["tonemap"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_framebufferObjects["fbo_2"]->release();



        m_framebufferObjects["fbo_4"]->bind();
        m_shaderPrograms["bilat_high"]->bind();
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
        renderTexturedQuad(width, height, false);
        m_shaderPrograms["bilat_high"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_framebufferObjects["fbo_4"]->release();

        if(m_isEdges){
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_4"]->texture());
            renderTexturedQuad(width, height, false);

            paintText();
            return;
        }

        m_framebufferObjects["fbo_5"]->bind();
        m_shaderPrograms["color"]->bind();
        glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
        renderTexturedQuad(width, height, false);
        m_shaderPrograms["color"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);
        m_framebufferObjects["fbo_5"]->release();

        m_framebufferObjects["fbo_6"]->bind();

            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_2"]->texture());
            renderTexturedQuad(width, height, false);
            m_shaderPrograms["bilat"]->release();
            glBindTexture(GL_TEXTURE_2D, 0);
            m_framebufferObjects["fbo_3"]->release();

            m_framebufferObjects["fbo_2"]->bind();
            m_shaderPrograms["tonemap"]->bind();
            m_shaderPrograms["tonemap"]->setUniformValue("exposure", m_exp);
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_3"]->texture());
            renderTexturedQuad(width, height, true);
            m_shaderPrograms["tonemap"]->release();
            glBindTexture(GL_TEXTURE_2D, 0);
            m_framebufferObjects["fbo_2"]->release();


            m_framebufferObjects["fbo_4"]->bind();
            m_shaderPrograms["bilat_high"]->bind();
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
            renderTexturedQuad(width, height, false);
            m_shaderPrograms["bilat_high"]->release();
            glBindTexture(GL_TEXTURE_2D, 0);
            m_framebufferObjects["fbo_4"]->release();

            m_framebufferObjects["fbo_5"]->bind();
            m_shaderPrograms["color"]->bind();
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_1"]->texture());
            renderTexturedQuad(width, height, false);
            m_shaderPrograms["color"]->release();
            glBindTexture(GL_TEXTURE_2D, 0);
            m_framebufferObjects["fbo_5"]->release();

            m_framebufferObjects["fbo_6"]->bind();

                glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_2"]->texture());
                renderTexturedQuad(width, height, false);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_4"]->texture());

                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                renderTexturedQuad(width, height, false);
                glDisable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_5"]->texture());

                glEnable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
                renderTexturedQuad(width, height, false);
                glDisable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D, 0);


            m_framebufferObjects["fbo_6"]->release();

            m_shaderPrograms["combine"]->bind();
            glBindTexture(GL_TEXTURE_2D, m_framebufferObjects["fbo_6"]->texture());
            renderTexturedQuad(width, height, true);
            m_shaderPrograms["combine"]->release();
            glBindTexture(GL_TEXTURE_2D, 0);



        }


    }


    paintText();

}

int fact(int n){
    if(n <= 0)
        return 1;
    else
        return (n * fact(n - 1));
}

float bernstein(int n, int j, float t){
    float b = fact(n)/((fact(j) * fact(n - j))) * pow(t, j) * pow((1-t), (n-j));
    return b;
}
//
//n = # petal
//P = [];
//at = 1/3*pi;
//a1 = n/3*pi;
//r1 = 1;
//r2 = 3;
//r3 = sqrt(r1*r1+r2*r2);
//a2 = pi/2 - at/2;
//a3 = asin((r2*sin(a2))/r3);
//P = [r1*cos(a1); r1*sin(a1); 0];
//P = [P [r3*cos(a1+at-a3); r3*sin(a1+at-a3); 0]];
//P = [P [r3*cos(a1+a3); r3*sin(a1+a3); 0]];
//P = [P [r1*cos(a1+at); r1*sin(a1+at); 0]];
//
//LOOP
//n = # loop
//P = [];
//at = 1/3*pi;
//a1 = n/3*pi;
//r1 = 1;
//r2 = 3;
//r3 = sqrt(r1*r1+r2*r2);
//a2 = pi/2 - at/2;
//a3 = asin((r2*sin(a2))/r3);
//P = [r1*cos(a1); r1*sin(a1); 0];
//P = [P [r3*cos(a1+a3); r3*sin(a1+a3); 0]];
//P = [P [r3*cos(a1+at-a3); r3*sin(a1+at-a3); 0]];
//P = [P [r1*cos(a1+at); r1*sin(a1+at); 0]];


// xs and ys are size 4, petal between 0 and 5
void makeBezPet(float* xs, float* ys, int petal){
    float at = M_PI/3.0;
    float a1 = (petal * M_PI)/3.0;
    float r1 = 2.0;
    float r2 = 3.0;
    float r3 = sqrt((r1 * r1) + (r2 * r2));
    float a2 = (M_PI/2.0) - (at/2.0);
    float a3 = asin((r2 * sin(a2))/r3);
    xs[0] = r1 * cos(a1);
    xs[1] = r3 * cos(a1+at-a3);
    xs[2] = r3 * cos(a1+a3);
    xs[3] = r1 * cos(a1+at);

    ys[0] = r1*sin(a1);
    ys[1] = r3*sin(a1+at-a3);
    ys[2] = r3*sin(a1+a3);
    ys[3] = r1*sin(a1+at);
}

void makeBezLoop(float* xs, float* ys, int petal){
    float at = M_PI/3.0;
    float a1 = (petal * M_PI)/3.0;
    float r1 = 2.0;
    float r2 = 3.0;
    float r3 = sqrt((r1 * r1) + (r2 * r2));
    float a2 = (M_PI/2.0) - (at/2.0);
    float a3 = asin((r2 * sin(a2))/r3);
    xs[1] = r1 * cos(a1);
    xs[0] = r3*cos(a1+at-a3);
    xs[3] = r3*cos(a1+a3);
    xs[2] = r1*cos(a1+at);

    ys[1] = r1*sin(a1);
    ys[0] = r3*sin(a1+at-a3);
    ys[3] = r3*sin(a1+a3);
    ys[2] = r1*sin(a1+at);
}

/**
  Renders the scene.  May be called multiple times by paintGL() if necessary.
**/
void GLWidget::renderScene() {

    float div_val = 1.0;
    if(!m_isBilat)
    {
    if(!m_isHDR){
        div_val = 250;
    }
    else{
        div_val = 100;
    }

    float time = m_increment++ / div_val;

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

    float pxs[13] = {-10, -5, 0, 5, 10, 7.5, 10, 5, 0, -5, -10, -7.5, -10};
    float pys[13] = {10, 10, 15, 10, 10, 0, -10, -10, 15, -10, -10, 0, 10};

    for(int i = 0; i < 13; i++){
        pxs[i] *= 0.8;
        pys[i] *= 0.8;
    }

    float pianoxs[8] = {4.7, 7.7, 19.6, 11.9, 13.8, 9.9, 14.1, 4.7};
    float pianozs[8] = {12.9, 7.1, 10.1, 29.1, 20.6, 19.2, 12.4, 12.9};
    float pianoys[8] = {0.0, 1.0, -1.0, 0.0, 1.0, -1.0, 5.0, 0.0};

    //Make star points...

    float t = fmod(time, 1);
    float tpiano = fmod(time, 6);
    float px = 0;
    float py = 0;

    float pianox = 0;
    float pianoy = 0;
    float pianoz = 0;

    for(int j = 0; j < 13; j++){
        px += pxs[j] * bernstein(12, j, t);
        py += pys[j] * bernstein(12, j, t);
    }

    for(int j = 0; j < 8; j++){
        pianox += pianoxs[j] * bernstein(7, j, tpiano/2) - 1;
        pianoy += pianoys[j] * bernstein(7, j, tpiano/2);
        pianoz += pianozs[j] * bernstein(7, j, tpiano/2) - 2;
    }

    float arcx = 0;
    float arcy = 0;

    float* arc0xs = new float[4];
    float* arc0ys = new float[4];
    float* arc1xs = new float[4];
    float* arc1ys = new float[4];
    float* arc2xs = new float[4];
    float* arc2ys = new float[4];
    float* arc3xs = new float[4];
    float* arc3ys = new float[4];
    float* arc4xs = new float[4];
    float* arc4ys = new float[4];
    float* arc5xs = new float[4];
    float* arc5ys = new float[4];

    makeBezPet(arc0xs, arc0ys, 0);
    makeBezPet(arc1xs, arc1ys, 1);
    makeBezPet(arc2xs, arc2ys, 2);
    makeBezPet(arc3xs, arc3ys, 3);
    makeBezPet(arc4xs, arc4ys, 4);
    makeBezPet(arc5xs, arc5ys, 5);

//    for(int i = 0; i < 4; i++){
//        std::cout<<"xarc0: "<<arc0xs[i]<<std::endl;
//        std::cout<<"yarc0: "<<arc0ys[i]<<std::endl;
//    }
//    for(int i = 0; i < 4; i++){
//        std::cout<<"xarc1: "<<arc1xs[i]<<std::endl;
//        std::cout<<"yarc1: "<<arc1ys[i]<<std::endl;
//    }
   // std::cout<<arc1xs[1]<<std::endl;


    if((tpiano >= 0.85) && (tpiano < 1.85)){
        for(int j = 0; j < 4; j++){
            arcx += arc1xs[j] * bernstein(4, j, (tpiano - 1));
            arcy += arc1ys[j] * bernstein(4, j, (tpiano - 1));
        }
    }
    else if((tpiano >= 1.85) && (tpiano < 2.85)){
        for(int j = 0; j < 4; j++){
            arcx += arc2xs[j] * bernstein(4, j, (tpiano - 2));
            arcy += arc2ys[j] * bernstein(4, j, (tpiano - 2));
        }
    }
    else if((tpiano >= 2.85) && (tpiano < 3.85)){
        for(int j = 0; j < 4; j++){
            arcx += arc3xs[j] * bernstein(4, j, (tpiano - 3));
            arcy += arc3ys[j] * bernstein(4, j, (tpiano - 3));
        }
    }
    else if((tpiano >=  3.85) && (tpiano < 4.85)){
        for(int j = 0; j < 4; j++){
            arcx += arc4xs[j] * bernstein(4, j, (tpiano - 4));
            arcy += arc4ys[j] * bernstein(4, j, (tpiano - 4));
        }
    }
    else if((tpiano >= 4.85) && (tpiano < 5.85)){
        for(int j = 0; j < 4; j++){
            arcx += arc5xs[j] * bernstein(4, j, (tpiano - 5));
            arcy += arc5ys[j] * bernstein(4, j, (tpiano - 5));
        }
    }
    else{
        for(int j = 0; j < 4; j++){
            arcx += arc0xs[j] * bernstein(4, j, tpiano);
            arcy += arc0ys[j] * bernstein(4, j, tpiano);
        }
    }

    glPushMatrix();
        glTranslatef(arcx, arcy, 0);
        glScalef(0.3f, 0.3f, 0.3f);
        glCallList(m_model2.idx);
    glPopMatrix();


    float scoopx = 0;
    float scoopy = 0;

    float* scoop0xs = new float[4];
    float* scoop0ys = new float[4];
    float* scoop1xs = new float[4];
    float* scoop1ys = new float[4];
    float* scoop2xs = new float[4];
    float* scoop2ys = new float[4];
    float* scoop3xs = new float[4];
    float* scoop3ys = new float[4];
    float* scoop4xs = new float[4];
    float* scoop4ys = new float[4];
    float* scoop5xs = new float[4];
    float* scoop5ys = new float[4];

    makeBezLoop(scoop0xs, scoop0ys, 0);
    makeBezLoop(scoop1xs, scoop1ys, 1);
    makeBezLoop(scoop2xs, scoop2ys, 2);
    makeBezLoop(scoop3xs, scoop3ys, 3);
    makeBezLoop(scoop4xs, scoop4ys, 4);
    makeBezLoop(scoop5xs, scoop5ys, 5);

//    for(int i = 0; i < 4; i++){
//        std::cout<<"xscoop0: "<<scoop0xs[i]<<std::endl;
//        std::cout<<"yscoop0: "<<scoop0ys[i]<<std::endl;
//    }
//    for(int i = 0; i < 4; i++){
//        std::cout<<"xscoop1: "<<scoop1xs[i]<<std::endl;
//        std::cout<<"yscoop1: "<<scoop1ys[i]<<std::endl;
//    }
   // std::cout<<scoop1xs[1]<<std::endl;


    if((tpiano >= 0.92) && (tpiano < 1.85)){
        for(int j = 0; j < 4; j++){
            scoopx += scoop1xs[j] * bernstein(4, j, (tpiano - 1));
            scoopy += scoop1ys[j] * bernstein(4, j, (tpiano - 1));
        }
    }
    else if((tpiano >= 1.92) && (tpiano < 2.85)){
        for(int j = 0; j < 4; j++){
            scoopx += scoop2xs[j] * bernstein(4, j, (tpiano - 2));
            scoopy += scoop2ys[j] * bernstein(4, j, (tpiano - 2));
        }
    }
    else if((tpiano >= 2.92) && (tpiano < 3.85)){
        for(int j = 0; j < 4; j++){
            scoopx += scoop3xs[j] * bernstein(4, j, (tpiano - 3));
            scoopy += scoop3ys[j] * bernstein(4, j, (tpiano - 3));
        }
    }
    else if((tpiano >=  3.92) && (tpiano < 4.85)){
        for(int j = 0; j < 4; j++){
            scoopx += scoop4xs[j] * bernstein(4, j, (tpiano - 4));
            scoopy += scoop4ys[j] * bernstein(4, j, (tpiano - 4));
        }
    }
    else if((tpiano >= 4.92) && (tpiano < 5.85)){
        for(int j = 0; j < 4; j++){
            scoopx += scoop5xs[j] * bernstein(4, j, (tpiano - 5));
            scoopy += scoop5ys[j] * bernstein(4, j, (tpiano - 5));
        }
    }
    else{
        for(int j = 0; j < 4; j++){
            scoopx += scoop0xs[j] * bernstein(4, j, tpiano);
            scoopy += scoop0ys[j] * bernstein(4, j, tpiano);
        }
    }
    glPushMatrix();
        glTranslatef(scoopx, 0, scoopy);
        glScalef(0.3f, 0.3f, 0.3f);
        glCallList(m_model1.idx);
    glPopMatrix();

//    glPushMatrix();
//    glTranslatef(px, py, 0);
//    glScalef(0.5f, 0.5f, 0.5f);
//    glCallList(m_model1.idx);
//    glPopMatrix();
//

    glPushMatrix();
        float rad =1.5f;
        float a1 = -rad*cos(fmod(time, (2*M_PI)));
        float a2 = rad*sin(fmod(time, (2*M_PI)));
        float a3 = 0.f;
        glTranslatef(a1, a2, a3);
        glScalef(0.3f, 0.3f, 0.3f);

        float angle;
        if(a1 >=0){
            angle= atan(a2/a1)*180.0/M_PI - 90;
        }
        else{
            angle= atan(a2/a1)*180.0/M_PI + 90;
        }
        glRotatef(angle, 0, 0, 1);
        glRotatef(180, 0, 1, 0);
        glCallList(m_dragon.idx);
    glPopMatrix();

    glPushMatrix();

        a1 =-rad*cos(fmod(time+0.5, (2*M_PI)));
        a2 =rad*sin(fmod(time+0.5, (2*M_PI)));
        glTranslatef(a1, a2, a3);
        glScalef(0.3f, 0.3f, 0.3f);

        angle;
        if(a1 >=0){
            angle = atan(a2/a1)*180.0/M_PI - 90;
        }
        else{
            angle = atan(a2/a1)*180.0/M_PI + 90;
        }
        glRotatef(angle, 0, 0, 1);
        glRotatef(90, 0, 1, 0);
        glCallList(m_elephant.idx);
    glPopMatrix();

    m_shaderPrograms["refract"]->release();

    // Render the dragon with the reflection shader bound
    m_shaderPrograms["reflect"]->bind();
    m_shaderPrograms["reflect"]->setUniformValue("envMap", GL_TEXTURE0);
    m_shaderPrograms["reflect"]->setUniformValue("r0", 0.4f);
    glPushMatrix();
    //glTranslatef(0.0f,0.f,0.f);
    glCallList(m_sphere.idx);
    glPopMatrix();
    m_shaderPrograms["reflect"]->release();

    // Disable culling, depth testing and cube maps
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    }

    else
    {
        float time = m_increment++ / div_val;

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
            float rad =1.5f;
            glTranslatef(rad, 0.f, 0.f);
            glScalef(0.3f, 0.3f, 0.3f);
            glRotatef(-45, 0, 0, 1);
            glRotatef(180, 0, 1, 0);
            glCallList(m_dragon.idx);
        glPopMatrix();

        m_shaderPrograms["refract"]->release();

        // Render the dragon with the reflection shader bound
        m_shaderPrograms["reflect"]->bind();
        m_shaderPrograms["reflect"]->setUniformValue("envMap", GL_TEXTURE0);
        m_shaderPrograms["reflect"]->setUniformValue("r0", 0.4f);
        glPushMatrix();
        glCallList(m_sphere.idx);
        glPopMatrix();
        m_shaderPrograms["reflect"]->release();

        // Disable culling, depth testing and cube maps
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_CUBE_MAP,0);
        glDisable(GL_TEXTURE_CUBE_MAP);
    }

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


//Bilateral blur kernel, does not use offsets for spatial pyramiding
void GLWidget::createBilatKernel(int radius, int width, int height, GLfloat* kernel)
{
    int size = radius * 2 + 1;
    float sigma = radius / 3.0f;
    float twoSigmaSigma = 2.0f * sigma * sigma;
    float rootSigma = sqrt(twoSigmaSigma * M_PI);
    float total = 0.0f;
    for (int y = -radius, idx = 0; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x,++idx)
        {
            float d = x * x + y * y;
            kernel[idx] = exp(-d / twoSigmaSigma) / rootSigma;
            total += kernel[idx];
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
            m_exp += 0.2;
            paintGL();
        }
        break;
        case Qt::Key_D:
        {
            m_exp -= 0.2;
            paintGL();
        }
        break;
        case Qt::Key_H:
        {
            m_isHDR = true;
            m_isBilat = true;
            m_isEdges = false;
            std::cout<<"USING BILATERAL"<<std::endl;
            paintGL();
        }
        break;
        case Qt::Key_G:
        {
            m_isHDR = true;
            m_isBilat = false;
            m_isEdges = false;
            std::cout<<"USING GLOBAL"<<std::endl;
            paintGL();
        }
        break;
        case Qt::Key_L:
        {
            m_isHDR = false;
            m_isBilat = false;
            m_isEdges = false;
            paintGL();
        }
        break;
        case Qt::Key_W:
        {
            m_isHDR = true;
            m_isBilat = true;
            m_isEdges = true;
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
    renderText(10, 80, "D: Decrease exposure", m_font);
    renderText(10, 95, "H: HDR scene -bilateral fusion", m_font);
    renderText(10, 110, "G: HDR scene -global tone mapping", m_font);
    renderText(10, 125, "L: LDR scene", m_font);
    renderText(10, 140, "W: Draw edges", m_font);

}
