#include <utility>
#include <vector>
#include <thread>
#include <queue>
#include "functions.h"



vector<pair<string, int>> secondNodes;
bool online = true;
int port = 45151;

void serverCreate(int SD, const string &nameNode) { //FINO
    auto label = readLabel(SD);//Tipo de creado
    switch (label) {
        case 'N': {//Crear un nuevo nodo
            printTrack(true, "127.0.0.1", port, "C " + nameNode + " N");//Solo para testeo
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);//SN= socket Node
            // Envío el protocolo correspondiente
            auto protocol = ProtocolCreateNode(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir confirmación
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, "Respuesta");
            auto response = responseCreate(SN, nameNode);//Recibo respuesta del nodo y creo respuesta al cliente
            sendProtocol(SD, response);
            printTrack(false, "127.0.0.1", port, response);
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }
        case 'R': {//Crear una nueva relación
            auto relation = readString(SD);
            auto node2 = readString(SD);

            printTrack(true, "127.0.0.1", port, "C " + nameNode + " R " + relation + " " + node2);

            //Me conecto al nodo1
            int dest1 = simpleHash(nameNode, (int) secondNodes.size());
            auto SN1 = connectionToServer(secondNodes[dest1]);// Socket del Nodo 1
            // Envío el protocolo correspondiente
            auto protocol1 = ProtocolCreateRelation(nameNode, relation, node2);
            sendProtocol(SN1, protocol1);
            printTrack(false, secondNodes[dest1].first, secondNodes[dest1].second, protocol1);

            //Recibir confirmación
            char labelResponse1 = readLabel(SN1);//Respuesta del nodo
            printTrack(true, secondNodes[dest1].first, secondNodes[dest1].second, string(1, labelResponse1));
            char labelError1;
            if (labelResponse1 == 'e') {
                labelError1 = readLabel(SN1);//Respuesta del nodo
                shutdown(SN1, SHUT_RDWR);
                close(SN1);
                if (labelError1 == 'f') {
                    string msg = "No se pudo crear la relation " + nameNode + "<-" + relation + "->" + node2 + ".";
                    auto response = ProtocolMessage("e", msg);
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    //Cierro la conexión 1
                    shutdown(SN1, SHUT_RDWR);
                    close(SN1);
                    break;
                }
            }

            //Me conecto al nodo2
            int dest2 = simpleHash(node2, (int) secondNodes.size());
            auto SN2 = connectionToServer(secondNodes[dest2]);// Socket del Nodo 2
            // Envío el protocolo correspondiente
            auto protocol2 = ProtocolCreateRelation(node2, relation, nameNode);
            sendProtocol(SN2, protocol2);
            printTrack(false, secondNodes[dest2].first, secondNodes[dest2].second, protocol2);

            //Recibir confirmación
            auto labelResponse2 = readLabel(SN2);
            printTrack(true, secondNodes[dest2].first, secondNodes[dest2].second, string(1, labelResponse2));
            char labelError2;
            if (labelResponse2 == 'e') {
                labelError2 = readLabel(SN2);//Respuesta del nodo
                shutdown(SN2, SHUT_RDWR);
                close(SN2);
                if (labelError2 == 'f') {
                    //TODO:Hacer delete al anterior
                    string msg = "No se pudo crear la relation " + nameNode + "<-" + relation + "->" + node2 + ".";
                    auto response = ProtocolMessage("e", msg);
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    //Cierro la conexión 1
                    shutdown(SN1, SHUT_RDWR);
                    close(SN1);
                    //Cierro la conexión 2
                    shutdown(SN2, SHUT_RDWR);
                    close(SN2);
                    break;
                }

            }
            if (labelResponse1 == 'c' && labelResponse2 == 'c') {
                string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " fue creada.";
                auto response = ProtocolMessage("c", msg);
                printTrack(false, "127.0.0.1", port, response);
                sendProtocol(SD, response);
                //Cierro la conexión 1
                shutdown(SN1, SHUT_RDWR);
                close(SN1);
                //Cierro la conexión 2
                shutdown(SN2, SHUT_RDWR);
                close(SN2);
                break;
            }
            if (labelError1 == 'x' && labelError2 == 'x') {
                string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " ya existe.";
                auto response = ProtocolMessage("c", msg);
                printTrack(false, "127.0.0.1", port, response);
                sendProtocol(SD, response);
                //Cierro la conexión 1
                shutdown(SN1, SHUT_RDWR);
                close(SN1);
                //Cierro la conexión 2
                shutdown(SN2, SHUT_RDWR);
                close(SN2);
                break;
            }

            string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " fue corregida.";
            auto response = ProtocolMessage("c", msg);
            printTrack(false, "127.0.0.1", port, response);
            sendProtocol(SD, response);
            //Cierro la conexión 1
            shutdown(SN1, SHUT_RDWR);
            close(SN1);
            //Cierro la conexión 2
            shutdown(SN2, SHUT_RDWR);
            close(SN2);
            break;
        }

        default: {
            cout << "Error protocolo create" << endl;
            break;
        }
    }
}

