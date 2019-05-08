#include <iostream>
#include <random>
#include <cmath>
#include <algorithm>
#include <cstring>

#include <openssl/rand.h>

#include "Scalar.h"
#include "PrivateKey.h"
    
const unsigned int PRIVATE_KEY_STORE_SIZE = 32;
const unsigned int PRIVATE_KEY_SIZE = 279;
const unsigned int PUBLIC_KEY_SIZE = 65;
const unsigned int SIGNATURE_SIZE = 72;

PrivateKey::PrivateKey()
	: context(secp256k1_context_create(SECP256K1_CONTEXT_SIGN))
{
	// Randomize private key.
	do
	{
		unsigned char seed[32] = { 0 };
	    RAND_bytes(seed, 32);
	    this->private_key.assign(seed, seed + 32);
		std::cout << "PrivateKey(constructor) Seed: " << seed << std::endl;
	} while(!VerifyKey());

	// Calculate public key from private key.
	CalculatePublicKey(true);
}

PrivateKey::PrivateKey(const std::vector<unsigned char> &priv_key_data)
	: context(secp256k1_context_create(SECP256K1_CONTEXT_SIGN))
	, private_key(priv_key_data)
{
	// Calculate public key from private key.
	CalculatePublicKey(true);
}

PrivateKey::PrivateKey(const std::string& priv_key_data)
	: context(secp256k1_context_create(SECP256K1_CONTEXT_SIGN))
	, private_key(priv_key_data.begin(), priv_key_data.end())
{
	// Calculate public key from private key.
	CalculatePublicKey(true);
}

PrivateKey::PrivateKey(PrivateKey &&rhs) 
{
	this->private_key.assign(std::begin(rhs.private_key),
         	              std::end(rhs.private_key));
	
	this->public_key.assign(std::begin(rhs.public_key),
         	              std::end(rhs.public_key));
	this->context = rhs.context;

	rhs.private_key.clear();
	rhs.public_key.clear();
	rhs.context = nullptr;
}

PrivateKey::~PrivateKey()
{
	if(this->context)
	{
		secp256k1_context_destroy(this->context);
		this->context = nullptr;
	}
}

bool PrivateKey::VerifyKey() const
{
	return secp256k1_ec_seckey_verify(this->context, this->private_key.data());
}

PublicKey PrivateKey::SetPublicKey() const
{	
	return PublicKey(this->public_key);
}

bool PrivateKey::CalculatePublicKey(bool compressed)
{
	secp256k1_pubkey pubkey;
	int is_valid_key = secp256k1_ec_pubkey_create(this->context, &pubkey, this->private_key.data());
	if(!is_valid_key)
	{
		return false;
	}

	// Serialize public key.
	size_t output_length = PUBLIC_KEY_SIZE;
	this->public_key.resize(output_length);
	secp256k1_ec_pubkey_serialize(this->context, this->public_key.data(), &output_length, &pubkey,
									compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
	this->public_key.resize(output_length);


	return true;
}


PrivateKey &PrivateKey::operator=(PrivateKey &&rhs)
{
	this->private_key.assign(std::begin(rhs.private_key),
         	              std::end(rhs.private_key));
	
	this->public_key.assign(std::begin(rhs.public_key),
         	              std::end(rhs.public_key));
	this->context = rhs.context;

	rhs.private_key.clear();
	rhs.public_key.clear();
	rhs.context = nullptr;

	return *this;
}

/*For GTest*/
bool PrivateKey::operator==(const PrivateKey& other) const
{
	return this->private_key == other.private_key;
}

std::tuple<std::vector<unsigned char>, bool> PrivateKey::Sign(
									    const std::vector<unsigned char> &hash) const 
{
  // Make signature.
  secp256k1_ecdsa_signature signature;
  int is_valid = secp256k1_ecdsa_sign(this->context, &signature, hash.data(), this->private_key.data(),
                                 secp256k1_nonce_function_rfc6979, nullptr);
  if (!is_valid) 
  {
    // Failed to sign.
    return std::make_tuple(std::vector<unsigned char>(), false);
  }

  // Serialize signature.
  std::vector<unsigned char> signature_output(72);
  size_t signature_output_size = 72;
  is_valid = secp256k1_ecdsa_signature_serialize_der(
      this->context, signature_output.data(), &signature_output_size, &signature);
  if (!is_valid) {
    // Failed to serialize.
    return std::make_tuple(std::vector<unsigned char>(), false);
  }

  // Returns
  signature_output.resize(signature_output_size);
  return std::make_tuple(signature_output, true);
}

std::string PrivateKey::Hash(const std::string& data)
{
	unsigned char *SHA256(const unsigned char *d, size_t n,
 	     unsigned char *md);
	//Need to create and convert many things in order to get proper API


	size_t buffer_size = data.size();
	unsigned char buffer[32];
    SHA256((const unsigned char*)data.data(), buffer_size, buffer);

	std::string result(buffer, buffer + 32);

	return result;
}

unsigned char *SHA256(const unsigned char *d, size_t n,
      unsigned char *md);

std::vector<unsigned char> PrivateKey::Hash(const std::vector<unsigned char>& data)
{
	size_t buffer_size = data.size();
	unsigned char buffer[32];
    SHA1(data.data(), buffer_size, buffer);

    std::vector<unsigned char> result(buffer, buffer + 32);
    return result;
}


void PrivateKey::PrintKey(const std::vector<unsigned char>& key) const
{
	for (int i = 0; i < key.size(); ++i)
	{
		std::cout << key[i];
	}
	std::cout << std::endl;
}

void PrivateKey::Print() const
{
	std::cout << "Private Key ->";
	PrintKey(this->private_key);
	std::cout << "Private Key size: " << this->private_key.size() << std::endl;

	std::cout << "Public Key ->";
	PrintKey(this->public_key);
	std::cout << "Public Key size: " << this->public_key.size() << std::endl;
}


// template <typename Archive>
// void PrivateKey::serialize(Archive& ar, const unsigned int version) const
// {
// 	ar & this->context;
// 	ar & this->private_key;
// 	ar & this->public_key;
// }

// void PrivateKey::Serialize(const PrivateKey &privatekey, const std::string& buffer)
// {
// 	// make an archive
// 	std::ostringstream archive_stream(buffer);
// 	boost::archive::text_oarchive output_archive(archive_stream);
// 	output_archive << privatekey;

// 	std::cout << "In serialize: " << std::endl;
// 	std::cout << archive_stream.str();
// }

// void PrivateKey::Deserialize(const PrivateKey &privatekey, const std::string& buffer)
// {
// 	// make an archive
// 	std::istringstream archive_stream(buffer);
// 	boost::archive::text_oarchive input_archive(archive_stream);
// 	input_archive >> privatekey;

// 	std::cout << "In deserialize: " << std::endl;
// 	privatekey.Print();
// }
