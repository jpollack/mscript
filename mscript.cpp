#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "matrix.h"

Engine *ep;

int runFile (char *fname, int argc, char **argv);
char *stripRChars (char *str, const char *chars);
char *stripLChars (char *str, const char *chars);

inline char *chomp (char* str) { return stripRChars (str, "\n\r"); }
inline char *stripChars (char *str, const char *chars)
{
    return (stripLChars (stripRChars (str, chars), chars));
}
inline char *stripWhitespace (char *str) { return stripChars (str, " \t\r\n"); }

int main (int argc, char *argv[])
{
    fclose (stderr);
    ep = engOpen ("matlab -nosplash");
    if (!ep)
    {
        fprintf (stderr, "engOpen failed.\n");
        exit (1);
    }

    runFile (argv[1], argc - 1, argv + 1);

    engClose (ep);
    return 0;
}

int runFile (char *fname, int argc, char **argv)
{
    FILE* fp = NULL;
    char lineBuf[4096];
    char evalBuf[4096];

    fp = fopen (fname, "r");
    if (!fp)
    {
        return 1;
    }

    mxArray *mxArgc = mxCreateNumericMatrix (1, 1, mxINT32_CLASS, mxREAL);
    (static_cast<int *> (mxGetData (mxArgc)))[0] = argc;
    engPutVariable (ep, "argc", mxArgc);

    mxArray *mxArgv = mxCreateCellMatrix (1, argc);
    for (int idx = 0; idx < argc; idx++)
    {
        mxSetCell (mxArgv, idx, mxCreateString (argv[idx]));
    }
    engPutVariable (ep, "argv", mxArgv);
    evalBuf[0] = 0;

    char engBuf[65536];
    engOutputBuffer (ep, engBuf, 65536);

    for (int lineNo = 1; fgets (lineBuf, 4096, fp); lineNo++)
    {
        stripWhitespace (lineBuf);

        if ((lineNo == 1) 
            && (lineBuf[0] == '#')
            && (lineBuf[1] == '!'))
        {
            continue;
        }

        if (!lineBuf[0] || (lineBuf[0] == '%'))
        {
            continue;
        }

        int lineLen = strlen (lineBuf);
        if ((lineLen > 3)
            && (lineBuf[lineLen - 1] == '.')
            && (lineBuf[lineLen - 2] == '.')
            && (lineBuf[lineLen - 3] == '.'))
        {
            lineBuf[lineLen - 3] = 0;
            strncat (evalBuf, lineBuf, 4096);
        }
        else
        {
            strncat (evalBuf, lineBuf, 4096);
            if (engEvalString (ep, evalBuf))
            {
                return 3;
            }
            evalBuf[0] = 0;
            if (engBuf[0])
            {
                printf ("%s", engBuf);
            }
        }

    }

    mxDestroyArray (mxArgc);
    mxDestroyArray (mxArgv);

    if (fclose (fp))
    {
        return 2;
    }

    return 0;
}


// Remove all characters from the RHS.
char *stripRChars (char *str, const char *chars)
{
    int pos = strlen (str);
    int nChars = strlen (chars);

    while (pos--)
    {
        int idx = nChars;
        while (idx && (chars[--idx] != str[pos]));
        if (chars[idx] == str[pos])
        {
            str[pos] = 0;
        }
        else
        {
            pos = 0;
        }
    }

    return str;
}

// Remove all characters from the LHS.
char *stripLChars (char *str, const char *chars)
{
    int sLen = strlen (str);
    int nChars = strlen (chars);
    int pos;
    for (pos = 0; (pos < sLen); pos++)
    {
        int idx = nChars;
        while (idx && (chars[--idx] != str[pos]));
        if (chars[idx] == str[pos])
        {
            continue;
        }
        else
        {
            break;
        }
    }

    // pos is the first non-matching character
    int ssLen = strlen (str + pos);
    if (ssLen && (ssLen != sLen))
    {
        memmove (str, str + pos, (ssLen + 1));
    }

    return str;

}
