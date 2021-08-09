import _fsprg
import binascii
import sys
# etat 0 --> content in file fss
state = _fsprg.setup_key()

epoch = _fsprg.get_epoch(state)
print(f"epoch : {str(epoch)} ")
key = _fsprg.get_key(state)
print(f"key : {key}")
key = bytes.fromhex(key)


print("--------------------------------------")
#rotation des clés
print(" ----- Evolve --------")
first = _fsprg.evolve(state)
#new = _fsprg.evolve(first)

key = _fsprg.get_key(first)
epoch =  _fsprg.get_epoch(first)
print(f"The key is {key} for {epoch}")

# saut pour vérification
#state_900 = _fsprg.seek(state, 900, "a83e8b-5d792f-e8a118-6a47cb/1f0e7c2-2faf080")
#_fsprg.get_key(state_900)
#_fsprg.get_epoch(state_900)

print("---- Recover ------ ")
s = _fsprg.seek(first, 1, "a83e8b-5d792f-e8a118-6a47cb/1f0e7c2-2faf080")
key =  _fsprg.get_key(s)
epoch = _fsprg.get_epoch(s)
print(f"The key is {key} for {epoch}")
