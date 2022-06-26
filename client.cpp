#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "functions.h"

using namespace std;


int port = 45151;
char ip[] = "127.0.0.1";

void printErr(int SD) {
    cout << "Comando desconocido." << endl;
    auto protocol = "Z" + stringToProtocol("error");
    sendProtocol(SD, protocol);
}

void create(int argc, char *argv[], int SD) {
    if (argc == 3) {
        auto protocol = ProtocolCreateNode(string(argv[2]));
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);

        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo

    } else if (argc == 5) {
        auto node1 = string(argv[2]);
        auto relation = string(argv[3]);
        auto node2 = string(argv[4]);
        auto protocol = ProtocolCreateRelation(node1, relation, node2);
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);

        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);
    } else {
        printErr(SD);
    }
}

void read(int argc, char *argv[], int SD) {
    string protocol;
    if (argc == 5) {//Read relaciones por nivel
        protocol = ProtocolReadRelations(argv[2], stoi(argv[4]));
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);

        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo
    } else if (argc == 4) {
        if (string(argv[3]) == "A") {
            protocol = ProtocolReadAttributes(argv[2]);
            printTrack(false, ip, port, protocol);
            sendProtocol(SD, protocol);

            //Recibir respuesta
            auto message = readMessage(SD);
            cout << message << endl;
            printTrack(true, ip, port, message);//Solo para testeo
        } else if (string(argv[3]) == "I") {
            protocol = ProtocolReadInfo(argv[2]);
            printTrack(false, ip, port, protocol);
            sendProtocol(SD, protocol);

            //Recibir respuesta
            auto message = readMessage(SD);
            cout << message << endl;
            printTrack(true, ip, port, message);//Solo para testeo
        } else {
            printErr(SD);
        }
    } else {
        printErr(SD);
    }
}

void update(int argc, char *argv[], int SD) {
    if (argc == 4) {
        auto protocol = ProtocolUpdateNode(string(argv[2]), string(argv[3]));
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);
        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo
    } else if (argc == 8 and string(argv[3]) != "CA") {
        vector<string> items;
        for (int i = 3; i < argc; i++) {
            items.emplace_back(argv[i]);
        }
        auto protocol = ProtocolUpdateRelation(string(argv[2]), items);
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);
        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo
    } else if (argc > 4 and string(argv[3]) == "CA") {
        vector<pair<string, string>> items;
        for (int i = 4; i < argc; i += 2) {
            items.emplace_back(argv[i], argv[i + 1]);
        }
        auto protocol = ProtocolUpdateAttributes(string(argv[2]), items);
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);
        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo
    } else {
        printErr(SD);
    }
}

void erase(int argc, char *argv[], int SD) {
    if (argc == 3) {//DeleteNode
        auto protocol = ProtocolDeleteNode(string(argv[2]));
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);
        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo

    } else if (argc == 4) {//DeleteAttribute
        auto protocol = ProtocolDeleteAttribute(string(argv[2]), string(argv[3]));
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);
        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo

    } else if (argc == 5) {//DeleteRelation
        auto node1 = string(argv[2]);
        auto relation = string(argv[3]);
        auto node2 = string(argv[4]);
        auto protocol = ProtocolDeleteRelation(node1, relation, node2);
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);

        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo

    } else {
        printErr(SD);
    }
}

void info(int argc, char *argv[], int SD) {
    if (argc == 2) {//Info de los nodos
        auto protocol = ProtocolInfoServer();
        printTrack(false, ip, port, protocol);
        sendProtocol(SD, protocol);

        //Recibir respuesta
        auto message = readMessage(SD);
        cout << message << endl;
        printTrack(true, ip, port, message);//Solo para testeo

    } else {
        printErr(SD);
    }
}

int main(int argc, char *argv[]) {
    auto SocketFD = connectionToServer(make_pair(ip, port));

    if (argc > 2) {
        if (*argv[1] == 'C' || *argv[1] == 'c') {
            create(argc, argv, SocketFD);
        } else if (*argv[1] == 'U' || *argv[1] == 'u') {
            update(argc, argv, SocketFD);
        } else if (*argv[1] == 'D' || *argv[1] == 'd') {
            erase(argc, argv, SocketFD);
        } else if (*argv[1] == 'R' || *argv[1] == 'r') {
            read(argc, argv, SocketFD);
        } else if (*argv[1] == 'I' || *argv[1] == 'i') {
            info(argc, argv, SocketFD);
        } else {
            printErr(SocketFD);
        }
    } else {
        printErr(SocketFD);
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}