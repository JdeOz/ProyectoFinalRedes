#pragma once
#include "sqlite3.h"
#include <string>
#include <iostream>
using namespace std;

int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

class sqliteManager
{
    public:
        sqlite3* db;

        sqliteManager(){
            db=nullptr;
        }
        ~sqliteManager(){
            sqlite3_close(db);
        }
        void setport(string name){
            sqlite3_open(name.c_str(), &db); 

            const char *sql = "create table relaciones ( "  \
            "Nodo1 text not null, " \
            "relacion text," \
            "Nodo2 text,"\
            "unique(Nodo1, relacion, Nodo2));";
            const char* sql1 = "create table atributos ( "  \
            "Nodo text not null, " \
            "nombre text not null," \
            "valor text not null,"\
            "unique(Nodo, nombre));";
            
            char* zErrMsg;
            int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            cout<<rc<<"\n";
            
            int rc2=sqlite3_exec(db, sql1, callback, 0, &zErrMsg);
            cout<<rc2<<"\n";
            
        }

        bool SQLiteCreate(string nameNode, string Rel = "NULL", string nameNode2 = "NULL") {
            cout<<"creando"<<endl;
            char* zErrMsg;
            string comando;
            comando = "insert into relaciones (Nodo1, relacion, Nodo2) values ('"+nameNode+"','"+Rel+"','"+nameNode2+"')";
            const char* sql = comando.c_str();
            int rc;
            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                //fprintf(stderr, "Error SQL: %s\n", zErrMsg);
                //sqlite3_free(zErrMsg);
                return false;
            }
            const char* sql2 ="SELECT * FROM relaciones";
            sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
            return true;
        } 

        bool SQLiteDeleteNode(string nameNode) {
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
        
        bool SQLiteDeleteRel( string nameNode1,string nameRel, string nameNode2) {

            char* zErrMsg;
            string comando;
            comando = "delete from relaciones where relacion = '" + nameRel + "' and Nodo1 = '"+nameNode1+"' and Nodo2 = '"+nameNode2+"';";
            const char* com2 = comando.c_str();
            int rc;
            rc = sqlite3_exec(db, com2, callback, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                return false;
            }
            return true;
        }

        
};

