#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

char    *ft_strjoin(char *str1, char *str2)
{
    char    *new;
    size_t  len;
    size_t  i = 0;
    size_t  j = 0;

    len = ft_strlen(str1) + ft_strlen(str2);
    new = malloc(len + 1);
    if (!new)
        return (NULL);
    while (str1[i] != '\0')
    {
        new[i] = str1[i];
        i++;
    }
    while (str2[j])
    {
        new[i] = str2[j];
        i++;
        j++;
    }
    new[i] = '\0';
    return (new);
}

size_t  ft_strlen(char *str)
{
    size_t  len;

    len = 0;
    while (str[len] != '\0')
        len++;
    return (len);
}

int	ft_strncmp(char *src, char *str, int numb)
{
	int	i = 0;
	while ((src[i] != '\0' || str[i] != '\0') && i < numb)
	{
		if (src[i] != str[i])
			return ((const unsigned char)src[i] - (const unsigned char)str[i]);
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
