#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

int sizeNumber = 4;

struct node {
    string name;
    vector<string> road;
    int lvl;

    node(string name, int lvl, vector<string> road = {}) {
        this->name = std::move(name);
        this->lvl = lvl;
        this->road = std::move(road);
        this->road.push_back(this->name);
    }
};

// Funciones conexión Socket
int createServer(int portServer);

int connectionToServer(const pair<string, int> &server);


// Funciones Lectura de Socket
char readLabel(int SD);

int readInt(int SD, int size = sizeNumber);

string readString(int SD);

string readMessage(int SD);

vector<pair<string, string>> readMulti(int SD) {
    vector<pair<string, string>> ret;
    int tam = readInt(SD);
    for (int i = 0; i < tam; i++) {
        auto relation = readString(SD);
        auto node2 = readString(SD);
        ret.emplace_back(relation, node2);
    }
    return ret;
}

vector<int> readInfo(int SD) {
    vector<int> info;
    info.push_back(readInt(SD));
    info.push_back(readInt(SD));
    info.push_back(readInt(SD));
    return info;
}

vector<string> readRelations(int SD) {
    auto num = readInt(SD);
    vector<string> relations;
    for (int i = 0; i < num; i++) {
        relations.push_back(readString(SD));
    }
    return relations;
}

// Funciones para convertir en Protocolo Auxiliares
string intToProtocol(int x, int digits = sizeNumber);

string stringToProtocol(const string &x);

string attributeToProtocol(const string &attribute, const string &val);

string relationToProtocol(const string &relation, const string &node2);


// Funciones para convertir en Protocolo
string ProtocolMessage(const string &type, const string &msg);

string ProtocolCreateNode(const string &nameNode);

string ProtocolCreateRelation(const string &node1, const string &relation, const string &node2);

string ProtocolSendItems(const vector<pair<string, string>> &res);

string ProtocolDeleteNode(const string &nameNode);

string ProtocolDeleteRelation(const string &node1, const string &relation, const string &node2);

string ProtocolDeleteAttribute(const string &nameNode, const string &attribute);

string ProtocolUpdateNode(const string &nameNode, const string &newNameNode);

string ProtocolUpdateRelation(const string &nameNode, vector<string> items);

string ProtocolUpdateAttributes(const string &nameNode, const vector<pair<string, string>> &items);

string ProtocolReadRelations(const string &nameNode, int level);

string ProtocolReadAttributes(const string &nameNode);

string ProtocolReadInfo(const string &nameNode);

string ProtocolInfoServer();


// Funciones auxiliares
char *stringToChar(const std::string &str);

void sendProtocol(int SD, const string &protocol);

int simpleHash(const string &name, int x);

void printTrack(bool in, const string &ip, int port, const string &protocol);

string attributesToMessage(const string &nameNode, const vector<pair<string, string>> &res);

string relationsToMessage(const string &nameNode, int level, const vector<node> &out);


// Funciones del servidor
string responseCreate(int SN, const string &nameNode) {
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
                case 'f': {// Error de SQL
                    response = ProtocolMessage("f", "No se pudo crear el nodo " + nameNode + ".");
                    break;
                }
                default: {
                    cout << "Etiqueta de error create equivocada." << endl;
                    response = ProtocolMessage("f", "No se pudo crear el nodo " + nameNode + ".");
                    break;
                }
            }
            break;
        }
        default: {
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
    return string(1, label) + " " + message;
}

string ProtocolDeleteNode(const string &nameNode) {
    return "D" + stringToProtocol(nameNode) + "N";
}

string ProtocolDeleteRelation(const string &node1, const string &relation, const string &node2) {
    return "D" + stringToProtocol(node1) + "R" + stringToProtocol(relation) + stringToProtocol(node2);
}

string ProtocolDeleteAttribute(const string &nameNode, const string &attribute) {
    return "D" + stringToProtocol(nameNode) + "A" + stringToProtocol(attribute);
}

string ProtocolSendItems(const vector<pair<string, string>> &res) {
    string protocol = "r";
    protocol += intToProtocol((int) res.size());
    for (const auto &r: res) {
        protocol += relationToProtocol(r.first, r.second);
    }
    return protocol;
}

string ProtocolSendInfo(int port, int numRelations, int numAttributes) {
    string protocol = "r";
    protocol += intToProtocol(port);
    protocol += intToProtocol(numRelations);
    protocol += intToProtocol(numAttributes);
    return protocol;
}

string ProtocolSendRelations(const vector<string> &relations) {
    string protocol = "r";
    protocol += intToProtocol((int) relations.size());
    for (const auto &r: relations) {
        protocol += stringToProtocol(r);
    }
    return protocol;
}

string ProtocolUpdateNode(const string &nameNode, const string &newNameNode) {
    return "U" + stringToProtocol(nameNode) + "N" + stringToProtocol(newNameNode);
}

string ProtocolUpdateRelation(const string &nameNode, vector<string> items) {
    string protocol = "U";
    protocol += stringToProtocol(nameNode);//Node 1
    protocol += "R";
    protocol += stringToProtocol(items[0]);//Relación
    protocol += stringToProtocol(items[1]);//Node 2
    protocol += stringToProtocol(items[2]);//New node 1
    protocol += stringToProtocol(items[3]);//New Relación
    protocol += stringToProtocol(items[4]);//New node 2
    return protocol;
}

string ProtocolUpdateAttributes(const string &nameNode, const vector<pair<string, string>> &items) {
    string protocol = "U";
    protocol += stringToProtocol(nameNode);
    protocol += "U";
    protocol += intToProtocol((int) items.size());
    for (auto &item: items) {
        protocol += attributeToProtocol(item.first, item.second);
    }
    return protocol;
}

string ProtocolReadRelations(const string &nameNode, int level) {
    return "R" + stringToProtocol(nameNode) + "R" + intToProtocol(level);
}

string ProtocolReadRelationsNode(const string &nameNode) {
    return "R" + stringToProtocol(nameNode) + "R";
}

string ProtocolReadAttributes(const string &nameNode) {
    return "R" + stringToProtocol(nameNode) + "A";
}

string ProtocolReadInfo(const string &nameNode) {
    return "R" + stringToProtocol(nameNode) + "I";
}

string attributesToMessage(const string &nameNode, const vector<pair<string, string>> &res) {
    string msg = "Atributos de " + nameNode + ":\n";
    msg += "\tCLAVE\tVALOR\n\n";
    for (const auto &r: res) {
        msg += "\t" + r.first + "\t" + r.second + "\n";
    }
    return msg;
}

string relationsToMessage(const string &nameNode, int level, const vector<node> &out) {
    string msg = "Relaciones de " + nameNode + " a un nivel de " + to_string(level) + ":\n";
    for (const auto& n: out) {
        string road="\t";
        for(const auto& i:n.road){
            road+=i+" - ";
        }
        road.pop_back();
        road.pop_back();
        road.pop_back();
        road+="\n";
        msg+=road;
    }
    return msg;
}

string ProtocolInfoServer() {
    return "I"+ intToProtocol(0);
}

string infoToMessage(const string &nameNode, vector<int> info) {
    string msg = "Informacion del nodo: " + nameNode + ":\n";
    msg += "\tNodo ubicacion: " + to_string(info[0]) + "\n";
    msg += "\tNumero de relaciones: " + to_string(info[1]) + "\n";
    msg += "\tNumero de atributos: " + to_string(info[2]) + "\n";
    return msg;
}


