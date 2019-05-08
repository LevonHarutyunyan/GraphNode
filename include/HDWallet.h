#ifndef _HD_WALLET_H__
#define _HD_WALLET_H__

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <cstring>
#include <random>
#include <cmath>
#include <algorithm>

#include <openssl/rand.h>
#include <boost/lexical_cast.hpp>

#include "Serialize.h"
#include "PrivateKey.h"
#include "PublicKey.h"

#include "utils.h"

struct child_key
{
	child_key(const child_key &rhs) = delete;
	child_key &operator=(const child_key &rhs) = delete;
	child_key();
	child_key(child_key&& child);
	
	PrivateKey private_key;
	PublicKey public_key;
	//chain code is a seed used to produce children ( ͡° ͜ʖ ͡°)
	std::vector<unsigned char> chain_code;

	int index;
};

class HDWallet
{

private:

	std::vector<std::string> mnemonic_words;
	std::string passphrase; // optional, used for more security

	PrivateKey master_private_key;
	
	//used to create children
	std::vector<unsigned char> master_chain_code;
	std::vector<child_key> children;
	
	void generate_mnemonic_words();
	void generate_master_key();

public:
	PublicKey master_public_key;

	HDWallet(const std::string& passphrase = 0);
	//In case user lost his wallet
	HDWallet(const std::vector<std::string>& mnemonic_words, const std::string& passphrase = 0);
	void derive_child();
	
	std::string serialize();
	HDWallet deserialize();

	void PrintInfo();
};


#endif //_HD_WALLET_H__