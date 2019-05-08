#include <gtest/gtest.h>

#include "GraphNode.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "HDWallet.h"

// TEST(PublicKeyTest, random_stuff)
// {
// 	for(int i = 0; i < 10; ++i)
// 	{
// 		PrivateKey secret;

// 		PublicKey Key(secret.SetPublicKey());
// 		Key.Print();	
		
// 		std::string buffer;
// 		ser::Serialize(Key, buffer);

// 		//awkward deserialization process
// 		PublicKey Dummy; 
// 		ser::Deserialize(Dummy, buffer);
// 		PublicKey Deserialized (Dummy.get_public_key());
// 		Deserialized.Print();
// 	}
// }

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cout << "Usage: -port -Passphrase" << std::endl;
		return 1;
	}

	GraphNode node(std::atoi(argv[1]), argv[2]);
	node.run_connection_process();
	// std::string passphrase = "wasd";
	// HDWallet wallet(passphrase);
	// wallet.derive_child();
	// wallet.derive_child();
	// wallet.derive_child();
	// wallet.derive_child();
	// std::cout << "Children count: " << wallet.children.size() << std::endl;;

	// HDWallet wallet("Passphrase");
	// wallet.PrintInfo();

	// PrivateKey secret;

	// PublicKey Key(secret.SetPublicKey());
	// Key.Print();	
	
	// std::string buffer;
	// ser::Serialize(Key, buffer);

	// //awkward deserialization process
	// PublicKey Dummy; 
	// ser::Deserialize(Dummy, buffer);
	// PublicKey Deserialized (Dummy.get_public_key());
	// Deserialized.Print();

	// testing::InitGoogleTest(&argc, argv);
	// return RUN_ALL_TESTS();
}
