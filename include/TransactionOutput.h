#ifndef _TRANSACTION_OUTPUT_H__
#define _TRANSACTION_OUTPUT_H__

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <sstream>
#include <string>

#include "PublicKey.h"

class Transaction_Output
{

private:

	int value;
	PublicKey pub_key;

public:

	Transaction_Output(const PublicKey& pub_key);

	std::string Serialize();
	static Transaction_Output Deserialize(
				const std::string& serialized);

};

#endif //_TRANSACTION_OUTPUT_H__