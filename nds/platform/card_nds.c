#include <string.h>
#include <psyq/libapi.h>

static int s_cardStarted;

void InitCARD(int value)
{
    (void)value;
    s_cardStarted = 0;
}

int StartCARD(void)
{
    s_cardStarted = 1;
    return 1;
}

int StopCARD(void)
{
    s_cardStarted = 0;
    return 1;
}

int _card_info(int channel)
{
    (void)channel;
    return s_cardStarted ? 1 : 0;
}

int _card_clear(int channel)
{
    (void)channel;
    return s_cardStarted ? 1 : 0;
}

int _card_load(int channel)
{
    (void)channel;
    return s_cardStarted ? 1 : 0;
}

long format(char* path)
{
    (void)path;
    return 1;
}

long erase(char* path)
{
    (void)path;
    return 1;
}

struct DIRENTRY* firstfile(char* path, struct DIRENTRY* entry)
{
    (void)path;
    if (entry != NULL)
    {
        memset(entry, 0, sizeof(*entry));
    }
    return NULL;
}

struct DIRENTRY* nextfile(struct DIRENTRY* entry)
{
    (void)entry;
    return NULL;
}
