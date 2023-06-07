#include "util.h"
#include "Game.h"
#include "TCPServer.h"
#include "UDPServer.h"



/// STUDENT ID --- B1061770
/// CODENAME --- MYSTIC SITOIANU
/// PROJECT --- Network and Multiplayer Games ICA

/*
--- NMG.cpp :: main, Client and the game logic

--- TCP & UDP servers are within their repective files and kept there for easier access.

--- Use ARROW keys to move around.
*/



string pcIPadd = "192.168.0.159"; /// LOCAL ADDRESS -- TO BE CHANGED BASED ON THE PC



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
            IpAddress UDPip = pcIPadd; // Local IP address
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
                clog << "Receive loop has been dropped.";
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
    if (!font.loadFromFile("extras/FutureLight.ttf"))
    {
        cerr << "ERROR: Font not found \n";
        return false;
    }
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
                    thread uServerTh(&UDPServer);
                    uServerTh.detach();
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
            text.setPosition(pos);
            text.setFillColor(Color::White);
            window.draw(text);


            /// Menu options basically
            {
                int count{ 0 };
                for (auto p : choices)
                {
                    pos.y += 90;

                    text.setString(p);
                    text.setPosition(pos);
                    text.setStyle(Text::Regular);

                    if (count == select) { text.setFillColor(Color::Green); }
                    else { text.setFillColor(Color::White); }

                    window.draw(text);
                    count++;
                }

                text.setString("Press 'space' to select");
                text.setPosition(10, 10);
                text.setFillColor(Color::White);
                text.setCharacterSize(10);
                window.draw(text);
            }
                        
        }
        window.display();
    }
    return 0;
}



