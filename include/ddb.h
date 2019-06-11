#include <mysql/mysql.h>

#define MYSQL_DB_ERROR 0x0001
#define MYSQL_DB_OK 0x0002
#define MYSQL_DB_TRYAGAIN 0x0004
#define MYSQL_DB_BIND_PARAM_ERROR 0x0008
#define MYSQL_DB_EXE_STMT_ERROR 0x0010
#define MYSQL_DB_STORE_ERROR 0x0020
#define MYSQL_DB_BIND_RESULT_ERROR 0x0040
#define MYSQL_DB_QUERY_ERROR 0x0080
#define STMT_CREATE_SCHEMA "CREATE SCHEMA IF NOT EXISTS undernode"
#define STMT_USE_SCHEMA "USE undernode"
#define STMT_PASSWORD "SELECT username, password " \
                      "FROM users WHERE username = ? AND password = BINARY ? LIMIT 1"
#define STMT_FETCH_NICK "SELECT username FROM users WHERE " \
                        "username = ? LIMIT 1"
#define STMT_CREATE_USERS_TABLE "CREATE TABLE IF NOT EXISTS users ("    \
                                "id INT PRIMARY KEY AUTO_INCREMENT,"    \
                                "username VARCHAR(%d) NOT NULL UNIQUE," \
                                "password VARCHAR(%d) NOT NULL,"        \
                                "vhost VARCHAR(%d))"
#define STMT_CREATE_CHANNELS_TABLE "CREATE TABLE IF NOT EXISTS channels (" \
                                   "id INT PRIMARY KEY AUTO_INCREMENT,"    \
                                   "channel VARCHAR(%d) NOT NULL UNIQUE,"  \
                                   "owner VARCHAR(%d) NOT NULL UNIQUE, "   \
                                   "FOREIGN KEY (id) REFERENCES users(id))"
/**
 * Info for MySQL Connection
 **/
struct ddb
{
    char *host;
    char *user;
    char *pass;
    char *dbname;
    int port;
};
/**
 * Create db schema 
 **/
extern int ddb_create_schema(void);
/**
 * Close a connection to database
 **/
extern void ddb_end_transaction(void);
/**
 * Initialize ddb handler (or check if MySQL is available)
 **/
extern int ddb_init(int withdb);
/**
 * Match nick password
 **/
extern int ddb_match_nickname(char *nick, char *passwd);
/**
 * Find a registered nick
 **/
extern int ddb_fetch_nick(char *nick);