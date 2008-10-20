/**
 * @file   gemtcl-client.cpp
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Sun Feb  4 16:22:43 2007
 * 
 * @brief  A xmlrpc client for the salfw viewer.
 * 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gnet_xmlrpc.h"

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
    GNetXmlRpcClient *client;
    int argp = 1;
    char *params;
    char *gem_host = "localhost";
    int gem_port = 8822;
    bool do_debug = false;
    gchar *reply;

    while(argp < argc && argv[argp][0] == '-') {
        char *S_ = argv[argp++];

        CASE("-help") {
            printf("gemtcl-client - A client for sending xmlrpc commands to the gemtcl\n\n"
                   "Syntax:\n"
                   "    gemtcl-client command1 [params] {command2 params ...}\n"
                   "\n"
                   "Options:\n"
                   "    -port p   Set port. Default is 8822\n"
                   "    -host h   Set host. Default is localhost\n"
                   "    -debug    Output debug output\n"
                   );
            exit(0);
        }
        CASE("-port") {
            gem_port = atoi(argv[argp++]);
            continue;
        }
        CASE("-host") {
            gem_host = argv[argp++];
            continue;
        }
        CASE("-debug") {
            do_debug = true;
            continue;
        }
        die("Unknown option %s!\n", S_);
    }

    gnet_init ();
    
    client = gnet_xmlrpc_client_new(gem_host, gem_port);

    if (!client)
        die("Failed connecting to %s:%s\n", gem_host, gem_port);

    const char* cmd = "TclEval";
    params = argv[argp++];
    if (do_debug)
        printf("Calling %s:%s\n", cmd, params);

    if (gnet_xmlrpc_client_call(client,
                                cmd,
                                params,
                                /* output  */
                                &reply) == 0) {
        printf("%s\n", reply);
        g_free(reply);
    }
    else
        die("Failed executing xmlrpc command!\n");

    exit(0);
    return(0);
}
