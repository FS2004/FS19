/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
 * Copyright(C) 2019 ****, Inc.
 *
 *  File Name: xgit.cpp
 *  Created  : 19/11/23
 *
 *  Function
 *    Scan folder
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *  $Revision: 1.1.1.1 $
 *  $Date: 2008/07/27 10:32:48 $
 *  $Author: poko $
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
 */

/****************************************************************************
 * Included files
 ****************************************************************************/

#include <stdio.h>
#include <TCHAR.h>
#include <sys/stat.h>
#include <shlwapi.h>
#include "xgit.h"

/* Librarys */

#pragma comment(lib, "shlwapi.lib")

/****************************************************************************
 * Public type declarations
 ****************************************************************************/

/****************************************************************************
 * Public function prototypes
 ****************************************************************************/

/****************************************************************************
 * Public data
 ****************************************************************************/

/****************************************************************************
 * Pre-processor definitions
 ****************************************************************************/

#define _STAT   struct _stat

/****************************************************************************
 * Private types
 ****************************************************************************/

typedef struct
{
  TCHAR     path[MAX_PATH];
  FILETIME  ctime;
  FILETIME  atime;
  FILETIME  wtime;
}
LAST_IDX;

/****************************************************************************
 * Private data
 ****************************************************************************/

LAST_IDX  _last_idx = {0};

/****************************************************************************
 * Private functions
 ****************************************************************************/

static BYTE get_byte( const BYTE** pp_data )
{
  BYTE        data;
  const BYTE* bp = *pp_data;

  data = *bp;

  bp++;

  *pp_data = bp;

  return data;
}

/* ------------------------------------------------------------------------ */
static WORD get_word( const BYTE** pp_data )
{
  WORD  data = 0;

  data |= get_byte(pp_data);
  data <<= 8;
  data |= get_byte(pp_data);

  return data;
}

/* ------------------------------------------------------------------------ */
static DWORD get_dword( const BYTE** pp_data )
{
  DWORD data = 0;

  data |= get_word( pp_data );
  data <<= 16;
  data |= get_word( pp_data );

  return data;
}

/* ------------------------------------------------------------------------ */
static const BYTE *Header( const BYTE *bp, int *total = NULL, DWORD *version = NULL )
{
  const BYTE *ret = NULL;
  DWORD       _version;
  DWORD       _total;

  if ( get_dword( &bp ) == 0x44495243 )   /* 0x43524944 */
    {
      _version = get_dword( &bp );
      _total   = get_dword( &bp );

      if ( version )
        {
          *version = _version;
        }

      if ( total )
        {
          *total = _total;
        }

      ret = bp;
    }

  return ret;
}

/* ------------------------------------------------------------------------ */
static const BYTE *Body( const BYTE *bp, const TCHAR *root, GIT_NODE *g_idx )
{
  WORD         len;
  int          i;
  CHAR         cc[ MAX_PATH ];
  const BYTE  *top = bp;

  /* ctime  */

  g_idx->ctime_hi = get_dword( &bp );
  g_idx->ctime_lo = get_dword( &bp );

  /* mtime  */

  g_idx->mtime_hi = get_dword( &bp );
  g_idx->mtime_lo = get_dword( &bp );

  /* dev */

  g_idx->dev = get_dword( &bp );

  /* inode */

  g_idx->inode = get_dword( &bp );

  /* mode */

  g_idx->mode = get_dword( &bp );

  /* uid */

  g_idx->uid = get_dword( &bp );

  /* guid */

  g_idx->guid = get_dword( &bp );

  /* sha1 size */

  get_dword( &bp );

  /* sha1 */

  for ( i = 0; i < 20; i++ )
    {
      g_idx->sha1[i] = get_byte( &bp );
    }

  /* path length */

  len = get_word( &bp );

  /* path */

  for ( i = 0; i < len; i++ )
    {
      cc[ i ] = get_byte( &bp );
    }

  cc[ i ] = '\0';

  /* path terminate */

  get_byte( &bp );

  /* path conv */

  _tcscpy_s(g_idx->path, MAX_PATH, root);

  _tcscat(g_idx->path, L"/");

  ::MultiByteToWideChar( CP_UTF8, 0U,
                         cc,
                         -1,
                         g_idx->path + _tcsclen(g_idx->path),
                         MAX_PATH );

  /* next point */

  bp = ( BYTE* )( top + ( ( bp - top + 7 ) & ~7 ) );

  return bp;
}

