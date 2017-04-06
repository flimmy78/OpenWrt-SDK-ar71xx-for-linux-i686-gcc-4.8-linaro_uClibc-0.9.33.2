//将字符转化为十进制
#include<stdio.h>
#include<string.h>
int charToHexTCP(char c)
{
    if((c >= '0') && (c <= '9'))
        return c - '0';
    else if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    else if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    else
        return 0x10;
}

int strToHexTCP(char* sour, unsigned char* dest)
{
    int t, t1;
    int rlen = 0;
    int len = strlen(sour);
    int i = 0;
    for (i = 0;i < len;)
    {
        char l,h = sour[i];
        if (h == ' ')
        {
            i++;
            continue;
        }
        i++;
        if (i >= len)
        {
            break;
        }
        l = sour[i];
        t = charToHexTCP(h);
        t1 = charToHexTCP(l);
        if ((t == 16) || (t1 == 16))
            break;
        else
            t = t * 16 + t1;
        i++;
        dest[rlen] = (char)t;
        rlen++;
    }
    return rlen;

}
/****
int main()
{
	int i;
	char sour[1024];
	unsigned char dest[1024];
	printf("please input string:");
	scanf("%s",sour);
	strToHexTCP(sour, dest);
	printf("output hex:");
	for(i=0; i<strlen(sour)/2; ++i)
	{
		printf("%02x ",dest[i]);
	
	}
	printf("\n");
	return 0;
}

******/




