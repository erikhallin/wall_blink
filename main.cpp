#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <time.h>

#define _pi 3.1415926

int  g_window_width=512;
int  g_window_height=512;
float g_color[3];
float g_color_start[3];
float g_color_goal[3];
float g_time_counter=0.0;
float g_time_max=5.0;
float g_time_last=0.0;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
bool draw_scene(void);
void draw_circle(float cx,float cy,float r,int num_segments,float color[3]);
bool update_color(void);
bool set_color(void);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "Color Simulator";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    g_window_width = desktop.right;
    g_window_height = desktop.bottom;

    hwnd = CreateWindowEx(0,
                          "Color Simulator",
                          "Color Simulator",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_window_width,
                          g_window_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);
    ShowCursor(FALSE);

    EnableOpenGL(hwnd, &hDC, &hRC);

    //reset color
    g_color[0]=g_color_goal[0]=g_color_start[0]=0.2;
    g_color[1]=g_color_goal[1]=g_color_start[1]=0.2;
    g_color[2]=g_color_goal[2]=g_color_start[2]=0.2;

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            update_color();
            draw_scene();
            SwapBuffers(hDC);
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);

    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE: PostQuitMessage(0); break;

                case VK_SPACE: set_color(); break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_window_width,g_window_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_window_width,g_window_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

bool draw_scene(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();

    //go to center
    glTranslatef(g_window_width*0.5,g_window_height*0.5,0.0);

    //draw rows
    int numof_rows=10;
    int numof_leds=20;
    float radius_in=float(g_window_height)*0.5*0.2;
    float radius_out=float(g_window_height)*0.5*0.9;
    float black[3]={0.2,0.2,0.2};
    for(int row_i=0;row_i<numof_rows;row_i++)
    {
        float angle = 2.0*_pi*float(row_i)/float(numof_rows);//get the current angle
		//draw LEDs
		for(int led_i=0;led_i<numof_leds;led_i++)
		{
            float radius_curr=radius_in+(radius_out-radius_in)*float(led_i)/(float)numof_leds;
            float x = radius_curr * cosf(angle);
            float y = radius_curr * sinf(angle);

            if(g_time_counter>4.0 && led_i>(5.0-g_time_counter)*float(numof_leds)) draw_circle(x,y,5.0,10,black);
            else draw_circle(x,y,5.0,10,g_color);
		}
    }


    glPopMatrix();



    return true;
}

void draw_circle(float cx, float cy, float r, int num_segments,float color[3])
{
	float theta = 2 * 3.1415926 / float(num_segments);
	float tangetial_factor = tanf(theta);//calculate the tangential factor

	float radial_factor = cosf(theta);//calculate the radial factor

	float x = r;//we start at angle = 0

	float y = 0;

	glBegin(GL_TRIANGLE_FAN);
	glColor3fv(color);
	glVertex2f(cx,cy);
	glColor3f(color[0]*0.5,color[1]*0.5,color[2]*0.5);
	for(int ii = 0; ii < num_segments; ii++)
	{
		glVertex2f(x + cx, y + cy);//output vertex

		//calculate the tangential vector
		//remember, the radial vector is (x, y)
		//to get the tangential vector we flip those coordinates and negate one of them

		float tx = -y;
		float ty = x;

		//add the tangential vector

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		//correct using the radial factor

		x *= radial_factor;
		y *= radial_factor;
	}
	//last point
	glVertex2f(r + cx, 0.0 + cy);
	glEnd();
}

bool update_color(void)
{
    float time_now=(float)clock()/CLOCKS_PER_SEC;
    float time_dif=time_now-g_time_last;
    g_time_last=time_now;

    if(g_time_counter>0.0)
    {
        g_time_counter-=time_dif;
        if(g_time_counter<=0.0)
        {
            g_time_counter=0.0;
        }
    }

    //update color
    for(int i=0;i<3;i++)
     g_color[i]=g_color_goal[i]+(g_color_start[i]-g_color_goal[i])*g_time_counter/g_time_max;

    return true;
}

bool set_color(void)
{
    g_time_counter=g_time_max;

    //set random color
    int rand_val=rand()%4;

    switch(rand_val)
    {
        case 0: g_color_goal[0]=1.0;g_color_goal[1]=0.0;g_color_goal[2]=0.0; break;
        case 1: g_color_goal[0]=0.0;g_color_goal[1]=1.0;g_color_goal[2]=0.0; break;
        case 2: g_color_goal[0]=0.0;g_color_goal[1]=0.0;g_color_goal[2]=1.0; break;
        case 3: g_color_goal[0]=1.0;g_color_goal[1]=1.0;g_color_goal[2]=0.0; break;
    }

    g_color_start[0]=float(rand()%100)/100.0;
    g_color_start[1]=float(rand()%100)/100.0;
    g_color_start[2]=float(rand()%100)/100.0;

    return true;
}

