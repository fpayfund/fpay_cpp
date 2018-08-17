#include <getopt.h>
#include "ecc_helper.h"
#include "common/byte.h"
#include <string>

using namespace std;
void transferIntFromBase58(const string& base58)
{
	unsigned char bin[32] = {0};
	KeyFromBase58(base58,bin);
	DumpHex(bin,32);
}
	

void createAddress()
{
	EC_KEY* pkey = ECKey_new();
	Byte32 private_key;
	ECKey_GeneratePrivKey(pkey,private_key.u8);

    uint32_t key_size = ECKey_GenKeypair(pkey, private_key.u8);

    Byte32 public_key;
	unsigned char pubkey[65];
	uint32_t pubKeySize = ECKey_GetPubkey(pkey, pubkey, 1);

	for( uint32_t i = 0; i < 32; i++ ) {
		public_key.u8[i] = pubkey[i+1];
	}

    string address = PubkeyToBase58Address(public_key.u8, 32);
    fprintf(stderr,"a set:\n");
	fprintf(stderr,"address:%s\n",address.c_str());	    
	string sPrivKey = KeyToBase58(private_key.u8);
	fprintf(stderr,"private key :%s\n",sPrivKey.c_str());
	string sPubKey = KeyToBase58(public_key.u8);
    fprintf(stderr,"public key :%s\n",sPubKey.c_str());	
}


void createRandInit()
{
	unsigned char bin[32] = {0};
	ECKey_Rand(bin,32);
    string sBin = KeyToBase58(bin);
	fprintf(stderr,"rand 256bit int:%s\n",sBin.c_str());
}


int main(int argc,char* argv[])
{

	int oc;                 

	while((oc = getopt(argc, argv, "hrac:")) != -1)
	{
		switch(oc)
		{
			case 'h':
				fprintf(stderr,"create rand 256bit int,or create address\n");
				return 0;	
			case 'r':
				createRandInit();
				return 0 ;
			case 'a':
				createAddress();
			    return 0;
			case 'c':
				string base58 = optarg;
				transferIntFromBase58(base58);
				return 0;
		}
	}

	return 0;
}
