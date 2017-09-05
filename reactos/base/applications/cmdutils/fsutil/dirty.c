/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS FS utility tool
 * FILE:            base/applications/cmdutils/dirty.c
 * PURPOSE:         FSutil dirty bit handling
 * PROGRAMMERS:     Pierre Schweitzer <pierre@reactos.org>
 */

#include "fsutil.h"
#include <winioctl.h>

/* Add handlers here for subcommands */
static int QueryMain(int argc, const TCHAR *argv[]);
static int SetMain(int argc, const TCHAR *argv[]);
static HandlerItem HandlersList[] =
{
    /* Proc, name, help */
    { QueryMain, _T("query"), _T("Show the dirty bit") },
    { SetMain, _T("set"), _T("Set the dirty bit") },
};

static int
QueryMain(int argc, const TCHAR *argv[])
{
    HANDLE Volume;
    TCHAR VolumeID[PATH_MAX];
    ULONG VolumeStatus, BytesRead;

    /* We need a volume (letter or GUID) */
    if (argc < 2)
    {
        _ftprintf(stderr, _T("Usage: fsutil dirty query <volume>\n"));
        _ftprintf(stderr, _T("\tFor example: fsutil dirty query c:\n"));
        return 1;
    }

    /* Create full name */
    _stprintf(VolumeID, _T("\\\\.\\%s"), argv[1]);

    /* Open the volume */
    Volume = CreateFile(VolumeID, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (Volume == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Error: %d\n"), GetLastError());
        return 1;
    }

    /* And query the dirty status */
    if (DeviceIoControl(Volume, FSCTL_IS_VOLUME_DIRTY, NULL, 0, &VolumeStatus,
                        sizeof(ULONG), &BytesRead, NULL) == FALSE)
    {
        _ftprintf(stderr, _T("Error: %d\n"), GetLastError());
        CloseHandle(Volume);
        return 1;
    }

    CloseHandle(Volume);

    /* Print the status */
    _ftprintf(stdout, _T("The %s volume is %s\n"), argv[1], (VolumeStatus & VOLUME_IS_DIRTY ? _T("dirty") : _T("clean")));

    return 1;
}

static int
SetMain(int argc, const TCHAR *argv[])
{
    /* FIXME */
    _ftprintf(stderr, _T("Not implemented\n"));
    return 1;
}

static void
PrintUsage(const TCHAR * Command)
{
    int i;

    /* If we were given a command, print it's not supported */
    if (Command != NULL)
    {
        _ftprintf(stderr, _T("Unhandled DIRTY command: %s\n"), Command);
    }

    /* And dump any available command */
    _ftprintf(stderr, _T("---- Handled DIRTY commands ----\n\n"));
    for (i = 0; i < (sizeof(HandlersList) / sizeof(HandlersList[0])); ++i)
    {
        _ftprintf(stderr, _T("%s\t%s\n"), HandlersList[i].Command, HandlersList[i].Desc);
    }
}

int
DirtyMain(int argc, const TCHAR *argv[])
{
    int i;
    int ret;
    const TCHAR * Command;

    ret = 1;
    Command = NULL;
    i = (sizeof(HandlersList) / sizeof(HandlersList[0]));

    /* If we have a command, does it match a known one? */
    if (argc > 1)
    {
        /* Browse all the known commands finding the right one */
        Command = argv[1];
        for (i = 0; i < (sizeof(HandlersList) / sizeof(HandlersList[0])); ++i)
        {
            if (_tcsicmp(Command, HandlersList[i].Command) == 0)
            {
                ret = HandlersList[i].Handler(argc - 1, &argv[1]);
                break;
            }
        }
    }

    /* We failed finding someone to handle the caller's needs, print out */
    if (i == (sizeof(HandlersList) / sizeof(HandlersList[0])))
    {
        PrintUsage(Command);
    }

    return ret;
}
