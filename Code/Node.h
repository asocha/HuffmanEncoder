#ifndef NODE_H
#define	NODE_H

/**
 * @author Andrew
 *
 * Creates a node for the binary tree
 */
struct node{
    public:
        /**
         * Node constructor
         * 
         * @param byte the character stored at this node (upgraded from char to short, such that -1 indicates the EOF character, -2 indicates internal nodes, and -3 indicates the root)
         * @param freq the frequency of that character
         */
        node(short byte, int freq){character = byte; frequency = freq;}
        
        short character;
        int frequency;
        node* left = 0;
        node* right = 0;
};

#endif	/* NODE_H */

