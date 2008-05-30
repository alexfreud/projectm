/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2004 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */
#include "video_init.h"
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>

#include <projectM.hpp>

#include "sdltoprojectM.h"

#include "ConfigFile.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> // for mkdir
#include <GL/gl.h>

#define CONFIG_FILE "/share/projectM/config.inp"

std::string read_config();


#ifdef DEBUG
FILE *debugFile = NULL;
#endif

SDL_Surface *screen;

projectM *globalPM = NULL;

GLuint texture;

int dumpFrame = 0;
int frame = 0;


int texsize=512;
int gx=32,gy=24;
int wvw=512,wvh=512;
int fvw=1024,fvh=768;
int fps=30, fullscreen=0;




std::string read_config()
{

   int n;
   
   char num[512];
   FILE *in; 
   FILE *out;

   char* home;
   char projectM_home[1024];
   char projectM_config[1024];

   strcpy(projectM_config, PROJECTM_PREFIX);
   strcpy(projectM_config+strlen(PROJECTM_PREFIX), CONFIG_FILE);
   projectM_config[strlen(PROJECTM_PREFIX)+strlen(CONFIG_FILE)]='\0';
   printf("dir:%s \n",projectM_config);
   home=getenv("HOME");
   strcpy(projectM_home, home);
   strcpy(projectM_home+strlen(home), "/.projectM/config.inp");
   projectM_home[strlen(home)+strlen("/.projectM/config.inp")]='\0';

  
 if ((in = fopen(projectM_home, "r")) != 0) 
   {
     printf("reading ~/.projectM/config.inp \n");
     fclose(in);
     return std::string(projectM_home);
   }
 else
   {
     printf("trying to create ~/.projectM/config.inp \n");

     strcpy(projectM_home, home);
     strcpy(projectM_home+strlen(home), "/.projectM");
     projectM_home[strlen(home)+strlen("/.projectM")]='\0';
     mkdir(projectM_home,0755);

     strcpy(projectM_home, home);
     strcpy(projectM_home+strlen(home), "/.projectM/config.inp");
     projectM_home[strlen(home)+strlen("/.projectM/config.inp")]='\0';
     
     if((out = fopen(projectM_home,"w"))!=0)
       {
	
	 if ((in = fopen(projectM_config, "r")) != 0) 
	   {
	     
	     while(fgets(num,80,in)!=NULL)
	       {
		 fputs(num,out);
	       }
	     fclose(in);
	     fclose(out);
	    

	     if ((in = fopen(projectM_home, "r")) != 0) 
	       { 
		 printf("created ~/.projectM/config.inp successfully\n");  
		 fclose(in);
		 return std::string(projectM_home);
	       }
	     else{printf("This shouldn't happen, using implementation defualts\n");abort();}
	   }
	 else{printf("Cannot find projectM default config, using implementation defaults\n");abort();}
       }
     else
       {
	 printf("Cannot create ~/.projectM/config.inp, using default config file\n");
	 if ((in = fopen(projectM_config, "r")) != 0) 
	   { printf("Successfully opened default config file\n");
	     fclose(in);
	     return std::string(projectM_config);}
	 else{ printf("Using implementation defaults, your system is really messed up, I'm suprised we even got this far\n");  abort();}
	   
       }

   }



  
 abort();
} 

void renderLoop() {

    int i;
    int x, y;
    int index;
    short pcm_data[2][512];

    while ( 1 ) {
        projectMEvent evt;
        projectMKeycode key;
        projectMModifier mod;

        /** Process SDL events */
        SDL_Event event;
        while ( SDL_PollEvent( &event ) ) {
            /** Translate into projectM codes and process */
            evt = sdl2pmEvent( event );
            key = sdl2pmKeycode( event.key.keysym.sym );
            mod = sdl2pmModifier( event.key.keysym.mod );

	    if ( evt == PROJECTM_KEYDOWN ) {                 
	      if(key == PROJECTM_K_ESCAPE)
                {
		  delete(globalPM);
		  exit(0);
		}
                 

                if(key == PROJECTM_K_f)
                {
                 
                   
                    resize_display(fvw, fvh, fullscreen);
		    globalPM->projectM_resetGL( fvw, fvh ); 
                }
		else if(key == PROJECTM_K_q) { exit (1);}
                else  {globalPM->key_handler(evt,key,mod);}

            }
            else if ( evt == PROJECTM_VIDEORESIZE )
            {
	      wvw=event.resize.w;
	      wvh=event.resize.h;
              resize_display(wvw, wvh, 0);
	      globalPM->projectM_resetGL(  wvw, wvh ); 

            } 

          }



        /** Render the new frame */
        globalPM->renderFrame( );

	glClear( GL_COLOR_BUFFER_BIT );
	glClear( GL_DEPTH_BUFFER_BIT );
		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0,0,wvw,wvh);
      
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
	glFrustum(-1, 1,-1,1,2,10);

	glEnable(GL_DEPTH_TEST);
 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	      
	glTranslatef(cos(frame*0.023),cos(frame*0.017), -5+sin(frame*0.022)*2);
	glRotatef(sin(frame*0.0043)*360,sin(frame*0.0017)*360, sin(frame*0.0032)*360,1);
       

	glEnable(GL_TEXTURE_2D); 
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBindTexture( GL_TEXTURE_2D, texture );
	glColor4d(1.0,1.0,1.0,1.0);	

	glBegin(GL_QUADS);
	glTexCoord2d(0, 1); glVertex3d(-0.8, 0.8,0);
	glTexCoord2d(0, 0); glVertex3d(-0.8, -0.8,0);
	glTexCoord2d(1, 0); glVertex3d(0.8, -0.8,0);
	glTexCoord2d(1, 1); glVertex3d(0.8, 0.8,0);
	glEnd();


	glDisable(GL_TEXTURE_2D); 



	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);

	glFlush();
      
        SDL_GL_SwapBuffers();

	frame++;

      }
	
  printf("Worker thread: Exiting\n");
  }
           

int main( int argc, char **argv ) {
        atexit(SDL_Quit);

	int i;
 char projectM_data[1024];


std::string config_file;
 config_file = read_config();

 ConfigFile config(config_file);

 wvw = config.read<int>( "Window Width", 512 );
 wvh = config.read<int>( "Window Height", 512 );
 int fullscreen = 0;
 if (config.read("Fullscreen", true)) fullscreen = 1;
      else fullscreen = 0;
 

#ifdef DEBUG
	int value;
	int rgb_size[3];
#endif

  const SDL_VideoInfo* info = NULL;
  int bpp = 0;
  /* Flags we will pass into SDL_SetVideoMode. */
  int flags = 0;

  init_display(wvw,wvh,&fvw,&fvh,fullscreen);   
  
    /** Setup some window stuff */
    SDL_WM_SetCaption( PROJECTM_TITLE, NULL );
  globalPM = new projectM(config_file);
  texture = globalPM->initRenderToTexture();
    /** Initialise projectM */
 



    
    /** Initialise the thread */
    renderLoop();

    return 1;
}


