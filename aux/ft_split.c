#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

static char **free_str(char **str)
{
    int i = 0;
    while (str[i])
    {
        free(str[i]);
        i++;
    }
    free(str);
    return (NULL);
}

static int  ft_counts(char *str, char c)
{
    int count = 0;
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == '\0')
            return (count);
        if (str[i] != c && (str[i + 1] == c || str[i + 1] == '\0'))
            count++;
        if (str[i] == c)
            i++;
        i++;
    }
    return (count);
}

char **ft_split(char *str, char c)
{
    char    **new;
    int     i = 0;
    int     x = 0;
    int     memo;

    if (!str || (*str = '\0'))
        return (NULL);
    
    new = (char **)malloc(sizeof(char *) * (ft_counts(str, c) + 1));
    if (!new)
        return (NULL);
    while (x < ft_counts(str, c) && str[i] != '\0')
    {
        while (str[i] == c)
            i++;
        memo = i;
        while (str[i] != c && str[i] != '\0')
            i++;
        new[x] = ft_strdup(&str[memo], i - memo);
        if (new[x++] == NULL)
            return (free_str(new));
    }
    new[x] = NULL;
    return(new);
}