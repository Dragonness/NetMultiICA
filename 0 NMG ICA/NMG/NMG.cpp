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


class Client
{
private:
    shared_ptr<TcpSocket> tSocket;
    UdpSocket* uSocket;
    list<Packet>& queue;
public:
    Client(UdpSocket* UDPsock, shared_ptr<TcpSocket> TCPsock, list<Packet>& cQueue) :
        tSocket(TCPsock), uSocket(UDPsock), queue(cQueue) {}

    void Receive()
    {
        char buffer[256];
        {
            stringstream stream;
            stream << "Reciever : " << tSocket->getRemoteAddress() << " : " << tSocket->getRemotePort() << endl;
            cout << stream.str();
        }

        tSocket->setBlocking(false);
        uSocket->setBlocking(false);



        while (true)
        {
            memset(buffer, 0, 256);
            size_t received;
            Packet packet;
            IpAddress UDPip = "192.168.0.159"; /// Local ip address of my laptop so needs to get changed based on the PC
            unsigned short UDPport = UDPPORT;

            auto tstatus = tSocket->receive(buffer, 256, received);
            if (tstatus == Socket::Done)
            {
                packet.append(buffer, received);

                stringstream stream;
                stream << "TCP - Received: \"" << buffer << "\", " << received << " bytes." << endl;
                cout << stream.str();

                queue.push_back(packet);
            }
            else if (tstatus == Socket::Disconnected)
            {
                cout << "Receive loop has been dropped.";
                break;
            }

            auto uStatus = uSocket->receive(buffer, sizeof(buffer), received, UDPip, UDPport);
            if (uStatus == Socket::Done)
            {
                packet.append(buffer, received);
                queue.push_back(packet);
            }

            /*if (uStatus == Socket::NotReady || tstatus == Socket::NotReady)
            {
                ///do something to ease up on the CPU
            }*/
        }
    }
};


void Run(RenderWindow& window);


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
    Text text;
    text.setFont(font);

    vector<string> choices;
    choices.push_back("Host Game");
    choices.push_back("Join Game");
    choices.push_back("Quit");
    int select{ 0 };
    bool keyPress{ false };



    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) { window.close(); }
        }
        
        window.clear();

        if (window.hasFocus())
        {
            if (Keyboard::isKeyPressed(Keyboard::Up) && !keyPress)
            {
                keyPress = true;
                if (select > 0) { select--; }
            }
            if (Keyboard::isKeyPressed(Keyboard::Down) && !keyPress)
            {
                keyPress = true;
                if (select < choices.size() - 1) { select++; }
            }

            if (keyPress && !Keyboard::isKeyPressed(Keyboard::Up) && !Keyboard::isKeyPressed(Keyboard::Down)) { keyPress = false; }

            if (Keyboard::isKeyPressed(Keyboard::Space) || Keyboard::isKeyPressed(Keyboard::Return))
            {
                if (select == 0)
                {
                    keyPress = true;
                    thread tServerTh(&TCPServer);
                    tServerTh.detach();
                    //thread uServerTh(&UDPServer);
                    //uServerTh.detach();
                    Run(window);
                }
                if (select == 1)
                {
                    keyPress = true;
                    Run(window);
                }
                if (select == 2) { return false; }
            }

            Vector2f pos{ 20,100 };
            text.setCharacterSize(20);
            text.setString("Racing Games");
            text.setStyle(Text::Bold);


        }
    }
}




//{
//    // Initialise
//    srand(time(NULL));
//
//    RenderWindow app(VideoMode(800, 800), "Trials");
//    app.setFramerateLimit(60);
//
//
//    /*Texture t1, t2;
//    t1.loadFromFile("images/background.png");
//    t2.loadFromFile("images/car.png");
//    t1.setSmooth(true);
//    t2.setSmooth(true);
//
//    Sprite sBackground(t1), sCar(t2);
//    sBackground.scale(2, 2);
//    sCar.setOrigin(22, 22);
//
//    float R = 22;
//    const int N = 5;
//    Car car[N];
//
//    Color colors[5] = { Color::Red, Color::Green, Color::Magenta, Color::Blue, Color::White };*/
//}




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