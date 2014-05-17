#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "Node.h"

using namespace std;

void encode(FILE* file);
void decode();
void searchTree(node* aNode, vector<string>* bits, vector<short>* chars, string currentBits, FILE* huffman);
void deleteTree(node* aNode);

/** 
 * @author Andrew
 *
 * Implements a Huffman compressor/decompressor.
 */
int main(int argc, char** argv) {
    FILE* file = fopen("Input Text.txt", "rb");
    if (file){
        encode(file);
        decode();
    }
    else{
        cout << "Couldn't open input file." << endl;
    }
    return 0;
}

/**
 * Huffman encode a file. First, creates a frequency table and then converts that to a frequency tree.
 * Then, uses the tree to create a conversion table from characters to bits. Finally,
 * uses the table to encode the file.
 * 
 * @param file The file to encode
 */
void encode(FILE* file){
    short input;
    vector<short> chars;
    vector<int> frequencies;
    
    //create the frequencies table
    while(!feof(file)){
        input = fgetc(file);
        for (int i = 0; i < chars.size(); i++){
            if (chars[i] == input){ //found a character we've seen before
                frequencies[i]++;
                goto nextChar;
            }
        }
        //found a new character
        chars.push_back(input);
        frequencies.push_back(1);
nextChar: ;
    }
    
    //print the frequencies table
    cout << "Frequencies" << endl;
    for (int i = 0; i < chars.size(); i++){
        cout << char(chars[i]) << ": " << frequencies[i] << endl;
    }
    
    //create the nodes for the frequencies tree (sorted from highest frequency to least)
    vector<node*> nodes;
    for (int i = 0; i < chars.size(); i++){
        for (int j = 0; j <= nodes.size(); j++){
            if (j == nodes.size() || nodes[j]->frequency <= frequencies[i]){
                nodes.insert(nodes.begin()+j, new node(chars[i], frequencies[i]));
                break;
            }
        }
    }
    
    //create the frequencies tree
    while (nodes.size() > 1){
        node* left = nodes[nodes.size()-1];
        node* right = nodes[nodes.size()-2];
        
        //create a new node with the combined frequency of the 2 lowest frequency nodes
        node* newNode = new node(-2, left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        
        nodes.pop_back();
        nodes.pop_back();
        
        for (int j = 0; j <= nodes.size(); j++){
            if (j == nodes.size() || nodes[j]->frequency <= newNode->frequency){
                nodes.insert(nodes.begin()+j, newNode);
                break;
            }
        }
    }
    
    //create the table of bits for each character (also write the frequency tree to file)
    FILE* huffman = fopen("Huffman Text.txt", "wb");
    vector<string> bits;
    chars.clear();
    cout << endl << "Huffman Codes" << endl;
    nodes[0]->character = -3; //add a way to distinguish the root (used when reading the tree from file during decoding)
    searchTree(nodes[0], &bits, &chars, "", huffman);
    
    //translate the file to the Huffman code
    rewind(file);
    char byte;
    int bitCount = 0;
    while(!feof(file)){
        input = fgetc(file);
        for (int i = 0; i < chars.size(); i++){
            if (chars[i] == input){ //found the character in our char-to-bits table
                for (int j = 0; j < bits[i].size(); j++){
                    //move the bit-string into this byte one bit at a time
                    byte = byte << 1;
                    byte |= bits[i].at(j) - '0'; // -'0' to convert from characters '0' or '1' to bits 0 or 1
                    bitCount++;
                    
                    //when the byte is full of 8 bits, write it to file
                    if (bitCount == 8){
                        fwrite(&byte, 1, 1, huffman);
                        bitCount = 0;
                    }
                }

                break;
            }
        }
    }
    //write out the last few bits
    while (bitCount < 8){
        byte = byte << 1;
        bitCount++;
    }
    fwrite(&byte, 1, 1, huffman);
    
    fclose(huffman);
    fclose(file);
}

/**
 * Traverses the tree, converting it into a table of characters to Huffman codes
 * 
 * @param aNode         the current node of the tree, originally called on the root of the tree
 * @param bits          the list of bits in the chars-to-bits table, originally empty
 * @param chars         the list of chars in the chars-to-bits table, originally empty
 * @param currentBits   the current string of bits at this point in the tree, originally empty
 * @param huffman       the encoded file
 */
void searchTree(node* aNode, vector<string>* bits, vector<short>* chars, string currentBits, FILE* huffman){
    if (aNode->left){
        searchTree(aNode->left, bits, chars, currentBits + "0", huffman);
        searchTree(aNode->right, bits, chars, currentBits + "1", huffman);
    }
    else{
        //make the translation table
        chars->push_back(aNode->character);
        bits->push_back(currentBits);
        
        cout << char(aNode->character) << ": " << currentBits << endl; //print the translation table
    }
    fwrite(&aNode->character, sizeof(short), 1, huffman); //write the tree to file (only need to write the characters)
    delete aNode;
}

/**
 * Decode the Huffman-encoded file back to its original form.
 */
void decode(){
    FILE* huffman = fopen("Huffman Text.txt", "rb");
    
    //read the conversion tree from the file
    vector<node*> nodes;
    node inputNode(0, 0);
    while(!feof(huffman)){
        fread(&inputNode.character, sizeof(short), 1, huffman);
        if (inputNode.character > -2) nodes.push_back(new node(inputNode.character, 0)); //leaf node
        else{ //internal node
            node* internal = new node(0, inputNode.frequency);
            internal->right = nodes.at(nodes.size()-1);
            nodes.pop_back();
            internal->left = nodes.at(nodes.size()-1);
            nodes.pop_back();
            nodes.push_back(internal);
            
            if (inputNode.character == -3) break; //found the root of the tree, done reconstructing the tree
        }
    }
    
    //use the tree to decode the file
    FILE* decompressed = fopen("Decompressed Text.txt", "wb");
    char input;
    node* currentNode = nodes[0];
    while(!feof(huffman)){
        fread(&input, 1, 1, huffman);
        for (int i = 0; i < 8; i++){
            if ((input & 128) == 0) currentNode = currentNode->left;
            else currentNode = currentNode->right;
            input = input << 1;
            
            if (currentNode->left == 0){ //reached a leaf, write character to file and return to the root of the tree
                if (currentNode->character == EOF) goto finished;
                fwrite(&currentNode->character, sizeof(char), 1, decompressed);
                currentNode = nodes[0];
            }
        }
    }
finished:
    deleteTree(nodes[0]);
    fclose(huffman);
    fclose(decompressed);
}

/**
 * Delete a tree
 * 
 * @param aNode the current node of the tree, originally the root of the tree
 */
void deleteTree(node* aNode){
    if (aNode->left){
        deleteTree(aNode->left);
        deleteTree(aNode->right);
    }
    delete aNode;
}