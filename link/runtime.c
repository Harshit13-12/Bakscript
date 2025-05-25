#include <windows.h>

// convert long long to string
void int_to_string(long long value, char *buffer)
{
    char temp[21];
    int i = 0, j = 0;
    int negative = 0;

    if (value < 0)
    {
        negative = 1;
        value = -value;
    }

    do
    {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value);

    if (negative)
        temp[i++] = '-';
    while (i-- > 0)
        buffer[j++] = temp[i];

    buffer[j] = '\0';
}

void show_num(long long value)
{
    char buf[64];
    int_to_string(value, buf);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    WriteConsoleA(hConsole, buf, lstrlenA(buf), &written, NULL);
    WriteConsoleA(hConsole, "\n", 1, &written, NULL);
}

void show_str(const char *str)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    if (!str)
    {
        const char *nullMsg = "null\n";
        WriteConsoleA(hConsole, nullMsg, lstrlenA(nullMsg), &written, NULL);
        return;
    }

    WriteConsoleA(hConsole, str, lstrlenA(str), &written, NULL);
    WriteConsoleA(hConsole, "\n", 1, &written, NULL);
}

void process_exit(int exit_code)
{
    ExitProcess((UINT)exit_code);
}
