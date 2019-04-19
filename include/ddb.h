#include <mysql/mysql.h>

#define MYSQL_DB_ERROR      0x0001
#define MYSQL_DB_OK         0x0002
#define MYSQL_DB_TRYAGAIN   0x0004

#define STMT_PASSWORD   "SELECT usuario, password " \
                            "FROM usuarios WHERE usuario = ? AND password = BINARY ? LIMIT 1"
/**
 * Info for MySQL Connection
 * */
struct ddb
{
    char *host;
    char *user;
    char *pass;
    char *dbname;
    int port;
};

/**
 * Close a connection to database
 * */
extern void finish_mysql_connection(void);
/**
 * Initialize ddb handler (or check if MySQL is available)
 * */
extern int ddb_init(struct ddb *ddb);
/**
 * Match nick password
 * */
extern int ddb_match_nickname(char *nick, char *passwd);