//for compilation in linux terminal :"gcc -std=gnu99 -Wall main.c -o shadowmap.out -I. -lz -lglut -lGL -lGLU -w -lm"
//for run "./shadowmap.out "
#include<GL/glut.h>
#include <stdio.h>


// Expressed as float so gluPerspective division returns a float and not 0 (640/480 != 640.0/480.0).
#define RENDER_WIDTH 640.0
#define RENDER_HEIGHT 480.0
#define SHADOW_MAP_RATIO 2


//initial parameter camera position, camera lookat, light source position, and light lookat 
float cPosition[3] = {32,20,0};
float cLookAt[3] = {2,0,-10};
float lPosition[3] = {3,20,0};
float lLookAt[3] = {0,0,-5};


//Light mouvement circle radius
float light_mvnt = 30.0f;


GLuint fboId;
GLuint depthTextureId;
GLhandleARB shadowShaderId;
GLuint shadowMapUniform;

//load the shader functions from their files
GLhandleARB loadShader(char* filename, unsigned int type)
{
	FILE *file;
	GLhandleARB handle;
	const GLcharARB* files[1];
	
	GLint result;
	GLint errorLoglength ;
	char* errorLogText;
	GLsizei actualErrorLogLength;
	
	char buffer[400000];
	memset(buffer,0,400000);
	
	file = fopen(filename, "rb");
	if(!file)
	{
		printf("can not open the file: '%s'.\n", filename);
		exit(0);
	}
	
	fread(buffer,sizeof(char),400000,file);
	fclose(file);
	
	handle = glCreateShaderObjectARB(type);
	if (!handle)
	{
		printf("failing to create vertex shader object from: %s.",filename);
		exit(0);
	}
	
	files[0] = (const GLcharARB*)buffer;
	glShaderSourceARB(handle,1, files,NULL);
	glCompileShaderARB(handle);
	glGetObjectParameterivARB(handle, GL_OBJECT_COMPILE_STATUS_ARB, &result);
		
	return handle;
}

void loadShadowShader()
{
	GLhandleARB vertexShaderHandle;
	vertexShaderHandle   = loadShader("VertexShader.c",GL_VERTEX_SHADER);
	GLhandleARB fragmentShaderHandle;
	fragmentShaderHandle = loadShader("FragmentShader.c",GL_FRAGMENT_SHADER);
	shadowShaderId = glCreateProgramObjectARB();
	glAttachObjectARB(shadowShaderId,vertexShaderHandle);
	glAttachObjectARB(shadowShaderId,fragmentShaderHandle);
	glLinkProgramARB(shadowShaderId);
	shadowMapUniform = glGetUniformLocationARB(shadowShaderId,"ShadowMap");
}

void FBO()
{
	int shadowMapWidth = RENDER_WIDTH * SHADOW_MAP_RATIO;
	int shadowMapHeight = RENDER_HEIGHT * SHADOW_MAP_RATIO;
	GLenum FBOstatus;
	glGenTextures(1, &depthTextureId);
	glBindTexture(GL_TEXTURE_2D, depthTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenFramebuffersEXT(1, &fboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId, 0);
	FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,RENDER_WIDTH/RENDER_HEIGHT,10,40000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,0,1,0);
}

void changelightposition(void)
{
	
	lPosition[0] = light_mvnt * cos(glutGet(GLUT_ELAPSED_TIME)/1000.0);
	lPosition[2] = light_mvnt * sin(glutGet(GLUT_ELAPSED_TIME)/1000.0);
	
}


void setTextureMatrix(void)
{
	static double modelView[16];
	static double projection[16];  
	const GLdouble bias[16] = {	
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0};
	
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	
	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	
	glLoadIdentity();	
	glLoadMatrixd(bias);
	
	glMultMatrixd (projection);
	glMultMatrixd (modelView);
	
	glMatrixMode(GL_MODELVIEW);
}

void drawObjects(void)
{
	//draw plan
	glColor4f(0.5f,0.5f,0.5f,1);
	glBegin(GL_QUADS);
	glVertex3f(-35,2,-35);
	glVertex3f(-35,2, 15);
	glVertex3f( 15,2, 15);
	glVertex3f( 15,2,-35);
	glEnd();
	//draw cube
	glColor4f(1.0f,0.0f,0.0f,1);

	glPushMatrix();
	glTranslatef(0,4,-16);
	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	glPushMatrix();
	glTranslatef(0,4,-16);
	glutSolidCube(5);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	
	
	
}

void renderScene(void) 
{
	changelightposition();	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);
	glUseProgramObjectARB(0);
	glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_RATIO,RENDER_HEIGHT* SHADOW_MAP_RATIO);
	glClear( GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	setupMatrices(lPosition[0],lPosition[1],lPosition[2],lLookAt[0],lLookAt[1],lLookAt[2]);
	glCullFace(GL_FRONT);
	drawObjects();
	setTextureMatrix();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	glViewport(0,0,RENDER_WIDTH,RENDER_HEIGHT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgramObjectARB(shadowShaderId);
	glUniform1iARB(shadowMapUniform,7);
	glActiveTextureARB(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D,depthTextureId);
	setupMatrices(cPosition[0],cPosition[1],cPosition[2],cLookAt[0],cLookAt[1],cLookAt[2]);
	glCullFace(GL_BACK);
	drawObjects();	
	glutSwapBuffers();
}



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(RENDER_WIDTH,RENDER_HEIGHT);
	glutCreateWindow("Shadow map");
	FBO();
	loadShadowShader();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0,0,0,1.0f);
	glEnable(GL_CULL_FACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutMainLoop();
}