void serverDelete(int SD, const string &nameNode) {
    auto label = readLabel(SD);
    switch (label) {
        case 'N': {//Borrar un nodo entero
            printTrack(true, "127.0.0.1", port, "D " + nameNode + " N");
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);

            // Envío el protocolo correspondiente
            auto protocol = ProtocolDeleteNode(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir confirmación

            auto labelResponse = readLabel(SN);
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, string(1, labelResponse));
            switch (labelResponse) {
                case 'e': {
                    string response;
                    auto labelError = readLabel(SN);
                    switch (labelError) {
                        case 'f': {
                            response = ProtocolMessage("e", "No se pudo borrar el nodo " + nameNode + ".");
                            break;
                        }
                        case 'x': {
                            response = ProtocolMessage("e", "No se pudo encontrar el nodo " + nameNode + ".");
                            break;
                        }
                        default:
                            response = ProtocolMessage("e", "No se pudo borrar el nodo " + nameNode + ".");
                            cout << "error label error delete" << endl;
                    }
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    break;
                }

                case 'r': {
                    auto relations = readMulti(SN);
                    printTrack(true, "127.0.0.1", port, "relations");
                    for (const auto &r: relations) {
                        dest = simpleHash(r.second, (int) secondNodes.size());
                        auto SNS = connectionToServer(secondNodes[dest]);
                        auto protocolDelete = ProtocolDeleteRelation(r.second, r.first, nameNode);
                        printTrack(false, "127.0.0.1", port, protocolDelete);
                        sendProtocol(SNS, protocolDelete);
                        //Recibir confirmación
                        char labelResponseS = readLabel(SNS);//Respuesta del nodo
                        if (labelResponseS == 'e') {
                            auto labelErrorS = readLabel(SNS);
                            printTrack(true, secondNodes[dest].first, secondNodes[dest].second,
                                       string(1, labelResponseS) + string(1, labelErrorS));
                        } else {
                            printTrack(true, secondNodes[dest].first, secondNodes[dest].second,
                                       string(1, labelResponseS));
                        }
                        shutdown(SNS, SHUT_RDWR);
                        close(SNS);
                    }
                    auto response = ProtocolMessage("c", "El nodo " + nameNode + " se borro correctamente");
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    break;
                }

                default: {
                    auto response = ProtocolMessage("e", "No se puedo borrar el nodo " + nameNode + ".");
                    cout << "error label delete relation" << endl;
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    break;
                }
            }


            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }
        case 'R': {//Borrar uan relación
            auto relation = readString(SD);
            auto node2 = readString(SD);

            printTrack(true, "127.0.0.1", port, "D " + nameNode + " N " + relation + " " + node2);

            //Me conecto al nodo1
            int dest1 = simpleHash(nameNode, (int) secondNodes.size());
            auto SN1 = connectionToServer(secondNodes[dest1]);
            // Envío el protocolo correspondiente
            auto protocol1 = ProtocolDeleteRelation(nameNode, relation, node2);
            sendProtocol(SN1, protocol1);
            printTrack(false, secondNodes[dest1].first, secondNodes[dest1].second, protocol1);
            //Recibir confirmación
            char labelResponse1 = readLabel(SN1);//Respuesta del nodo
            printTrack(true, secondNodes[dest1].first, secondNodes[dest1].second, string(1, labelResponse1));
            char labelError1;
            if (labelResponse1 == 'e') {
                labelError1 = readLabel(SN1);//Respuesta del nodo
                if (labelError1 == 'f') {
                    string msg = "No se pudo eliminar la relation " + nameNode + "<-" + relation + "->" + node2 + ".";
                    auto response = ProtocolMessage("e", msg);
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    //Cierro la conexión 1
                    shutdown(SN1, SHUT_RDWR);
                    close(SN1);
                    break;
                }
            }


            //Me conecto al nodo2
            int dest2 = simpleHash(node2, (int) secondNodes.size());
            auto SN2 = connectionToServer(secondNodes[dest2]);
            // Envío el protocolo correspondiente
            auto protocol2 = ProtocolDeleteRelation(node2, relation, nameNode);
            sendProtocol(SN2, protocol2);
            printTrack(false, secondNodes[dest2].first, secondNodes[dest2].second, protocol2);

            //Recibir confirmación
            auto labelResponse2 = readLabel(SN2);
            printTrack(true, secondNodes[dest2].first, secondNodes[dest2].second, string(1, labelResponse2));
            char labelError2;
            if (labelResponse2 == 'e') {
                labelError2 = readLabel(SN2);//Respuesta del nodo
                if (labelError2 == 'f') {
                    //TODO:Hacer create al anterior
                    string msg = "No se pudo eliminar la relation " + nameNode + "<-" + relation + "->" + node2 + ".";
                    auto response = ProtocolMessage("e", msg);
                    printTrack(false, "127.0.0.1", port, response);
                    sendProtocol(SD, response);
                    //Cierro la conexión 1
                    shutdown(SN1, SHUT_RDWR);
                    close(SN1);
                    //Cierro la conexión 2
                    shutdown(SN2, SHUT_RDWR);
                    close(SN2);
                    break;
                }
            }
            if (labelResponse1 == 'c' && labelResponse2 == 'c') {
                string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " fue eliminada.";
                auto response = ProtocolMessage("c", msg);
                printTrack(false, "127.0.0.1", port, response);
                sendProtocol(SD, response);
                //Cierro la conexión 1
                shutdown(SN1, SHUT_RDWR);
                close(SN1);
                //Cierro la conexión 2
                shutdown(SN2, SHUT_RDWR);
                close(SN2);
                break;
            }
            if (labelError1 == 'x' && labelError2 == 'x') {
                string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " no existe.";
                auto response = ProtocolMessage("c", msg);
                printTrack(false, "127.0.0.1", port, response);
                sendProtocol(SD, response);
                //Cierro la conexión 1
                shutdown(SN1, SHUT_RDWR);
                close(SN1);
                //Cierro la conexión 2
                shutdown(SN2, SHUT_RDWR);
                close(SN2);
                break;
            }

            string msg = "La relation " + nameNode + "<-" + relation + "->" + node2 + " fue corregida.";
            auto response = ProtocolMessage("c", msg);
            printTrack(false, "127.0.0.1", port, response);
            sendProtocol(SD, response);
            //Cierro la conexión 1
            shutdown(SN1, SHUT_RDWR);
            close(SN1);
            //Cierro la conexión 2
            shutdown(SN2, SHUT_RDWR);
            close(SN2);
            break;
        }
        case 'A': {//Borrar un atributo
            auto attribute = readString(SD);
            printTrack(true, "127.0.0.1", port, "D " + nameNode + " A " + attribute);
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);
            auto protocol = ProtocolDeleteAttribute(nameNode, attribute);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir confirmación
            auto labelResponse = readLabel(SN);
            string response;
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, string(1, labelResponse));
            switch (labelResponse) {
                case 'c': {
                    response = ProtocolMessage("c", "El atributo fue eliminado correctamente.");
                    break;
                }
                case 'e': { // Respuesta de confirmación
                    auto labelError = readLabel(SN);//Tipo de error
                    switch (labelError) {
                        case 'x': {// Error de existencia
                            response = ProtocolMessage("e", "El nodo " + nameNode + " con atributo " + attribute +
                                                            " no existe.");
                            break;
                        }
                        case 'f': {// Error de SQL
                            response = ProtocolMessage("e", "No se pudo eliminar el atributo.");
                            break;
                        }
                        default: {
                            cout << "error labelError update atributo" << endl;
                            response = ProtocolMessage("e", "No se pudo eliminar el atributo.");
                        }
                    }
                    break;
                }
                default: {
                    cout << "error labelResponse update atributo" << endl;
                    response = ProtocolMessage("e", "No se pudo eliminar el atributo.");
                }
            }
            sendProtocol(SD, response);
            printTrack(false, "127.0.0.1", port, response);
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }
        default: {

            break;
        }
    }
}

