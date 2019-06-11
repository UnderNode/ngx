/*
 * IRC - Internet Relay Chat, ircd/svsnick.c
 * Copyright (C) 1990 Jarkko Oikarinen and
 *                    University of Oulu, Computing Center
 *
 * See file AUTHORS in IRC package for additional names of
 * the programmers.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 */

/*
 * m_functions execute protocol messages on this server:
 *
 *    cptr    is always NON-NULL, pointing to a *LOCAL* client
 *            structure (with an open socket connected!). This
 *            identifies the physical socket where the message
 *            originated (or which caused the m_function to be
 *            executed--some m_functions may call others...).
 *
 *    sptr    is the source of the message, defined by the
 *            prefix part of the message if present. If not
 *            or prefix not found, then sptr==cptr.
 *
 *            (!IsServer(cptr)) => (cptr == sptr), because
 *            prefixes are taken *only* from servers...
 *
 *            (IsServer(cptr))
 *                    (sptr == cptr) => the message didn't
 *                    have the prefix.
 *
 *                    (sptr != cptr && IsServer(sptr) means
 *                    the prefix specified servername. (?)
 *
 *                    (sptr != cptr && !IsServer(sptr) means
 *                    that message originated from a remote
 *                    user (not local).
 *
 *            combining
 *
 *            (!IsServer(sptr)) means that, sptr can safely
 *            taken as defining the target structure of the
 *            message in this server.
 *
 *    *Always* true (if 'parse' and others are working correct):
 *
 *    1)      sptr->from == cptr  (note: cptr->from == cptr)
 *
 *    2)      MyConnect(sptr) <=> sptr == cptr (e.g. sptr
 *            *cannot* be a local connection, unless it's
 *            actually cptr!). [MyConnect(x) should probably
 *            be defined as (x == x->from) --msa ]
 *
 *    parc    number of variable parameter strings (if zero,
 *            parv is allowed to be NULL)
 *
 *    parv    a NULL terminated list of parameter pointers,
 *
 *                    parv[0], sender (prefix string), if not present
 *                            this points to an empty string.
 *                    parv[1]...parv[parc-1]
 *                            pointers to additional parameters
 *                    parv[parc] == NULL, *always*
 *
 *            note:   it is guaranteed that parv[0]..parv[parc-1] are all
 *                    non-NULL pointers.
 */

#include "handlers.h"
#include "client.h"
#include "ircd_defs.h"
#include "s_misc.h"
#include "send.h"
#include "ircd.h"
#include "hash.h"
#include "numeric.h"
#include "ircd_reply.h"
#include "random.h"
#include "ircd_features.h"
#include "ircd_snprintf.h"
#include "msg.h"
#include "whowas.h"
#include "ircd_alloc.h"
#include "s_user.h"
#include "s_debug.h"
#include "sys.h"

#include <string.h>
#include <stdio.h>

/** 
	Solo se permite el rename a nombre generico, no se permite 
	modificaciones a nicks especificos ya que puede forzarse el
	cambio a un nick registro lo que puede ocasionar problemas 
	de seguridad en la autenticacion de nicks.
*/

#define PREFIX_NICK "user"

int mo_rename(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{

    struct Client *acptr;

    if (parc < 2)
    {
        need_more_params(cptr, "RENAME");
        return 0;
    }

    char *nick = parv[1];

    if (!(acptr = FindUser(nick)))
    {
        send_reply(cptr, ERR_NOSUCHNICK, nick);
        return 0;
    }

    /** No puede usar rename sobre si mismo **/
    if (cptr != acptr)
    {
        int len = IRCD_MIN(NICKLEN, feature_int(FEAT_NICKLEN));

        char *oldnick = (char *)MyMalloc(len);

        char *newnick = (char *)MyMalloc(len);

        if (oldnick && newnick)
        {
            strcpy(oldnick, cli_name(acptr));

            ircd_snprintf(0, newnick, len, "%s%u", PREFIX_NICK, ircrandom());

            sendcmdto_common_channels_butone(acptr, CMD_NICK, NULL, ":%s", newnick);
            add_history(acptr, 1);
            sendcmdto_serv_butone(acptr, CMD_NICK, NULL, "%s %Tu", newnick,
                                  cli_lastnick(acptr));

            hRemClient(acptr);
            strcpy(cli_name(acptr), newnick);
            hAddClient(acptr);

            sendto_opmask_butone(0, SNO_OLDSNO, "\002%s\002 (%s@%s) used RENAME on \002%s\002 (%s@%s) lastnick: \002%s\002",
                                 cli_name(cptr), cli_user(cptr)->username, cli_user(cptr)->host, cli_name(acptr),
                                 cli_user(acptr)->username, cli_user(acptr)->host, oldnick);
            MyFree(oldnick);
            MyFree(newnick);
        }
    }

    return 0;
}
