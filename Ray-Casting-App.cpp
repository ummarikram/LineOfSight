#include <GLFW/glfw3.h>
#include <time.h>
#include <vector>
#include <string>

#define Screen_Width 600
#define Screen_Height 600
#define PI 3.141592653589793238
#define NoOfRays 50

// For FPS
double previousTime = glfwGetTime();
int frameCount = 0;

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
    Point Position, Scale;
    const int size = 20;

public:

    Map()
    {
        Position.x = Screen_Width / size; Position.y = Screen_Height / size;
        Scale.x = 0; Scale.y = 0;

        bool IsBoundary = false;

        // Assigning Walls
        for (unsigned int i = 0; i < Position.y; i++)
        {
            for (unsigned int j = 0; j < Position.x; j++)
            {
                IsBoundary = (i == 0 || j == 0 || i == Position.y - 1 || j == Position.x - 1);
         
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

    // Function to check if ray/player collides with wall
    bool CheckCollision(float Px, float Py, float MoveFactor)
    {
        float Round = MoveFactor * 10;

        for (auto itr = Walls.begin(); itr != Walls.end(); itr++)
        {
            // if collides
            if (Px + Round >= itr->X && Px - Round <= itr->X1
                && Py + Round >= itr->Y && Py - Round <= itr->Y1)
            {
                return false;
            }
        }

        // else
        return true;
    }

};

class Entity
{

private:

    Point Position;  // Source position
    Point Direction; // Source direction w.r.t Ray
    Point Directions[NoOfRays];  // Directions for Rays
    Point Ray;  // Alpha Direction Ray
    Point Rays[NoOfRays]; // Rays with w.r.t Alpha Ray
    float Angle;
    const float AngleFactor = 0.008; // Increase it for faster rotation.
    float MoveFactor;  // For collision detection
    const int BorderLimit = 5; // To remain in bounds
    float Magnify;  // Helps in calculating ray length
    const float Precision = 0.05; // Decrease it for more accuracy with walls
                                  // Decreasing will lower FPS.
    const int Speed = 15; // Speed at which Source moves
                          // Decrease it to move source faster.

public:

    Entity()
    {
        Position.x = Screen_Width / 2; Position.y = Screen_Height / 2; MoveFactor = 0.5; Angle = 0.0f;
        
        Magnify = 1.5;

        Direction.x = cos(Angle) * Magnify;
        Direction.y = sin(Angle) * Magnify;

        Ray.x = Position.x + Direction.x * Magnify;
        Ray.y = Position.y + Direction.y * Magnify;

        for (int i = 0; i < NoOfRays; i++)
        {
            Directions[i].x = cos(Angle) * Magnify;
            Directions[i].y = sin(Angle) * Magnify;
            Rays[i].x = Position.x + Directions[i].x * Magnify;
            Rays[i].y = Position.y + Directions[i].y * Magnify;
        }
   
    }

    
    float GetPositionX() const
    {
        return Position.x;
    }

    float GetPositionY() const
    {
        return Position.y;
    }

    float GetMoveFactor() const
    {
        return MoveFactor;
    }

    int GetBorderLimit() const
    {
        return BorderLimit;
    }

    // Draw Player
    void Draw() const
    {
        glPointSize(15);
        glBegin(GL_POINTS);
        glColor4f(1, 1, 1, 1);
        glVertex2f(this->GetPositionX(), this->GetPositionY());
        glEnd();
    }

    void DrawRay(Map& World)
    {
        bool Continue = true;

        // Calculating lenght of all rays.
        for (int i = 0; i < NoOfRays; i++)
        {
            Continue = true;
            
            Magnify = 1.5;

            while (Continue)
            {
                // if ray collides with wall or boundary
                if (!World.CheckCollision(Rays[i].x, Rays[i].y, this->GetMoveFactor())
                    || Rays[i].x > Screen_Width - BorderLimit || Rays[i].x < BorderLimit
                    || Rays[i].y < BorderLimit || Rays[i].y > Screen_Height - BorderLimit)
                {
                    // break loop
                    Continue = false;

                    // Decrease ray length
                    if (Magnify > 0)
                    {
                        Magnify -= Precision;
                    }

                }

                // else increase ray length
                else
                {

                    Magnify += Precision;
                }

                // Update Ray Position
                Rays[i].x = Position.x + Directions[i].x * Magnify;
                Rays[i].y = Position.y + Directions[i].y * Magnify;
            }


            // Draw Ray
            glColor4f(0.95, 0.95, 0.4, 1);
            glLineWidth(0.5);
            glBegin(GL_LINES);
            glVertex2f(Position.x, Position.y);
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
            float CheckY = Position.y - Direction.y / Speed,
                CheckX = Position.x - Direction.x / Speed;

            // if within border limit
            if (CheckY > BorderLimit && CheckY < Screen_Height - BorderLimit
                && CheckX > BorderLimit && CheckX < Screen_Width - BorderLimit)
            {
                // if not colliding with a wall then allow move
                if (AllowMove)
                {
                    Position.y -= Direction.y / Speed;
                    Position.x -= Direction.x / Speed;

                }
                // if colliding with a wall then don't allow to pass
                else
                {
                    Position.y += Direction.y;
                    Position.x += Direction.x;

                }

            }

        }

        // if 'W' Pressed
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            float CheckY = Position.y + Direction.y / Speed,
                CheckX = Position.x + Direction.x / Speed;

            // if within border limit
            if (CheckY > BorderLimit && CheckY < Screen_Height - BorderLimit
                && CheckX > BorderLimit && CheckX < Screen_Width - BorderLimit)
            {
                // if not colliding with a wall then allow move
                if (AllowMove)
                {
                    Position.y += Direction.y / Speed;
                    Position.x += Direction.x / Speed;

                }
                // if colliding with a wall then don't allow to pass
                else
                {
                    Position.y -= Direction.y;
                    Position.x -= Direction.x;

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
            Direction.x = cos(Angle) * 5;
            Direction.y = sin(Angle) * 5;

            // For Other Rays
            float TempAngle = Angle;

            for (int i = 0; i < 50; i++)
            {
                TempAngle += AngleFactor;

                if (TempAngle > 2 * PI)
                {
                    TempAngle -= 2 * PI;
                }

                Directions[i].x = cos(TempAngle) * 5;
                Directions[i].y = sin(TempAngle) * 5;
            }

            // Reset Magnify
            Magnify = 1.5;

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
            Direction.x = cos(Angle) * 5;
            Direction.y = sin(Angle) * 5;

            // For other Rays
            float TempAngle = Angle;

            for (int i = 0; i < 50; i++)
            {
                TempAngle -= AngleFactor;

                if (TempAngle < 0)
                {
                    TempAngle += 2 * PI;
                }

                Directions[i].x = cos(TempAngle) * 5;
                Directions[i].y = sin(TempAngle) * 5;
            }

            // Reset Magnify
            Magnify = 1.5;

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

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(Screen_Width, Screen_Height, "Ray-Casting", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
   
    // Start window at center
    glfwSetWindowPos(window,
        abs(glfwGetVideoMode(glfwGetPrimaryMonitor())->width - Screen_Width) / 2,
        abs(glfwGetVideoMode(glfwGetPrimaryMonitor())->height - Screen_Height) / 2);

    // Player and Map Object
    Entity Player; Map World;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // To draw
        glLoadIdentity();
        glOrtho(0, Screen_Width, Screen_Height, 0, 100, -100);

        // Calculate & Show FPS
        {
            double currentTime = glfwGetTime();
            frameCount++;

            // If a second has passed.
            if (currentTime - previousTime >= 1.0)
            {
                std::string Title = "Ray-Casting | FPS : ";
                Title += std::to_string(frameCount);

                glfwSetWindowTitle(window, Title.c_str());

                frameCount = 0;
                previousTime = currentTime;
            }
        }

        // Draw Source
        Player.Draw();

        // Draw World
        World.DrawGrid();

        // returns true if no collision
        bool AllowMove = World.CheckCollision(Player.GetPositionX(), Player.GetPositionY(), Player.GetMoveFactor());

        // Source Movement
        Player.Input(window, AllowMove);

        // Draw Rays
        Player.DrawRay(World);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}