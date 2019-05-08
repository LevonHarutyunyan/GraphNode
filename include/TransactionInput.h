#ifndef _TRANSACTION_INPUT_H__
#define _TRANSACTION_INPUT_H__

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <string> 

class TransactionInput
{
private:

	std::string prev_output_hash;
	unsigned int prev_output_index;
	std::string signature;

public:

	unsigned int* hash();
	std::string Seserialize() const;
	static TransactionInput Deserialize(const std::string& serialized);
};

#endif //_TRANSACTION_INPUT_H__