void serverUpdate(int SD, const string &nameNode) { //FINO
    auto label = readLabel(SD);//Tipo de creado
    switch (label) {
        case 'N': {//Update Node
            //TODO
            break;
        }
        case 'R': {//Update relacion
            //TODO
            break;
        }

        case 'U': {//Update Atributo
            printTrack(true, "127.0.0.1", port, "U " + nameNode + " U");//Solo para testeo
            auto attributes = readMulti(SD);
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);//SN= socket Node
            // Envío el protocolo correspondiente
            auto protocol = ProtocolUpdateAttributes(nameNode, attributes);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir confirmación
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, "Respuesta");
            auto labelResponse = readLabel(SN);
            string response;//Respuesta al cliente
            switch (labelResponse) {
                case 'c': { // Respuesta de confirmación
                    if (attributes.size() > 1) {
                        response = ProtocolMessage("c", "Los atributos fueron creados correctamente.");
                    } else {
                        response = ProtocolMessage("c", "El atributo fue creado correctamente.");
                    }
                    break;
                }
                case 'e': { // Respuesta de confirmación
                    auto labelError = readLabel(SN);//Tipo de error
                    switch (labelError) {
                        case 'x': {// Error de existencia
                            response = ProtocolMessage("e", "El nodo " + nameNode + " no existe.");
                            break;
                        }
                        case 'f': {// Error de SQL
                            if (attributes.size() > 1) {
                                response = ProtocolMessage("e", "No se pudieron crear los atributos.");
                            } else {
                                response = ProtocolMessage("e", "No se pudo crear el atributo.");
                            }
                            break;
                        }
                        default: {
                            cout << "error labelError update atributo" << endl;
                            if (attributes.size() > 1) {
                                response = ProtocolMessage("e", "No se pudieron crear los atributos.");
                            } else {
                                response = ProtocolMessage("e", "No se pudo crear el atributo.");
                            }
                            break;
                        }
                    }
                    break;
                }
                default: {
                    cout << "error labelResponse update atributo" << endl;
                    if (attributes.size() > 1) {
                        response = ProtocolMessage("e", "No se pudieron crear los atributos.");
                    } else {
                        response = ProtocolMessage("e", "No se pudo crear el atributo.");
                    }
                }
            }
            sendProtocol(SD, response);
            printTrack(false, "127.0.0.1", port, response);
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }

        default: {
            cout << "Error protocolo create" << endl;
            break;
        }
    }
}

