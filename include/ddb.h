#include <mysql/mysql.h>

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