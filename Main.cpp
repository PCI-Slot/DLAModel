
#include <stdio.h>
#include <iostream>
#include <GL\freeglut.h>
#include <ctime>
#include <math.h>
#include <array>
#include <vector>
#include "kdtree.h"

typedef std::array<float,2> xy;

//x coord, y coord, i = index of other point
typedef struct line : std::array<float, 2> { float r, g, b; struct line* i; } line;


static const double pi = 3.14159265359;
static const int Range = 1.0f;//distance between each point
static const int sqsize = 2000;

#define getrandom(min, max) (((double)rand() / (double)RAND_MAX)*(max-min)+min)

kdtree<line, 2, 10, sqsize, Range> tree;

bool keys[256];
int width = 1024, height = 768;
//float *px;float *py;
std::vector<line> pxy;

int np = 60000;// Maximum number of points
float r = 5.0f,startingr =  5.0f;
//adverage of points
float ax=0, ay=0;
int close = 0;
float zoom = 500;
float t0;
bool timerend = false,run=false;
float Rangep2 = Range * Range;
void addpoint();
bool inrange(float, float, float);

void quadpoint(line l,float size){
	glBegin(GL_QUADS);
		glColor3f(l.r,l.g,l.b);
		glVertex3f(l[0] - size, l[1] - size, 0);
		glVertex3f(l[0] - size, l[1] + size, 0);
		glVertex3f(l[0] + size, l[1] + size, 0);
		glVertex3f(l[0] + size, l[1] - size, 0);
			//glColor3f(pxy[l.i].r, pxy[l.i].g, pxy[l.i].b);
			//glVertex3f(pxy[l.i][0], pxy[l.i][1], 0);
			//glVertex3f(pxy[l.i][0], pxy[l.i][1], 0);
	glEnd();
}

void drawdebugcross(float xx,float yy,float ss){
	glBegin(GL_LINES);
		glColor3f(0,1,1);
		glVertex3f(xx+ss, yy,0);
		glVertex3f(xx-ss, yy,0);
		glVertex3f(xx, yy-ss,0);
		glVertex3f(xx, yy+ss,0);
	glEnd();
}
void drawcircle(float rad){
	glBegin(GL_POINTS);
	for (float i = 0; i<2 * pi; i += pi / 50){
		glColor3f(0, 1, 1);
		glVertex3f(ax / (pxy.size()+1) + cos(i)*rad, ay / (pxy.size()+1) + sin(i)*rad, 0);
	}
	glEnd();
}

float cheapdist(float ax, float ay, float bx, float by){
	float vx, vy;
	vx = ax - bx;
	vy = ay - by;
	return vx*vx + vy*vy;
}

float euclidis(float ax, float ay, float bx, float by){
	float vx, vy;
	vx = ax - bx;
	vy = ay - by;
	return sqrt(vx*vx + vy*vy);
}
float random2(float max, float min){
	float r = ((float)rand()) / (float)RAND_MAX;
	return r*(max - min) + min;
}

void drawscene(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -zoom);
	glPointSize(1);
	//glBegin(GL_POINTS);
	for (int i = 0; i < pxy.size(); i++){
	//	glColor3f(pxy[i].r, pxy[i].g, pxy[i].b);
	//	glVertex3f(pxy[i][0], pxy[i][1], 0);
		quadpoint(pxy[i],0.5f);
		//glColor3f((*pxy[i].i).r, (*pxy[i].i).g, (*pxy[i].i).b);
		//glVertex3f((*pxy[i].i)[0], (*pxy[i].i)[1], 0);
	}
	//glEnd();
	drawdebugcross(ax / pxy.size(), ay / pxy.size(), 3);
	glBegin(GL_POINTS);
	glColor3f (0,   1, 0);
	glVertex3f(0, 0.1, 0);
	glVertex3f(1, 0.1, 0);
	glEnd();

	//circles for spawning radius/destruction radius
	drawcircle(r);
	drawcircle(sqsize);
	drawcircle(r + 5.0f);
	drawcircle(r - 5.0f);
	if (run){
		if (pxy.size() < np){
			for (int i = 0; i < 100; i++){
				addpoint();
			}
		}
		else if (!timerend){
			printf("finished in %f seconds", ((float)clock() - t0) / CLOCKS_PER_SEC);
			timerend = true;
			run = false;
		}
	}

	glutSwapBuffers();
}
void addpoint(){
	float f, dd = 0, d = 0;
	float *t;
	bool b = false;

	f = random2(0, 2 * pi);
	line templ;
	line pxyp;
	templ[0] = ax / pxy.size() + cos(f)*r;
	templ[1] = ay / pxy.size() + sin(f)*r;

	
	do{
		if (dd>r + 10){
			f = random2(0, 2 * pi);
			templ[0] = (ax / pxy.size()) + cos(f)*r;
			templ[1] = (ay / pxy.size()) + sin(f)*r;
		}
		templ[0] += random2(-5, 5);
		templ[1] += random2(-5, 5);

	//sucks points into the center. speeds thing up alot and makes the model more rounded
		templ[0] *= 0.9999;
		templ[1] *= 0.9999;

		dd = euclidis(templ[0], templ[1], ax / pxy.size(), ay / pxy.size());
		std::vector<line*> a = tree.getbucket(templ);
		for (int i = 0; i<a.size(); i++){
			float d = cheapdist(templ[0], templ[1], (*a[i])[0], (*a[i])[1]);
			if (d <= Rangep2){
				b = true;
				pxyp.i = a[i];
				break;
			}
		}
	} while (!b);
	
	//printf("x= %f, y = %f \n",nx,ny);
	pxyp[0] = templ[0];
	pxyp[1] = templ[1];
	pxyp.r = 0;
	pxyp.g = (sinf(dd/20) + 1) / 2;
	pxyp.b = (sinf(dd/50) + 1) / 2;
	pxy.push_back(pxyp);
	tree.insert(&pxy.back());
	ax += pxyp[0];
	ay += pxyp[1];
	if (dd > r){
		r = dd+5;
	}
}

