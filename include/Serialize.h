#ifndef _SERIALIZE_H__
#define _SERIALIZE_H__

#include <math.h>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

namespace ser
{

template<class T>
static void Serialize(const T& object, std::string& buffer)
{
	std::ostringstream archive_stream(buffer);
	boost::archive::text_oarchive output_archive(archive_stream); 
	output_archive << object;
	
	buffer = archive_stream.str();
}

template<class T>
static void Deserialize(T& object, std::string& buffer) 
{
	std::istringstream archive_stream(buffer);
	boost::archive::text_iarchive input_archive(archive_stream);

	input_archive >> object;
}

} //ser

#endif // _SERIALIZE_H__
