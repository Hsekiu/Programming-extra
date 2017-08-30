#pragma once

#include<iostream>
#include<iomanip>
using namespace std;

// A hash table class for mapping strings to ints.
// Note, this could be templated to allow mapping anything to anything.

class HashTable {
 public:
	// Insert a (key,value) pair into hash table.  Returns true if successful, false if not.
	bool insert(string key, int value); 

	bool remove(string key, int value);

	// Lookup a key in hash table.  Copies value to value if found and returns true, returns false if key not in table.
	bool lookup(string key, int& value);

	// Modify a (key,value) pair in hash table.  Changes value to value if found and returns true, returns false if key not in table.
	bool modify(string key, int value);

	// Return an array of all the keys in the table.  Stores these nkeys in array keys.
	void getKeys(string*& all_keys, int& nkeys);

	// Return the number of (key,value) pairs currently in the table.
	int numStored();

	// Create a default sized hash table.
	HashTable();

	// Create a hash table that can store nkeys keys (allocates 4x space).
	HashTable(int nkeys);
	~HashTable();

	// Print the contents of the hash table data structures, useful for debugging.
	void printTable();

 private:
	int tsize;  // size of hash table arrays
	int nstored;  // number of keys stored in table
	string *keys;
	int *values;
	int *sentinels; // 0 if never used, 1 if currently used, -1 if previously used.
	static const int curr_used = 1;
	static const int never_used = 0;
	static const int prev_used = -1;

	static const int default_size = 10000;  // Default size of hash table.


	// Probing function, returns location to check on iteration iter starting from initial value val.
	int probeFunction(int val, int iter);  
	void init(int tsizei);


	int hash(string key);
	// A couple of hash functions to use.
	int sillyHash(string key);
	int smarterHash(string key);

};

