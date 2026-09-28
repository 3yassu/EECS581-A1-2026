#include <iostream>
#include <string>

// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort)
{
    outAddress = 0;
    outPort    = -1;

    int len = static_cast<int>(str.size());

    // Try every position in the string as a candidate start.
    int i = 0;
    while (i < len) {
        // A token character is a digit, '.', or ':'.
        // Skip non-token characters immediately.
        if (!isdigit(str[i]) && str[i] != '.' && str[i] != ':') {
            ++i;
            continue;
        }

        // Attempt to parse an IPv4 address (with optional port) starting at i.
        // The token runs while we only see digits, '.', ':'.
        // We record where this candidate starts so we can advance by 1 on failure.
        int start = i;

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
                // The first digit of this octet was str[i - digitCount].
                // If it was '0' the whole run has a leading zero.
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
                // Expect a dot separator.
                if (i >= len || str[i] != '.') {
                    octetOk = false;
                    break;
                }
                ++i; // consume the dot
            }
        }

        if (!octetOk) {
            // This candidate failed; restart from start+1.
            i = start + 1;
            continue;
        }

        // --- After the four octets, what follows? ---
        // Valid endings:
        //   (a) end-of-string / non-token character  → no port
        //   (b) ':' followed by a valid port number, then end-of-string / non-token char
        //
        // Invalid endings (reject this candidate):
        //   '.' → stray period immediately after the 4th octet
        //   ':' not followed by valid port digits
        //   any token character that would make the matched address part of a longer token

        // Check what comes immediately after the address.
        if (i < len && str[i] == '.') {
            // Stray period — this is a longer run; reject.
            i = start + 1;
            continue;
        }

        int port = -1;

        if (i < len && str[i] == ':') {
            // Must be followed by at least one digit.
            int colonPos = i;
            ++i; // skip ':'

            if (i >= len || !isdigit(str[i])) {
                // Colon with no digit after → reject candidate.
                i = start + 1;
                continue;
            }

            // Accumulate port digits by hand.
            unsigned long portVal = 0;
            int portDigits = 0;
            while (i < len && isdigit(str[i])) {
                portVal = portVal * 10 + (str[i] - '0');
                ++portDigits;
                ++i;
            }

            // Port must be 0–65535.
            if (portVal > 65535) {
                i = start + 1;
                continue;
            }

            // After the port, the next character must NOT be a token character
            // that would extend this token (digit, '.', or ':').
            if (i < len && (isdigit(str[i]) || str[i] == '.' || str[i] == ':')) {
                // Something is still attached — reject.
                i = start + 1;
                continue;
            }

            port = static_cast<int>(portVal);

            // Also reject a second colon (caught above because after port digits
            // another ':' is a token char and we'd reject, but be explicit).
            (void)colonPos; // already consumed
        } else if (i < len && (isdigit(str[i]) || str[i] == ':')) {
            // A digit or colon directly after the 4th octet without a separator
            // means this isn't a clean address boundary.
            i = start + 1;
            continue;
        }
        // If str[i] is anything else (non-token char or end-of-string), we're good.

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
