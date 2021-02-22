#include <GLFW/glfw3.h>
#include <time.h>
#include <vector>
#include <string>

int Get_Width() { return glfwGetVideoMode(glfwGetPrimaryMonitor())->width; }
int Get_Height() { return glfwGetVideoMode(glfwGetPrimaryMonitor())->height; }

#define Screen_Width Get_Width()
#define Screen_Height Get_Height()
#define PI 3.141592653589793238
#define NoOfRays 50

bool QuitApp = false;

// To store coordinates
struct Point
{
    float x, y;

    Point(float X = 0.0f, float Y = 0.0f)
    {
        x = X, y = Y;
    }

    void Add(float X, float Y)
    {
        x = X; y = Y;
    }

    bool operator==(const Point& o) const {
        return x == o.x && y == o.y;
    }

    bool operator<(const Point& o)  const {
        return x < o.x || (x == o.x && y < o.y);
    }
};

struct Quad
{
    float X, X1, Y, Y1;

    Quad()
    {
        X = 0; X1 = 0; Y = 0; Y1 = 0;
    }

    Quad(float x, float x1, float y, float y1)
    {
        X = x; X1 = x1; Y = y; Y1 = y1;
    }

    bool operator==(const Quad& o) const {
        return X == o.X && X1 == o.X1 && Y == o.Y && Y1 == o.Y1;
    }

    bool operator<(const Quad& o)  const {
        return X < o.X || (X == o.X && Y < o.Y);
    }
};

class Map
{
private:

    // To store coordinates of Walls
    std::vector<Quad> Walls;
    Point SourcePosition, Scale;
    const int size = 20;

public:

    Map()
    {
        SourcePosition.x = Screen_Width / size; SourcePosition.y = Screen_Height / size;
        Scale.x = 0; Scale.y = 0;

        bool IsBoundary = false;
        Walls.reserve(int(SourcePosition.x));
      
        // Assigning Walls
        for (unsigned int i = 0; i < SourcePosition.y; i++)
        {
            for (unsigned int j = 0; j < SourcePosition.x; j++)
            {
                IsBoundary = (i == 0 || j == 0 || i == SourcePosition.y - 1 || j == SourcePosition.x - 1);
         
                // Random walls inside
                if (!IsBoundary && rand() % i == 3 && rand() % 2 == 0)
                {
                    Scale.x = j * size; Scale.y = i * size;

                    if (Scale.x != Screen_Width / 2 && Scale.y != Screen_Height / 2)
                    {
                        Walls.emplace_back(Scale.x, Scale.x + size, Scale.y, Scale.y + size);
                    }
                }
            }
        }
    }

    void DrawGrid()
    {
        for (auto itr = Walls.begin(); itr != Walls.end(); itr++)
        {
            glColor4f(0.80, 0.80, 0.80, 1);
            glBegin(GL_QUADS);
            glVertex2f(itr->X, itr->Y);
            glVertex2f(itr->X, itr->Y1);
            glVertex2f(itr->X1, itr->Y1);
            glVertex2f(itr->X1, itr->Y);
            glEnd();
        }
    }

    // Function to check if ray collides with wall
    bool CheckCollision(float Px, float Py, float MoveFactor, int Precision) const
    {
        float Round = MoveFactor * Precision;
        float X1 = Px + Round, X = Px - Round, Y1 = Py + Round, Y = Py - Round;

        for (auto itr = Walls.begin(); itr != Walls.end(); itr++)
        {
            // if collides
            if (X1 >= itr->X && X <= itr->X1
                && Y1 >= itr->Y && Y <= itr->Y1)
            {
                return false;
            }
        }

        // else
        return true;
    }
};

class RayCasting
{
private:

    Point SourcePosition;  // Source position
    Point AlphaRayDirection; // The direction in which the source moves
    Point Directions[NoOfRays];  // Directions for Rays
    Point AlphaRay;  // Alpha Direction Ray
    Point Rays[NoOfRays]; // Rays with w.r.t Alpha Ray
    Point Borders;
    float Angle;
    const float AngleFactor = 0.01; // Increase it for faster rotation.
    const float MoveFactor = 0.75;  // For collision detection
    const int BorderLimit = 5; // To remain in bounds
    const float Magnify = 1.5;  // Helps in calculating ray length
    const float Precision = 0.5; // Decrease it for more accuracy with walls, Decreasing will lower FPS.                   
    const int Speed = 20; // Speed at which Source moves, Decrease it to move source faster.

public:

