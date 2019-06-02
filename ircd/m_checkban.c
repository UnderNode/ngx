#include "handlers.h"
#include "s_debug.h"
#include "channel.h"
#include "ircd_string.h"
#include "ircd_reply.h"
#include "numeric.h"
#include "hash.h"
#include "ircd.h"
#include "client.h"
#include "ircd_relay.h"
#include "ircd_snprintf.h"
#include "ircd_alloc.h"
#include "checkban.h"
#include "s_misc.h"

#include <string.h>
#include <time.h>

/**
 * Check if a user is banned on channel given
 * parv[1] nick to check if banned
 * parv[2] channel where checking, if null, will be searched in global channel list.
 **/

int mo_checkban(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{

    struct Channel *channel;
    struct Client *acptr;
    struct Ban *ban;

    if (parc < 2)
    {
        need_more_params(sptr, "CHECKBAN");
        return 0;
    }

    /**
     * required
     **/
    char *nick = parv[1];
    /**
     * If not channel given, search in global channel list
     **/
    char *chan = (parv[2] != NULL ? parv[2] : NULL);

    if (chan)
    {

        if (!IsChannelName(chan) || !strIsIrcCh(chan) || !(channel = FindChannel(chan)))
        {
            send_reply(sptr, ERR_NOSUCHCHANNEL, chan);
            return 0;
        }
        if (!(acptr = FindUser(nick)))
        {
            send_reply(sptr, ERR_NOSUCHNICK, nick);
            return 0;
        }
        if (!(ban = find_ban(acptr, channel->banlist)))
        {
            send_reply(sptr, RPL_NOTFOUNDCHECKBAN, acptr->cli_name, channel->chname);
        }
        if (ban)
        {

            int len = BUFSIZE;
            char *buf = NULL;
            char *_buf = NULL;
            buf = (char *)MyMalloc(len);
            memset(buf, 0, sizeof(buf));
            ircd_snprintf(0,
                          buf,
                          len,
                          CHECKBAN_INFO,
                          channel->chname,
                          ban->who, date(ban->when),
                          ban->banstr);
            send_reply(sptr, RPL_CHECKBAN, buf);
            MyFree(buf);
        }
    }
    else
    {

        if (!(acptr = FindUser(nick)))
        {
            send_reply(sptr, ERR_NOSUCHNICK, nick);
            return 0;
        }
        int len = BUFSIZE;
        char *buf = NULL;
        char *_buf = NULL;

        for (channel = GlobalChannelList; channel; channel = channel->next)
        {
            if (!(ban = find_ban(acptr, channel->banlist)))
            {
                continue;
            }
            else
            {

                buf = (char *)MyMalloc(len);
                memset(buf, 0, sizeof(buf));
                ircd_snprintf(0,
                              buf,
                              len,
                              CHECKBAN_INFO,
                              channel->chname,
                              ban->who, date(ban->when),
                              ban->banstr);
                send_reply(sptr, RPL_CHECKBAN, buf);
                MyFree(buf);
            }
        }
        len = 0;
    }
    send_reply(sptr, RPL_ENDOFCHECKBAN);
    return 0;
}