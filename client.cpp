#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "functions.h"

using namespace std;


int port = 45151;
char ip[] = "127.0.0.1";

string printErr() {
    cout << "Comando desconocido." << endl;
    return "Z" + stringToProtocol("error");
}

string create(int argc, char *argv[]) {
    // Crear nodo
    if (argc == 3) {
        return ProtocolCreateNode(string(argv[2]));
    }
    // Crear relación
    if (argc == 5) {
        auto node1 = string(argv[2]);
        auto relation = string(argv[3]);
        auto node2 = string(argv[4]);
        return ProtocolCreateRelation(node1, relation, node2);
    }
    // Retornar error
    return printErr();
}

string read(int argc, char *argv[]) {
    // Leer relaciones por nivel
    if (argc == 5) {
        return ProtocolReadRelations(argv[2], stoi(argv[4]));
    }
    // Leer atributos
    if (argc == 4 and string(argv[3]) == "A") {
        return ProtocolReadAttributes(argv[2]);
    }
    // Leer información
    if (argc == 4 and string(argv[3]) == "I") {
        return ProtocolReadInfo(argv[2]);
    }
    // Retornar error
    return printErr();
}

string update(int argc, char *argv[]) {
    // Actualizar nodo
    if (argc == 4) {
        return ProtocolUpdateNode(string(argv[2]), string(argv[3]));
    }
    // Actualizar relación
    if (argc == 8 and string(argv[3]) != "CA") {
        vector<string> items;
        for (int i = 3; i < argc; i++) {
            items.emplace_back(argv[i]);
        }
        return ProtocolUpdateRelation(string(argv[2]), items);
    }
    // Crear argumentos
    if (argc > 4 and string(argv[3]) == "CA") {
        vector<pair<string, string>> items;
        for (int i = 4; i < argc; i += 2) {
            items.emplace_back(argv[i], argv[i + 1]);
        }
        return ProtocolUpdateAttributes(string(argv[2]), items);
    }
    // Retornar error
    return printErr();
}

string erase(int argc, char *argv[]) {
    // Borrar nodo
    if (argc == 3) {
        return ProtocolDeleteNode(string(argv[2]));
    }
    // Borrar atributo
    if (argc == 4) {
        return ProtocolDeleteAttribute(string(argv[2]), string(argv[3]));
    }
    // Borrar relación
    if (argc == 5) {
        auto node1 = string(argv[2]);
        auto relation = string(argv[3]);
        auto node2 = string(argv[4]);
        return ProtocolDeleteRelation(node1, relation, node2);
    }
    // Retornar error
    return printErr();
}

string info(int argc) {
    //Info de los nodos
    if (argc == 2) {
        return ProtocolInfoServer();
    }
    return printErr();
}

int main(int argc, char *argv[]) {
    auto SocketFD = connectionToServer(make_pair(ip, port));
    string protocol;
    if (argc > 2) {
        if (*argv[1] == 'C' || *argv[1] == 'c') {
            protocol = create(argc, argv);
        } else if (*argv[1] == 'U' || *argv[1] == 'u') {
            protocol = update(argc, argv);
        } else if (*argv[1] == 'D' || *argv[1] == 'd') {
            protocol = erase(argc, argv);
        } else if (*argv[1] == 'R' || *argv[1] == 'r') {
            protocol = read(argc, argv);
        } else if (*argv[1] == 'I' || *argv[1] == 'i') {
            protocol = info(argc);
        } else {
            protocol = printErr();
        }
    } else {
        protocol = printErr();
    }

    // Enviar protocolo
    sendProtocol(SocketFD, protocol);
    printTrack(false, ip, port, protocol);

    //Recibir respuesta
    auto message = readMessage(SocketFD);
    printTrack(true, ip, port, message);
    cout << message << endl;

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}