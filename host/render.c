#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include <GL/gl.h>
#include "types.h"
#include "render.h"
#include "compat.h"
#include "machine.h"
#include "process.h"





#define WINDOW_W 800
#define WINDOW_H 600
#define RATIO (800.0/600.0)






void set_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


int resizeWindow(render_context_type *rc, int width, int height )
{
    int x0,y0;
	static uint32_t w_width;
	static uint32_t w_height;

    w_width=width;
    w_height=height;
    if(width/RATIO > height)
    {
        w_width=height*RATIO;
        w_height=height;
        x0=(width-w_width)/2;
        y0=0;
    } else
    {
        w_width=width;
        w_height=width/RATIO;
        x0=0;
        y0=(height-w_height)/2;
    }

    rc->sdl_surface = SDL_SetVideoMode(width, height, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER );
    /* Setup our viewport. */
    glViewport( x0, y0, ( GLsizei )w_width, ( GLsizei )w_height );
    
    /* change to the projection matrix and set our viewing volume. */
    set_perspective();

    return 1;
}





void init_opengl(render_context_type *rc)
{
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    if (NULL == (rc->sdl_surface = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER )))
    {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    } 
    glClearColor(0.0,0.0,0.0,0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_WM_SetCaption("video capture",NULL);
    glViewport(0,0,WINDOW_W,WINDOW_H);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glClearStencil(0);     
    glClearDepth(1.0f);
    glGenTextures(1,&rc->fb_texture);
	set_perspective();

}



render_context_type *render_init(void *machine_context, void *process_context)
{
	render_context_type *rc;
	rc=malloc(sizeof(render_context_type));
	rc->machine_context=machine_context;
	rc->process_context=process_context;
	
	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      printf ("Unable to init SDL: %s\n", SDL_GetError ());
      exit (1);
    }
    machine_get_area(rc->machine_context,&rc->tx0,&rc->ty0,&rc->tx1,&rc->ty1);
	init_opengl(rc);
	return rc;
}

void render_done(render_context_type *rc)
{
	free(rc);
}


void update_texture( void* buf, GLint texture)
{
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t_width, t_height, 0, GL_RGBA,GL_UNSIGNED_BYTE,buf);
}

void show_frame(render_context_type *rc)
{
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    update_texture(rc->process_context->framebuf,rc->fb_texture);

    glBegin(GL_QUADS);

        glTexCoord2f(rc->tx0,rc->ty1);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(rc->tx1,rc->ty1);
        glVertex2f(1.0,0.0);
        
        glTexCoord2f(rc->tx1,rc->ty0);
        glVertex2f(1.0,1.0);
        
        glTexCoord2f(rc->tx0,rc->ty0);
        glVertex2f(0.0,1.0);
    glEnd();
      
    glFlush();
    SDL_GL_SwapBuffers();
}

int video_output(render_context_type *rc)
{
        show_frame(rc);
        SLEEP(1);
    return 0;
}
