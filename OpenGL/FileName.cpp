#include <GL/freeglut.h>

float theta = 0.0f;

void display()
{
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();

    glRotatef(theta, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.6f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(-0.6f, -0.6f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(0.6f, -0.6f);

    glEnd();

    glutSwapBuffers();
}

void update(int value)
{
    theta += 1.0f;

    if (theta >= 360.0f)
        theta -= 360.0f;

    glutPostRedisplay();

    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB
    );

    glutInitWindowSize(800, 600);

    glutCreateWindow("OpenGL Rainbow Triangle");

    glutDisplayFunc(display);

    glutTimerFunc(16, update, 0);

    glutMainLoop();

    return 0;
}