bool inrange(float range, float npx, float npy){
	for (int i = 0; i<pxy.size(); i += 2){
		float d = cheapdist(npx, npy, pxy[i][0], pxy[i][1]);
		if (d <= range){
			return true;
		}
	}
	return false;
}
void init(){
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	glLineWidth(0.5f);
	pxy.reserve(np+1);
	
	line pxy1;
	//center point seed
	pxy1[0] = 0;
	pxy1[1] = 0;
	pxy1.i = &pxy1;
	pxy1.r = 0;
	pxy1.b = 0;
	pxy1.g = 0;
	pxy.push_back(pxy1);
	tree.insert(&pxy.back());
}
void specialkeyups(int key, int x, int y){
	keys[key] = false;
}
void specialkeys(int key, int x, int y){
	keys[key] = true;
	//start/pause toggle
	if (key == GLUT_KEY_F1){
		run = !run;
		//if reset make seed point
		if (pxy.size() == 0 && run){
			line pxy1;
			//center point seed
			pxy1[0] = 0;
			pxy1[1] = 0;
			pxy1.i = &pxy1;
			pxy1.r = 0;
			pxy1.b = 0;
			pxy1.g = 0;
			pxy.push_back(pxy1);
			tree.insert(&pxy.back());

		}
		if (run){
			printf("started \n");
		}
		else{
			printf("paused \n");
		}
	}
	//clear
	if (key == GLUT_KEY_F2){
		if (!run){
			printf("cleared \n");
			pxy.clear();
			tree.clear();
			r = startingr;
			timerend = false;
			t0 = clock();
			ax = 0;
			ay = 0;
		}
	}
	if (key == GLUT_KEY_F3){
		startingr += startingr / 10;
		r = startingr;
	}
	if (key == GLUT_KEY_F4){
		if (startingr - startingr/10 > 1){
			startingr -= startingr / 10;
			r = startingr;
		}
	}
	
}
void keyups(unsigned char key, int x, int y){
	keys[key] = false;
}
void keyevent(unsigned char key, int x, int y){
	keys[key] = true;
	if (key == 27){
		glutExit();
	}
}
void mwheel(int buton, int dir, int x, int y){
	if (dir > 0){
		zoom += zoom / 10.0f;
	}
	else{
		if (zoom > 1){
			zoom -= zoom / 10.0f;
		}
	}
}
void reshape(int w, int h){
	if (h == 0)
		h = 1;
	float ratio = 1.0f* w / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(60, ratio, 0.1, 15000);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	t0 = clock();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Diffuse Limited Aggregation");
	init();

	glutDisplayFunc(drawscene);
	glutReshapeFunc(reshape);
	glutIdleFunc(drawscene);

	glutKeyboardFunc(keyevent);
	glutKeyboardUpFunc(keyups);
	glutSpecialUpFunc(specialkeyups);
	glutSpecialFunc(specialkeys);
	glutMouseWheelFunc(mwheel);
	printf("Keys: \n Esc to Exit \n F1: start/pause \n F2: clear if paused \n F3-4 increase/decrease rings \n");
	glutMainLoop();
	return 0;
}