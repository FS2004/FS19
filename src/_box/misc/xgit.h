/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
 * Copyright(C) 2019 ****, Inc.
 *
 *  File Name: xgit.h
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
 * Public type declarations
 ****************************************************************************/

typedef struct
{
  DWORD   ctime_hi;
  DWORD   ctime_lo;
  DWORD   mtime_hi;
  DWORD   mtime_lo;
  DWORD   dev;
  DWORD   inode;
  DWORD   mode;
  DWORD   uid;
  DWORD   guid;
  DWORD   size;
  BYTE    sha1[20];
  TCHAR   path[MAX_PATH];
}
GIT_NODE;

typedef void (*git_node_callback)(GIT_NODE *p_node, void *p_param);

/****************************************************************************
 * Public function prototypes
 ****************************************************************************/

int scan_git_dir( const TCHAR dir[], git_node_callback cb, void *p_param);
