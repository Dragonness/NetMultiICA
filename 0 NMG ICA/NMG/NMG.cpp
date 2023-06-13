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



std::string pcIPadd = "192.168.0.160"; /// LOCAL ADDRESS -- TO BE CHANGED BASED ON THE PC



class Client
{
private:
    std::shared_ptr<sf::TcpSocket> tSocket;
    sf::UdpSocket* uSocket;
    std::list<sf::Packet>& queue;
public:
    Client(sf::UdpSocket* UDPsock, std::shared_ptr<sf::TcpSocket> TCPsock, std::list<sf::Packet>& cQueue) :
        tSocket(TCPsock), uSocket(UDPsock), queue(cQueue) {}

    void Receive()
    {
        char buffer[256];
        {
            std::stringstream stream;
            stream << "Reciever : " << tSocket->getRemoteAddress() << " : " << tSocket->getRemotePort() << std::endl;
            std::cout << stream.str();
        }


        tSocket->setBlocking(false);
        uSocket->setBlocking(false);


        while (true)
        {
            memset(buffer, 0, 256);
            size_t received;
            sf::Packet packet;
            sf::IpAddress UDPip = pcIPadd; // Local IP address
            unsigned short UDPport = UDPPORT;

            auto tstatus = tSocket->receive(buffer, 256, received);
            if (tstatus == sf::Socket::Done)
            {
                packet.append(buffer, received);

                std::stringstream stream;
                stream << "TCP - Received: \"" << buffer << "\", " << received << " bytes." << std::endl;
                std::cout << stream.str();

                queue.push_back(packet);
            }
            else if (tstatus == sf::Socket::Disconnected)
            {
                std::clog << "Receive loop has been dropped.";
                break;
            }

            auto uStatus = uSocket->receive(buffer, sizeof(buffer), received, UDPip, UDPport);
            if (uStatus == sf::Socket::Done)
            {
                packet.append(buffer, received);
                queue.push_back(packet);
            }
        }
    }
};


void Run(sf::RenderWindow& window);


/// -------- RUNNING EVERYTHING TOGETHER --------
int main()
{
    sf::RenderWindow window(sf::VideoMode(730, 730), "Networking ICA");
    sf::Font font;
    if (!font.loadFromFile("extras/FutureLight.ttf"))
    {
        std::cerr << "ERROR: sf::Font not found \n";
        return false;
    }
    sf::Text text;
    text.setFont(font);

    std::vector<std::string> choices;
    choices.push_back("Host Game");
    choices.push_back("Join Game");
    choices.push_back("Quit");
    int select{ 0 };
    bool keyPress{ false };



    while (window.isOpen())
    {
        //UDPServer udp;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
        }
        
        window.clear();

        if (window.hasFocus())
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !keyPress)
            {
                keyPress = true;
                if (select > 0) { select--; }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !keyPress)
            {
                keyPress = true;
                if (select < choices.size() - 1) { select++; }
            }

            if (keyPress && !sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { keyPress = false; }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
            {
                if (select == 0)
                {
                    keyPress = true;
                    std::thread tServerTh(&TCPServer);
                    tServerTh.detach();

                    //std::thread uServerTh([&udp]() { udp.Server(); });    // Tired maybe putting it all in a class to help fix the exception... didn't work :/
                    std::thread uServerTh(&UDPServer);
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

            sf::Vector2f pos{ 20,100 };
            text.setCharacterSize(20);
            text.setString("Racing Games");
            text.setStyle(sf::Text::Bold);
            text.setPosition(pos);
            text.setFillColor(sf::Color::White);
            window.draw(text);


            /// Menu options basically
            {
                int count{ 0 };
                for (auto p : choices)
                {
                    pos.y += 90;

                    text.setString(p);
                    text.setPosition(pos);
                    text.setStyle(sf::Text::Regular);

                    if (count == select) { text.setFillColor(sf::Color::Green); }
                    else { text.setFillColor(sf::Color::White); }

                    window.draw(text);
                    count++;
                }

                text.setString("Press 'space' to select");
                text.setPosition(10, 10);
                text.setFillColor(sf::Color::White);
                text.setCharacterSize(10);
                window.draw(text);
            }
                        
        }
        window.display();
    }
    return 0;
}



void Run(sf::RenderWindow& window)
{
    Game game;

    /// ----- TCP SERVER CONNECTION -----
    std::shared_ptr<sf::TcpSocket> tSockets = std::make_shared<sf::TcpSocket>();

    if (tSockets->connect(sf::IpAddress::getLocalAddress(), TCPPORT) != sf::Socket::Done)
    {
        std::stringstream stream;
        stream << "ERROR: Client failed to connect to TCP Server, PORT: " << TCPPORT << std::endl;
        std::cerr << stream.str();
        return;
    }


    /// ----- UDP SERVER CONNECTION -----
    sf::UdpSocket uSocket;
    unsigned int clientPort;
    if (uSocket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
    {
        clientPort = uSocket.getLocalPort();
        std::stringstream stream;
        stream << "ERROR: Client failed to connect to UDP Server, PORT: " << clientPort << std::endl;
        std::cerr << stream.str();
        return;
    }

    clientPort = uSocket.getLocalPort();
    std::stringstream stream;
    stream << "Client bounded to UDP Server PORT: " << clientPort << std::endl;
    std::cout << stream.str();

    stream.clear();

    unsigned short serverPort = UDPPORT;
    size_t received = 0;
    sf::IpAddress serverIP = "192.168.0.160"/*pcIPadd*/; // Local IP address



    /// ----- LAUCH CLIENT RECEIVER -----
    std::list<sf::Packet> queue;
    Client cReceiver(&uSocket, tSockets, queue);
    std::thread receiverTh(&Client::Receive, cReceiver);
    receiverTh.detach();



    /// ----- PLAYER SETUP -----
    ClientInfo setupInfo;
    setupInfo.type = Message::Setup;
    sf::Packet setupPack;
    setupPack << setupInfo;

    if (tSockets->send(setupPack.getData(), setupPack.getDataSize()) != sf::Socket::Done)
    {
        stream << "ERROR: Client to TCP Server -- failed to send setup information." << std::endl;
        std::cerr << stream.str();
        return; // Find a way to retry
    }

    if (queue.empty())
    {
        std::clog << "Client queue -- EMPTY." << std::endl;
        while (queue.empty())
        {
            /// do something ?
        }
    }

    std::clog << "sf::Packet received." << std::endl;
    sf::Packet receivePack = queue.front();
    ClientInfo receiveInfo;
    receivePack >> setupInfo;
    if (setupInfo.type == Message::Setup) { queue.pop_front(); }
    else
    {
        std::cerr << "ERROR: Message packet is not TYPE 'Setup'." << std::endl;
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
    
    sf::Sprite bgSprite(tBg), carSprite(tCar);
    bgSprite.scale(2, 2);
    carSprite.setOrigin(22, 22);

    sf::Font font;
    if (!font.loadFromFile("")) { std::cout << "ERROR: sf::Font not found"; return; }

    sf::Text playerName;
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
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
        }

        bool up = false, left = false, down = false, right = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { up = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { left = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { down = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { right = true; }


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

            sf::Packet pack;
            pack << cInfo;
            uSocket.send(pack.getData(), pack.getDataSize(), serverIP, serverPort);
        }


        /// --- SERVER UPDATES ---
        while (!queue.empty())
        {
            ClientInfo recInfo;
            sf::Packet recPack = queue.front();
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
        window.clear(sf::Color::White);

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