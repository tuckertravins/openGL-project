#include "CSCIx229.h"

#define fps 60

#define wk 0 //use these for the key array
#define ak 1
#define sk 2
#define dk 3
#define spacek 4
#define zk 5
#define ck 6
#define hk 7
#define gk 8
#define esck 9
#define bk 10
#define mk 11

//global variables for projections and lighting
int axes=1;       //  Display axes
int mode=1;       //  Projection mode (dont change)
int move=1;       //  Move light
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
int obj=0;        //  Scene/opbject selection
double asp=1;     //  Aspect ratio
double dim=10;     //  Size of world
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =   5;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  10;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   .3;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light
int moveMode = 0; //0 for plane mode 1 for free roam
int speed = 1; //0 to 5 for speed of plane normally, afterburner adds .5
int hud = 1; //toggles f22 hud
int pause = 0; //if it is paused then it goe sto pause menu, time stops
double timeRate = 1; //make time (only affects sun) go from 0 to 2 (0 stops time, 2 makes time go twice as fast)
int envMode = 0; //go between 0 regular env and 1 looking at plane
int pov = 0; //0 for first person 1 for third person 2 for second person
int keys[20]; //this will be used for key strokes
float up[] = {0,1,0};
float forward[] = {1,0,0};
int locMenu;    
int lightMenu;    
int modeMenu;    
int fovMenu;    
int mainMenu; 
int keybindsMenu;
int fullScreenMode = 0;
double timeSpeed=1.0;
double t = 0;
int playTime = 1;
int numFrames = 0;

unsigned int texture[7]; // textures
double locx=0;
double locy=0;
double locz=0;
double Ex;
double Ey;
double Ez;
double Cx;
double Cy;
double Cz;

void timer(int oogaoob);

//just hand out
void idle()
{
   //glutPostRedisplay();
}

void keyInput(void){
   if (keys[wk]){
      double Dx = Cos(th);
      double Dy = 0;
      double Dz = Sin(th);
      locx = locx + (.1*speed) * Dx;
      locy = locy + (.1*speed) * Dy;
      locz = locz + (.1*speed) * Dz;
   }
   if (keys[sk]){
      double Dx = Cos(th);
      double Dy = 0;
      double Dz = Sin(th);
      locx = locx - (.1*speed) * Dx;
      locy = locy - (.1*speed) * Dy;
      locz = locz - (.1*speed) * Dz;
   }
   if(keys[spacek]){
      locy+=(.1*speed);
   }
   if(keys[bk]){
      locy-=(.1*speed);
   }
   if(keys[dk]){
      th++;
   }
   if(keys[ak]){
      th--;
   }
   th %= 360;
   ph %= 360;
   Project(mode?fov:0,asp,dim);
   glutIdleFunc(move?idle:NULL);
   //glutPostRedisplay();
}

//polar vertex
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

