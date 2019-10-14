"""
Generates deterministic test vectors for ecdsa secp256k1 using the python 
ecdsa library.

To install the ecdsa library, run::

    $ pip3 install ecdsa

"""

import ecdsa
from ecdsa import SigningKey, SECP256k1
from binascii import hexlify, unhexlify


def main():
    msg = b"jolt wallet"

    sk = SigningKey.from_string(
            unhexlify("1294d9f787ad6274b3814badd3871966ed6c11cd684feff1515f4014827a2d9e"),
            curve=SECP256k1)
    vk = sk.get_verifying_key()

    signature = sk.sign_deterministic( msg, sigencode=ecdsa.util.sigencode_der )
    assert vk.verify(signature, msg, sigdecode=ecdsa.util.sigdecode_der )

    print("Secret Key: %s" % hexlify(sk.to_string()))
    print("Public Key: %s" % hexlify(vk.to_string()))
    print("Message: \"%s\"" % msg)
    print("Signature of (len %d): %s" % (len(signature), hexlify(signature)))

    # Verifying some other signature
    signature = unhexlify("3044022021B9A6BA074124A78D7A42DEA09677DB0D"
                          "22132CC163F493C3B0AEC359801DAD022060A4AA2A"
                          "EF1CE818CCD7319DBEB61C7C5F671EA7C10B99F79F"
                          "770F51C315A9B8")
    assert vk.verify(signature, msg, sigdecode=ecdsa.util.sigdecode_der )

if __name__ == "__main__":
    main()
