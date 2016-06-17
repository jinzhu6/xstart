#if 0

#define _WINSOCKAPI_
#include <Windows.h>
#include <gl/GL.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "corela.h"
#include "glext.h"

#define PI 3.14159265f

void gluPerspective(float FOV, float ASPECT, float NEARPLANE, float FARPLANE)
{
    double left, right;
    double bottom, top;
    top = tan (FOV*3.14159/360.0)*NEARPLANE;
    bottom = -top;
    left = ASPECT*bottom;
    right = ASPECT*top;
    glFrustum(left, right, bottom, top, NEARPLANE, FARPLANE);
}


void draw_bitmap(TEXTURE* tex, float pos_x, float pos_y, float depth, float scale, float rotate)
{
    GLint tid = tex->handle;
    
    /*glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 1.0f);*/
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pos_x, pos_y, 0.0f);
    glRotatef(rotate*(180.0f/PI), 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tid);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
//    glDepthMask(GL_TRUE);
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LEQUAL);
    
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, depth);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(+0.5f, -0.5f, depth);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(+0.5f, +0.5f, depth);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, +0.5f, depth);
    glEnd();
    
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}
#endif

#if 0
void draw_sub_bitmap(ALLEGRO_BITMAP* bmp, float t0_x, float t0_y, float t1_x, float t1_y, float pos_x, float pos_y, float depth, float scale, float rotate)
{
    GLint tid = al_get_opengl_texture(bmp);
    if(!tid)
        return;
    
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0.0f, 1366, 768, 0.0f, 0.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pos_x, pos_y, 0.0f);
    glRotatef(rotate*(180.0f/PI), 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glBindTexture(GL_TEXTURE_2D, tid);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
//	glDepthMask(GL_TRUE);
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
    
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(t0_x, t1_y); glVertex3f(-0.5f, -0.5f, depth);
    glTexCoord2f(t1_x, t1_y); glVertex3f(+0.5f, -0.5f, depth);
    glTexCoord2f(t1_x, t0_y); glVertex3f(+0.5f, +0.5f, depth);
    glTexCoord2f(t0_x, t0_y); glVertex3f(-0.5f, +0.5f, depth);
    glEnd();
    
    //glMatrixMode(GL_PROJECTION);
    //glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void draw_bitmap_quad(ALLEGRO_BITMAP* bmp, VEC* p1, VEC* p2, VEC* p3, VEC* p4, float depth)
{
    //return;
    
    GLint tid = al_get_opengl_texture(bmp);
    if(!tid)
        return;
    
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0.0f, 1366, 768, 0.0f, 0.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glBindTexture(GL_TEXTURE_2D, tid);
    glEnable(GL_TEXTURE_2D);
    
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(p1->x, p1->y, depth);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(p2->x, p2->y, depth);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(p3->x, p3->y, depth);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(p4->x, p4->y, depth);
    glEnd();
    
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void draw_colored_quad(VEC3* c1, VEC3* c2, VEC* p1, VEC* p2, VEC* p3, VEC* p4, float depth)
{
    //return;
    
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glDisable(GL_TEXTURE_2D);
    
//	glDepthMask(GL_TRUE);
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
    
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glColor4f(c2->x,c2->y,c2->z,1.0f); glVertex3f(p1->x, p1->y, depth);
    glTexCoord2f(1.0f, 1.0f); glColor4f(c1->x,c1->y,c1->z,1.0f); glVertex3f(p2->x, p2->y, depth);
    glTexCoord2f(1.0f, 0.0f); glColor4f(c1->x,c1->y,c1->z,1.0f); glVertex3f(p3->x, p3->y, depth);
    glTexCoord2f(0.0f, 0.0f); glColor4f(c2->x,c2->y,c2->z,1.0f); glVertex3f(p4->x, p4->y, depth);
    glEnd();
    
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
//	glDisable(GL_DEPTH_TEST);
//	glDepthMask(GL_FALSE);
}


void draw_as_background(ALLEGRO_BITMAP* bmp, VEC* scroll, float div, float scale, float alpha)
{
    //return;
    
    GLint tid = al_get_opengl_texture(bmp);
    if(!tid) return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glTranslatef(scroll->x/div, -scroll->y/div, 0.0f);
    glScalef(scale, scale/((float)SCREEN_Y/(float)SCREEN_X), 1.0f);
    
    glBindTexture(GL_TEXTURE_2D, tid);
    glEnable(GL_TEXTURE_2D);
//	glTranslatef(0, 0, -10);
    
    // dont use z-buffer for backgrounds
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    
    glColor4f(1.0f,1.0f,1.0f,alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(+1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(+1.0f, +1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, +1.0f, 1.0f);
    glEnd();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}


#endif
