//
//  Particles System
//  
//
//  Created by BOWEN LI
//

/* Author Bowen Li */

#ifdef MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "frames.h"

#define MAX 1000000                             // limit the maximum number of particles
#define PI 3.1415926
#define RUN_SPEED 0.5                           // used in fly around view
#define ORIGINAL_VIEW 2
#define FLY_AROUND 3

static GLfloat angle = 0;                       // in degree
static int changing;                            // indicate the mouse moveing the window or not
static int previous;                            // previous location
static int newView = 1;                         // when moving the mouse to get the new view
static int numberParticles = 100;               // initial number of particles

static GLfloat particleArray[MAX][3];           // structure to hold each particles
static GLfloat particleTime[MAX];               // the lifetime of each particles
static GLfloat particleVelocity[MAX][2];
static GLfloat particleDirection[MAX][2];
static GLfloat colorList[MAX][4];
static int down[MAX];                           // indicate the particles are in down direction
static int start = 1;                           // start or stop animation
static GLfloat meanVelocity = 3.0;              // decide the speed of emitting
static GLfloat gravity = 2.0;                   // decide the speed of dropping
static int current_view;
static int point = 1;                           // the particles are rendered as points
static int square = 0;                          // the particlse are rendered as billboarded sprite
static int sphere = 0;                          // the particles are rendered as sphere
static GLfloat squareSize = 0.02;               // initial size of billboarded sprite
static int pointSize = 2;                       // initial size of the point
static GLfloat slicesStacks = 2;                // initial size of stack and slice in sphere
static int textureEnable = 0;                   // enable or disable the texture function

static GLfloat  eyex,    eyey,    eyez;         // eye point
static GLfloat  centerx, centery, centerz;      // look point


// get the random value
static double myRandom(void)
{
    // return random double within range [0,1]
    return rand() / (double) RAND_MAX;
}
#define RANDOM_RANGE(lo, hi) ((lo) + (hi - lo) * myRandom())

// the speed of time
#define TIME_DELTA 0.025

// modelling units of ground extent in each X and Z direction
#define EDGE 10

// initialise each particle's attributes
void pointInt(int i)
{
    GLfloat angle, velocity, direction;
    particleArray[i][0] = 0.0;                              // x coordinate
    particleArray[i][1] = RANDOM_RANGE(8.0, 10.0);          // y coordinate
    particleArray[i][2] = 0.0;                              // z coordinate
    particleTime[i] = 0.0;                                  // initialise the life time of each particle
    angle = (RANDOM_RANGE(60.0, 70.0)) * PI/180.0;          // emitting angle
    direction = RANDOM_RANGE(-15.0, 15.0) * PI/180.0;       // angle for velocity
    particleDirection[i][0] = cos(direction);               // x direction
    particleDirection[i][1] = sin(direction);               // z direction
    velocity = meanVelocity + RANDOM_RANGE(-1.0, 1.0);      // initialise basic velocity
    particleVelocity[i][0] = velocity * cos(angle);         // velocity in xz plane
    particleVelocity[i][1] = velocity * sin(angle);         // velocity in y direction
    colorList[i][0] = RANDOM_RANGE(0.1, 1.0);               // red
    colorList[i][1] = RANDOM_RANGE(0.1, 1.0);               // green
    colorList[i][2] = RANDOM_RANGE(0.1, 1.0);               // blue
    colorList[i][3] = RANDOM_RANGE(0.7, 1.0);               // alpha
    down[i] = 1;                                            // indicate the particle is in down direction or not
}

// create particles
void makeParticleArray(void)
{
    int i;
    for (i = 0; i < numberParticles; i++)
    {
        pointInt(i);
    }
}

