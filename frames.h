//
//  frames.h
//  
//
//  Created by BOWEN LI on 04/12/2017.
//

#include <sys/time.h>
#include <stdio.h>

struct timeval frameStartTime, frameEndTime;

void frameStart(void) {
    gettimeofday(&frameStartTime, NULL);
}

void frameEnd(void *font, GLclampf r, GLclampf g, GLclampf b,
              GLfloat x, GLfloat y) {
    /* font: font to use, e.g., GLUT_BITMAP_HELVETICA_10
     r, g, b: text colour
     x, y: text position in window: range [0,0] (bottom left of window)
     to [1,1] (top right of window). */
    
    float elapsedTime;
    char str[30];
    char *ch;
    GLint matrixMode;
    GLboolean lightingOn;
    
    gettimeofday(&frameEndTime, NULL);
    
    elapsedTime= frameEndTime.tv_sec - frameStartTime.tv_sec +
    ((frameEndTime.tv_usec - frameStartTime.tv_usec)/1.0E6);
    sprintf(str, "Frames per second: %2.0f", 1.0/elapsedTime);
    
    lightingOn= glIsEnabled(GL_LIGHTING);        /* lighting on? */
    if (lightingOn) glDisable(GL_LIGHTING);
    
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  /* matrix mode? */
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
    glColor3f(r, g, b);
    glRasterPos3f(x, y, 0.0);
    for(ch= str; *ch; ch++) {
        glutBitmapCharacter(font, (int)*ch);
    }
    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixMode);
    if (lightingOn) glEnable(GL_LIGHTING);
}

/* end of frames.h */
