#include <vector>
#include <thread>
#include "sqliteManager.h"
#include "functions.h"


bool online = true;
int port;
sqliteManager sqlm;

void protocoloCreate(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            printTrack(true, "ip", "port", "C " + nameNode + " N");
            bool ok=sqlm.SQLiteCreate(nameNode);
            //Devolver confirmación
            //bool ok = true;
            if (ok) {
                auto protocol = message("C", "El nodo " + nameNode + " se creo correctamente.");
                printTrack(false, "ip", "port", protocol);
                sendProtocol(SD, protocol);
            } else {
                auto protocol = message("E", "No se pudo crear el nodo " + nameNode + ".");
                printTrack(false, "ip", "port", protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);
            printTrack(true, "ip", "port", "C " + nameNode + " R " + relation + " " + node2);
            bool ok=sqlm.SQLiteCreate(nameNode, relation, node2);
            //Devolver confirmación

            //bool ok = true;
            if (ok) {
                auto protocol = message("M", "La relacion " + nameNode + " <- " + relation + " -> " + node2 +
                                             " se creo correctamente.");
                printTrack(false, "ip", "port", protocol);
                sendProtocol(SD, protocol);
            } else {
                auto protocol = message("E", "No se pudo crear la relacion " + nameNode + " <- " + relation + " -> " +
                                             node2 + ".");
                printTrack(false, "ip", "port", protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }

        default: {

            break;
        }
    }
}

void protocoloDelete(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            printTrack(true, "ip", to_string(port), "C " + nameNode + " N");
            bool ok=sqlm.SQLiteDeleteNode(nameNode);
            //Devolver confirmación
            if (ok) {
                auto protocol = message("C", "El nodo " + nameNode + " se borro correctamente.");
                printTrack(false, "ip", to_string(port), protocol);
                sendProtocol(SD, protocol);
            } else {
                auto protocol = message("E", "No se pudo crear el nodo " + nameNode + ".");
                printTrack(false, "ip", to_string(port), protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);
            printTrack(true, "ip", to_string(port), "C " + nameNode + " R " + relation + " " + node2);
            bool ok=sqlm.SQLiteDeleteRel(nameNode, relation, node2);
            //Devolver confirmación

            //bool ok = true;
            if (ok) {
                auto protocol = message("M", "La relacion " + nameNode + " <- " + relation + " -> " + node2 +
                                             " se borro correctamente.");
                printTrack(false, "ip", to_string(port), protocol);
                sendProtocol(SD, protocol);
            } else {
                auto protocol = message("E", "No se pudo crear la relacion " + nameNode + " <- " + relation + " -> " +
                                             node2 + ".");
                printTrack(false, "ip", to_string(port), protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }

        default: {

            break;
        }
    }
}


void readServer(int SD) {
    auto label = readLabel(SD);
    auto nameNode = readString(SD);
    switch (label) {
        case 'C': {
            protocoloCreate(SD, nameNode);
            break;
        }
        case 'D': {
            protocoloDelete(SD, nameNode);
            break;
        }
        default: {
            break;
        }

    }

}

int main(int argc, char *argv[]) {




    vector<string> arguments;
    for (int i = 0; i < argc; i++) {
        arguments.emplace_back(argv[i]);
    }
    if (arguments.size() < 2) {
        port = 45155;
    } else {
        port = stoi(arguments[1]);
        string nameDb="Nodo"+string(arguments[1])+".db";
        sqlm.setport(nameDb);
    }

    auto SocketFD = createServer(port);

    while (online) {
        int ConnectFD = accept(SocketFD, nullptr, nullptr);//Como parámetro al thread
        if (0 > ConnectFD) {
            perror("error accept failed");
        } else {
            thread(readServer, ConnectFD).detach();
        }
    }
    close(SocketFD);
    return 0;
}