    RayCasting()
    {
        Borders.Add(Screen_Width - BorderLimit, Screen_Height - BorderLimit);

        SourcePosition.x = Screen_Width / 2; SourcePosition.y = Screen_Height / 2;
        Angle = 0.0f; 

        AlphaRayDirection.x = cos(Angle) * Magnify;
        AlphaRayDirection.y = sin(Angle) * Magnify;

        AlphaRay.x = SourcePosition.x + AlphaRayDirection.x * Magnify;
        AlphaRay.y = SourcePosition.y + AlphaRayDirection.y * Magnify;

        int Ray = 0; float TempAngle = Angle;

        // 1st Quadrant rays w.r.t Alpha Ray
        for (; Ray < NoOfRays/2; Ray++)
        {
            TempAngle -= AngleFactor;

            if (TempAngle < 0)
            {
                TempAngle += 2 * PI;
            }

            Directions[Ray].x = cos(TempAngle) * 5;
            Directions[Ray].y = sin(TempAngle) * 5;
          
            Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
            Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
        }

        // Alpha Ray which is in the middle
        Directions[NoOfRays / 2].x = AlphaRayDirection.x;
        Directions[NoOfRays / 2].y = AlphaRayDirection.y;

        Rays[NoOfRays / 2].x = SourcePosition.x + Directions[NoOfRays / 2].x * Magnify;
        Rays[NoOfRays / 2].y = SourcePosition.y + Directions[NoOfRays / 2].y * Magnify;

        TempAngle = Angle; Ray++;

        // 4th Quadrant Rays w.r.t to Alpha Ray
        for (; Ray < NoOfRays; Ray++)
        {
            TempAngle += AngleFactor;

            if (TempAngle > 2 * PI)
            {
                TempAngle -= 2 * PI;
            }

            Directions[Ray].x = cos(TempAngle) * 5;
            Directions[Ray].y = sin(TempAngle) * 5;
            Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
            Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
        }
   
    }

    float GetPositionX() const
    {
        return SourcePosition.x;
    }

    float GetPositionY() const
    {
        return SourcePosition.y;
    }

    float GetMoveFactor() const
    {
        return MoveFactor;
    }

    void DrawSource() const
    {
        glPointSize(15);
        glBegin(GL_POINTS);
        glColor4f(1, 1, 1, 1);
        glVertex2f(SourcePosition.x, SourcePosition.y);
        glEnd();
    }

    void DrawRays(const Map& World)
    {
        bool Continue = true;
        float LocalMagnify = Magnify;
  
        // Calculating length of all rays.
        for (int i = 0; i < NoOfRays; i++)
        {
            Continue = true;
            LocalMagnify = Magnify;

            // needs optimization
            while (Continue)
            {
                // if going out of bounds
                if (Rays[i].x > Borders.x || Rays[i].x < BorderLimit
                    || Rays[i].y < BorderLimit || Rays[i].y > Borders.y)
                {
                    // break loop
                    Continue = false;

                    // Decrease ray length
                    LocalMagnify -= (Precision * (LocalMagnify > 0));
                }

                // if colliding with a wall
                else if (!World.CheckCollision(Rays[i].x, Rays[i].y, MoveFactor, 2))
                {
                    // break loop
                    Continue = false;

                    // Decrease ray length
                    LocalMagnify -= (Precision * (LocalMagnify > 0));
                }

                // else increase ray length
                else
                {
                    LocalMagnify += Precision;
                }

                // Update Ray Position
                Rays[i].x = SourcePosition.x + Directions[i].x * LocalMagnify;
                Rays[i].y = SourcePosition.y + Directions[i].y * LocalMagnify;
            }


            // Draw Ray
            glColor4f(0.95, 0.95, 0.4, 1);
            glLineWidth(1);
            glBegin(GL_LINES);
            glVertex2f(SourcePosition.x, SourcePosition.y);
            glVertex2f(Rays[i].x, Rays[i].y);
            glEnd();
        }
        
    }

    // Move Source
    void Input(GLFWwindow* window, bool AllowMove)
    {
        // if 'S' Pressed
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            float CheckY = SourcePosition.y - AlphaRayDirection.y / Speed,
                CheckX = SourcePosition.x - AlphaRayDirection.x / Speed;

            // if within border limit
            if (CheckY > BorderLimit && CheckY < Screen_Height - BorderLimit
                && CheckX > BorderLimit && CheckX < Screen_Width - BorderLimit)
            {
                // if not colliding with a wall then allow move
                if (AllowMove)
                {
                    SourcePosition.y -= AlphaRayDirection.y / Speed;
                    SourcePosition.x -= AlphaRayDirection.x / Speed;

                }
                // if colliding with a wall then don't allow to pass
                else
                {
                    SourcePosition.y += AlphaRayDirection.y;
                    SourcePosition.x += AlphaRayDirection.x;

                }

            }

        }

        // if 'W' Pressed
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            float CheckY = SourcePosition.y + AlphaRayDirection.y / Speed,
                CheckX = SourcePosition.x + AlphaRayDirection.x / Speed;

