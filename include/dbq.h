typedef struct
{
    char *method;
    int (*func)(void *, int, char *[]);
} Methods;

extern Methods *find_method(char *);
/**
 * DBQ GET N <nick>
 * DBQ GET C <channel>
 * */
extern int parse_get(void *, int, char *[]);
/**
 * DBQ POST N <nick> <password> [vhost]
 * DBQ POST C <channel> <owner>
 * */
extern int parse_post(void *, int, char *[]);
