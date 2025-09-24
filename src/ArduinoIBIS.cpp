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

#include "ArduinoIBIS.h"

bool ArduinoIBIS::Port::Begin(int8_t txPin, int8_t rxPin, bool invert)
{
	// Don't do anything if there's already a port created
	if (_port != nullptr)
	{
		if (_debug) _debugOutput->println("ArduinoIBIS: Cannot begin port as there's a SoftwareSerial open");
		return false;
	}

	// Create a new port and begin transmission
	_port = new EspSoftwareSerial::UART(rxPin, txPin, invert);
	_port->begin(IBIS_BAUD, IBIS_SERIAL_CONFIG);
	if ((*_port) == false)
	{
		// The port has failed to initialize
		if (_debug) _debugOutput->println("ArduinoIBIS: Failed to begin SoftwareSerial port");
		return false;
	}

	if (_debug) _debugOutput->println("ArduinoIBIS: Successfully created IBIS port");
	return true;
}

void ArduinoIBIS::Port::End()
{
	if (_port != nullptr)
	{
		_port->end();
		_port = nullptr;
	}
}

void ArduinoIBIS::Port::SetDebugOutput(bool enable, Stream* outputStream)
{
	_debug = enable;
	_debugOutput = outputStream;
}

void ArduinoIBIS::Port::DS010e(const char* sign, uint16_t delay)
{
	char buf[IBIS_TELEGRAM_BUFFER_SIZE];
	int len = sprintf(buf, "xV%.1s%03d", sign, delay);

	String telegram;
	telegram.concat(buf, len);

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::DS003a(const String& text)
{
	String telegram = "zA";

	uint8_t numBlocks = ceil(text.length() / 16.0);
	telegram.concat(ToHexString(numBlocks));
	telegram.concat(text);

	// Fill the remaining string with blank spaces (if any)
	uint8_t remainder = text.length() % 16;
	if (remainder > 0)
	{
		for (uint8_t i = 16; i > remainder; i--)
		{
			telegram.concat(" ");
		}
	}

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::DS003c(const String& text)
{
	String telegram = "zI";

	uint8_t numBlocks = ceil(text.length() / 4.0);
	telegram.concat(ToHexString(numBlocks));
	telegram.concat(text);

	// Fill the remaining string with blank spaces (if any)
	uint8_t remainder = text.length() % 4;
	if (remainder > 0)
	{
		for (uint8_t i = 4; i > remainder; i--)
		{
			telegram.concat(" ");
		}
	}

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::DS021(uint8_t address, String text)
{
	String telegram = "aA";

	uint8_t numBlocks = ceil(text.length() / 4.0);
	telegram.concat(ToHexString(address));
	telegram.concat(ToHexString(numBlocks));
	telegram.concat(text.substring(0, numBlocks * 16));

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::DS021a(uint8_t address, uint8_t stopId, String stopText, String changeText)
{
	String telegram = "aL";

	char data[80];
	int len = sprintf(data, "\x03%02d\x04%s\x05%s", stopId, stopText.c_str(), changeText.c_str());

	uint8_t numBlocks = ceil(len / 4.0);
	uint8_t remainder = len % 4;
	telegram.concat(ToHexString(address));
	telegram.concat(ToHexString(numBlocks));
	telegram.concat(ToHexString(remainder));
	telegram.concat(data, len);

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::GSP(uint8_t address, String line1, String line2)
{
	String lines = "";
	lines.concat(line1);
	if (line2.length() > 0)
	{
		lines.concat('\x0a'); // LF
	}
	lines.concat(line2);
	lines.concat("\x0a\x0a"); // LF LF

	uint8_t numBlocks = ceil(lines.length() / 16.0);
	uint8_t remainder = lines.length() % 16;
	if (remainder > 0)
	{
		for (uint8_t i = 16; i > remainder; i--)
		{
			lines.concat(" ");
		}
	}

	String telegram = "aA";
	telegram.concat(ToHexString(address));
	telegram.concat(ToHexString(numBlocks));
	telegram.concat(lines);

	SendTelegram(telegram);
}

void ArduinoIBIS::Port::SendTelegram(String telegram)
{
	if (_port == nullptr)
	{
		if (_debug) _debugOutput->println("ArduinoIBIS: Cannot send, port is null!");
		return;
	}

	// German umlauts need to be handled: IBIS telegrams (and any text transmitted inside them) are plain in ASCII format,
	// where umlauts are not part of. To fix this, the VDV 300 document uses a slightly altered ASCII table, which replaces
	// a couple of never-used characters from the original ASCII tables with the german umlauts (see VDV 300 page 50).
	// The replacement of any umlauts with the "real" ASCII values is happening here.
	telegram.replace("ä", "{");
	telegram.replace("ö", "|");
	telegram.replace("ü", "}");
	telegram.replace("ß", "~");
	telegram.replace("Ä", "[");
	telegram.replace("Ö", "\\");
	telegram.replace("Ü", "]");

	// Add a CR character at the end
	telegram.concat('\x0d');

	// Calculate the telegram checksum by XOR-ing every byte, starting at 0x7F, and append it to the telegram
	char checksum = 0x7F;
	for (unsigned int i = 0; i < telegram.length(); i++)
	{
		checksum ^= telegram[i];
	}
	telegram.concat(checksum);

	// Debug print the whole telegram
	if (_debug)
	{
		_debugOutput->print("ArduinoIBIS: Sending telegram with length=");
		_debugOutput->print(telegram.length());
		_debugOutput->print(" checksum=0x");
		if (checksum < 10)
		{
			_debugOutput->print("0");
		}
		_debugOutput->print(checksum, HEX);
		_debugOutput->print(": ");

		for (unsigned int i = 0; i < telegram.length(); i++)
		{
			if (telegram.charAt(i) < 0x10)
			{
				_debugOutput->print("0");
			}
			_debugOutput->print(telegram.charAt(i), HEX);
			_debugOutput->print(" ");
		}
		_debugOutput->println();
	}

	// Finally send the fully wrapped telegram through the serial port
	_port->print(telegram);
}

String ArduinoIBIS::Port::ToHexString(uint8_t value)
{
	// The VDV 300 document describes how hex numbers should be encoded (page 50)
	// Basically, the digits 0..9 are used as-is, and A..F is encoded by :;<=>? respectfully
	static String hexCharacters = "0123456789:;<=>?";
	String hexString;

	uint8_t highNibble = value >> 4;
	uint8_t lowNibble = value & 15;

	if (highNibble > 0)
	{
		hexString += hexCharacters.charAt(highNibble);
	}

	hexString += hexCharacters.charAt(lowNibble);
	return hexString;
}
