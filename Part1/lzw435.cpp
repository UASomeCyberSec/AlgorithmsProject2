/*
    Nathan Brannan
    nb121@uakron.edu
*/

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <algorithm>

/* This code is derived in parts from LZW@RosettaCode for UA CS435
 */

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.

template <typename Iterator>
Iterator compress(const std::string &uncompressed, Iterator result)
{
    // Build the dictionary, start with 256.
    int dictSize = 256;
    std::map<std::string, int> dictionary;
    for (int i = 0; i < dictSize; i++)
        dictionary[std::string(1, i)] = i;

    std::string w;
    for (std::string::const_iterator it = uncompressed.begin();
         it != uncompressed.end(); ++it)
    {
        char c = *it;
        std::string wc = w + c;
        if (dictionary.count(wc))
            w = wc;
        else
        {
            *result++ = dictionary[w];
            // Add wc to the dictionary. Assuming the size is 4096!!!
            if (dictionary.size() < 4096)
                dictionary[wc] = dictSize++;
            w = std::string(1, c);
        }
    }

    // Output the code for w.
    if (!w.empty())
        *result++ = dictionary[w];
    return result;
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end)
{
    // Build the dictionary.
    int dictSize = 256;
    std::map<int, std::string> dictionary;
    for (int i = 0; i < dictSize; i++)
        dictionary[i] = std::string(1, i);

    std::string w(1, *begin++);
    std::string result = w;
    std::string entry;
    for (; begin != end; begin++)
    {
        int k = *begin;
        if (dictionary.count(k))
            entry = dictionary[k];
        else if (k == dictSize)
            entry = w + w[0];
        else
            throw "Bad compressed k";

        result += entry;

        // Add w+entry[0] to the dictionary. 4096 = 2^12
        if (dictionary.size() < 4096)
            dictionary[dictSize++] = w + entry[0];

        w = entry;
    }
    return result;
}