void serverRead(int SD, const string &nameNode) { //FINO
    auto label = readLabel(SD);//Tipo de creado
    switch (label) {
        case 'R': {//Read relaciones por nivel
            auto level = readInt(SD);
            printTrack(true, "127.0.0.1", port, "R " + nameNode + " R " + to_string(level));//Solo para testeo

            queue<node> bfs;
            vector<node> out;
            //Inicio del BFS
            bfs.push(node(nameNode, 0));
            while (!bfs.empty()) {
                auto currNode = bfs.front();
                if(currNode.lvl>level){
                    break;
                }
                out.push_back(currNode);
                bfs.pop();
                int dest = simpleHash(currNode.name, (int) secondNodes.size());
                auto SN = connectionToServer(secondNodes[dest]);
                auto protocol = ProtocolReadRelationsNode(currNode.name);
                sendProtocol(SN, protocol);
                //Recibo relaciones
                auto labelResponse = readLabel(SN);
                auto relations = readRelations(SN);
                for (const auto &r: relations) {
                    bool insert = true;
                    for (const auto &b: currNode.road) {
                        if (r == b) {
                            insert = false;
                        }
                    }
                    if (insert) {
                        bfs.push(node(r, currNode.lvl + 1, currNode.road));
                    }
                }
                shutdown(SN, SHUT_RDWR);
                close(SN);
            }
            auto response=relationsToMessage(nameNode,level, out);
            auto protocol= ProtocolMessage("r",response);
            sendProtocol(SD, protocol);
            break;
        }
        case 'A': {//Read atributos
            printTrack(true, "127.0.0.1", port, "R " + nameNode + " A");//Solo para testeo
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);//SN= socket Node
            // Envío el protocolo correspondiente
            auto protocol = ProtocolReadAttributes(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir respuesta
            auto labelResponse = readLabel(SN);
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, string(1, labelResponse));
            string response;
            switch (labelResponse) {
                case 'r': {
                    auto attributes = readMulti(SN);
                    auto msg = attributesToMessage(nameNode, attributes);
                    response = ProtocolMessage("r", msg);
                    break;
                }
                case 'e': {
                    auto labelError = readLabel(SN);
                    switch (labelError) {
                        case 'f': {
                            response = ProtocolMessage("e", "No se pudo leer los atributos del nodo " + nameNode + ".");
                            break;
                        }
                        case 'x': {
                            response = ProtocolMessage("e", "No se pudo encontrar el nodo " + nameNode + ".");
                            break;
                        }
                        default:
                            response = ProtocolMessage("e", "No se pudo leer los atributos del nodo " + nameNode + ".");
                            cout << "error label error read atributos" << endl;
                            break;
                    }
                    break;
                }
                default: {
                    response = ProtocolMessage("e", "No se pudo leer los atributos del nodo " + nameNode + ".");
                    cout << "error label error read atributos" << endl;
                    break;
                }

            }
            sendProtocol(SD, response);
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }

        case 'I': {//Read información
            printTrack(true, "127.0.0.1", port, "R " + nameNode + " A");//Solo para testeo
            //Me conecto al nodo
            int dest = simpleHash(nameNode, (int) secondNodes.size());
            auto SN = connectionToServer(secondNodes[dest]);//SN= socket Node
            // Envío el protocolo correspondiente
            auto protocol = ProtocolReadInfo(nameNode);
            sendProtocol(SN, protocol);
            printTrack(false, secondNodes[dest].first, secondNodes[dest].second, protocol);
            //Recibir respuesta
            auto labelResponse = readLabel(SN);
            printTrack(true, secondNodes[dest].first, secondNodes[dest].second, string(1, labelResponse));
            string response;
            switch (labelResponse) {
                case 'r': {
                    auto info = readInfo(SN);
                    auto msg = infoToMessage(nameNode, info);
                    response = ProtocolMessage("r", msg);
                    break;
                }
                case 'e': {
                    auto labelError = readLabel(SN);
                    switch (labelError) {
                        case 'f': {
                            response = ProtocolMessage("e", "No se pudo leer la info nodo " + nameNode + ".");
                            break;
                        }
                        case 'x': {
                            response = ProtocolMessage("e", "No se pudo encontrar el nodo " + nameNode + ".");
                            break;
                        }
                        default:
                            response = ProtocolMessage("e", "No se pudo leer la info del nodo " + nameNode + ".");
                            cout << "error label error read info" << endl;
                            break;
                    }
                    break;
                }
                default: {
                    response = ProtocolMessage("e", "No se pudo leer los atributos del nodo " + nameNode + ".");
                    cout << "error label response read info" << endl;
                    break;
                }

            }
            sendProtocol(SD, response);
            shutdown(SN, SHUT_RDWR);
            close(SN);
            break;
        }

        default: {
            cout << "Error protocolo create" << endl;
            break;
        }
    }
}

void serverInfo(int SD){
    vector<int> infoNode;
    for(const auto& n:secondNodes){
        auto SN = connectionToServer(n);//SN= socket Node
        auto protocol = ProtocolInfoServer();
        sendProtocol(SN, protocol);

        auto labelResponse = readLabel(SN);//Leo la etiqueta r
        auto numNodes = readInt(SN);
        infoNode.push_back(numNodes);
    }
}

void readServer(int SD) {
    auto label = readLabel(SD);
    auto nameNode = readString(SD);
    switch (label) {
        case 'C': {
            serverCreate(SD, nameNode);
            break;
        }
        case 'D': {
            serverDelete(SD, nameNode);
            break;
        }
        case 'U': {
            serverUpdate(SD, nameNode);
            break;
        }
        case 'R': {
            serverRead(SD, nameNode);
            break;
        }
        case 'I': {
            serverInfo(SD);
            break;
        }

        default: {
            cout << "Comando desconocido." << endl;
            break;
        }
    }
    shutdown(SD, SHUT_RDWR);
    close(SD);
}

int main() {
    cout << "Se inicio el server Principal" << endl;
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