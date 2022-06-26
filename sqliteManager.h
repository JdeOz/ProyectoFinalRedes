#pragma once

#include "sqlite3.h"
#include <string>
#include <iostream>
#include <cstring>

using namespace std;

bool exist = false;
vector<pair<string, string>> cbItems;


int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int callbackExist(void *NotUsed, int argc, char **argv, char **azColName) {
    if (argc) exist = true;
    return 0;
}

int callbackItems(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i += 2) {
        string relation = argv[i] ? string(argv[i]) : "NULL";
        string node2 = argv[i + 1] ? string(argv[i + 1]) : "NULL";
        cbItems.emplace_back(relation, node2);
    }
    // Return successful
    return 0;
}

static int callbackCount(void *count, int argc, char **argv, char **azColName) {
    int *c = static_cast<int *>(count);
    *c = atoi(argv[0]);
    return 0;
}


class sqliteManager {
public:
    sqlite3 *db;

    sqliteManager() {
        db = nullptr;
    }

    ~sqliteManager() {
        sqlite3_close(db);
    }

    void setport(string name) {
        sqlite3_open(name.c_str(), &db);

        const char *sql = "create table relaciones ( "  \
            "Nodo1 text not null, " \
            "relacion text," \
            "Nodo2 text,"\
            "unique(Nodo1, relacion, Nodo2));";
        const char *sql1 = "create table atributos ( "  \
            "Nodo text not null, " \
            "nombre text not null," \
            "valor text not null,"\
            "unique(Nodo, nombre));";

        char *zErrMsg;
        int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        cout << rc << "\n";

        int rc2 = sqlite3_exec(db, sql1, callback, 0, &zErrMsg);
        cout << rc2 << "\n";

    }

    char SQLiteCreate(const string &nameNode, const string &Rel = "NULL", const string &nameNode2 = "NULL") {
        char *zErrMsg;
        string comando = "insert into relaciones (Nodo1, relacion, Nodo2) values ('" + nameNode + "','" + Rel + "','" +
                         nameNode2 + "')";
        int rc = sqlite3_exec(db, comando.c_str(), callback, nullptr, &zErrMsg);
        if (rc == 0) return 'c';
        if (rc == 19) return 'x';
        return 'f';
    }

    char SQLiteUpdateAttributes(const string &nameNode, const vector<pair<string, string>> &attributes) {
        char *zErrMsg;

        exist = false;
        string select = "select * from relaciones where Nodo1 = '" + nameNode + "';";
        int rcs = sqlite3_exec(db, select.c_str(), callbackExist, 0, &zErrMsg);
        if (rcs != SQLITE_OK) return 'f';
        if (!exist) {
            return 'x';
        }

        bool rc = false;
        for (const auto &attribute: attributes) {
            exist = false;
            select = "select * from atributos where Nodo = '" + nameNode + "' and nombre = '" + attribute.first + "';";
            int rca = sqlite3_exec(db, select.c_str(), callbackExist, 0, &zErrMsg);
            if (rca != SQLITE_OK) return 'f';
            if (!exist) {
                string comando =
                        "insert into atributos (Nodo, nombre, valor) values ('" + nameNode + "','" + attribute.first +
                        "','" + attribute.second + "')";
                rc = rc or sqlite3_exec(db, comando.c_str(), callback, nullptr, &zErrMsg);
            } else {
                string comando =
                        "update atributos set valor='" + attribute.second + "' where Nodo='" + nameNode +
                        "' and nombre='" + attribute.first + "';";
                rc = rc or sqlite3_exec(db, comando.c_str(), callback, nullptr, &zErrMsg);
            }

        }
        if (rc == 0) return 'c';
        return 'f';
    }

    bool SQLiteDeleteNode(string nameNode) {
        char *zErrMsg;
        string comando;
        comando = "delete from relaciones where Nodo1 = '" + nameNode + "';";
        const char *com2 = comando.c_str();
        int rc;
        rc = sqlite3_exec(db, com2, callback, 0, &zErrMsg);
        cout << rc;
        if (rc != SQLITE_OK) {
            return false;
        }
        return true;
    }

    char SQLiteDeleteAttribute(const string &nameNode, const string &attribute) {
        char *zErrMsg;
        exist = false;
        string select = "select * from atributos where Nodo = '" + nameNode + "' and nombre = '" + attribute + "';";
        int rc = sqlite3_exec(db, select.c_str(), callbackExist, 0, &zErrMsg);
        if (rc != SQLITE_OK) return 'f';
        if (!exist) {
            return 'x';
        }

        string comando = "delete from atributos where Nodo = '" + nameNode + "' and nombre = '" + attribute + "';";
        rc = sqlite3_exec(db, comando.c_str(), callback, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            return 'f';
        }
        return 'c';
    }

    char SQLiteDeleteRel(const string &nameNode1, const string &nameRel, const string &nameNode2) {
        char *zErrMsg;
        exist = false;
        string select = "select * from relaciones where relacion = '" + nameRel + "' and Nodo1 = '" + nameNode1 +
                        "' and Nodo2 = '" + nameNode2 + "';";
        int rc = sqlite3_exec(db, select.c_str(), callbackExist, 0, &zErrMsg);
        if (rc != SQLITE_OK) return 'f';
        if (!exist) {
            return 'x';
        }

        string comando = "delete from relaciones where relacion = '" + nameRel + "' and Nodo1 = '" + nameNode1 +
                         "' and Nodo2 = '" + nameNode2 + "';";
        rc = sqlite3_exec(db, comando.c_str(), callback, 0, &zErrMsg);
        cout << "rc: " << rc << endl;
        if (rc != SQLITE_OK) {
            return 'f';
        }
        return 'c';
    }

    char SQLSelectRelations(const string &nameNode, vector<pair<string, string>> &relations) {
        char *zErrMsg;
        string comando = "select relacion, Nodo2 from relaciones where Nodo1='" + nameNode + "';";
        cbItems.clear();
        int rc = sqlite3_exec(db, comando.c_str(), callbackItems, nullptr, &zErrMsg);
        cout << "rc: " << rc << endl;
        relations = cbItems;
        if (rc == 0) return 'c';
        return 'f';
    }

    char SQLSelectAtributes(const string &nameNode, vector<pair<string, string>> &attributes) {
        char *zErrMsg;
        string comando = "select nombre, valor from atributos where Nodo='" + nameNode + "';";
        cbItems.clear();
        int rc = sqlite3_exec(db, comando.c_str(), callbackItems, nullptr, &zErrMsg);
        attributes = cbItems;
        if (rc == 0) return 'c';
        return 'f';
    }

    char SQLSelectInfo(const string &nameNode, int &numRelations, int &numAttributes) {
        char *zErrMsg;
        //Contamos los atributos
        int attributes = 0;
        string comando = "select count(*) from atributos where Nodo='" + nameNode + "';";
        int rc1 = sqlite3_exec(db, comando.c_str(), callbackCount, &attributes, &zErrMsg);
        numAttributes=attributes;

        //Contamos los atributos
        int relations = 0;
        comando = "select count(*) from relaciones where Nodo1='" + nameNode + "';";
        int rc2 = sqlite3_exec(db, comando.c_str(), callbackCount, &relations, &zErrMsg);
        numRelations=relations;

        if (rc1 == 0 && rc2 == 0) return 'c';
        return 'f';
    }
};