            // if within border limit
            if (CheckY > BorderLimit && CheckY < Screen_Height - BorderLimit
                && CheckX > BorderLimit && CheckX < Screen_Width - BorderLimit)
            {
                // if not colliding with a wall then allow move
                if (AllowMove)
                {
                    SourcePosition.y += AlphaRayDirection.y / Speed;
                    SourcePosition.x += AlphaRayDirection.x / Speed;

                }
                // if colliding with a wall then don't allow to pass
                else
                {
                    SourcePosition.y -= AlphaRayDirection.y;
                    SourcePosition.x -= AlphaRayDirection.x;

                }
            }

        }

        // if 'D' Pressed
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            // Increase Angle
            Angle += AngleFactor;

            // if Angle greater than 360
            if (Angle > 2 * PI)
            {
                // Reset
                Angle -= 2 * PI;
            }

            // Update Direction for Alpha Ray
            AlphaRayDirection.x = cos(Angle) * 5;
            AlphaRayDirection.y = sin(Angle) * 5;

            int Ray = 0; float TempAngle = Angle;

            // 1st Quadrant rays w.r.t Alpha Ray
            for (; Ray < NoOfRays / 2; Ray++)
            {
                TempAngle -= AngleFactor;

                if (TempAngle < 0)
                {
                    TempAngle += 2 * PI;
                }

                Directions[Ray].x = cos(TempAngle) * 5;
                Directions[Ray].y = sin(TempAngle) * 5;

                Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
                Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
            }

            // Alpha Ray which is in the middle
            Directions[NoOfRays / 2].x = AlphaRayDirection.x;
            Directions[NoOfRays / 2].y = AlphaRayDirection.y;

            Rays[NoOfRays / 2].x = SourcePosition.x + Directions[NoOfRays / 2].x * Magnify;
            Rays[NoOfRays / 2].y = SourcePosition.y + Directions[NoOfRays / 2].y * Magnify;

            TempAngle = Angle; Ray++;

            // 4th Quadrant Rays w.r.t to Alpha Ray
            for (; Ray < NoOfRays; Ray++)
            {
                TempAngle += AngleFactor;

                if (TempAngle > 2 * PI)
                {
                    TempAngle -= 2 * PI;
                }

                Directions[Ray].x = cos(TempAngle) * 5;
                Directions[Ray].y = sin(TempAngle) * 5;
                Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
                Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
            }
        }

        // if 'A' Pressed
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            // Decrease Angle
            Angle -= AngleFactor;

            // if Angle less than 0
            if (Angle < 0)
            {
                // Reset
                Angle += 2 * PI;
            }

            // Update Direction for Alpha Ray
            AlphaRayDirection.x = cos(Angle) * 5;
            AlphaRayDirection.y = sin(Angle) * 5;

            int Ray = 0; float TempAngle = Angle;

            // 1st Quadrant rays w.r.t Alpha Ray
            for (; Ray < NoOfRays / 2; Ray++)
            {
                TempAngle -= AngleFactor;

                if (TempAngle < 0)
                {
                    TempAngle += 2 * PI;
                }

                Directions[Ray].x = cos(TempAngle) * 5;
                Directions[Ray].y = sin(TempAngle) * 5;

                Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
                Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
            }

            // Alpha Ray which is in the middle
            Directions[NoOfRays / 2].x = AlphaRayDirection.x;
            Directions[NoOfRays / 2].y = AlphaRayDirection.y;

            Rays[NoOfRays / 2].x = SourcePosition.x + Directions[NoOfRays / 2].x * Magnify;
            Rays[NoOfRays / 2].y = SourcePosition.y + Directions[NoOfRays / 2].y * Magnify;

            TempAngle = Angle; Ray++;

            // 4th Quadrant Rays w.r.t to Alpha Ray
            for (; Ray < NoOfRays; Ray++)
            {
                TempAngle += AngleFactor;

                if (TempAngle > 2 * PI)
                {
                    TempAngle -= 2 * PI;
                }

                Directions[Ray].x = cos(TempAngle) * 5;
                Directions[Ray].y = sin(TempAngle) * 5;
                Rays[Ray].x = SourcePosition.x + Directions[Ray].x * Magnify;
                Rays[Ray].y = SourcePosition.y + Directions[Ray].y * Magnify;
            }
        }


        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            QuitApp = true;
        }
    }

};

int main(int argc, char** argv)
{
    GLFWwindow* window;

    // Random time seed generator
    srand(time(NULL));

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(Screen_Width, Screen_Height, "Ray-Casting", glfwGetPrimaryMonitor(), NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
   
   
    // Player and Map Object
    RayCasting RayCaster; Map World;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && !QuitApp)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // To draw
        glLoadIdentity();
        glOrtho(0, Screen_Width, Screen_Height, 0, 100, -100);

        // Draw Source
        RayCaster.DrawSource();

        // Draw World
        World.DrawGrid();

        // returns true if no collision
        bool AllowMove = World.CheckCollision(RayCaster.GetPositionX(), RayCaster.GetPositionY(), RayCaster.GetMoveFactor(), 15);

        // Source Movement
        RayCaster.Input(window, AllowMove);

        // Draw Rays
        RayCaster.DrawRays(World);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}