static void ground(){
   glPushMatrix();

   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   glColor3f(1,1,1);

   //textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[1]);

   glBegin(GL_QUADS);
   glNormal3f( 0, 1, 0);
   glTexCoord2f(0,0);
   glVertex3f(-1,0, -1);
   glTexCoord2f(1,0);
   glVertex3f(1,0, -1);
   glTexCoord2f(1,1);
   glVertex3f(1,0, 1);
   glTexCoord2f(0,1);
   glVertex3f(-1,0, 1);
   glEnd();

   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//sun for the light source
static void sun(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (int ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

//make cube
static void cubeHelper(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th, double r, double g, double b, int tex, int shin)
{
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shin);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   r = pow(r,0.5);
   g = pow(g,0.5);
   b = pow(b,0.5);

   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //texture time
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   //glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[tex]);
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glColor3f(r,g,b);
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0);
   glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0);
   glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1);
   glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1);
   glVertex3f(-1,+1, 1);
   //  Back
   glColor3f(r,g,b);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0);
   glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0);
   glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1);
   glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1);
   glVertex3f(+1,+1,-1);
   //  Right
   glColor3f(r,g,b);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0);
   glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0);
   glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1);
   glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1);
   glVertex3f(+1,+1,+1);
   //  Left
   glColor3f(r,g,b);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0);
   glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0);
   glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1);
   glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1);
   glVertex3f(-1,+1,-1);
   //  Top
   glColor3f(r,g,b);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0);
   glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0);
   glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1);
   glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1);
   glVertex3f(-1,+1,-1);
   //  Bottom
   glColor3f(r,g,b);
   glNormal3f( 0,-1, 0);
   glTexCoord2f(0,0);
   glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0);
   glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1);
   glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1);
   glVertex3f(-1,-1,+1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//display func
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   Ex = locx;
   Ey = locy;
   Ez = locz;
   Cx = Ex + Cos(th);
   Cy = Ey;
   Cz = Ez + Sin(th);
   gluLookAt(Ex,Ey,Ez , Cx,Cy,Cz , 0,1,0);

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //========================draw stuff==========================
   cubeHelper(0,0,0,1,2,1,45,.8,.8,.8,texture[1],.3);




   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      sun(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);

   //  Display parameters
   //glWindowPos2i(5,5);
   //Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s",
   //  th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off");
   glWindowPos2i(5,5);
   Print("%d",numFrames);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();

   keyInput();
}

//do stuff when special key is pressed
void special(int key,int x,int y)
{

}

void specialUp(int key, int x, int y){
   
}

//do stuff when key is pressed
void key(unsigned char ch,int x,int y)
{
   switch(ch){
      case 27: 
         exit(0);
         break;
      case 'w':
         keys[wk] = 1;
         break;
      case 'a':
         keys[ak] = 1;
         break;
      case 's':
         keys[sk] = 1;
         break;
      case 'd':
         keys[dk] = 1;
         break;
      case 32:
         keys[spacek] = 1;
         break;
      case 'z':
         speed -= 1;
         keys[zk] = 1;
         break;
      case 'c':
         speed += 1;
         keys[ck] = 1;
         break;
      case 'h':
         keys[hk] = 1;
         break;
      case 'g':
         keys[gk] = 1;
         break;
      case 'b':
         keys[bk] = 1;
         break;
      case 'm':
         keys[mk] = 1;
         break;
   }
   // Project(mode?fov:0,asp,dim);
   // glutIdleFunc(move?idle:NULL);
   // glutPostRedisplay();
}

void keyUp(unsigned char ch, int x, int y){
   switch(ch){
      case 27: 
         exit(0);
         break;
      case 'w':
         keys[wk] = 0;
         break;
      case 'a':
         keys[ak] = 0;
         break;
      case 's':
         keys[sk] = 0;
         break;
      case 'd':
         keys[dk] = 0;
         break;
      case 32:
         keys[spacek] = 0;
         break;
      case 'z':
         keys[zk] = 0;
         break;
      case 'c':
         keys[ck] = 0;
         break;
      case 'h':
         keys[hk] = 0;
         break;
      case 'g':
         keys[gk] = 0;
         break;
      case 'b':
         keys[bk] = 0;
         break;
      case 'm':
         keys[mk] = 0;
         break;
   }
}

void menuChecker(int button){
   switch (button){
      case 3:
         exit(0);
         break;
      case 2:
         if(fullScreenMode) {
            glutReshapeWindow(1500,900);
            fullScreenMode = 0;
         }
         else {
            glutFullScreen();
            fullScreenMode = 1;
         }
         break;
      case 11:
         playTime = playTime ? 0 : 1;
         break;
      case 12:
         timeSpeed *= 2;
         break;
      case 13:
         timeSpeed *= 0.5;
         break;
      case 14:
         timeSpeed = 1;
         break;
      case 21:
         fov += 10;
         break;
      case 22:
         fov += 1;
         break;
      case 23:
         fov -= 10;
         break;
      case 24:
         fov -= 1;
         break;
      case 25: 
         fov = 55;
         break;
      default:
         break;
   }
}

