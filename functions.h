#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

int sizeNumber = 4;

// Funciones conexión Socket
int createServer(int portServer);

int connectionToServer(const pair<string, int> &server);


// Funciones Lectura de Socket
char readLabel(int SD);

int readInt(int SD, int size = sizeNumber);

string readString(int SD);

string readMessage(int SD);


// Funciones para convertir en Protocolo Auxiliares
string intToProtocol(int x, int digits = sizeNumber);

string stringToProtocol(const string &x);

string attributeToProtocol(const string &attribute, const string &val);

string relationToProtocol(const string &relation, const string &node2);


// Funciones para convertir en Protocolo
string ProtocolMessage(const string &type, const string &msg);

string ProtocolCreateNode(const string &nameNode);

string ProtocolCreateRelation(const string &node1, const string &relation, const string &node2);


// Funciones auxiliares
char *stringToChar(const std::string &str);

void sendProtocol(int SD, const string &protocol);

int simpleHash(const string &name, int x);

void printTrack(bool in, const string &ip, int port, const string &protocol);


// Funciones del servidor
string responseCreate(int SN, const string &nameNode){
    auto labelResponse = readLabel(SN);//Respuesta del nodo
    string response;//Respuesta al cliente
    switch (labelResponse) {
        case 'c': { // Respuesta de confirmación
            response = ProtocolMessage("c", "El nodo " + nameNode + " fue creado.");
            break;
        }
        case 'e': {// Respuesta de error
            auto labelError = readLabel(SN);//Tipo de error
            switch (labelError) {
                case 'x': {// Error de existencia
                    response = ProtocolMessage("e", "El nodo " + nameNode + " ya existe.");
                    break;
                }
                case 'e': {// Error de SQL
                    response = ProtocolMessage("e", "No se pudo crear el nodo " + nameNode + ".");
                    break;
                }
                default: {
                    cout << "Etiqueta de error create equivocada." << endl;
                    response = ProtocolMessage("e", "No se pudo crear el nodo " + nameNode + ".");
                    break;
                }
            }
            break;
        }
        default:{
            cout << "Etiqueta de respuesta create equivocada." << endl;
            response = ProtocolMessage("e", "No se pudo crear el nodo " + nameNode + ".");
            break;
        }
    }
    return response;
}

/*##################################################################################################################*/
//IMPLEMENTACIONES

int createServer(int portServer) {
    struct sockaddr_in stSockAddr{};
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == SocketFD) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(portServer);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(SocketFD, (const struct sockaddr *) &stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(SocketFD, 10)) {
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    return SocketFD;
}

int connectionToServer(const pair<string, int> &server) {
    char *ip = new char[server.first.size()];
    strcpy(ip, server.first.c_str());
    struct sockaddr_in stSockAddr{};
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);


    if (-1 == SocketFD) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;//Tipo de ip
    stSockAddr.sin_port = htons(server.second);
    Res = inet_pton(AF_INET, ip, &stSockAddr.sin_addr);//Traduce la dirección ip de string a int

    if (0 > Res) {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    } else if (0 == Res) {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }


    if (-1 == connect(SocketFD, (const struct sockaddr *) &stSockAddr, sizeof(struct sockaddr_in))) {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    return SocketFD;
}

char readLabel(int SD) {
    char buffer[0];
    int n = (int) read(SD, buffer, 1);
    return buffer[0];
}

int readInt(int SD, int size) {
    char buffer[size];
    int n = (int) read(SD, buffer, size);
    return stoi(string(buffer, size));
}

string readString(int SD) {
    auto size = readInt(SD);
    char buffer[size];
    int n = (int) read(SD, buffer, size);
    auto str = string(buffer, size);
    return str;
}

string intToProtocol(int x, int digits) {
    string casting = to_string(x);
    while (casting.size() < digits)casting.insert(0, "0");
    return casting;
}

string stringToProtocol(const string &x) {
    auto size = intToProtocol((int) x.size(), 4);
    return size + x;
}

char *stringToChar(const string &str) {
    char *charArray = new char[str.size()];
    std::strcpy(charArray, str.c_str());
    return charArray;
}

void sendProtocol(int SD, const string &protocol) {
    write(SD, &protocol[0], protocol.size());
}

int simpleHash(const string &name, int x) {
    int total = 0;
    for (auto c: name) {
        total += c;
    }
    return total % x;
}

string ProtocolCreateNode(const string &nameNode) {
    return "C" + stringToProtocol(nameNode) + "N";
}

string ProtocolCreateRelation(const string &node1, const string &relation, const string &node2) {
    return "C" + stringToProtocol(node1) + "R" + stringToProtocol(relation) +
           stringToProtocol(node2);
}

string ProtocolMessage(const string &type, const string &msg) {
    return type + stringToProtocol(msg);
}

void printTrack(bool in, const string &ip, int port, const string &protocol) {
    in ? cout << "<<" : cout << ">>";
    cout << ip << " , " << port << ": " << protocol << endl;
}

string attributeToProtocol(const string &attribute, const string &val) {
    string protocol = stringToProtocol(attribute) + stringToProtocol(val);
    return protocol;
}

string relationToProtocol(const string &relation, const string &node2) {
    string protocol = stringToProtocol(relation) + stringToProtocol(node2);
    return protocol;
}

string readMessage(int SD) {
    auto label = readLabel(SD);
    auto message = readString(SD);
    return string(1,label) + " " + message;
}

string deleteNode(const string &nameNode) {
    return "D" + stringToProtocol(nameNode) + "N";
}

string deleteRelation(const string &node1, const string &relation, const string &node2) {
    return "D" + stringToProtocol(node1) + "R" + stringToProtocol(relation) +
           stringToProtocol(node2);
}


