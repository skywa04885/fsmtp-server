#pragma once

#include <vector>
#include <netdb.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <cstring>
#include <iosfwd>
#include <iostream>

namespace Fannst::dns
{
    class ResolverMXRecord
    {
    public:
        char *r_Exchange;
        char *r_Name;
        int r_Type;
        int r_TTL;
        int r_Class;
        int r_RdLen;

        /**
         * The default constructor of ResolverMXRecord
         * @param r_Exchange
         * @param r_Name
         * @param r_Type
         * @param r_TTL
         * @param r_Class
         * @param r_RdLen
         */
        ResolverMXRecord(const char *r_Exchange, const char *r_Name, const int &r_Type, const int &r_TTL,
                         const int &r_Class, const int &r_RdLen);

        /**
         * Clears the current record memory
         */
        void clear();

        /**
         * Clears complete vector of records
         * @param target
         */
        static void clearVector(std::vector<ResolverMXRecord> &target);
    };

    /**
     * Resolves MX Records
     * @param hostname
     * @param target
     * @return
     */
    int resolveMX(const char *hostname, std::vector<ResolverMXRecord> &target);

    /**
     * Resolves IP to hostname
     * @param hostname
     * @param target
     * @return
     */
    int resolveIpAddress(const char *hostname, char *target);
}

/**
 * Prints an MX Record
 * @param out
 * @param data
 * @return
 */
inline std::ostream &operator << (std::ostream &out, Fannst::dns::ResolverMXRecord const &data)
{
    out << "\033[34m[MX Record]\033[0m:" << std::endl;
    out << " - \033[34m[Exchange]\033[0m: \033[33m" << data.r_Exchange << "\033[0m" << std::endl;
    out << " - \033[34m[Name]\033[0m: \033[33m" << data.r_Name << "\033[0m" << std::endl;
    out << " - \033[34m[r_Type]\033[0m: \033[33m" << data.r_Type << "\033[0m" << std::endl;
    out << " - \033[34m[r_TTL]\033[0m: \033[33m" << data.r_TTL << "\033[0m" << std::endl;
    out << " - \033[34m[r_Class]\033[0m: \033[33m" << data.r_Class << "\033[0m" << std::endl;
    out << " - \033[34m[r_RdLen]\033[0m: \033[33m" << data.r_RdLen << "\033[0m" << std::endl;
    return out;
}