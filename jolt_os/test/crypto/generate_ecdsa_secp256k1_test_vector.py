"""
Generates deterministic test vectors for ecdsa secp256k1 using the python 
ecdsa library.

To install the ecdsa library, run::

    $ pip3 install ecdsa

"""

import ecdsa
from ecdsa import SigningKey, SECP256k1
from binascii import hexlify, unhexlify
from hashlib import sha256

def main():
    msg = b"jolt wallet"

    sk = SigningKey.from_string(
            unhexlify("1294d9f787ad6274b3814badd3871966ed6c11cd684feff1515f4014827a2d9e".zfill(64)),
            curve=SECP256k1)
    vk = sk.get_verifying_key()

    digest = sha256(msg).digest()

    signature = sk.sign_digest_deterministic( digest, hashfunc=sha256, sigencode=ecdsa.util.sigencode_der )
    assert vk.verify_digest(signature, digest, sigdecode=ecdsa.util.sigdecode_der )

    print("Secret Key: %s" % hexlify(sk.to_string()))
    print("Public Key: %s" % hexlify(vk.to_string()))
    print("Message: \"%s\"" % msg)
    print("Digest: %s" % hexlify(digest))
    print("Signature of (len %d): %s" % (len(signature), hexlify(signature)))

if __name__ == "__main__":
    main()
