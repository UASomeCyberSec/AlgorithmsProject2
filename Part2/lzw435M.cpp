#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <algorithm>
#include <cmath>

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
    int expValue = 9;
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
            // Increase dictionary size to a cap of 65536
            if (dictionary.size() < pow(2, expValue))
            {
                dictionary[wc] = dictSize++;
                if (dictionary.size() >= pow(2, expValue) && expValue < 16)
                {
                    ++expValue;
                    std::cout << "Increased code length to: " << pow(2, expValue) << "\n";
                }
            }
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
    int expValue = 9;
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
        if (dictionary.size() < pow(2, expValue))
        {
            dictionary[dictSize++] = w + entry[0];
            if (dictionary.size() >= pow(2, expValue) && expValue < 16)
            {
                ++expValue;
            }
        }

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

int main(int argc, char **argv)
{
    if (argc != 3 || (argv[1][0] != 'c' && argv[1][0] != 'e'))
    {
        std::string argvIndex1(argv[1]);
        if (argc != 4 || argvIndex1 != "diff")
        {
            std::cout << "Options:\n";
            std::cout << "Usage: " << argv[0] << " diff <filename> <filename>.2M\n";
            std::cout << "Usage: " << argv[0] << " c|e <filename>\n";
        }
        else
        {
            std::string filename1 = argv[2];
            std::string filename2 = argv[3];
            std::ifstream inputFile1, inputFile2;
            std::string input1, input2;
            inputFile1.open(filename1 + ".txt");
            inputFile2.open(filename2);
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
                    return 0;
                }
                else if (inputFile1.is_open())
                {
                    std::cout << "Error: Could not open file " << filename2 << "\n";
                    return 0;
                }
                else
                {
                    std::cout << "Error: Could not open files " << filename1 << ".txt & " << filename2 << "\n";
                }
            }
        }
    }
    else
    {
        std::string filename = argv[2];
        std::ifstream inputFile;
        std::ofstream outputFile;
        std::vector<int> compressed;
        std::vector<int> compressedCheck;

        if (argv[1][0] == 'c')
        {
            // write file to string
            std::string input;
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
                return 0;
            }
            inputFile.close();

            // compress file to .lzw2
            std::cout << "Compressing " << filename << " to " << filename << ".lzw2\n";
            outputFile.open(filename + ".lzw2");
            if (outputFile.is_open())
            {
                compress(input, std::back_inserter(compressed));
                for (auto itr = compressed.begin(); itr != compressed.end(); itr++)
                {
                    outputFile << *itr << "\n";
                }
                outputFile.close();
                std::cout << "Compression complete" << std::endl;
            }
            else
            {
                std::cout << "Error: Could not open file " << filename << ".lzw2\n";
                return 0;
            }
        }
        else
        {
            // decompress .lzw2 file
            std::string decompressed;
            std::cout << "Decompressing " << filename << "\n";
            inputFile.open(filename);
            if (inputFile.is_open())
            {
                while (!inputFile.eof())
                {
                    std::string line;
                    std::getline(inputFile, line);
                    if (is_number(line))
                        compressedCheck.push_back(std::stoi(line));
                }

                decompressed = decompress(compressedCheck.begin(), compressedCheck.end());
                std::cout << "Decompression complete" << std::endl;
            }
            else
            {
                std::cout << "Error: Could not open file " << filename << "\n";
                return 0;
            }

            filename.resize(filename.size() - 5);
            outputFile.open(filename + ".2M");
            if (outputFile.is_open())
            {
                outputFile << decompressed;
                outputFile.close();
            }
            else
            {
                std::cout << "Error: Could not open file " << filename << ".2M\n";
                return 0;
            }
        }
    }

    return 0;
}
