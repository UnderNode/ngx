#include "handlers.h"
#include "s_debug.h"

int mo_dbq(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
    Debug((DEBUG_INFO, "dbq params count %d", parc));
    return 0;
}
int ms_dbq(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
    Debug((DEBUG_INFO, "dbq params count %d", parc));
    return 0;
}