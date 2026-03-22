/*
 cheie: HKLM\SYSTEM\CurrentControlSet\Services
*/

#include <windows.h>
#include <stdio.h>

#define REG_PATH    "SYSTEM\\CurrentControlSet\\Services"
#define MAX_KEY_LEN  256

int main(void)
{
    HKEY    hKey = NULL;
    LONG    lResult = ERROR_SUCCESS;
    DWORD   dwIndex = 0;
    char    szSubKey[MAX_KEY_LEN];
    DWORD   dwSubKeyLen = 0;

    lResult = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        REG_PATH,
        0,
        KEY_READ,
        &hKey);

    if (lResult != ERROR_SUCCESS)
    {
        fprintf(stderr, " eroare deschidere cheie cod Win32: %ld\n", lResult);
        return 1;
    }

    printf("Subcheile din HKLM\\%s :\n", REG_PATH);
    printf("================================================\n");

    while (1)
    {
        dwSubKeyLen = MAX_KEY_LEN;

        lResult = RegEnumKeyExA(
            hKey,
            dwIndex,
            szSubKey,
            &dwSubKeyLen,
            NULL,   // rezervat
            NULL,   //clasa
            NULL,   // dim clasa 
            NULL);  // ultima mod

        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

        if (lResult != ERROR_SUCCESS)
        {
            fprintf(stderr, " RegEnumKeyExA a esuat la index %lu. Cod: %ld\n", dwIndex, lResult);
            break;
        }

        printf("  [%4lu]  %s\n", dwIndex, szSubKey);
        dwIndex++;
    }

    printf("================================================\n");
    printf("Total subchei gasite: %lu\n", dwIndex);

    RegCloseKey(hKey);

    return 0;
}