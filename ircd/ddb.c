#include "ddb.h"
#include "s_debug.h"
#include "ircd_defs.h"
#include "ircd_string.h"
#include "ircd_snprintf.h"
#include "ircd_alloc.h"
#include "ircd_features.h"
#include "ircd.h"
#include "channel.h"
#include <string.h>
#include <strings.h>

/**
 * MySQL Pointer connection
 * */
MYSQL *conn;
/**
 * database pointer
 **/
struct ddb *db = (struct ddb *)NULL;

int ddb_init(int withdb)
{
    /**
     * Allocator for database config
     * */
    db = (struct ddb *)MyMalloc(sizeof(struct ddb));

    /**
     * This config must be in configuration file....
     **/
    db->host = "localhost";
    db->user = "undernode";
    db->pass = "eik9159a";
    db->dbname = (withdb ? "undernode" : NULL);
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

int ddb_create_schema(void)
{
    Debug((DEBUG_NOTICE, "Schema transaction started."));
    if (ddb_init(0))
    {
        if (!mysql_real_query(conn, STMT_CREATE_SCHEMA, (unsigned long)strlen(STMT_CREATE_SCHEMA)))
        {
            if (!mysql_real_query(conn, STMT_USE_SCHEMA, (unsigned long)strlen(STMT_USE_SCHEMA)))
            {
                char *buf = NULL;
                int len = 0;
                buf = (char *)MyMalloc(strlen(STMT_CREATE_USERS_TABLE) + 8);
                len = (strlen(STMT_CREATE_USERS_TABLE) + 8);
                memset(buf, 0, sizeof(buf));
                ircd_snprintf(0, buf, len,
                              STMT_CREATE_USERS_TABLE,
                              NICKLEN,
                              PASSWDLEN, HOSTLEN);
                if (!mysql_real_query(conn, buf, (unsigned long)strlen(buf)))
                {
                    Debug((DEBUG_NOTICE, "Table %s created successfully", "users"));
                }
                MyFree(buf);
                buf = (char *)MyRealloc(buf, strlen(STMT_CREATE_CHANNELS_TABLE) + 8);
                len = (strlen(STMT_CREATE_CHANNELS_TABLE) + 8);
                memset(buf, 0, sizeof(buf));
                ircd_snprintf(0, buf, len,
                              STMT_CREATE_CHANNELS_TABLE,
                              CHANNELLEN,
                              NICKLEN);
                if (!mysql_real_query(conn, buf, (unsigned long)strlen(buf)))
                {
                    Debug((DEBUG_NOTICE, "Table %s created successfully", "channels"));
                }
                MyFree(buf);
            }
        }
        else
        {
            Debug((DEBUG_NOTICE, "Error creating schema %s (%s).", "undernode", mysql_error(conn)));
        }
        ddb_end_transaction();
        return 1;
    }
    else
    {
        Debug((DEBUG_ERROR, "Schema transaction failed, Try connection to database server."));
    }
    return 0;
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
        return MYSQL_DB_BIND_PARAM_ERROR;
    }
    if (mysql_stmt_execute(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_EXE_STMT_ERROR;
    }
    if (mysql_stmt_store_result(stmt))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_STORE_ERROR;
    }
    char nickbuf[NICKLEN + 1];

    memset(nickbuf, 0, sizeof(nickbuf));

    params[0].buffer = nickbuf;
    params[0].buffer_length = strlen(nick);
    params[0].length = &n;

    if (mysql_stmt_bind_result(stmt, params))
    {
        mysql_stmt_free_result(stmt);
        return MYSQL_DB_BIND_RESULT_ERROR;
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