#include <vector>
#include <thread>
#include "functions.h"

vector<pair<string, int>> secondNodes;
bool online = true;
int port = 45151;

void protocoloCreate(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            printTrack(true, "ip", to_string(port), "C " + nameNode + " N");
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);

            // Envío el protocolo correspondiente
            auto protocol = createNode(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, to_string(secondNodes[dest].second), protocol);
            //Recibir confirmación
            auto labelResponse = readLabel(SN);
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN);
                auto msg = readString(SN);
                printTrack(true, secondNodes[dest].first, to_string(secondNodes[dest].second), labelType + msg);
                //Devolver confirmación
                auto protocolResponse = message(string(1, labelType), msg);
                sendProtocol(SD, protocolResponse);
                printTrack(false, "ip", "port", protocolResponse);
            }
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);

            printTrack(true, "ip", to_string(port), "C " + nameNode + " N " + relation + " " + node2);

            //Me conecto al nodo1
            int dest1 = simpleHash(nameNode, (int) secondNodes.size());
            auto SN1 = connectionToServer(secondNodes[dest1]);
            // Envío el protocolo correspondiente
            auto protocol1 = createRelation(nameNode, relation, node2);
            sendProtocol(SN1, protocol1);
            printTrack(false, secondNodes[dest1].first, to_string(secondNodes[dest1].second), protocol1);
            //Recibir confirmación
            auto labelResponse = readLabel(SN1);
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN1);
                auto msg = readString(SN1);
                printTrack(true, secondNodes[dest1].first, to_string(secondNodes[dest1].second), labelType + msg);
            }


            //Me conecto al nodo2
            int dest2 = simpleHash(node2, (int) secondNodes.size());
            auto SN2 = connectionToServer(secondNodes[dest2]);
            // Envío el protocolo correspondiente
            auto protocol2 = createRelation(node2, relation, nameNode);
            sendProtocol(SN2, protocol2);
            printTrack(false, secondNodes[dest2].first, to_string(secondNodes[dest2].second), protocol2);

            //Recibir confirmación
            labelResponse = readLabel(SN2);
            cout<<"respuesta label"<<labelResponse<<endl;
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN2);
                auto msg = readString(SN2);
                printTrack(true, secondNodes[dest2].first, to_string(secondNodes[dest2].second), labelType + msg);
            }

            //TODO:Aplicar transacciones(si hubo error revertir cambios)

            //Devolver confirmación
            auto protocolResponse = message("C", "La relacion " + nameNode + " <- " + relation + " -> " + node2 +
                                                 " se creo correctamente.");
            sendProtocol(SD, protocolResponse);
            printTrack(false, "ip", "port", protocolResponse);
            //Cierro la conexión 1
            shutdown(SN1, SHUT_RDWR);
            close(SN1);
            //Cierro la conexión 2
            shutdown(SN2, SHUT_RDWR);
            close(SN2);
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
            printTrack(true, "ip", to_string(port), "D " + nameNode + " N");
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);

            // Envío el protocolo correspondiente
            auto protocol = deleteNode(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, to_string(secondNodes[dest].second), protocol);
            //Recibir confirmación
            auto labelResponse = readLabel(SN);
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN);
                auto msg = readString(SN);
                printTrack(true, secondNodes[dest].first, to_string(secondNodes[dest].second), labelType + msg);
                //Devolver confirmación
                auto protocolResponse = message(string(1, labelType), msg);
                sendProtocol(SD, protocolResponse);
                printTrack(false, "ip", "port", protocolResponse);
            }
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);

            printTrack(true, "ip", to_string(port), "D " + nameNode + " N " + relation + " " + node2);

            //Me conecto al nodo1
            int dest1 = simpleHash(nameNode, (int) secondNodes.size());
            auto SN1 = connectionToServer(secondNodes[dest1]);
            // Envío el protocolo correspondiente
            auto protocol1 = deleteRelation(nameNode, relation, node2);
            sendProtocol(SN1, protocol1);
            printTrack(false, secondNodes[dest1].first, to_string(secondNodes[dest1].second), protocol1);
            //Recibir confirmación
            auto labelResponse = readLabel(SN1);
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN1);
                auto msg = readString(SN1);
                printTrack(true, secondNodes[dest1].first, to_string(secondNodes[dest1].second), labelType + msg);
            }


            //Me conecto al nodo2
            int dest2 = simpleHash(node2, (int) secondNodes.size());
            auto SN2 = connectionToServer(secondNodes[dest2]);
            // Envío el protocolo correspondiente
            auto protocol2 = deleteRelation(node2, relation, nameNode);
            sendProtocol(SN2, protocol2);
            printTrack(false, secondNodes[dest2].first, to_string(secondNodes[dest2].second), protocol2);

            //Recibir confirmación
            labelResponse = readLabel(SN2);
            if (labelResponse == 'M') {
                auto labelType = readLabel(SN2);
                auto msg = readString(SN2);
                printTrack(true, secondNodes[dest2].first, to_string(secondNodes[dest2].second), labelType + msg);
            }

            //TODO:Aplicar transacciones(si hubo error revertir cambios)

            //Devolver confirmación
            auto protocolResponse = message("C", "La relacion " + nameNode + " <- " + relation + " -> " + node2 +
                                                 " se eliminó correctamente.");
            sendProtocol(SD, protocolResponse);
            printTrack(false, "ip", "port", protocolResponse);
            //Cierro la conexión 1
            shutdown(SN1, SHUT_RDWR);
            close(SN1);
            //Cierro la conexión 2
            shutdown(SN2, SHUT_RDWR);
            close(SN2);
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

int main() {
    secondNodes.emplace_back("127.0.0.1", 45155);
    secondNodes.emplace_back("127.0.0.1", 45156);
    secondNodes.emplace_back("127.0.0.1", 45157);
    secondNodes.emplace_back("127.0.0.1", 45158);


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