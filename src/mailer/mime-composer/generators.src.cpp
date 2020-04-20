/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "generators.src.hpp"

namespace Fannst::FSMTPServer::Mailer::Composer
{
    static char _messageIdDictionary[] {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9'
    };
    static int _messageIdDictionarySize = sizeof(_messageIdDictionary) / sizeof(char);

    static char _boundaryDictionary[] {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'n', 'm', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z'
    };
    static int _boundaryDictionarySize = sizeof(_boundaryDictionary) / sizeof(char);

    /**
     * Generates an message id
     * @param mesRet
     * @param mesRetLen
     */
    void _generateMessageID(char **mesRet, std::size_t *mesRetLen)
    {
        // How wil the message id be structured ?
        // <8Bytes>-<4Bytes>-<4Bytes>-<12Bytes>@<DOMAIN><NULL>
        // Random size: ( 8 + 4 + 4 + 12 ) + ( 3 * 1 ) + 1 = 33 Bytes

        // ----
        // Allocates the memory in the ret
        // ----


        // Sets the result size
        *mesRetLen = strlen(&GE_DOMAIN[0]) + 33;

        // Allocates 32 bytes
        *mesRet = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(*mesRetLen)));
        PREP_ALLOCATE_INVALID(*mesRet);

        // Sets the null termination char
        (*mesRet)[32] = '\0';

        // ----
        // Prepares the random generators
        // ----

        // Creates the random device
        std::random_device randomDevice;

        // Creates the generator
        std::mt19937 generator(randomDevice());

        // Creates the uniform int distribution
        std::uniform_int_distribution<int> distribution(0, _messageIdDictionarySize - 1);

        // ----
        // Generates the message id
        // ----

        // Generates 8 random bytes, and appends them to the result
        for (BYTE i = 0; i < 8; i++) {
            (*mesRet)[i] = _messageIdDictionary[distribution(generator)];
        }

        // Adds an stripe
        (*mesRet)[8] = '-';

        // Generates 4 random bytes, and appends them to the result
        for (BYTE i = 9; i < 13; i++) {
            (*mesRet)[i] = _messageIdDictionary[distribution(generator)];
        }

        // Adds an stripe
        (*mesRet)[13] = '-';

        // Generates 4 random bytes, and appends them to the result
        for (BYTE i = 14; i < 18; i++) {
            (*mesRet)[i] = _messageIdDictionary[distribution(generator)];
        }

        // Adds an stripe
        (*mesRet)[18] = '-';

        // Generates 12 random bytes, and appends them to the result
        for (BYTE i = 19; i < 31; i++) {
            (*mesRet)[i] = _messageIdDictionary[distribution(generator)];
        }

        // ----
        // Appends the domain name
        // ----

        strcat(&(*mesRet)[0], "@");
        strcat(&(*mesRet)[0], &GE_DOMAIN[0]);
    }

    /**
     * Generates an message boundary
     * @param bouRet
     * @param bouRetLen
     */
    void _generateBoundary(char **bouRet, std::size_t *bouRetLen)
    {
        // ----
        // Allocates the required memory
        // ----

        // Allocates the memory
        *bouRet = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(30)));
        PREP_ALLOCATE_INVALID(*bouRet);

        // Sets the null termination char
        (*bouRet)[30] = '\0';

        // ----
        // Prepares the random generator
        // ----

        // Creates the random device
        std::random_device randomDevice;

        // Creates the random generator
        std::mt19937 generator(randomDevice());

        // Creates the int distribution
        std::uniform_int_distribution<int> distribution(0, _boundaryDictionarySize - 1);

        // ----
        // Creates the boundary
        // ----

        // Appends the first two dashes, and lower dash
        (*bouRet)[0] = '-';
        (*bouRet)[1] = '-';
        (*bouRet)[2] = '_';

        // Creates the other 27 random chars
        for (BYTE i = 3; i < 30; i++)
        {
            (*bouRet)[i] = _boundaryDictionary[distribution(generator)];
        }

        // Sets the boundary len
        *bouRetLen = 30;
    }
}