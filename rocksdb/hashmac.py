import hmac
import hashlib 
import binascii

def create_sha256_signature(key: str, message: str):
    byte_key =  bytes.fromhex(key)
    message = message.encode()
    return hmac.new(byte_key, message, hashlib.sha256).hexdigest().upper()

hmac = create_sha256_signature("E49756B4C8FAB4E48222A3E7F3B97CC3", "TEST STRING")

print(hmac)