// there are lots of approxiation: 1. assume the velocity would be 80% whenever particles bounce
// on the floor to appropriate the actual energy loss 2. assume the velocity in xz plane is constant
// 3. assume the floor is flat and have perfect reflection 4. ignore the effect between each particle
void updateParticleArray(void)
{
    GLfloat distance;
    int i;
    
    // update all points
    for (i = 0; i < numberParticles; i++) {
        distance = particleVelocity[i][0] * particleTime[i];      // the distance in xz plane
        
        particleArray[i][0] = particleDirection[i][0] * distance; // x distance
        particleArray[i][2] = particleDirection[i][1] * distance; // z distance
        
        // y
        if (down[i])        // in down direction
        {
            // distance = distance - velocity * time_delta - 1/2 * gravity * (time_delta ^ 2)
            particleArray[i][1] = particleArray[i][1] - (particleVelocity[i][1] + 0.5 * gravity * TIME_DELTA) * TIME_DELTA;
            particleVelocity[i][1] += gravity * TIME_DELTA;    // velocity = velocity + gravity * time_delta
        }
        else                // in up direction
        {   // approximation as we cannot get the velocity which is exactly equal to 0
            if (particleVelocity[i][1] > 0)    // keep increasing
            {
                // distance = distance + velocity * time_delta - 1/2 * gravity * (time_delta ^ 2)
                particleArray[i][1] += particleVelocity[i][1] * TIME_DELTA - 0.5 * gravity * TIME_DELTA * TIME_DELTA;
                particleVelocity[i][1] -= gravity * TIME_DELTA;    // velocity = velocity - gravity * time_delta
            }
            else            // reach the peak
                down[i] = 1;
        }
        
        // if particles hit the ground, bounce the particles upward again
        if ((particleArray[i][1] <= 5.0 && distance < 3) || (particleArray[i][1] <= 0.0 && distance > 3)) {
            // if the distance in xz plane exceed the edge, then make this particle die and re-initialise this particle
            if (distance > EDGE) {
                pointInt(i);
                continue;
            }
            down[i] = 0;
            particleVelocity[i][1] *= 0.8;  // 80% of previous velocity
        }
        particleTime[i] += TIME_DELTA;
    }
}

// used in glutIdleFunc function: sets the global idle callback
void idle(void)
{
    updateParticleArray();
    glutPostRedisplay();
}

// using mouse rotate the whole scene
void recalcModelView(void)
{
    glPopMatrix();
    glPushMatrix();
    glRotatef(angle, 0.0, 1.0, 0.0);
    newView = 0;
}

// set different views: Standard view and Fly around
void setView (void)
{
    if (current_view == 0)                  // as we do not want to fix the scene
        return;                             // mouse can change the scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    switch (current_view)
    {
        case ORIGINAL_VIEW:
            gluLookAt(0.0, 12.0, 20.0,      // eye location
                      5.0, 3.0, 0.0,        // center is at (5.0,3.0,0.0)
                      0.0, 1.0, 0.0);       // up is in postivie Y direction
            current_view = 0;
            break;
        case FLY_AROUND:
            gluLookAt(eyex, eyey, eyez,
                      centerx, centery, centerz,
                      0.0, 1.0, 0.0);
            changing = 0;                   // disable the mouse movement
            break;
    }
}

