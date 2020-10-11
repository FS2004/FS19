#include <windows.h>
#include <Wincrypt.h>
#include <stdio.h>

DWORD GetSHA1(TCHAR *in_file, BYTE *sha, DWORD sha_len)
{
  HCRYPTPROV  hProv   = 0;
  HCRYPTHASH  hHash   = 0;
  DWORD       hashLen = 0;
  int         res     = -1;

  if (CryptAcquireContext(&hProv,
                          0,
                          0,
                          PROV_RSA_AES,
                          CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))
    {
      if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
        {
          HANDLE  hFile = CreateFile(in_file,
                                     GENERIC_READ,
                                     0,
                                     0,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);

          if (hFile != INVALID_HANDLE_VALUE)
            {
              DWORD size;
              BYTE *bp;
              DWORD head;

              size = GetFileSize(hFile, NULL);

              bp = (BYTE *)malloc(size + 1024);

              if (bp)
                {
                  sprintf((char *)bp, "blob %d", size);

                  head = strlen((char *)bp) + 1;

                  ReadFile(hFile, bp + head, size, NULL, 0);

                  CryptHashData(hHash, bp, size + head, 0);

                  free(bp);
                }

              CloseHandle(hFile);
            }

          CryptGetHashParam(hHash, HP_HASHVAL, 0, &hashLen, 0);

          if (0 < hashLen && hashLen <= sha_len)
            {
              CryptGetHashParam(hHash, HP_HASHVAL, sha, &hashLen, 0);
            }
          else
            {
              hashLen = 0;
            }

          CryptDestroyHash(hHash);
        }

      CryptReleaseContext(hProv, 0);
    }

  return hashLen;
}
