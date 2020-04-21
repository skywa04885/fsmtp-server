#include "resolver.src.hpp"
#include "../pre.hpp"

namespace Fannst::dns
{
    /**
     * Resolves MX Records
     * @param hostname
     * @param target
     * @return
     */
    int resolveMX(const char *hostname, std::vector<ResolverMXRecord> &target)
    {
        unsigned char buffer[512];
        char exchange[256];
        ns_msg msg;
        ns_rr rr;
        int res_length;

        // ----
        // Query's the message
        // ----

        // Performs an dns query, and stores the data in the buffer
        res_length = res_query(hostname, ns_c_in, ns_t_mx, buffer, sizeof(buffer));
        if (res_length < 0)
        {
            return -1;
        }

        // ----
        // Parses the message, and stores in array
        // ----

        // Initializes the parser and gets the message count
        ns_initparse(buffer, res_length, &msg);
        res_length = ns_msg_count(msg, ns_s_an);

        // Loops over the records
        for (std::size_t i = 0; i < res_length; i++)
        {
            // Parses the record
            ns_parserr(&msg, ns_s_an, i, &rr);

            // Gets the exchange
            dn_expand(ns_msg_base(msg), ns_msg_end(msg), ns_rr_rdata(rr) + 2, exchange, sizeof(exchange));

            // Pushes it to the final vector
            target.emplace(target.begin(), exchange, ns_rr_name(rr), ns_rr_type(rr),
                           ns_rr_ttl(rr), ns_rr_class(rr), ns_rr_rdlen(rr));
        }

        return 0;
    }

    /**
     * The default constructor of ResolverMXRecord
     * @param r_Exchange
     * @param r_Name
     * @param r_Type
     * @param r_TTL
     * @param r_Class
     * @param r_RdLen
     */
    ResolverMXRecord::ResolverMXRecord(const char *r_Exchange, const char *r_Name, const int &r_Type, const int &r_TTL,
                     const int &r_Class, const int &r_RdLen)
    {
        unsigned long len;

        // Copies the exchange
        len = strlen(r_Exchange);
        this->r_Exchange = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(len, 0)));
        memcpy(this->r_Exchange, r_Exchange, len);

        // Copies the name
        len = strlen(r_Name);
        this->r_Name = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(len, 0)));
        memcpy(this->r_Name, r_Name, len);

        // Stores the integers
        this->r_Type = r_Type;
        this->r_TTL = r_TTL;
        this->r_Class = r_Class;
        this->r_RdLen = r_RdLen;
    }

    ResolverMXRecord::~ResolverMXRecord()
    {
        free(this->r_Name);
        free(this->r_Exchange);
    }

    /**
     * Clears the current record memory
     */
    void ResolverMXRecord::clear()
    {
        delete this->r_Exchange;
        delete this->r_Name;
    }

    /**
     * Clears complete vector of records
     * @param target
     */
    void ResolverMXRecord::clearVector(std::vector<ResolverMXRecord> &target)
    {
        for (ResolverMXRecord &r : target) r.clear();
    }

    /**
     * Resolves IP to hostname
     * @param hostname
     * @param target
     * @return
     */
    int resolveIpAddress(const char *hostname, char *target)
    {
        // Gets the record
        hostent *record = gethostbyname(hostname);

        // Checks if it went successfully
        if (record == nullptr) return -1;

        // Parses the ip address
        in_addr *address = reinterpret_cast<in_addr *>(record->h_addr);

        // Stores the address in the target
        const char *addr = inet_ntoa(*address);
        memcpy(target, addr, sizeof(char) * strlen(addr) + 1);

        // Returns that it was fine
        return 0;
    }
}