void Run(RenderWindow& window)
{
    Game game;

    /// ----- TCP SERVER CONNECTION -----
    shared_ptr<TcpSocket> tSockets = make_shared<TcpSocket>();
    if (tSockets->connect(IpAddress::getLocalAddress(), TCPPORT) != Socket::Done)
    {
        stringstream stream;
        stream << "ERROR: Client failed to connect to TCP Server, PORT: " << TCPPORT << endl;
        cerr << stream.str();
        return;
    }



    /// ----- UDP SERVER CONNECTION -----
    UdpSocket uSocket;
    unsigned int clientPort;
    if (uSocket.bind(Socket::AnyPort) != Socket::Done)
    {
        clientPort = uSocket.getLocalPort();
        stringstream stream;
        stream << "ERROR: Client failed to connect to UDP Server, PORT: " << clientPort << endl;
        cerr << stream.str();
        return;
    }

    clientPort = uSocket.getLocalPort();
    stringstream stream;
    stream << "Client bounded to UDP Server PORT: " << clientPort << endl;
    cout << stream.str();

    stream.clear();

    unsigned short serverPort = UDPPORT;
    size_t received = 0;
    IpAddress serverIP = pcIPadd; // Local IP address



    /// ----- LAUCH CLIENT RECEIVER -----
    list<Packet> queue;
    Client cReceiver(&uSocket, tSockets, queue);
    thread receiverTh(&Client::Receive, cReceiver);
    receiverTh.detach();



    /// ----- PLAYER SETUP -----
    ClientInfo setupInfo;
    setupInfo.type = Message::Setup;
    Packet setupPack;
    setupPack << setupInfo;

    if (tSockets->send(setupPack.getData(), setupPack.getDataSize()) != Socket::Done)
    {
        stream << "ERROR: Client to TCP Server -- failed to send setup information." << endl;
        cerr << stream.str();
        return; // Find a way to retry
    }

    if (queue.empty())
    {
        clog << "Client queue -- EMPTY." << endl;
        while (queue.empty())
        {
            /// do something ?
        }
    }

    clog << "Packet received." << endl;
    Packet receivePack = queue.front();
    ClientInfo receiveInfo;
    receivePack >> setupInfo;
    if (setupInfo.type == Message::Setup) { queue.pop_front(); }
    else
    {
        cerr << "ERROR: Message packet is not TYPE 'Setup'." << endl;
        return;
    }
    


    /// ----- GAME INITIALISATION -----
    //srand(time(NULL));
    window.setFramerateLimit(45);
    int localPlayer{ setupInfo.ID };

    sf::Texture tBg, tCar;
    tBg.loadFromFile("images/background.png");
    tCar.loadFromFile("images/car.png");
    tBg.setSmooth(true);
    tCar.setSmooth(true);
    
    Sprite bgSprite(tBg), carSprite(tCar);
    bgSprite.scale(2, 2);
    carSprite.setOrigin(22, 22);

    Font font;
    if (!font.loadFromFile("")) { cout << "ERROR: Font not found"; return; }

    Text playerName;
    playerName.setFont(font);
    playerName.setCharacterSize(15);
    playerName.setOutlineThickness(3);

    float colVal = 22; /// Used during collision checks.

    const int players = 5;
    std::vector<Car> car(players, Car{}); 

    /*for (int i = 0; i < car.size(); i++)
    {
        car[i].name = game.names[i];
    }*/


    ClientInfo cInfo;
    cInfo.ip = "localhost";
    cInfo.port = clientPort;
    cInfo.ID = localPlayer;
    cInfo.type = Message::Update;

    
    /// Starting positions
    for (int i = 0; i < car.size(); i++)
    {
        car[i].pos.x = 300 + i * 50;
        car[i].pos.y = 1700 + i * 80;
        //car[i].speed = 7 + ((float)i / 5);
    }
    float offsetX = 0, offsetY = 0; /// Camera positions


    /// GAME RUNNING
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) { window.close(); }
        }

        bool up = false, left = false, down = false, right = false;

        if (Keyboard::isKeyPressed(Keyboard::Up)) { up = true; }
        if (Keyboard::isKeyPressed(Keyboard::Left)) { left = true; }
        if (Keyboard::isKeyPressed(Keyboard::Down)) { down = true; }
        if (Keyboard::isKeyPressed(Keyboard::Right)) { right = true; }


        /// CAR UPDATES
        auto tempPos = car[localPlayer].pos;
        if (window.hasFocus()) { car[localPlayer].Move(); }

        for (int i = 0; i < players; i++)
        {
            //if (i != localPlayer) { car[i].FindTarget(game.checkpoints); }
            car[i].Update();
        }

        /// COLLISIONS
        for (int i = 0; i < players; i++)
        {
            for (int j = 0; j < players; j++)
            {
                if (i == j)
                {
                    break;
                }
                int dx = 0, dy = 0;
                while ((dx * dx) + (dy * dy) < 4 * colVal * colVal)
                {
                    car[i].pos.x += dx / 10.0;
                    car[i].pos.x += dy / 10.0;
                    car[j].pos.x -= dx / 10.0;
                    car[j].pos.y -= dy / 10.0;
                    dx = car[i].pos.x - car[j].pos.x;
                    dy = car[i].pos.y - car[j].pos.y;
                    if (!dx && !dy) break;
                }
            }
        }


        /// --- SEND TO SERVER ---
        if (cInfo.pos != tempPos)
        {
            cInfo.pos = car[localPlayer].pos;
            cInfo.angle = car[localPlayer].angle;

            Packet pack;
            pack << cInfo;
            uSocket.send(pack.getData(), pack.getDataSize(), serverIP, serverPort);
        }


        /// --- SERVER UPDATES ---
        while (!queue.empty())
        {
            ClientInfo recInfo;
            Packet recPack = queue.front();
            recPack << recInfo;
            queue.pop_front();

            if (recInfo.ID != localPlayer && recInfo.ID <= players)
            {
                car[recInfo.ID].pos = recInfo.pos;
                car[recInfo.ID].angle = recInfo.angle;
                //car[recInfo.ID].speed = recInfo.speed;
            }
        }


        /// --- RENDERING --- 
        window.clear(Color::White);

        /// Cam stays within bounds
        if (car[localPlayer].pos.x > 400) { offsetX = car[localPlayer].pos.x - 400; }
        if (car[localPlayer].pos.y > 400) { offsetY = car[localPlayer].pos.y - 400; }
        if (car[localPlayer].pos.x > 2480) { offsetX -= car[localPlayer].pos.x - 2480; }
        if (car[localPlayer].pos.y > 3248) { offsetY -= car[localPlayer].pos.y - 3248; }

        /// Player stays within bounds
        if (car[localPlayer].pos.x < 20) { car[localPlayer].pos.x = 20; }
        if (car[localPlayer].pos.y < 20) { car[localPlayer].pos.y = 20; }
        if (car[localPlayer].pos.x > 2860) { car[localPlayer].pos.x = 2860; }
        if (car[localPlayer].pos.y > 3628) { car[localPlayer].pos.y = 3628; }


        bgSprite.setPosition(-offsetX, -offsetY);
        window.draw(bgSprite);

        for (int i = 0; i < players; i++)
        {
            carSprite.setPosition(car[i].pos.x - offsetX, car[i].pos.y - offsetY);
            carSprite.setRotation(car[i].angle * 180 / 3.142);
            carSprite.setColor(game.carColours[i]);
            window.draw(carSprite);

            // player name stuff if they had unique usernames
        }

        window.display();
    }
}