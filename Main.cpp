
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
typedef struct line : std::array<float, 2> { float r, g, b, dist; struct line* i; } line;
typedef struct spawnpoint : std::array < float, 2 > {float r; struct spawnpoint * left, *right; } spawnpoint;

static const double pi = 3.14159265359;
static const int Range = 2.0f;//distance between each point
static const int sqsize = 1000;

#define getrandom(min, max) (((double)rand() / (double)RAND_MAX)*(max-min)+min)

kdtree<line, 2, 10, sqsize, Range> tree;

bool keys[256];
int width = 1024, height = 768;
//float *px;float *py;
std::vector<line> pxy;

std::vector<spawnpoint> spawnring;

int np = 120000;// Maximum number of points
float r = 5.0f,startingr =  10.0f;

int close = 0;
float zoom = 500;
float t0;
bool timerend = false,run=false;
float Rangep2 = Range * Range;
void addpoint();
bool inrange(float, float, float);
int fff = 0;

void quadpoint(line l,float size){
	//glBegin(GL_QUADS);
	float xv = (l.i[0][0] - l[0])/l.dist,
		yv = (l.i[0][1] - l[1])/l.dist;

		glColor3f(l.r,l.g,l.b);
		glVertex3f(l[0] - yv*size, l[1] + xv*size, 0);
		glVertex3f(l[0] + yv*size, l[1] - xv*size, 0);

		glColor3f(l.i->r, l.i->g, l.i->b);
		glVertex3f(l.i[0][0] + yv*size, l.i[0][1] - xv*size, 0);
		glVertex3f(l.i[0][0] - yv*size, l.i[0][1] + xv*size, 0);
	//glEnd();
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
		glVertex3f(0 + cos(i)*rad, 0 + sin(i)*rad, 0);
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
	glBegin(GL_POINTS);
	for (int i = 0; i < pxy.size(); i++){
		glColor3f(pxy[i].r, pxy[i].g, pxy[i].b);
		glVertex3f(pxy[i][0], pxy[i][1], 0);
		//quadpoint(pxy[i],0.2f);
	}
	glEnd();
	
	glBegin(GL_POINTS);
	glColor3f (0,   1, 0);
	glVertex3f(0, 0.1, 0);
	glVertex3f(1, 0.1, 0);
	glEnd();

	glBegin(GL_POINTS);
	spawnpoint * sp = &spawnring[0];
	do{
		glColor3f(0, 1, 0);
		glVertex3f(sp[0][0] * sp->r, sp[0][1] * sp->r, 0);
		sp = sp->left;
	} while (sp != &spawnring[0]);
	glEnd();
	//circles for tree size (there might be errors if the model gets biggger)
	drawcircle(sqsize);
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
	float dd = 0, d = 0;
	float *t;
	bool b = false;
	int f = fff;// (int)(random2(0, 1) * 360);
	line templ;
	line pxyp;
	templ[0] = spawnring[f][0] * (spawnring[f].r - 5);
	templ[1] = spawnring[f][1] * (spawnring[f].r - 5);

	float a, degi;
	do{
		if (spawnring[f].r < dd + 2){
			f = fff;// (int)(random2(0, 1) * 360);
			templ[0] = spawnring[f][0] * (spawnring[f].r - 5);
			templ[1] = spawnring[f][1] * (spawnring[f].r - 5); 
		}
		for (int i = 0; i < 5 && !b; i++){
			templ[0] += random2(-3, 3);
			templ[1] += random2(-3, 3);

			dd = euclidis(templ[0], templ[1], 0, 0);
			std::vector<line*> a = tree.getbucket(templ);
			for (int i = 0; i < a.size(); i++){
				float d = cheapdist(templ[0], templ[1], (*a[i])[0], (*a[i])[1]);
				if (d <= Rangep2){
					b = true;
					pxyp.i = a[i];
					pxyp.dist = d;
					break;
				}
			}
		}
		
		a = (pi+atan2f(templ[1], -templ[0])) * (180.0f/pi);
		f = (int)a;
	} while (!b);
	
	if (spawnring[f].r < dd+5){
		spawnring[f].r = dd + 25;
		//cascade the r adjustments left and right
		spawnpoint * tsp = &spawnring[f];
		// this adjusts how tight the spawning ring is, at 0 it acts just like a circle
		// higher numbers increase the density
		float cascadeoffset = 1.2f;
		while (tsp->r > tsp->left->r + cascadeoffset){
			tsp->left->r = tsp->r - cascadeoffset;
			tsp = tsp->left;
		}
		tsp = &spawnring[f];
		while (tsp->r > tsp->right->r + cascadeoffset){
			tsp->right->r = tsp->r - cascadeoffset;
			tsp = tsp->right;
		}
	}
	//printf("x= %f, y = %f \n",nx,ny);
	pxyp[0] = templ[0];
	pxyp[1] = templ[1];
	pxyp.r = (sinf(dd/50) + 3) / 2;
	pxyp.g = (sinf(dd/50) + 1) / 2;
	pxyp.b = (sinf(dd/50) + 1) / 2;
	pxy.push_back(pxyp);
	tree.insert(&pxy.back());
	fff++;
	if (fff > 358){
		fff = 0;
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

void genspawnring(){
	spawnring.clear();
	spawnring.reserve(360);
	float pid360 = 2* pi / 360;
	float ii = 360;
	for (float i = 0; i < 2 * pi; i += pid360){
		spawnpoint sp;
		sp[0] = cosf(i);
		sp[1] = sinf(-i);
		sp.r = startingr*2;
		spawnring.push_back(sp);
	}
	
	for (int i = 1; i < spawnring.size()-1; i++){
		spawnring[i].left = &spawnring[i - 1];
		spawnring[i].right = &spawnring[i + 1];
	}

	spawnring[0].left = &spawnring[359];
	spawnring[0].right = &spawnring[1];

	spawnring[359].left =  &spawnring[358];
	spawnring[359].right = &spawnring[0];
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
	genspawnring();

	//int f = (int)((pi + atan2(0.5f, -0.5f)) * 180 / pi);

	//spawnring[f].r = 20;
	//spawnpoint * tsp = &spawnring[f];
	//while (tsp->r / tsp->left->r > 1.3f){

	//	tsp->left->r *= (tsp->r / tsp->left->r) / 2.0f;
	//	tsp = tsp->left;
	//}
	//tsp = &spawnring[f];
	//while (tsp->r / tsp->right->r > 1.3f){

	//	tsp->right->r *= (tsp->r / tsp->right->r) / 2.0f;
	//	tsp = tsp->right;
	//}


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