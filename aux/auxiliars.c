#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

char    *get_binary(char **str, char *find)
{
    char    *aux;
    char    *command;

    return (command);
}

size_t  len(char *str)
{
    size_t  len;

    len = 0;
    while (str[len] != '\0')
        len++;
    return (len);
}

int ft_strncmp(const char *s1, const char *s2, size_t len)
{
    size_t i = 0;

    while ((s1[i] != '\0' || s2[i] != '\0') && i < len)
    {
        if (s1[i] != s2[i])
            return ((char)s1[i] - (char)s2[i]);
        i++;
    }
    return (0);   
}

char    *ft_strdup(char *str, int len)
{
    char *new;
    int i;

    i = 0;
    if (len == 0)
        return (NULL);
    new = (char *)malloc(len + 1);
    if (!new)
        return (NULL);
    while (i < len)
    {
        new[i] = str[i];
        i++;
    }
    new[i] = '\0';
    return (new);
}
