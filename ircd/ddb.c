#include "ddb.h"
#include "s_debug.h"
#include <string.h>
#include <strings.h>

/**
 * MySQL Pointer connection
 * */
MYSQL *conn;
/**
 * MySQL Pointer results
 * */
MYSQL_RES *res;
/**
 * MySQL Pointer rows
 * */
MYSQL_ROW row;
/**
 * MySQL Pointer fields
 * */
MYSQL_FIELD *field;

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
    const char *query = "SELECT usuario, password FROM usuarios";

    if (mysql_query(conn, query) == 0)
    {
        res = mysql_use_result(conn);
        if (res)
        {
            int index = 0;
            while ((field = mysql_fetch_field(res)))
            {
                index++;
                if (strcmp(field->name, "password") == 0)
                {
                    while ((row = mysql_fetch_row(res)))
                    {
                        if (row[index - 1])
                        {
                            //Debug((DEBUG_INFO, "Connection with mysql is %d", ddb_init(db)));
                            if (strcasecmp(row[index - 1], passwd) == 0)
                            {
                                mysql_free_result(res);
                                return 1;
                            }
                        }
                    }
                }
            }
            mysql_free_result(res);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}