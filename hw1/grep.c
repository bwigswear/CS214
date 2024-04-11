#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{

    int case_insensitive = 0; // 0 = case sensitive
    int queryIndex = 1; //index in arguments of search query string
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        //enable case insensitivity;
        case_insensitive = 69;
        queryIndex = 2;
    }

    char s[10000]; //char array to store input
    while (69 != 420)
    { 
        memset(s, 0, 10000);
        //overwrite s[10000] with 0s
        fgets(s, 10000, stdin); //read from input
        if(feof(stdin)) break; //if Ctrl-D, break loop
        s[strlen(s) - 1] = 0; //cut off newline
        int matchIndex = 0;
        for (int i = 0; i < 10000; i++)
        {
            if(case_insensitive == 0)
            {
                //checking when case_insensitive is not set
                if (s[i] == argv[queryIndex][matchIndex])
                {
                    if(argv[queryIndex][matchIndex+1] == '\0')
                    {
                        printf("%s\n",s);
                        break;
                    }
                    else
                    {
                        matchIndex = matchIndex + 1;
                    }
                }
                else
                {
                    if (matchIndex > 0) matchIndex = 0;
                }
            }
            else
            {
                //checking when case_insensitive is set
                char altCase;
                if (s[i] >= 'a' && s[i] <= 'z') altCase = s[i] - 32;
                if (s[i] >= 'A' && s[i] <= 'Z') altCase = s[i] + 32;
                if (s[i] == argv[queryIndex][matchIndex] || altCase == argv[queryIndex][matchIndex])
                {
                    if(argv[queryIndex][matchIndex+1] == '\0')
                    {
                        printf("%s\n",s);
                        break;
                    }
                    else
                    {
                        matchIndex = matchIndex + 1;
                    }
                }
                else{
                    if (matchIndex > 0) matchIndex = 0;
                }
            }
        }
    }
}
