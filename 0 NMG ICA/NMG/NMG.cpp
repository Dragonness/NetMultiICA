#include "util.h"
#include "Game.h"
#include "TCPServer.cpp"
#include "UDPServer.cpp"


using namespace std;


/// STUDENT ID --- B1061770
/// CODENAME --- MYSTIC SITOIANU
/// PROJECT --- Network and Multiplayer Games ICA

/*
--- NMG.cpp :: main, Client and the game logic

--- TCP & UDP servers are within their repective files and kept there for easier access.

--- Use ARROW keys to move around.
*/


void Client()
{

}


void Run(RenderWindow& window)
{
    // Initialise
    srand(time(NULL));

    RenderWindow app(VideoMode(800, 800), "Trials");
    app.setFramerateLimit(60);


    /*Texture t1, t2;
    t1.loadFromFile("images/background.png");
    t2.loadFromFile("images/car.png");
    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite sBackground(t1), sCar(t2);
    sBackground.scale(2, 2);
    sCar.setOrigin(22, 22);

    float R = 22;
    const int N = 5;
    Car car[N];

    Color colors[5] = { Color::Red, Color::Green, Color::Magenta, Color::Blue, Color::White };*/
}


/// -------- RUNNING EVERYTHING TOGETHER --------
int main()
{
    RenderWindow window(VideoMode(730, 730), "Networking ICA");
    Font font;
    /*if (!font.loadFromFile("file/location/name.ttf"))
    {
        cerr << "ERROR: Font not found \n";
        return false;
    }*/

    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) { window.close(); }
        }
    }
}





/*int main()
{
    // ****************************************
    

    // Starting positions
    for (int i = 0; i < N; i++)
    {
        car[i].x = 300 + i * 50;
        car[i].y = 1700 + i * 80;
        car[i].speed = 7 + ((float)i / 5);
    }

    int offsetX = 0, offsetY = 0;

    // ****************************************
    // Loop

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }

        // Step 2: update car movement

        car[0].speed = speed;
        car[0].angle = angle;
        for (int i = 0; i < N; i++) car[i].move();
        for (int i = 1; i < N; i++) car[i].findTarget();

        //collision
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                if (i == j)
                {
                    break;
                }
                int dx = 0, dy = 0;
                while (dx * dx + dy * dy < 4 * R * R)
                {
                    car[i].x += dx / 10.0;
                    car[i].x += dy / 10.0;
                    car[j].x -= dx / 10.0;
                    car[j].y -= dy / 10.0;
                    dx = car[i].x - car[j].x;
                    dy = car[i].y - car[j].y;
                    if (!dx && !dy) break;
                }
            }

        // Step 3: Render
        app.clear(Color::White);
        // TODO: Stay within the limit of the map.
        // TODO: Don't show white at bottom/right.
        if (car[0].x > 320) offsetX = car[0].x - 320;
        if (car[0].y > 240) offsetY = car[0].y - 240;
        sBackground.setPosition(-offsetX, -offsetY);
        app.draw(sBackground);
        for (int i = 0; i < N; i++)
        {
            sCar.setPosition(car[i].x - offsetX, car[i].y - offsetY);
            sCar.setRotation(car[i].angle * 180 / 3.141593);
            sCar.setColor(colors[i]);
            app.draw(sCar);
        }
        app.display();
    }

    return 0;
}*/