void menu(){
   lightMenu = glutCreateMenu(menuChecker);
   glutAddMenuEntry("Pause/Resume Time",11);
   glutAddMenuEntry("2x Current Speed",12);
   glutAddMenuEntry(".5x Current Speed",13);
   glutAddMenuEntry("1x Speed",14);
   glutAddMenuEntry("Set Day",15);
   glutAddMenuEntry("Set Night",16);
   fovMenu = glutCreateMenu(menuChecker);
   glutAddMenuEntry("FOV +10",21);
   glutAddMenuEntry("FOV +1",22);
   glutAddMenuEntry("FOV -10",23);
   glutAddMenuEntry("FOV -1",24);
   glutAddMenuEntry("Reset FOV",25);
   locMenu = glutCreateMenu(menuChecker);
   glutAddMenuEntry("Area 51",31);
   glutAddMenuEntry("Desert",32);
   glutAddMenuEntry("Mountains",33);
   glutAddMenuEntry("City",34);
   glutAddMenuEntry("Suburbs",35);
   modeMenu = glutCreateMenu(menuChecker);
   glutAddMenuEntry("Explore Environment",31);
   glutAddMenuEntry("Inspect Models",32);
   keybindsMenu = glutCreateMenu(menuChecker);
   glutAddMenuEntry("====|||   Keybinds   |||====",0);
   glutAddMenuEntry("W,S:    Forward, Backward",0);
   glutAddMenuEntry("A,D:    Look Around",0);
   glutAddMenuEntry("Space:  Fly Up",0);
   glutAddMenuEntry("B:      Fly Down",0);
   glutAddMenuEntry("C:      Increase Speed",0);
   glutAddMenuEntry("Z:      Decrease Speed",0);
   glutAddMenuEntry("H:      Toggle HUD",0);
   glutAddMenuEntry("G:      Toggle POV (1st/3rd)",0);
   glutAddMenuEntry("M:      Cycle Plane Model",0);
   glutAddMenuEntry("Mouse2: Open Menu",0);
   glutAddMenuEntry("Esc:    Exit",0);
   mainMenu = glutCreateMenu(menuChecker);
   glutAddSubMenu("Light Settings", lightMenu);
   glutAddSubMenu("FOV Settings", fovMenu);
   glutAddSubMenu("Location Teleport", locMenu);
   glutAddSubMenu("Application Mode", modeMenu);
   glutAddSubMenu("Keybinds",keybindsMenu);
   glutAddMenuEntry("Fullscreen Toggle",2);
   glutAddMenuEntry("Exit",3);
   //glutCreateMenu(menuChecker);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//reshape window to make everything look normal
void reshape(int width,int height)
{
   asp = (height>0) ? (double)width/height : 1;
   glViewport(0,0, RES*width,RES*height);
   Project(mode?fov:0,asp,dim);
}

void timer(int oogaoob){
   numFrames++;
   t = t + timeSpeed * playTime * .5;
   zh = fmod(t,360.0);
   glutPostRedisplay();
   glutTimerFunc(17, timer, 0);
}

//main func
int main(int argc,char* argv[])
{

   //boring glut stuff
   glutInit(&argc,argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(1500,800);
   glutCreateWindow("Tucker Travins");
#ifdef USEGLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   menu();
   glutIgnoreKeyRepeat(1); //won't keep calling key functions for holding down
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   timer(0);
   glutSpecialFunc(special);
   glutSpecialUpFunc(specialUp);
   glutKeyboardFunc(key);
   glutKeyboardUpFunc(keyUp);
   glutIdleFunc(idle);
   texture[0] = LoadTexBMP("textures/cactus.bmp");
   texture[1] = LoadTexBMP("textures/ground.bmp");
   ErrCheck("init");
   glutMainLoop();
   return 0;
}