/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "server.src.hpp"

namespace Fannst::FSMTPServer::DS_API
{
    static struct mg_serve_http_opts serveHttpOpts{};

    /**
     * Handles an mongoose event
     * @param nc
     * @param ev
     * @param p
     */
    static void eventHandler(struct mg_connection *nc, int ev, void *p)
    {
        if (ev == MG_EV_HTTP_REQUEST) {
            mg_serve_http(nc, (struct http_message *) p, serveHttpOpts);
        }
    }

    /**
     * Runs the DS Api
     * @param port
     * @param argc
     * @param argv
     */
    void run(int port, int *argc, char ***argv)
    {
        struct mg_mgr mgr{};
        struct mg_connection *nc;

        char *httpPort;

        // ----
        // Prepares the port
        // ----

        // Allocates four bytes for the port
        httpPort = reinterpret_cast<char *>(alloca(9));
        // Copies the port into the buffer
        sprintf(&httpPort[0], "%d", port);

        // ----
        // Creates the web serer
        // ----

        // Initializes mongoose
        mg_mgr_init(&mgr, nullptr);

        // Binds to the port
        nc = mg_bind(&mgr, httpPort, eventHandler);
        if (nc == nullptr)
        {
            std::cout << "Could not bind" << std::endl;
        }

        // Sets the HTTP Server parameters
        mg_set_protocol_http_websocket(nc);
        serveHttpOpts.document_root = "../web/public";
        serveHttpOpts.enable_directory_listing = "yes";

        // Creates the infinite loop
        for (;;)
        {
            mg_mgr_poll(&mgr, 1000);
        }

        // ----
        // Frees the memory
        // ----

        mg_mgr_free(&mgr);
    }
}