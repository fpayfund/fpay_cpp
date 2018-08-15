#include "ecc_helper.h"
#include "common/byte.h"

int main(int argc,char* argv[])
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
	string sPrivKey = BinAddressToBase58(private_key.u8,32);
	fprintf(stderr,"private key :%s\n",sPrivKey.c_str());
	string sPubKey = BinAddressToBase58(public_key.u8,32);
    fprintf(stderr,"public key :%s\n",sPubKey.c_str());	


	return 0;
}
