#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include "sqlite3.h"

using namespace std;

int sizeNumber = 4;

// Funciones conexión Socket
int createServer(int portServer);

int connectionToServer(const pair<string, int> &server);


// Funciones Lectura de Socket
char readLabel(int SD);

int readInt(int SD, int size = sizeNumber);

string readString(int SD);


// Funciones para convertir en Protocolo Auxiliares
string intToProtocol(int x, int digits = sizeNumber);

string stringToProtocol(const string &x);


// Funciones para convertir en Protocolo
string message(const string &type, const string &msg);

string createNode(const string &nameNode);

string createRelation(const string &node1, const string &relation, const string &node2);

string deleteNode(const string &nameNode);

string deleteRelation(const string &node1, const string &relation, const string &node2);

// Funciones auxiliares
char *stringToChar(const std::string &str);

void sendProtocol(int SD, const string &protocol);

int simpleHash(const string &name, int x);

void printTrack(bool in, const string &ip, const string &port, const string &protocol);


// Funciones SQLite

int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void SQLiteCreateNode(const string &nameNode); //TODO: Diego

bool SQLiteCreate(sqlite3* db, const string& nameNode, const string& Rel = "NULL", const string& node2 = "NULL") {
    char* zErrMsg;
    string comando;
    comando = "insert into relaciones (Nodo1, relacion, Nodo2) values ('"+nameNode+"','"+Rel+"','"+node2+"')";
    const char* sql = comando.c_str();
    int rc;
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        //fprintf(stderr, "Error SQL: %s\n", zErrMsg);
        //sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

bool SQLiteDeleteNode(sqlite3* db, string nameNode) {
    char* zErrMsg;
    string comando;
    comando = "delete from relaciones where Nodo1 = '" + nameNode + "';";
    const char* com2 = comando.c_str();
    int rc;
    rc = sqlite3_exec(db, com2, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        return false;
    }
    return true;
}

//void SQLiteCreateRelation(const string &node1, const string &relation, const string &node2);//TODO: Diego
//
//bool SQLiteCreateRelation(sqlite3* db, const string &node1, const string &relation, const string &node2) {
//
//    char* zErrMsg;
//    string comando;
//    comando = "delete from relaciones where relacion = '" + relation + "';";
//    const char* com2 = comando.c_str();
//    int rc;
//    rc = sqlite3_exec(db, com2, callback, 0, &zErrMsg);
//    if (rc != SQLITE_OK) {
//        return false;
//    }
//    return true;
//}



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

string createNode(const string &nameNode) {
    return "C" + stringToProtocol(nameNode) + "N";
}

string createRelation(const string &node1, const string &relation, const string &node2) {
    return "C" + stringToProtocol(node1) + "R" + stringToProtocol(relation) +
           stringToProtocol(node2);
}

string message(const string &type, const string &msg) {
    return "M" + type + stringToProtocol(msg);
}

void printTrack(bool in, const string &ip, const string &port, const string &protocol) {
    in ? cout << "<<" : cout << ">>";
    cout << ip << " , " << port << ": " << protocol << endl;
}

string deleteNode(const string &nameNode) {
    return "D" + stringToProtocol(nameNode) + "N";
}

string deleteRelation(const string &node1, const string &relation, const string &node2) {
    return "D" + stringToProtocol(node1) + "R" + stringToProtocol(relation) +
           stringToProtocol(node2);
}


