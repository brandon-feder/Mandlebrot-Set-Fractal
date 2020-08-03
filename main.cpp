#include <GL/glut.h>
#include <iostream> 
#include <unistd.h> 
#include <signal.h>
#include <sys/shm.h>
#include <cmath>

#include "helpers.cpp"
#include "mandlebrot.cpp"
#include "shaders.cpp"

#define WIDTH 1500
#define HEIGHT 1000
#define nProcesses 12

#define DISPLAY_FPS false
#define WAIT_FOR_FULL_FRAME false

// Center Cordinates
double *cx;
double *cy;

// Zoom Factor
double *zoom;

// Shared Variabvles
int *pixels;
bool *processState;

// Get the max recursion depth to run
int getMaxRecursionDepth() {
    int x = 20;
    int n = x*log(zoom[0]);

    if(n < x) {
        return x;
    }
    return n;
}

// Determines whether the canvas should be updated
bool updateDisplay() {
    if(WAIT_FOR_FULL_FRAME) {
        for(int i = 0; i < nProcesses; i++) {
            if(processState[i] == true) {
                return false;
            }
        }

        return true;
    } else {
        return true;
    }
}

void display() {
    if(updateDisplay()) {
        glLoadIdentity(); // Idk what this does

        gluOrtho2D( 0.0, WIDTH, HEIGHT, 0.0 ); // Setup the shape to draw on, I think
        
        // Start drawing all the pixels. TODO: Use textures to make more efficient
        glBegin(GL_POINTS);
        
        int maxZoom = getMaxRecursionDepth(); // Get the max recursion depth. TODO: Update name
        
        // Max and min depths found. Used to scale colors
        int maxDepthFound = 0;
        int minDepthFound = maxZoom;
        for(int i = 0; i < WIDTH*HEIGHT; i++) {
            if(pixels[i] > maxDepthFound) {
                maxDepthFound = pixels[i];
            }

            if(pixels[i] < minDepthFound) {
                minDepthFound = pixels[i];
            }
        }

        // For each pixel, get its x and y cordinates, apply a shader, and draw
        for(int i = 0; i < WIDTH*HEIGHT; i += 1) {
            int x = i % WIDTH;
            int y = i / WIDTH;

            float lerpPercent;
            if(pixels[i] == -1) {
                lerpPercent = -1;
            } else {
                lerpPercent = ((float)pixels[i]-minDepthFound)/(maxDepthFound-minDepthFound);
            }

            Color color = Shaders::shader1(lerpPercent);
            glColor4f(
                (float)color.red/255, 
                (float)color.green/255, 
                (float)color.blue/255, 
                (float)color.alpha/255
            );
            glVertex2i(x, y);
        }

        glEnd();
        glFlush(); // Draw the new pixels to canvas
    }
}

void idle() {
    glutPostRedisplay();
}

// Call the mandlebrot function to calculate whether point is element of set
int compute(int x, int y) {
    x -= WIDTH/2;
    y -= HEIGHT/2;

    float xCord = (float)x/WIDTH/zoom[0] + cx[0];
    float yCord = (float)y/WIDTH/zoom[0] + cy[0];

    int maxZoom = getMaxRecursionDepth();
    int res = Mandlebrot::calc(xCord, yCord, maxZoom);
    return res;
}

void mProcess(int mainId, int portion) {
    long portionSize = WIDTH*HEIGHT/nProcesses; // The number of pixels to process
    while(kill(mainId, 0) == 0) { // While the parent process is live
        if(updateDisplay()) {
            int maxZoom = getMaxRecursionDepth(); // TODO: Update name

            // For every pixelm get its cordinate, calculate whether in set, and return update its value in pixel array
            for(int i = portion*portionSize; i < portionSize*(portion+1) && kill(mainId, 0) == 0; i++) {
                int x = i % WIDTH;
                int y = i / WIDTH;
                
                int res = compute(x, y);

                if(res == -1) {
                    pixels[i] = -1;
                } else {
                    pixels[i] = (100*(float)res)/maxZoom;
                }
            }

            processState[portion] = false;
        }
    }
} 

void inputFunc(unsigned char key, int x, int y) {
    if(key == 'w') {
        cy[0] -= 0.02/zoom[0];
    } else if(key == 's') {
        cy[0] += 0.02/zoom[0];
    } else if(key == 'a') {
        cx[0] -= 0.02/zoom[0];
    } else if(key == 'd') {
        cx[0] += 0.02/zoom[0];
    } else if(key == '-' || key == '_') {
        zoom[0] /= 1.1;
    }  else if(key == '=' || key == '+') {
        zoom[0] *= 1.1;
    }

    for(int i = 0; i < nProcesses; i++) {
        processState[i] = true;
    }
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
    int mainPid = getpid(); // The Process ID of the Parent

    // ===== Set up variables in shared memory ======
        int shmid1 =  shmget(123451, sizeof(int)*HEIGHT*WIDTH, IPC_CREAT | 0666);
        pixels = (int *)shmat(shmid1, NULL,0);
        
        int shmid2 =  shmget(123452, sizeof(double), IPC_CREAT | 0666);
        cx = (double *)shmat(shmid2, NULL,0);

        int shmid3 =  shmget(123453, sizeof(double), IPC_CREAT | 0666);
        cy = (double *)shmat(shmid3, NULL,0);

        int shmid4 =  shmget(123454, sizeof(double), IPC_CREAT | 0666);
        zoom = (double *)shmat(shmid4, NULL,0);

        int shmid5 =  shmget(123455, nProcesses*sizeof(bool), IPC_CREAT | 0666);
        processState = (bool *)shmat(shmid5, NULL,0);

    // Set Default Values of Shared Variables
    cx[0] = 0;
    zoom[0] = 0.1;
    cy[0] = 0;

    // Generate child processes and call the function they will be running
    for(int i = 0; i < nProcesses && getpid() == mainPid; i++) {
        fork();
        if(getpid() != mainPid) {
            mProcess(mainPid, i);
        }
    }

    // If it is the main process, setup OpenGL
    if(getpid() == mainPid) {
        glutInit(&argc, argv);                  // Initialize GLUT
        glutInitWindowSize(WIDTH, HEIGHT);      // Set the window's initial width & height
        glutCreateWindow("Mandlebrot Set!!!");  // Create a window with the given title
        glMatrixMode( GL_PROJECTION );          // I have not clue
        glutIdleFunc(idle);                     // When OpenGL is idling, call idle()
        glutKeyboardFunc(inputFunc);            // Func to handle keboard
        glutDisplayFunc(display);               // Register display callback handler for window re-paint
        glutMainLoop();                         // Enter the infinitely event-processing loop
    }

    return 0;
}