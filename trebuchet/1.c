#include <windows.h>
#include <stdio.h>

LPVOID crealloc(LPVOID lpSource, DWORD dwSourceNumber, DWORD dwNewNumber, size_t size)
{
    if(lpSource == NULL) return NULL;
    if((dwSourceNumber | dwNewNumber) == 0) return NULL;
    if(size == 0) return NULL;
    LPVOID newbuf = calloc(dwNewNumber, size);
    if(newbuf == NULL) return NULL;
    if(memcpy_s(newbuf, (dwNewNumber * size), lpSource, (dwSourceNumber * size)) != 0) return NULL;
    free(lpSource);
    return newbuf;
}

CHAR *readLine(HANDLE hFile, BOOL *endOfFile)
{
    CHAR charRead = '\0';
    DWORD stringBufLen = 100;
    DWORD bytesRead = 0;
    DWORD strLen = 0;
    DWORD watchdog = 0;
    *endOfFile = FALSE;
    CHAR * stringBuf = (CHAR *)calloc(stringBufLen, sizeof(CHAR));
    char * stringBufTail = (char *)stringBuf;
    if (stringBuf == NULL) return NULL;
    while(++watchdog < ULONG_MAX)
    {
        strLen += ReadFile(hFile, &charRead, 1, &bytesRead, NULL);
        if(charRead == '\r')
        {
            strLen--;
            continue;
        }
        if(charRead == '\n') break;
        if(bytesRead == 0 || strLen == 0 || charRead == '\0') 
        {
            free(stringBuf);
            *endOfFile = TRUE;
            break;
        }
        if(strLen >= stringBufLen - 1)
        {
            DWORD offset = (DWORD)(((DWORD)stringBufTail - (DWORD)stringBuf)/(sizeof(CHAR)));
            stringBufLen *= 2;
            stringBuf = (CHAR *)crealloc(stringBuf, strLen, stringBufLen, sizeof(CHAR));
            stringBufTail = stringBuf;
            stringBufTail += offset;
        }
        *stringBufTail = charRead;
        stringBufTail++;
    }
    if(*stringBufTail != '\0') *stringBufTail = '\0';
    return stringBuf;
}

LPCSTR *readFileIntoBuffer(HANDLE hFile, DWORD *items)
{
    BOOL endOfFile = FALSE;
    DWORD watchdog = 0;
    DWORD bufSize = 100;
    DWORD index = 0;
    CHAR **stringListBuf = (CHAR **)calloc(bufSize, sizeof(CHAR *));
    CHAR **stringListBufTail = stringListBuf;
    while(++watchdog < ULONG_MAX)
    {
        CHAR *lineRead = readLine(hFile, &endOfFile);
        if(endOfFile == TRUE || lineRead == NULL) break;
        index++;
        if(index >= bufSize)
        {
            DWORD offset = (DWORD)(((DWORD)stringListBufTail - (DWORD)stringListBuf)/(sizeof(CHAR *)));
            bufSize *= 2;
            stringListBuf = (CHAR **)crealloc(stringListBuf, index, bufSize, sizeof(CHAR *));
            stringListBufTail = stringListBuf;
            stringListBufTail += offset;
        }
        *stringListBufTail = lineRead;
        stringListBufTail++;
    }
    *items = index;
    return stringListBuf;
}

BOOL isAsciiNum(WORD wNum)
{
    if(wNum >= (WORD)'0' && wNum <= (WORD)'9') return TRUE;
    return FALSE;
}

WORD twoDigitNumbersInStr(LPCSTR str)
{
    if(str[0] == '\0') return 0;
    BOOL foundFirst = FALSE;
    BOOL foundLast = FALSE;
    WORD first = 0;
    WORD last = 0;
    size_t sLen = strlen(str);
    LPCSTR front = str;
    LPCSTR back = str + (sLen - 1);
    for(int i = 0; i < sLen; i++)
    {
        WORD frontAscii = (WORD)(*front);
        WORD backAscii = (WORD)(*back);
        if(!foundFirst)
        {
            if(isAsciiNum(frontAscii))
            {
                foundFirst = TRUE;
                first = frontAscii - (WORD)'0';
            }
        }
        if(!foundLast)
        {
            if(isAsciiNum(backAscii))
            {
                foundLast = TRUE;
                last = backAscii - (WORD)'0';
            }
        }
        if(foundFirst && foundLast) break;
        front++;
        back--;
    }
    return (first * 10) + last;
}

int main(int argc, char **argv)
{
    DWORD sum = 0;
    DWORD inItems = 0;
    HANDLE hFile = CreateFile(
        "input.txt",
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) return -1;
    LPCSTR *fileBuf = readFileIntoBuffer(hFile, &inItems);
    CloseHandle(hFile);
    if(fileBuf == NULL) return -1;
    for(int i = 0; i < inItems; i++)
    {
        sum += twoDigitNumbersInStr(fileBuf[i]);
    }
    printf("Sum total is: %d\n", sum);
    return 0;
}