#include "HDWallet.h"

child_key::child_key()
{}


child_key::child_key(child_key&& child)
{
	this->private_key = std::move(child.private_key);
	this->public_key = std::move(child.public_key);
	this->chain_code = std::move(child.chain_code);
	this->index = std::move(child.index);


	// this->private_key.assign(std::begin(child.private_key),
	//    	     	              std::end(child.private_key));
	// this->public_key.assign(std::begin(child.public_key),
	// 	     	              std::end(child.public_key));
	
	// this->chain_code = child.chain_code;

	// child.chain_code.clear();
}




//For debugging purposes only
void PrintVec(const std::vector<unsigned char>& data)
{
	for (int i = 0; i < data.size(); ++i)
	{
		std::cout << data[i];
	}
	std::cout << std::endl;
}

HDWallet::HDWallet(const std::string& passphrase)
		: passphrase(passphrase)
{
	generate_mnemonic_words();
	generate_master_key();
}

HDWallet::HDWallet(const std::vector<std::string>& mnemonic_words,
				 	const std::string& passphrase)
		: mnemonic_words(mnemonic_words)
		, passphrase(passphrase)
{
	generate_master_key();
	// TO DO
	// Probably need to generate childs too, but don't know how better implement that
}

void HDWallet::generate_mnemonic_words()
{
	// std::cout << "Entered generate_mnemonic_words" << std::endl;

	//Creating random sequence
    unsigned char seed[32] = { 0 };
    RAND_bytes(seed, 32);
    std::vector<unsigned char> payload(seed, seed + 32);
    PrivateKey dummy = PrivateKey(payload);
    payload = dummy.get_private_key();

    // std::cout << "Payload: ";
    	// PrintVec(payload);

    //Dividing the sequence into sections of 11 bits
    //Actually doing other thing, whatever
    //TO DO  implement actual mnemonic words as it should be

    //Dividing seed into 
    for(int i = 0; i < payload.size(); i += 3)
    {
    	int mapped_number = payload[i] * 4 + payload[i] * 2 + payload[i]; // just random multiplications
		// std::cout << "Mapped number: " << mapped_number << std::endl;
		std::string mnemonic_word = mnemonic_words_list[mapped_number];
		// std::cout << "New word: " << mnemonic_word << std::endl;
		this->mnemonic_words.push_back(mnemonic_word);
    }
	// std::cout << "Finished generate_mnemonic_words" << std::endl;
}

void HDWallet::generate_master_key()
{
	// std::cout << "Entered generate_master_key" << std::endl;
	std::string salt = "mnemonic" + this->passphrase;
	std::string mnemonics;
	for (int i = 0; i < this->mnemonic_words.size(); ++i)
	{
		mnemonics += this->mnemonic_words[i];
	}
	// std::cout << "Mnemonics: " << mnemonics << std::endl;
	mnemonics += salt;

	//Getting seed from mnemonic words + salt
	//Actualy there needed to be loop of PBKDF2 using CHMAC_SHA512 2048 times
	//Making our seed 32 bytes, by XOR-ing pre_seed bytes
	std::vector<unsigned char> seed(32);
	for (int i = 0; i < mnemonics.length(); ++i)
	{
		seed[i % 32] ^= mnemonics[i];
	}
	// std::cout << "Seed: "; 
			// PrintVec(seed);

	//Left part of seed is private_key, right part - chain code
	this->master_private_key = PrivateKey(seed);

	//Because of size of our key(32 bytes), we are hashing seed again to create master_chain_code(withs SHA256)  
	this->master_chain_code = PrivateKey::Hash(seed);

	//At last creating public key from privare key
 	 						
	this->master_public_key = this->master_private_key.SetPublicKey();  

	// std::cout << "master_private_key: " << std::endl;
				// this->master_private_key.Print();
	// std::cout << "master_public_key: " << std::endl;
				// this->master_public_key.Print();
	// std::cout << "master_chain_code: ";
				// PrintVec(this->master_chain_code);
	// std::cout << "Finished generate_master_key" << std::endl;
}

void HDWallet::derive_child()
{
	// std::cout << "Entered derive_child" << std::endl;
	//Setting arguments for CHMAC_SHA512
	std::vector<unsigned char> seed(32);
	//XOR-ing stuff together
	std::vector<unsigned char> vector_priv_key =
					this->master_private_key.get_private_key();
	std::vector<unsigned char> vector_pub_key = 
					this->master_public_key.get_public_key();
	for (int i = 0; i < sizeof(seed); ++i)
	{
		seed[i] = vector_priv_key[i] ^ vector_pub_key[i] ^ this->master_chain_code[i];	
	}	

	//Creating child
	child_key child;

	/*private key*/
	child.private_key = PrivateKey(seed);
	/*public key*/
	child.chain_code = PrivateKey::Hash(seed);
	/*chain code*/
	child.public_key = child.private_key.SetPublicKey();  
	/*index*/
	child.index = this->children.size();

	// child.private_key = PrivateKey(buffer);
	// child.public_key = child.private_key.get_public_key();
	// Scalar().hash(buffer, sizeof(buffer));
	// std::copy(hashed_seed, hashed_seed + sizeof(hashed_seed), this->master_chain_code);
	
	this->children.push_back(std::move(child));
	// std::cout << "Finished derive_child" << std::endl;
}

//For debugging
void HDWallet::PrintInfo()
{
	std::cout << "^^^^^^^^^Start^^^^^^^^^^" << std::endl;
	std::cout << "This HDWallet contains: "  << std::endl; 
	std::cout << "Master Private Key: " << std::endl; 
				this->master_private_key.Print();
	std::cout << "Master Public Key: " << std::endl;
				this->master_public_key.Print();
	std::cout << "Master Chain Code: ";
				PrintVec(this->master_chain_code);
	std::cout << "^^^^^^^^^End^^^^^^^^^^^^" << std::endl;
}