/* ------------------------------------------------------------------------ */
static void debug_git_index( GIT_NODE *g_idx )
{
  /* ctime  */

  _tprintf( L"ctime = %08X\n", g_idx->ctime_hi );
  _tprintf( L"ctime = %08X\n", g_idx->ctime_lo );

  /* mtime  */

  _tprintf( L"mtime = %08X\n", g_idx->mtime_hi );
  _tprintf( L"mtime = %08X\n", g_idx->mtime_lo );

  /* dev */

  _tprintf( L"%08X\n", g_idx->dev );

  /* inode */

  _tprintf( L"%08X\n", g_idx->inode );

  /* mode */

  _tprintf( L"mode = %08X\n", g_idx->mode );

  /* uid */

  _tprintf( L"%08X\n", g_idx->uid );

  /* guid */

  _tprintf( L"%08X\n", g_idx->guid );

  /* sha1 */

  _tprintf( L"sha1:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
    g_idx->sha1[0],
    g_idx->sha1[1],
    g_idx->sha1[2],
    g_idx->sha1[3],
    g_idx->sha1[4],
    g_idx->sha1[5],
    g_idx->sha1[6],
    g_idx->sha1[7],
    g_idx->sha1[8],
    g_idx->sha1[9],
    g_idx->sha1[10],
    g_idx->sha1[11],
    g_idx->sha1[12],
    g_idx->sha1[13],
    g_idx->sha1[14],
    g_idx->sha1[15],
    g_idx->sha1[16],
    g_idx->sha1[17],
    g_idx->sha1[18],
    g_idx->sha1[19]);

  _tprintf(L"[%s]\n", g_idx->path);
}

/* ------------------------------------------------------------------------ */
static HANDLE open_git_index(const TCHAR *input, TCHAR *git_path, size_t size)
{
  TCHAR   path[MAX_PATH];
  HANDLE  hFile = INVALID_HANDLE_VALUE;

  _tcscpy_s(git_path, size, input);

  do
    {
      _tcscpy_s(path, MAX_PATH, git_path);
      PathAppend(path, L".git\\index");

      hFile = CreateFile( path,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

      if ( hFile == INVALID_HANDLE_VALUE )
        {
          continue;
        }

      /* Check for file with the same name */

      LAST_IDX  t = {0};

      GetFileTime(hFile, &t.ctime, &t.ctime, &t.ctime);

      _tcscpy_s(t.path, MAX_PATH, path);

      if (memcmp(&t, &_last_idx, sizeof(t)) == 0)
        {
          CloseHandle(hFile);
          hFile = INVALID_HANDLE_VALUE;
        }

      _last_idx = t;

      break;
    }
  while (PathRemoveFileSpec(git_path));

  return hFile;
}

/* ------------------------------------------------------------------------ */
static int scan_git_index(const BYTE *bp, const TCHAR *git_path, git_node_callback cb, void *p_param)
{
  int       total = 0;
  DWORD     version;
  GIT_NODE  g_idx;

  bp = Header(bp, &total, &version);

  if (!bp)
    {
      return 0;
    }

  for (int i = 0; i < total; i++)
    {
      bp = Body(bp, git_path, &g_idx);

      /* Add file info */

      if (cb)
        {
          cb(&g_idx, p_param);
        }
    }

  return total;
}

/****************************************************************************
 * Public functions
 ****************************************************************************/

int scan_git_dir( const TCHAR dir[], git_node_callback cb, void *p_param)
{
  HANDLE  hFile;
  TCHAR   path[ MAX_PATH ];
  DWORD   size_lo;
  DWORD   size_hi;
  DWORD   size_ot;
  BYTE   *bp;
  int     total = 0;

  hFile = open_git_index(dir, path, MAX_PATH);

  if (hFile == INVALID_HANDLE_VALUE)
    {
      return 0;
    }

  /* .git/index reading analysis */

  size_lo = GetFileSize(hFile, &size_hi);

  if (size_lo == INVALID_FILE_SIZE)
    {
      /* File size acquisition failure! */
    }
  else if (size_hi > 0)
    {
      /* File size is too large! */
    }
  else if((bp = (BYTE *)malloc(size_lo)) == NULL)
    {
      /* Memory allocate error! */
    }
  else
    {
      if (ReadFile(hFile, bp, size_lo, &size_ot, NULL))
        {
          /* Analysis */

          total = scan_git_index(bp, path, cb, p_param);
        }

      free(bp);
    }

  CloseHandle(hFile);

  return total;
}
