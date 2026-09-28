#include <iostream>
#include <string>

// Advance i past all contiguous token characters (digits, '.', ':').
// Called on any parse failure so the entire bad token is discarded rather
// than retried character-by-character, which would allow truncated matches
// like "56.1.1.1" being found inside "256.1.1.1".
static void skipToken(const std::string& str, int& i)
{
    int len = static_cast<int>(str.size());
    while (i < len && (isdigit(str[i]) || str[i] == '.' || str[i] == ':'))
        ++i;
}

// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort)
{
    outAddress = 0;
    outPort    = -1;

    int len = static_cast<int>(str.size());

    // Try each token in the string.  A token is a maximal run of digits,
    // '.', and ':'.  Non-token characters act as separators between tokens.
    // On any parse failure the remainder of the current token is skipped so
    // we never extract a valid address that is a suffix of a longer numeric run.
    int i = 0;
    while (i < len) {
        // Skip non-token characters.
        if (!isdigit(str[i]) && str[i] != '.' && str[i] != ':') {
            ++i;
            continue;
        }

        // --- Parse four octets ---
        unsigned long octets[4] = {0, 0, 0, 0};
        bool octetOk = true;

        for (int o = 0; o < 4 && octetOk; ++o) {
            // Must start with a digit.
            if (i >= len || !isdigit(str[i])) {
                octetOk = false;
                break;
            }

            // Accumulate digits by hand.
            unsigned long val = 0;
            int digitCount = 0;
            while (i < len && isdigit(str[i])) {
                val = val * 10 + (str[i] - '0');
                ++digitCount;
                ++i;
            }

            // Reject leading zeros (e.g. "01").
            // A single '0' is fine; two or more digits starting with '0' is not.
            if (digitCount > 1) {
                char firstDigit = str[i - digitCount];
                if (firstDigit == '0') {
                    octetOk = false;
                    break;
                }
            }

            // Octet must be 0–255.
            if (val > 255) {
                octetOk = false;
                break;
            }

            octets[o] = val;

            // After the last octet we DON'T require a dot.
            if (o < 3) {
                if (i >= len || str[i] != '.') {
                    octetOk = false;
                    break;
                }
                ++i; // consume the dot
            }
        }

        if (!octetOk) {
            // Discard the rest of this token; never back up to try a suffix.
            skipToken(str, i);
            continue;
        }

        // --- After the four octets, what follows? ---
        // Valid endings:
        //   (a) end-of-string / non-token character  → no port
        //   (b) ':' followed by a valid port number, then end-of-string / non-token char
        //
        // Invalid endings (reject whole token):
        //   '.' → stray period immediately after the 4th octet
        //   ':' not followed by valid port digits
        //   port out of range, or port followed by more token characters

        // Stray period after 4th octet → longer run, discard.
        if (i < len && str[i] == '.') {
            skipToken(str, i);
            continue;
        }

        int port = -1;

        if (i < len && str[i] == ':') {
            ++i; // skip ':'

            if (i >= len || !isdigit(str[i])) {
                // Colon with no digit → discard token.
                skipToken(str, i);
                continue;
            }

            // Accumulate port digits by hand.
            unsigned long portVal = 0;
            while (i < len && isdigit(str[i])) {
                portVal = portVal * 10 + (str[i] - '0');
                ++i;
            }

            // Port must be 0–65535.
            if (portVal > 65535) {
                skipToken(str, i);
                continue;
            }

            // Port must not be followed by more token characters.
            if (i < len && (isdigit(str[i]) || str[i] == '.' || str[i] == ':')) {
                skipToken(str, i);
                continue;
            }

            port = static_cast<int>(portVal);

        } else if (i < len && (isdigit(str[i]) || str[i] == ':')) {
            // Digit or colon directly after 4th octet → longer run, discard.
            skipToken(str, i);
            continue;
        }
        // Anything else (non-token char or end-of-string) is a clean boundary.

        // --- Success: build the 32-bit address ---
        unsigned long addr =  (octets[0] << 24)
                            | (octets[1] << 16)
                            | (octets[2] <<  8)
                            |  octets[3];

        outAddress = addr;
        outPort    = port;
        return true;
    }

    // No valid address found.
    outAddress = 0;
    outPort    = -1;
    return false;
}

int main()
{
    std::string line;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, line)) {
            // EOF
            break;
        }

        if (line == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        unsigned long address = 0;
        int           port    = -1;

        if (extractIPv4(line, address, port)) {
            // Reconstruct the dotted-decimal string from the 32-bit value.
            unsigned long a = (address >> 24) & 0xFF;
            unsigned long b = (address >> 16) & 0xFF;
            unsigned long c = (address >>  8) & 0xFF;
            unsigned long d =  address        & 0xFF;

            std::cout << "Extracted IPv4 address: "
                      << a << "." << b << "." << c << "." << d
                      << " (decimal value: " << address
                      << ", port: ";
            if (port == -1)
                std::cout << "none";
            else
                std::cout << port;
            std::cout << ")" << std::endl;
        } else {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0;
}
