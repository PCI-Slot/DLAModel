
#include <stdio.h>
#include <iostream>
#include <GL\freeglut.h>
#include <ctime>

static const double pi = 3.14159265359;
static const float Range = 2.0f;//distance between each point

#define getrandom(min, max) (((double)rand() / (double)RAND_MAX)*(max-min)+min)
#define circrandx(i,r) (0+cos(i)*r)
#define circrandy(i,r) (0+sin(i)*r)

bool keys[256];
int width = 1024, height = 768;
//float *px;float *py;
float *pxy;
float *colours;
int np = 2000;// Maximum number of points
int p = 1;//current point amount
int cc = 0; //colours counter
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
	glBegin(GL_POINTS);
	int cc = 0;
	for (int i = 0; i<p; i += 2){
		glColor3f(colours[cc], colours[cc + 1], colours[cc + 2]);
		glVertex3f(pxy[i], pxy[i + 1], 0);
		cc += 3;
	}
	glEnd();

	glBegin(GL_POINTS);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0.1, 0);
	glVertex3f(1, 0.1, 0);
	glEnd();

	drawcircle(r);
	drawcircle(r + 5.0f);
	drawcircle(r - 5.0f);
	if (p<np * 2){
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
	float f, dd = 0, nx, ny, d = 0;
	float *t;
	bool b;
	f = random2(0, 2 * pi);
	nx = cos(f)*r;
	ny = sin(f)*r;
	do{
		if (dd>r + 5){
			f = random2(0, 2 * pi);
			nx = cos(f)*r;
			ny = sin(f)*r;
		}
		nx += random2(-3, 3);
		ny += random2(-3, 3);
		dd = euclidis(nx, ny, 0, 0);

		b = inrange(Rangep2, nx, ny);
	} while (!b);

	//printf("x= %f, y = %f \n",nx,ny);
	pxy[p++] = nx*0.999;
	pxy[p++] = ny*0.999;
	/// point colour
	colours[cc] = (float)p / np;
	colours[cc + 1] = (r >= 1 ? (float)p / np : 0);
	colours[cc + 2] = 1;
	cc += 3;
	if (dd > r){
		r = dd;
	}

}

bool inrange(float range, float npx, float npy){
	for (int i = 0; i<p; i += 2){
		float d = cheapdist(npx, npy, pxy[i], pxy[i + 1]);
		if (d <= range){
			return true;
		}
	}
	return false;
}

void init(){
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);

	colours = new float[np * 3];
	pxy = new float[np * 2];
	colours[cc] = 1;
	colours[cc + 1] = 1;
	colours[cc + 2] = 1;
	cc += 3;
	pxy[0] = 0;
	pxy[1] = 0;
	p = 2;
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