// ArduinoIBIS
// Implementation of the VDV 300 IBIS Wagenbus protocol for Arduino and Arduino-core based devices

// Copyright (c) 2025 Jonathan Verbeek

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <SoftwareSerial.h>

// IBIS connection parameters (1200 7E2)
#define IBIS_BAUD 1200
#define IBIS_SERIAL_CONFIG SWSERIAL_7E2

// Most telegrams don't have a more complex structure and can therefore be constructed with a simple printf string only
// Hence, IBIS_SIMPLE_TELEGRAM can be used to declare those
#define IBIS_TELEGRAM_BUFFER_SIZE 64
#define IBIS_SIMPLE_TELEGRAM(id, argType, fmt) \
	void DS##id(argType arg) \
	{ \
		char buf[IBIS_TELEGRAM_BUFFER_SIZE]; \
		sprintf(buf, fmt, arg); \
		SendTelegram(buf); \
	}

namespace ArduinoIBIS
{
	// This class acts as the main communication "port" handling an IBIS device.
	// IBIS communication happens at 1288 baud in 7E2 configuration and is internally
	// handled with a software serial interface, which is fine for slow baud rates like this.
	// This way we're keeping the hardware UART free to use for other uses.
	class Port
	{
	public:
		// Opens the software serial port for IBIS communication
		bool Begin(int8_t txPin = 12, int8_t rxPin = -1, bool invert = false);

		// Closes the software serial port
		void End();

		// When enabled, the library prints debug output to the hardware Serial interface (which still needs to be
		// initialized by you)
		void SetEnableDebugOutput(bool enable);

	public:
		IBIS_SIMPLE_TELEGRAM(001, uint16_t, "l%03d"); // Line Number, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(001neu, uint16_t, "q%04d"); // Line number, alphanumeric, 1-4 chars
		IBIS_SIMPLE_TELEGRAM(001a, uint8_t, "lE%02d"); // Line number symbol, 1-2 digits
		IBIS_SIMPLE_TELEGRAM(001b, uint16_t, "lF%05d"); // Radio
		IBIS_SIMPLE_TELEGRAM(001c, uint16_t, "lP%03d"); // Line tape reel position ID, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(001d, uint16_t, "lC%04d"); // Line number, alphanumeric, 1-4 chars
		IBIS_SIMPLE_TELEGRAM(001e, uint16_t, "lC%08d"); // Line number, alphanumeric, 1-8 chars
		IBIS_SIMPLE_TELEGRAM(001f, uint16_t, "lC%07d"); // Line number, alphanumeric, 1-7 chars
		IBIS_SIMPLE_TELEGRAM(002, uint8_t, "k%02d"); // Course number, 1-2 digits
		IBIS_SIMPLE_TELEGRAM(002a, uint16_t, "k%05d"); // Train number, 1-5 digits
		IBIS_SIMPLE_TELEGRAM(003, uint16_t, "z%03d"); // Destination text ID, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(003b, uint16_t, "zR%03d"); // Destination ID for IMU, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(003d, uint16_t, "zN%03d"); // Route number, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(003e, uint16_t, "zP%03d"); // Destination tape reel position ID, 1-3 digits
		IBIS_SIMPLE_TELEGRAM(003f, uint16_t, "zN%06d"); // Route number, 1-6 digits
		IBIS_SIMPLE_TELEGRAM(003g, uint16_t, "zL%04d"); // Line number, 1-4 digits

		IBIS_SIMPLE_TELEGRAM(004, uint16_t, "e%06d"); // Ticket validator attributes, 6 digits
		IBIS_SIMPLE_TELEGRAM(004a, uint16_t, "eA%04d"); // Additional ticket validator attributes, 4 digits
		IBIS_SIMPLE_TELEGRAM(004b, uint16_t, "eH%07d"); // Ticket validator stop number, 1-7 digits

		IBIS_SIMPLE_TELEGRAM(005, uint16_t, "u%04d"); // Time, HHMM
		IBIS_SIMPLE_TELEGRAM(006, uint16_t, "d%05d"); // Date, DDMMY

		IBIS_SIMPLE_TELEGRAM(007, uint8_t, "w%01d"); // Train length, 1 digit

		IBIS_SIMPLE_TELEGRAM(009, const char*, "v%-16s"); // Next stop text, 16 chars
		IBIS_SIMPLE_TELEGRAM(009a, const char*, "v%-20s"); // Next stop text, 20 chars
		IBIS_SIMPLE_TELEGRAM(009b, const char*, "v%-24s"); // Next stop text, 24 chars

		IBIS_SIMPLE_TELEGRAM(010, uint16_t, "x%04d"); // Line progress display stop ID, 1-4 digits
		IBIS_SIMPLE_TELEGRAM(010a, uint16_t, "xH%04d"); // Line progress display stop ID, 1-4 digits
		IBIS_SIMPLE_TELEGRAM(010b, uint8_t, "xI%02d"); // Line progress display stop ID, 1-2 digits
		IBIS_SIMPLE_TELEGRAM(010d, uint16_t, "xJ%04d"); // Year, YYYY

		void DS010e(const char* sign, uint16_t delay); // Delay, sign is either '+' or '-', delay is 1-3 digits

		void DS003a(const String& text); // Destination text
		void DS003c(const String& text); // Next stop name

		void DS021(uint8_t address, String text); // Destination text
		void DS021a(uint8_t address, uint8_t stopId, String stopText, String changeText); // Line progress display text

		void GSP(uint8_t address, String line1, String line2);

	private:
		// Wraps the telegram with required extra data and sends it to the serial port
		void SendTelegram(String telegram);

		// Converts a uint8 value to a VDV hex string
		static String ToHexString(uint8_t value);

	private:
		// Internal handle to the software serial port
		EspSoftwareSerial::UART* _port = nullptr;

		// Whether to print debug output to Serial0
		bool _debug = false;
	};
}
