/*
 * getopt_long() -- long options parser
 *
 * Portions Copyright (c) 1987, 1993, 1994
 * The Regents of the University of California.  All rights reserved.
 *
 * Portions Copyright (c) 2003
 * PostgreSQL Global Development Group
 *
 * Simple conversion to C++ by Andrew Tridgell for ArduPilot. Based on
 * getopt_long.cpp from ccache
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <AP_HAL/AP_HAL.h>
#if HAL_OS_POSIX_IO

#include "getopt_cpp.h"
#include <stdio.h>
#include <string.h>

#define GETOPT_ERROR(...) fprintf(stderr, __VA_ARGS__)


/*
  constructor
 */
GetOptLong::GetOptLong(int _argc, char *const _argv[], const char *_optstring, const GetOptLong::option * _longopts) :
    opterr(0),
    optind(1),
    optopt(0),
    longindex(-1),
    optarg(nullptr),
    argc(_argc),
    argv(_argv),
    optstring(_optstring),
    longopts(_longopts),
    place("")
{}

/*
  main parse code
 */
int GetOptLong::getoption(void)
{
    const char        *oli;          /* option letter list index */

    if (!*place)
    { /* update scanning pointer */
        if (optind >= argc)
        {
            place = "";
            return -1;
        }

        place = argv[optind];

        if (place[0] != '-')
        {
            place = "";
            return -1;
        }
        
        place++;
        
        if (place[0] && place[0] == '-' && place[1] == '\0')
        { /* found "--" */
            ++optind;
            place = "";
            return -1;
        }

        if (place[0] && place[0] == '-' && place[1])
        {
            /* long option */
            size_t namelen;
            int    i;
            
            place++;
            
            namelen = strcspn(place, "=");
            for (i = 0; longopts[i].name != nullptr; i++)
            {
                if (strlen(longopts[i].name) == namelen
                    && strncmp(place, longopts[i].name, namelen) == 0)
                {
                    if (longopts[i].has_arg)
                    {
                        if (place[namelen] == '=')
                            optarg = place + namelen + 1;
                        else if (optind < argc - 1)
                        {
                            optind++;
                            optarg = argv[optind];
                        }
                        else
                        {
                            if (optstring[0] == ':')
                                return BADARG;
                            if (opterr) {
                                GETOPT_ERROR("%s: option requires an argument -- %s\n",
                                             argv[0], place);
                            }
                            place = "";
                            optind++;
                            return BADCH;
                        }
                    }
                    else
                    {
                        optarg = nullptr;
                        if (place[namelen] != 0)
                        {
                            /* XXX error? */
                        }
                    }
                    
                    optind++;
                    
                    longindex = i;
                    
                    place = "";
                    
                    if (longopts[i].flag == nullptr)
                        return longopts[i].val;
                    else
                    {
                        *longopts[i].flag = longopts[i].val;
                        return 0;
                    }
                }
            }
            
            if (opterr && optstring[0] != ':') {
                GETOPT_ERROR("%s: illegal option -- %s\n", argv[0], place);
            }
            place = "";
            optind++;
            return BADCH;
        }
    }

    /* short option */
    optopt = (int) *place++;

    oli = strchr(optstring, optopt);
    if (!oli)
    {
        if (!*place)
            ++optind;
        if (opterr && *optstring != ':') {
            GETOPT_ERROR("%s: illegal option -- %c\n", argv[0], optopt);
        }
        return BADCH;
    }
    
    if (oli[1] != ':')
    { /* don't need argument */
        optarg = nullptr;
        if (!*place)
            ++optind;
    }
    else
    { /* need an argument */
        if (*place) /* no white space */
            optarg = place;
        else if (argc <= ++optind)
        { /* no arg */
            place = "";
            if (*optstring == ':')
                return BADARG;
            if (opterr) {
                GETOPT_ERROR("%s: option requires an argument -- %c\n",
                             argv[0], optopt);
            }
            return BADCH;
        }
        else
            /* white space */
            optarg = argv[optind];
        place = "";
        ++optind;
    }
    return optopt;
}

#endif // HAL_OS_POSIX_IO