// hardcode the texture, just use the red color as texture
void glInitTexture(void)
{
    GLuint texture;
    
    glGenTextures(1, &texture);                   // generate texture names
    glBindTexture(GL_TEXTURE_2D, texture);        // bind a named texture to a texturing target
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // sets the texture magnification function
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // sets the texture minifying function
    unsigned char data[] = { 255, 0, 0, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // specify a two-dimensional texture image
}

void display(void)
{
    int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    frameStart();
    
    if (newView)    // when using mouse rotate the scene
        recalcModelView();
    
    // Enable the z buffer
    glDepthMask(GL_TRUE);
    
    // Draw the smaller ground
    glColor3f(0.3, 0.0, 0.5);
    glBegin(GL_QUADS);
    glVertex3f(-3, 4.80, -3);
    glVertex3f(3, 4.80, -3);
    glVertex3f(3, 4.80, 3);
    glVertex3f(-3, 4.80, 3);
    glEnd();
  
    // Draw the bigger ground
    glColor3f(0.5, 0.0, 0.5);
    glBegin(GL_QUADS);
    glVertex3f(-5, -0.1, -5);
    glVertex3f(EDGE, -0.1, -5);
    glVertex3f(EDGE, -0.1, 5);
    glVertex3f(-5, -0.1, 5);
    glEnd();
    
    // check the rendering shape: point or square or sphere
    if (point)          // rendering as point
    {
        glBegin(GL_POINTS);
        for (i = 0; i < numberParticles; i++)
        {
            // draw particles
            glColor4f(colorList[i][0],colorList[i][1],colorList[i][2],colorList[i][3]);
            glVertex3fv(particleArray[i]);
        }
        glEnd();
    }
    else if (square)    // rendering as billboarded sprite
    {
        glBegin(GL_QUADS);
        for (i = 0; i < numberParticles; i++)
        {
            // draw particles
            glColor4f(colorList[i][0],colorList[i][1],colorList[i][2],colorList[i][3]);
        
            glVertex3f(particleArray[i][0]-squareSize, particleArray[i][1]+squareSize, particleArray[i][2]);     // left top
            glVertex3f(particleArray[i][0]-squareSize, particleArray[i][1]-squareSize, particleArray[i][2]);     // left bottom
            glVertex3f(particleArray[i][0]+squareSize, particleArray[i][1]-squareSize, particleArray[i][2]);     // right bottom
            glVertex3f(particleArray[i][0]+squareSize, particleArray[i][1]+squareSize, particleArray[i][2]);     // right top
        }
        glEnd();
    }
    
    else if (sphere)    // rendering as sphere
    {
        for (i = 0; i < numberParticles; i++)
        {
            glPushMatrix();
            if (textureEnable)          // enable or disable the texture function
            {
                // set texture environment parameters specifies how texture values are interpreted when a fragment is textured
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glEnable(GL_TEXTURE_2D);
            }
            // put the drawn particles in correct position
            glTranslatef (particleArray[i][0], particleArray[i][1], particleArray[i][2]);
            glColor4f(colorList[i][0],colorList[i][1],colorList[i][2],colorList[i][3]);
            
            // draw particles.
            glutWireSphere(0.05, slicesStacks, slicesStacks);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }
    }
    
    // set the view: original or fly around
    setView();
    frameEnd(GLUT_BITMAP_HELVETICA_10, 1.0, 1.0, 1.0, 0.05, 0.95);
    glutSwapBuffers();
}

// manipulate the mouse
// the state parameter is either GLUT_UP or GLUT_DOWN indicating
// whether the callback was due to a release or press respectively
void mouse(int button, int state, int x, int y)
{
    // Scene can be spun around Y axis using left
    // button movement and press
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        changing = 1;
        previous = x;
    }
    // left button and release
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        changing = 0;
    }
}

// rotate the all objects in the scene
void mouseMotion(int x, int y)
{
    if (changing) {
        angle = angle + (x - previous);
        previous = x;
        newView = 1;
        glutPostRedisplay();
    }
}

// used in fly around view
void cursor_keys(int key, int x, int y)
{
    if (current_view != FLY_AROUND)
        return;
    switch (key)
    {
        case GLUT_KEY_LEFT:
            centerx -= RUN_SPEED;
            break;
        case GLUT_KEY_RIGHT:
            centerx += RUN_SPEED;
            break;
        case GLUT_KEY_UP:
            eyez -= RUN_SPEED;
            break;
        case GLUT_KEY_DOWN:
            eyez += RUN_SPEED;
            break;
    }
}

// manipulate different attributes
void menu(int option)
{
    switch (option) {
        case 2:                             // standard view
            current_view= ORIGINAL_VIEW;
            break;
        case 3:                             // fly around all scene
            current_view= FLY_AROUND;
            break;
        case 4:                             // increase the gravity
            gravity += 1;
            break;
        case 5:                             // decrease the gravity
            if (gravity > 3)                // limit the minimum gravity
            {
                gravity -= 1;
            }
            break;
         case 6:                            // increase the velocity
            meanVelocity += 1;
            break;
        case 7:                             // decrease the velocity
            if (meanVelocity > 3)           // limit the minimum velocity
            {
                meanVelocity -= 1;
            }
            break;
        case 8:                             // rendering as the point
            point = 1;
            square = 0;
            sphere = 0;
            break;
        case 9:                             // rendering as the billboarded sprite
            square = 1;
            point = 0;
            sphere = 0;
            break;
        case 10:                             // rendering as the sphere
            sphere = 1;
            point = 0;
            square = 0;
            break;
        case 11:                            // increase the point size
            if (point)
            {
                if (pointSize * 2 < 64)     // limit the maximum point size
                {
                    pointSize *= 2;
                    glPointSize(pointSize);
                }
            }
            break;
        case 12:                            // decrease the point size
            if (point)
            {
                if (pointSize / 2 > 1)      // limit the minumum point size
                {
                    pointSize /= 2;
                    glPointSize(pointSize);
                }
            }
            break;
        case 13:                            // increase the billboarded sprite size
            if (square)
            {
                if (squareSize + 0.03 < 0.10)
                    squareSize += 0.03;
            }
            break;
        case 14:                            // decrease the billboarded sprite size
            if (square)
            {
                if (squareSize - 0.03 > 0)
                    squareSize -= 0.03;
            }
            break;
        case 15:                            // increase the sphere size
            if (sphere)
            {
                if (slicesStacks + 4 <= 20)
                    slicesStacks += 4;
            }
            break;
        case 16:                            // decrease the sphere size
            if (sphere)
            {
                if (slicesStacks - 4 > 0)
                    slicesStacks -= 4;
            }
            break;
        case 17:                            // initialise the number of particles
            numberParticles = 100;
            break;
        case 18:                            // increase the number of particles
            if (numberParticles * 10 <= MAX)
            {
                int i;
                int curNoPoint = numberParticles;
                numberParticles = curNoPoint * 10;
                for (i = curNoPoint; i < numberParticles; i++)
                    pointInt(i);
            }
            break;
        case 19:                            // decreases the number of particles
            if (numberParticles / 10 >= 1)
            {
                numberParticles /= 10;
            }
            break;
        case 20:                            // enable or disable the texture function
            if (sphere)                     // texture is used in the sphere
            {
                textureEnable = 1 - textureEnable;
            }
            break;
        case 666:
            exit(0);
    }
    glutPostRedisplay();
}

