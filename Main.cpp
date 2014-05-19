
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
static const float Range = 2.0f;//distance between each point
static const int sqsize = 400;

#define getrandom(min, max) (((double)rand() / (double)RAND_MAX)*(max-min)+min)
#define circrandx(i,r) (0+cos(i)*r)
#define circrandy(i,r) (0+sin(i)*r)

kdtree<line, 2, 20, sqsize, 2> tree;

bool keys[256];
int width = 1024, height = 768;
//float *px;float *py;
line *pxy;

int np = 60000;// Maximum number of points
int p = 1;//current point amount
float r = 5.0f;
int close = 0;
float zoom = 500;
float t0;
bool timerend = false;
float Rangep2 = Range * Range;
void addpoint();
bool inrange(float, float, float);


void drawcircle(float rad){
	glBegin(GL_POINTS);
	for (float i = 0; i<2 * pi; i += pi / 50){
		glColor3f(0, 1, 1);
		glVertex3f(cos(i)*rad, sin(i)*rad, 0);
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
	//HandleKeys();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -zoom);
	glPointSize(1);
	glBegin(GL_LINES);
	int cc = 0;
	for (int i = 0; i<p; i ++){
		glColor3f(pxy[i].r, pxy[i].g, pxy[i].b);		
		glVertex3f(pxy[i][0], pxy[i][1], 0);

		glColor3f((*pxy[i].i).r, (*pxy[i].i).g, (*pxy[i].i).b);
		glVertex3f((*pxy[i].i)[0], (*pxy[i].i)[1], 0);

		//glColor3f(pxy[pxy[i].i].r, pxy[pxy[i].i].g, pxy[pxy[i].i].b);
		//glVertex3f(pxy[pxy[i].i][0], pxy[pxy[i].i][1], 0);
		//cc += 3;
	}
	glEnd();

	glBegin(GL_POINTS);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0.1, 0);
	glVertex3f(1, 0.1, 0);
	glEnd();

	drawcircle(r);
	drawcircle(sqsize);
	drawcircle(r + 5.0f);
	drawcircle(r - 5.0f);
	if (p<np){
		for (int i = 0; i<100; i++){
			addpoint();
		}
	}
	else if (!timerend){
		printf("finished in %f seconds", ((float)clock() - t0) / CLOCKS_PER_SEC);
		timerend = true;
	}

	glutSwapBuffers();
}
void addpoint(){
	float f, dd = 0, d = 0;
	float *t;
	bool b = false;
	f = random2(0, 2 * pi);
	line templ;
	templ[0] = cos(f)*r;
	templ[1] = sin(f)*r;

	
	do{
		if (dd>r + 5){
			f = random2(0, 2 * pi);
			templ[0] = cos(f)*r;
			templ[1] = sin(f)*r;
		}
		templ[0] += random2(-3, 3);
		templ[1] += random2(-3, 3);

	//sucks points into the center. speeds thing up alot and makes the model more rounded
		templ[0] *= 0.999;
		templ[1] *= 0.999;

		dd = euclidis(templ[0], templ[1], 0, 0);
		std::vector<line*> a = tree.getbucket(templ);
		for (int i = 0; i<a.size(); i++){
			float d = cheapdist(templ[0], templ[1], (*a[i])[0], (*a[i])[1]);
			if (d <= Rangep2){
				b = true;
				pxy[p].i = a[i];
				break;
			}
		}
	} while (!b);

	//printf("x= %f, y = %f \n",nx,ny);
	pxy[p][0] = templ[0];
	pxy[p][1] = templ[1];
	pxy[p].r = 0;
	pxy[p].g = (sinf(dd/20) + 1) / 2;
	pxy[p].b = (sinf(dd/50) + 1) / 2;
	
	tree.insert(&pxy[p]);
	p++;
	if (dd > r){
		r = dd;
	}
}

bool inrange(float range, float npx, float npy){
	for (int i = 0; i<p; i += 2){
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
	pxy = new line[np];
	//center point seed
	pxy[0][0] = 0;
	pxy[0][1] = 0;
	pxy[0].i = &pxy[0];
	pxy[0].r = 0;
	pxy[0].b = 0;
	pxy[0].g = 0;
	p = 1;
	tree.insert(&pxy[0]);
}

void specialkeyups(int key, int x, int y){
	keys[key] = false;
}
void specialkeys(int key, int x, int y){
	keys[key] = true;
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

	glutMainLoop();
	return 0;
}