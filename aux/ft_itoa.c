#include "../incl/ft_strace.h"
#include "../lib/printf/ft_printf.h"

static int ft_getnumber(int number)
{
    long count = 0;
    long n = number;

    if (n == 0)
        return (1);
    if (n < 0)
    {
        n = -n;
        count++;
    }
    while (n > 0)
    {
        n = n / 10;
        count++;
    }
    return ((int)count);
}

char *ft_itoa(int number)
{
    char    *str;
    int     len;
    long    n;

    len = ft_getnumber(number);
    str = (char *)malloc(len + 1);
    if (!str)
        return (NULL);

    str[len] = '\0';
    if (number == 0)
    {
        str[0] = '0';
        return (str);
    }
    else if (number < 0)
    {
        str[0] = '-';
        n = -(long)number;
    }
    else
        n = number;
    while (n > 0)
    {
        len--;
        str[len] = '0' + (n % 10);
        n = n / 10;
    }
    return (str);
}