#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>
using namespace std;
#include "DES.h"


/*
 * This program is a command line utility to test variations on a bad DES key to find the
 * key that matches the given KCV.
 */
void printUsage(const char *name)
{
	printf("Usage %s key=XXX [KCV=XXX]\n\n", name);
	printf( "Inputs:  \n"
		"	key       Bad DES key in hex\n"
		"	KCV       6 character KCV of correct DES key in hex\n\n");
	printf(	"Given just they key, the program will calculate the 6 digit hexidecimal\n"
		"KCV (checksum).  Given a key and KCV, the program will either verify the\n"
		"KCV, or if the KCV is not correct, it will search for keys with up to two\n"
		"bad digits until it finds all the possible keys that match the given KCV.\n\n");
	exit(0);
}

// Converts a string of hex digits into the binary data it represents.  Output string is
// assumed to be large enough to hold the data (half the size of the input string).
void Hex2Bin(const string &input, vector<unsigned char> &output);
void Bin2Hex(const vector<unsigned char> &input, string &output);

// exit somewhat gracefully with error information
void fail(const char *str)
{
	fprintf(stderr, "%s\n", str);
	if(errno != 0)
		fprintf(stderr, "errno %i:  %s\n\n", errno, strerror(errno));
	exit(-1);
}

// do a binary comparison of two blocks of data of the given length, returns true for equal
bool BCompare(const vector<unsigned char> &one, const vector<unsigned char> &two)
{
	size_t len = one.size();
	if(len != two.size()) return false;

	for(size_t i = 0; i < len; ++i)
		if(one[i] != two[i]) return false;
	return true;
}

int main(int argc, char **argv)
{
	string kcv = "";
	string key = "";
	int depth = 2;

	if(argc < 2 || argc > 3)
		printUsage(argv[0]);

	for(int i = 1; i < argc; ++i)
	{
		if(strncmp(argv[i], "KCV=", 4) == 0)
			kcv = argv[i] + 4;
		else if(strncmp(argv[i], "key=", 4) == 0)
			key = argv[i] + 4;
		else
		{
			printf("Unrecognized option %s\n", argv[i]);
			return -1;
		}
	}

	if(key == "") fail("DES encryption key required.");
	if(key.length() % 16 != 0) fail("Key length must be a multiple of 8 bytes.");

	// create a vector of unsigned chars; this will hold the binary instance of the key
	vector<unsigned char> bkey;
	// keys are 8, 16, or 24 bytes long for single, double, and triple length DES
	bkey.resize(24, 0);

	// KCV check; block of zeros to encrypt, block to receive encrypted data
	vector<unsigned char> inblock;
	vector<unsigned char> outblock;

	// DES works on 8 byte blocks
	inblock.resize(8, (unsigned char)0);
	outblock.resize(8, (unsigned char)0);

	// convert from entered hex strings to binary for DES algorithm
	Hex2Bin(key, bkey);

	int blocklen = 8;
	// generate the KCV of the given key
	encryptECB(&bkey[0], bkey.size(), &inblock[0], blocklen, &outblock[0]);

	string keyout, blockout;
	Bin2Hex(bkey, keyout);
	Bin2Hex(outblock, blockout);

	// if we weren't given a KCV to compare against, print out the calculated KCV and exit
	if(kcv == "")
	{
		kcv = blockout.substr(0, 6);
		printf("Calcuated KCV is %s\n\n", kcv.c_str());
		return 0;
	}

	// if we were given a key and KCV, first check to see if we have a match, and if not, 
	// see if we can find a match

	vector<unsigned char> bkcv;
	bkcv.resize(8, 0);
	Hex2Bin(blockout, bkcv);

	if(BCompare(bkcv, outblock))
	{
		printf("Key matches KCV\n");
		exit(0);
	} 
	else
		printf("Key does not match given KCV\n");

	// Your code here

	return 0;
}

// converts a two digit hexidecimal character string into the binary byte it represents
unsigned char ByteFromStr(const char *input)
{
	unsigned char value = 0;
	switch (input[0])
	{
		case '0': break;
		case '1': value += 0x10; break;
		case '2': value += 0x20; break;
		case '3': value += 0x30; break;
		case '4': value += 0x40; break;
		case '5': value += 0x50; break;
		case '6': value += 0x60; break;
		case '7': value += 0x70; break;
		case '8': value += 0x80; break;
		case '9': value += 0x90; break;
		case 'A':
		case 'a': value += 0xA0; break;
		case 'B':
		case 'b': value += 0xB0; break;
		case 'C':
		case 'c': value += 0xC0; break;
		case 'D':
		case 'd': value += 0xD0; break;
		case 'E':
		case 'e': value += 0xE0; break;
		case 'F':
		case 'f': value += 0xF0; break;
		default:
			fail("Invalid hexidecimal digit encountered.");
	}

	switch (input[1])
	{
		case '0': break;
		case '1': value += 0x1; break;
		case '2': value += 0x2; break;
		case '3': value += 0x3; break;
		case '4': value += 0x4; break;
		case '5': value += 0x5; break;
		case '6': value += 0x6; break;
		case '7': value += 0x7; break;
		case '8': value += 0x8; break;
		case '9': value += 0x9; break;
		case 'A':
		case 'a': value += 0xA; break;
		case 'B':
		case 'b': value += 0xB; break;
		case 'C':
		case 'c': value += 0xC; break;
		case 'D':
		case 'd': value += 0xD; break;
		case 'E':
		case 'e': value += 0xE; break;
		case 'F':
		case 'f': value += 0xF; break;
		default:
			fail("Invalid hexidecimal digit encountered.");
	}

	return value;
}

// convert a string of hex data into binary
void Hex2Bin(const string &input, vector<unsigned char> &output)
{
	size_t len = input.length();

	// must be an even number of characters
	if (len % 2)
		fail("Hex input must be an even number of digits");

	output.resize(len / 2);

	for (int i = 0; i < len; i += 2)
	    	output[i / 2] = ByteFromStr(&(input.c_str())[i]);
}

// convert a string of binary data into a hex string
void Bin2Hex(const vector<unsigned char> &input, string &output)
{
	size_t len = input.size();
	output = "";

	for(size_t i = 0; i < len; ++i)
	{
		// buffer to print 2 hex digits and a newline into
		char buff[3] = {(char)0};
		sprintf(buff, "%02X", input[i]);
		output += buff;
	}
	return;
}

