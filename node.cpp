#include <vector>
#include <thread>
#include "sqliteManager.h"
#include "functions.h"


bool online = true;
int port;
sqliteManager sqlm;

void nodeCreate(int SD, const string &nameNode) {
    auto label = readLabel(SD);// Tipo de create
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            printTrack(true, "127.0.0.1", port, "C " + nameNode + " N");
            auto ok = sqlm.SQLiteCreate(nameNode);
            //Devolver confirmación
            if (ok == 'c') {
                string protocol = "c";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (ok == 'x') {
                string protocol = "ex";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);
            printTrack(true, "127.0.0.1", port, "C " + nameNode + " R " + relation + " " + node2);
            auto ok = sqlm.SQLiteCreate(nameNode, relation, node2);
            //Devolver confirmación
            if (ok == 'c') {
                string protocol = "c";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (ok == 'x') {
                string protocol = "ex";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }

        default: {

            break;
        }
    }
}

void nodeDelete(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'N': {//Eliminar nodo
            vector<pair<string, string>> res;
            auto selectConfirm = sqlm.SQLSelectRelations(nameNode, res);
            if (selectConfirm == 'f') {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
                break;
            } else if (selectConfirm == 'c') {
                if (res.empty()) {
                    string protocol = "ex";
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                    break;
                }
                auto ok = sqlm.SQLiteDeleteNode(nameNode);
                if (ok) {
                    auto protocol = ProtocolSendItems(res);
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                    break;
                } else {
                    string protocol = "ef";
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                    break;
                }
            }
            break;
        }
        case 'R': {//eliminar una relación
            auto relation = readString(SD);
            auto node2 = readString(SD);
            printTrack(true, "127.0.0.1", port, "D " + nameNode + " R " + relation + " " + node2);
            auto ok = sqlm.SQLiteDeleteRel(nameNode, relation, node2);
            //Devolver confirmación
            if (ok == 'c') {
                string protocol = "c";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (ok == 'x') {
                string protocol = "ex";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }
        case 'A': {//Eliminar atributo de nodo
            auto attribute = readString(SD);
            printTrack(true, "127.0.0.1", port, "D " + nameNode + " A " + attribute);
            auto ok = sqlm.SQLiteDeleteAttribute(nameNode, attribute);
            //Devolver confirmación
            if (ok == 'c') {
                string protocol = "c";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (ok == 'x') {
                string protocol = "ex";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }
        default: {

            break;
        }
    }
}

void nodeUpdate(int SD, const string &nameNode) {
    auto label = readLabel(SD);// Tipo de create
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            break;
        }
        case 'U': {//Crear una nueva relación
            auto attributes = readMulti(SD);
            printTrack(true, "127.0.0.1", port, "U " + nameNode + " U ");
            auto ok = sqlm.SQLiteUpdateAttributes(nameNode, attributes);
            //Devolver confirmación
            if (ok == 'c') {
                string protocol = "c";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (ok == 'x') {
                string protocol = "ex";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            }
            break;
        }

        default: {

            break;
        }
    }
}

void nodeRead(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'R': {//Eliminar nodo
            vector<pair<string, string>> relationsC;
            auto selectConfirm = sqlm.SQLSelectRelations(nameNode, relationsC);
            vector<string> relations;
            for(const auto& r:relationsC){
                relations.push_back(r.second);
            }
            auto protocol = ProtocolSendRelations(relations);
            printTrack(false, "127.0.0.1", port, protocol);
            sendProtocol(SD, protocol);
            break;
        }

        case 'A': {//Eliminar nodo
            vector<pair<string, string>> attributes;
            auto selectConfirm = sqlm.SQLSelectAtributes(nameNode, attributes);
            if (selectConfirm == 'f') {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (selectConfirm == 'c') {
                if (attributes.empty()) {
                    string protocol = "ex";
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                } else {
                    auto protocol = ProtocolSendItems(attributes);
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                }
            }
            break;
        }

        case 'I': {//Eliminar nodo
            int numRelations;
            int numAttributes;
            auto selectConfirm = sqlm.SQLSelectInfo(nameNode, numRelations, numAttributes);
            if (selectConfirm == 'f') {
                string protocol = "ef";
                printTrack(false, "127.0.0.1", port, protocol);
                sendProtocol(SD, protocol);
            } else if (selectConfirm == 'c') {
                if (numAttributes == 0) {
                    string protocol = "ex";
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                } else {
                    int ubi = port % 10;
                    auto protocol = ProtocolSendInfo(ubi, numRelations, numAttributes);
                    printTrack(false, "127.0.0.1", port, protocol);
                    sendProtocol(SD, protocol);
                }
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
            nodeCreate(SD, nameNode);
            shutdown(SD, SHUT_RDWR);
            close(SD);
            break;
        }
        case 'D': {
            nodeDelete(SD, nameNode);
            break;
        }
        case 'U': {
            nodeUpdate(SD, nameNode);
            break;
        }
        case 'R': {
            nodeRead(SD, nameNode);
            break;
        }
        default: {
            break;
        }

    }
    shutdown(SD, SHUT_RDWR);
    close(SD);
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
        cout << "Se inicio el nodo en el puerto:" << port << endl;
        string nameDb = "Nodo" + string(arguments[1]) + ".db";
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