#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "functions.h"

using namespace std;


int port = 45151;
char ip[] = "127.0.0.1";

//string getIp(int SD) {
//    struct sockaddr_in name;
//    socklen_t namelen = sizeof(name);
//    int err = getsockname(SD, (struct sockaddr *) &name, &namelen);
//
//    char buffer[INET_ADDRSTRLEN];
//    const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, INET_ADDRSTRLEN);
//    string myIp(buffer, INET_ADDRSTRLEN);
//    return myIp;
//}
//
//string getPort(int SD) {
//    struct sockaddr_in sin;
//    socklen_t len = sizeof(sin);
//    if (getsockname(SD, (struct sockaddr *) &sin, &len) == -1)
//        perror("getsockname");
//    else
//        printf("port number %d\n", ntohs(sin.sin_port));
//    auto x = ntohs(sin.sin_port);
//    return to_string(x);
//}

void printErr() {
    cout << "argumentos invalidos" << endl;
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
        printErr();
    }
}

void read(int argc, char *argv[], int SD) {
    string protocolo = "R";
    if (argc > 3 && argc <= 5) {
        protocolo += stringToProtocol(string(argv[2]));
        if (*argv[3] == 'R') {
            protocolo += argv[3];
            protocolo += intToProtocol(stoi(argv[4]));
            cout << protocolo << endl;
            write(SD, &protocolo[0], protocolo.size());
        } else if (*argv[3] == 'I' || *argv[3] == 'A') {
            protocolo += argv[3];

            cout << protocolo << endl;
            write(SD, &protocolo[0], protocolo.size());
        } else {
            printErr();
        }
    } else {
        printErr();
    }
}

void update(int argc, char *argv[], int SD) {
    string protocolo = "U";
    if (argc == 4) {
        protocolo += stringToProtocol(string(argv[2]));
        protocolo += "N";
        protocolo += stringToProtocol(string(argv[3]));
        cout << protocolo << endl;
        write(SD, &protocolo[0], protocolo.size());
    } else if (argc >= 6) {
        protocolo += stringToProtocol(string(argv[2]));
        if (string(argv[3]) != "CA" && argc == 8) {
            protocolo += "R";
            protocolo += relationToProtocol(string(argv[3]), string(argv[4]));
//            protocolo += getRelacionProtocol(string(argv[5]), string(argv[6]), string(argv[7]));
            cout << protocolo << endl;
            write(SD, &protocolo[0], protocolo.size());
        } else {
            if (string(argv[3]) == "CA") {
                protocolo += "U";
                double atributos = (argc - 4) / 2;
                if (atributos == int(atributos)) {
                    protocolo += intToProtocol(atributos);

                    for (int i = 4; i < argc; i += 2) {
                        protocolo += attributeToProtocol(argv[i], argv[i + 1]);
                    }
                }

            }
            cout << protocolo << endl;
            write(SD, &protocolo[0], protocolo.size());
        }

    } else {
        printErr();
    }
}

void erase(int argc, char *argv[], int SD) {
    string protocolo = "D";
    if (argc == 3) {
        protocolo += stringToProtocol(string(argv[2]));
        protocolo += "N";
        cout << ">>" << ip << "," << port << ":" << protocolo << endl;
        write(SD, &protocolo[0], protocolo.size());
    } else if (argc == 4) {
        protocolo += stringToProtocol(string(argv[2]));
        protocolo += "A";
        protocolo += stringToProtocol(argv[3]);
        cout << ">>" << ip << "," << port << ":" << protocolo << endl;
        write(SD, &protocolo[0], protocolo.size());
    } else if (argc == 5) {
        protocolo += stringToProtocol(string(argv[2]));
        protocolo += "R";
        protocolo += relationToProtocol(string(argv[3]), string(argv[4]));
        cout << ">>" << ip << "," << port << ":" << protocolo << endl;
        write(SD, &protocolo[0], protocolo.size());

    } else {
        printErr();
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
        } else {
            printErr();
        }
    } else {
        printErr();
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);


}