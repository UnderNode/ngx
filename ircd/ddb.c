#include "ddb.h"
#include "s_debug.h"
#include "ircd_defs.h"
#include "ircd_string.h"
#include <string.h>
#include <strings.h>

/**
 * MySQL Pointer connection
 * */
MYSQL *conn;

int ddb_init(struct ddb *db)
{
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
void finish_mysql_connection(void)
{
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
        Debug((DEBUG_INFO, "sin stmt %s", mysql_stmt_error(stmt)));
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
        Debug((DEBUG_INFO, "no se puede hacer binding %s", mysql_stmt_error(stmt)));
        return MYSQL_DB_ERROR;
    }

    if (mysql_stmt_execute(stmt))
    {
        Debug((DEBUG_INFO, "no se puede ejecutar query %s", mysql_stmt_error(stmt)));
        return MYSQL_DB_ERROR;
    }

    if (mysql_stmt_store_result(stmt))
    {
        Debug((DEBUG_INFO, "no se puede almacenar resultados %s", mysql_stmt_error(stmt)));
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
        Debug((DEBUG_INFO, "bind result failed %s", mysql_stmt_error(stmt)));
        return MYSQL_DB_ERROR;
    }

    mysql_stmt_fetch(stmt);

    mysql_stmt_fetch_column(stmt, params, 0, 0);

    if (EmptyString(nickbuf) || EmptyString(passbuf))
    {
        return MYSQL_DB_TRYAGAIN;
    }

    mysql_stmt_free_result(stmt);

    finish_mysql_connection();

    return MYSQL_DB_OK;
}