// manipulate the keyboard
void keyboard(unsigned char c, int x, int y)
{
    switch (c) {
        case 13:                            // the enter key stop all animation
            start = 1 - start;
            if (start)
            {
                glutIdleFunc(idle);
            }
            else
            {
                glutIdleFunc(NULL);
            }
            break;
        case ' ':                           // the space key restart the whole system
            start = 1;
            makeParticleArray();
            glutIdleFunc(idle);
            break;
        case 'a':
            centery += RUN_SPEED;
            break;
        case 's':
            centery -= RUN_SPEED;
            break;
        case 27:
            exit(0);
    }
}

void initGraphics(int argc, char *argv[])
{
    glutInit(&argc, argv);
    // sets the initial display mode
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize (1000, 700);
    glutCreateWindow("COMP37111 Particles");
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc (cursor_keys);
    glutCreateMenu(menu);
    glutAddMenuEntry("Original View", 2);
    glutAddMenuEntry("Fly Around", 3);
    glutAddMenuEntry("+ Gravity", 4);
    glutAddMenuEntry("- Gravity", 5);
    glutAddMenuEntry("+ Velocity", 6);
    glutAddMenuEntry("- Velocity", 7);
    glutAddMenuEntry("Point", 8);
    glutAddMenuEntry("Square", 9);
    glutAddMenuEntry("Sphere", 10);
    glutAddMenuEntry("+ Point size", 11);
    glutAddMenuEntry("- Point size", 12);
    glutAddMenuEntry("+ Square size", 13);
    glutAddMenuEntry("- Square size", 14);
    glutAddMenuEntry("+ Slices and Stacks", 15);
    glutAddMenuEntry("- Slices and Stacks", 16);
    glutAddMenuEntry("100 points ", 17);
    glutAddMenuEntry("x 10 points ", 18);
    glutAddMenuEntry("/ 10 points ", 19);
    glutAddMenuEntry("On/Off texture ", 20);
    glutAddMenuEntry("Quit", 666);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    // Set the initial view parameter
    current_view= ORIGINAL_VIEW;
    eyex=  0.0;
    eyey=  12.0;
    eyez=  20.0;
    
    centerx = 5.0;
    centery = 3.0;
    centerz = 0.0;
    
    // initialise the texture
    glInitTexture();
    
    // Set the initial point size
    glPointSize(pointSize);

}


int main(int argc, char **argv)
{
    // makes use of the computer's internal clock to control the choice of the seed.
    srand(time(NULL));
    
    initGraphics(argc, argv);
    
    // flat shading selects the computed color of just one vertex
    // and assigns it to all the pixel fragments
    glShadeModel(GL_FLAT);
    
    // do depth comparisons and update the depth buffer
    glEnable(GL_DEPTH_TEST);
    
    // if enabled, draw points with proper filtering. Otherwise, draw aliased points
    glEnable(GL_POINT_SMOOTH);
    
    // enable the alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0,                       // field of view in degree
                   1.0,                        // aspect ratio
                   0.5,                        // z near
                   40.0);                      // z far

    glPushMatrix();       // push so we can pop on model recalcModelView function
    
    makeParticleArray();
    
    glutMainLoop();
    return 0;
}

