#include "ddb.h"
#include "s_debug.h"
#include "ircd_defs.h"
#include "ircd_string.h"
#include "ircd_alloc.h"
#include <string.h>
#include <strings.h>

/**
 * MySQL Pointer connection
 * */
MYSQL *conn;
/**
 * database pointer
 * */
struct ddb *db = (struct ddb *)NULL;

int ddb_init(void)
{
    /**
     * Allocator for database config
     * */
    db = (struct ddb *)MyMalloc(sizeof(struct ddb));

    db->host = "localhost";
    db->user = "undernode";
    db->pass = "eik9159a";
    db->dbname = "undernode";
    db->port = 3306;

    conn = mysql_init(NULL);

    if (!conn)
    {
        Debug((DEBUG_ERROR, "Error init MySQL Client (%s)", mysql_errno(conn)));
        return 0;
    }
    if (!mysql_real_connect(
            conn,
            (const char *)db->host,
            (const char *)db->user,
            (const char *)db->pass,
            (const char *)db->dbname,
            (unsigned int)db->port,
            NULL, 0))
    {
        Debug((DEBUG_ERROR, "Error connecting to MySQL Server (%s)", mysql_errno(conn)));
        return 0;
    }
    return 1;
}
void ddb_end_transaction(void)
{
    MyFree(db);
    mysql_close(conn);
}

int ddb_match_nickname(char *nick, char *passwd)
{
    /**
    * MySQL Pointer statement
     * */
    MYSQL_STMT *stmt;
    /**
    * MySQL Pointer bind params
    * */
    MYSQL_BIND params[2];

    stmt = mysql_stmt_init(conn);

    if (!stmt)
    {
        return MYSQL_DB_ERROR;
    }

    mysql_stmt_prepare(stmt, STMT_PASSWORD, (unsigned long)strlen(STMT_PASSWORD));

    memset(params, 0, sizeof(params));

    unsigned long n = strlen(nick);
    unsigned long p = strlen(passwd);

    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)nick;
    params[0].buffer_length = strlen(nick) + 1;
    params[0].is_null = 0;
    params[0].length = &n;

    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *)passwd;
    params[1].buffer_length = strlen(passwd) + 1;
    params[1].is_null = 0;
    params[1].length = &p;

    if (mysql_stmt_bind_param(stmt, params))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }

    if (mysql_stmt_execute(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }

    if (mysql_stmt_store_result(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }
    char nickbuf[NICKLEN + 1];
    char passbuf[PASSWDLEN + 1];

    memset(nickbuf, 0, sizeof(nickbuf));
    memset(passbuf, 0, sizeof(passbuf));

    params[0].buffer = nickbuf;
    params[0].buffer_length = strlen(nick) + 30;
    params[0].length = &n;

    params[1].buffer = passbuf;
    params[1].buffer_length = strlen(passbuf) + 30;
    params[1].length = &p;

    if (mysql_stmt_bind_result(stmt, params))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }

    mysql_stmt_fetch(stmt);

    mysql_stmt_fetch_column(stmt, params, 0, 0);

    if (EmptyString(nickbuf) || EmptyString(passbuf))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_TRYAGAIN;
    }

    mysql_stmt_free_result(stmt);

    return MYSQL_DB_OK;
}
int ddb_fetch_nick(char *nick)
{

    /**
    * MySQL Pointer statement
     * */
    MYSQL_STMT *stmt;
    /**
    * MySQL Pointer bind params
    * */
    MYSQL_BIND params[1];

    stmt = mysql_stmt_init(conn);

    if (!stmt)
    {
        return MYSQL_DB_ERROR;
    }

    mysql_stmt_prepare(stmt, STMT_FETCH_NICK, (unsigned long)strlen(STMT_FETCH_NICK));

    memset(params, 0, sizeof(params));

    unsigned long n = strlen(nick);

    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = nick;
    params[0].buffer_length = strlen(nick);
    params[0].is_null = 0;
    params[0].length = &n;

    if (mysql_stmt_bind_param(stmt, params))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }
    if (mysql_stmt_execute(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }
    if (mysql_stmt_store_result(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }
    char nickbuf[NICKLEN + 1];

    memset(nickbuf, 0, sizeof(nickbuf));

    params[0].buffer = nickbuf;
    params[0].buffer_length = strlen(nick);
    params[0].length = &n;

    if (mysql_stmt_bind_result(stmt, params))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_ERROR;
    }

    mysql_stmt_fetch(stmt);

    mysql_stmt_fetch_column(stmt, params, 0, 0);

    if (EmptyString(nickbuf))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_TRYAGAIN;
    }

    mysql_stmt_free_result(stmt);

    return MYSQL_DB_OK;
}