// Function for checking if a given string is a number
bool is_number(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

//
std::string int2BinaryString(int c, int cl) {
    std::string p = ""; //a binary code string with code length = cl
    int code = c;
    while (c>0) {         
        if (c%2==0)
        p="0"+p;
        else
        p="1"+p;
        c=c>>1;   
    }
    int zeros = cl-p.size();
    if (zeros<0) {
        std::cout << "\nWarning: Overflow. code " << code <<" is too big to be coded by " << cl <<" bits!\n";
        p = p.substr(p.size()-cl);
    }
    else {
        for (int i=0; i<zeros; i++)  //pad 0s to left of the binary code if needed
        p = "0" + p;
    }
    return p;
}

//
int binaryString2Int(std::string p) {
    int code = 0;
    if (p.size()>0) {
        if (p.at(0)=='1') 
            code = 1;
        p = p.substr(1);
        while (p.size()>0) { 
            code = code << 1; 
            if (p.at(0)=='1')
                code++;
            p = p.substr(1);
        }
    }
    return code;
}

void compressLZW(std::string filename)
{
    // Collect file text in a string
    std::string input;
    std::ifstream inputFile;
    std::ofstream outputFile;
    std::vector<int> compressed;
    std::string zeros = "00000000";
    int bits;
    inputFile.open(filename + ".txt");
    if (inputFile.is_open())
    {
        while (!inputFile.eof())
        {
            std::string line;
            std::getline(inputFile, line);
            input += line;
        }
    }
    else
    {
        std::cout << "Error: Could not open file " << filename << ".txt\n";
    }
    inputFile.close();

    // Place compressed text into .lzw file
    outputFile.open(filename + ".lzw", std::ios::binary);
    if (outputFile.is_open())
    {
        compress(input, std::back_inserter(compressed));

        std::string p = "";
        std::string bcode= "";
        bits = 12;
        for (std::vector<int>::iterator it = compressed.begin() ; it != compressed.end(); ++it) 
        {
            p = int2BinaryString(*it, bits);
            bcode += p;
        }

        if (bcode.size() % 8 != 0) //make sure the length of the binary string is a multiple of 8
            bcode += zeros.substr(0, 8 - bcode.size() % 8);

        int b; 
        for (int i = 0; i < bcode.size(); i += 8) { 
            b = 1;
            for (int j = 0; j < 8; ++j) {
                b = b<<1;
                if (bcode.at(i + j) == '1')
                    b += 1;
            }
            char c = (char) (b & 255); //save the string byte by byte
            outputFile.write(&c, 1);
        }

        outputFile.close();
    }
    else
    {
        std::cout << "Error: Could not open file " << filename << ".lzw\n";
    }
}

std::vector<int> decompressLZW(std::string filename)
{
    int bits = 12;
    std::vector<int> lzwCodes;
    std::ifstream inputFile;
    std::string zeros = "00000000";

    inputFile.open(filename, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cout << "Error: Could not open file " << filename << "\n";
        return lzwCodes;
    }
    struct stat filestatus;
    stat(filename.c_str(), &filestatus );
    long fsize = filestatus.st_size; //get the size of the file in bytes
    
    char c2[fsize];
    inputFile.read(c2, fsize);

    inputFile.close();
    
    std::string s = "";
    for(long i = 0; i < fsize; ++i)
    {
        unsigned char uc =  (unsigned char) c2[i];
        std::string p = ""; //a binary string
        for (int j = 0; j < 8 && uc > 0; j++) {         
            if (uc % 2 == 0)
                p = "0" + p;
            else
                p = "1" + p;
            uc = uc >> 1;   
        }
        p = zeros.substr(0, 8-p.size()) + p; //pad 0s to left if needed
        s += p;
    }

    while(s.size() > bits - 1)
    {
        lzwCodes.push_back(binaryString2Int(s.substr(0, bits)));
        s.erase(0, bits);
    }

    return lzwCodes;
}

void diffCheck(std::string filename1, std::string filename2)
{
    std::ifstream inputFile1, inputFile2;
    std::string input1, input2;
    inputFile1.open(filename1 + ".txt");
    inputFile2.open(filename2 + ".txt");
    
    // Check whether two inputted files are the same or different
    if (inputFile1.is_open() && inputFile2.is_open())
    {
        std::string line;
        while (!inputFile1.eof())
        {
            std::getline(inputFile1, line);
            input1 += line;
        }
        while (!inputFile2.eof())
        {
            std::getline(inputFile2, line);
            input2 += line;
        }
        
        if (input1 == input2)
        {
            std::cout << "File content is the same\n";
        }
        else
        {
            std::cout << "File content is different:\n";
        }
    }
    else
    {
        if (inputFile2.is_open())
        {
            std::cout << "Error: Could not open file " << filename1 << ".txt\n";
        }
        else if (inputFile1.is_open())
        {
            std::cout << "Error: Could not open file " << filename2 << ".txt\n";
        }
        else
        {
            std::cout << "Error: Could not open files " << filename1 << ".txt & " << filename2 << ".txt\n";
        }
    }
}

int main(int argc, char **argv)
{
    // Check for first c|e type arguments
    if (argc != 3 || (argv[1][0] != 'c' && argv[1][0] != 'e'))
    {
        std::string argvIndex1(argv[1]);

        // Check for diff argument
        if (argc != 4 || argvIndex1 != "diff")
        {
            std::cout << "Options:\n";
            std::cout << "Usage: " << argv[0] << " diff <filename> <filename2>\n";
            std::cout << "Usage: " << argv[0] << " c|e <filename>\n";
        }
        else
        {
            // Check the difference between two given .txt files
            std::string filename1 = argv[2];
            std::string filename2 = argv[3];
            diffCheck(filename1, filename2);
        }
    }
    else
    {
        std::string filename = argv[2];

        // Handle argument for compressing <filename>
        if (argv[1][0] == 'c')
        {
            // Compress file to .lzw
            std::cout << "Compressing " << filename << " to " << filename << ".lzw\n";
            compressLZW(filename);
        }
        else
        {
            // Handle argument for decompressing <filename.lzw>
            // Decompress .lzw file
            std::string decompressed;
            std::cout << "Decompressing " << filename << "\n";
            std::vector<int> lzwCodes = decompressLZW(filename);
            decompressed = decompress(lzwCodes.begin(), lzwCodes.end());
            std::cout << "\nfinal decompressed: " << decompressed << std::endl;

            // Write decompressed text to <filename>2.txt
            std::ofstream outputFile;
            filename.resize(filename.size() - 4);
            outputFile.open(filename + "2.txt");
            outputFile << decompressed;
            outputFile.close();
        }
    }

    return 0;
}
