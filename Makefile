CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g

TARGET  = ipv4_extractor
SOURCES = main.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Pipe tests.txt into the binary, strip the repeating prompt prefix from every
# output line, then diff against expected.txt.  A clean run prints nothing and
# exits 0; any mismatch is shown as a unified diff.
test: $(TARGET)
	@echo "--- Running $(shell wc -l < tests.txt | tr -d ' ') test cases ---"
	@./$(TARGET) < tests.txt \
	    | sed "s/^Enter a string (or 'END' to quit): //" \
	    > /tmp/ipv4_actual.txt
	@if diff -u expected.txt /tmp/ipv4_actual.txt; then \
	    echo "All tests passed."; \
	else \
	    echo "TESTS FAILED: see diff above (- expected, + actual)"; \
	    exit 1; \
	fi

clean:
	rm -f $(TARGET) /tmp/ipv4_actual.txt
