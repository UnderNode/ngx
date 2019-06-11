#include "ircd_string.h"
#include "ircd_reply.h"
#include "ircd_relay.h"
#include "ircd_alloc.h"
#include "handlers.h"
#include "numeric.h"
#include "s_debug.h"
#include "client.h"
#include "ircd.h"
#include "ddb.h"
#include "dbq.h"
#include "channel.h"
#include "ircd_snprintf.h"

#include <string.h>

Methods methods[] = {
    {"GET", parse_get},
    {"POST", parse_post},
    {NULL}};

Methods *find_method(char *m)
{
    Methods *mm;
    for (mm = methods; mm->method; mm++)
    {
        if (ircd_strcmp(m, mm->method) == 0)
        {
            return mm;
        }
    }
    return NULL;
}

int parse_get(void *sptr, int parc, char *parv[])
{
    Debug((DEBUG_INFO, "From method GET"));
    return 0;
}

int parse_post(void *sptr, int parc, char *parv[])
{
    struct Client *acptr = (struct Client *)sptr;

    if (parc < 3)
    {
        need_more_params(sptr, "\002DBQ\002 POST");
        return 0;
    }
    int ltok = (int)strlen((const char *)parv[2]);

    if (ltok > 1)
    {
        relay_private_notice(sptr, cli_name(acptr), "Invalid token, try again.");
        return 0;
    }

    char action = ToUpper((char)*(parv[2]));

    switch (action)
    {
    case 'N':
        if (parc < 5)
        {
            need_more_params(sptr, "\002DBQ\002 POST N");
        }
        else
        {
            char *n = (parv[3] ? parv[3] : NULL);
            char *p = (parv[4] ? parv[4] : NULL);
            char *v = (parv[5] ? parv[5] : NULL);
            char *buf = (char *)MyMalloc(BUFSIZE);
            memset(buf, 0, BUFSIZE);
            if (ddb_init(1))
            {
                int errno = 0;
                switch ((errno = ddb_fetch_nick(n)))
                {
                case MYSQL_DB_BIND_PARAM_ERROR:
                case MYSQL_DB_BIND_RESULT_ERROR:
                case MYSQL_DB_ERROR:
                case MYSQL_DB_EXE_STMT_ERROR:
                case MYSQL_DB_STORE_ERROR:
                    ircd_snprintf(0, buf, BUFSIZE, "An db error ocurred (%d)", errno);
                    break;
                case MYSQL_DB_OK:
                    ircd_snprintf(0, buf, BUFSIZE, "nickname \002%s\002 already registered.", n);
                    break;
                case MYSQL_DB_TRYAGAIN:
                    ircd_snprintf(0, buf, BUFSIZE, "nickname \002%s\002 is ready to be registered.", n);
                    break;
                }
                relay_private_notice(acptr, acptr->cli_name, buf);
                ddb_end_transaction();
            }
            MyFree(buf);
        }
        break;
    case 'C':
        break;
    default:
        break;
    }
    Debug((DEBUG_INFO, "From method POST %d %d %d %c", sizeof(parv[2]), sizeof(char), sizeof(action), action));
    return 0;
}

int mo_dbq(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
    if (parc < 2)
    {
        need_more_params(sptr, "DBQ");
    }
    char *m = parv[1];
    Methods *_m;
    if (!(_m = find_method(m)))
    {
        Debug((DEBUG_INFO, "Method %s not found", m));
    }
    else
    {
        _m->func(sptr, parc, parv);
    }
    return 0;
}
int ms_dbq(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
    if (parc < 2)
    {
        need_more_params(sptr, "DBQ");
    }
    return 0;
}