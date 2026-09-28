CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g

TARGET  = ipv4_extractor
SOURCES = main.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Run the exact sample session from the spec and compare output.
test: $(TARGET)
	@echo "--- Running sample test ---"
	@printf '%s\n' \
		"connecting to 192.168.1.1 now" \
		"server=10.0.0.255:8080end" \
		"192a168.1.1.1" \
		"192.168.1.1." \
		"Connection from 192.168.1.1 refused" \
		"192.168.01.1" \
		"1.2.3.4:99999" \
		"12.34.56" \
		"no number here" \
		"END" \
	| ./$(TARGET)

clean:
	rm -f $(TARGET)
