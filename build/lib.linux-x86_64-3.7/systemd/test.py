import _fsprg
# etat 0 --> content in file fss
state = [0, 95, 231, 105, 80, 240, 130, 125, 121, 29, 209, 93, 94, 235, 173, 169, 205, 62, 71, 215, 231, 116, 8, 156, 103, 170, 148, 178, 53, 114, 14, 197, 49, 172, 31, 221, 27, 140, 146, 57, 9, 202, 82, 244, 83, 31, 171, 143, 41, 242, 178, 231, 213, 11, 145, 47, 66, 199, 168, 181, 79, 190, 27, 216, 215, 235, 82, 140, 162, 65, 84, 159, 130, 1, 14, 152, 77, 171, 230, 207, 40, 101, 98, 22, 36, 207, 240, 8, 125, 217, 191, 143, 156, 184, 145, 75, 200, 120, 216, 162, 10, 27, 176, 234, 19, 172, 1, 159, 197, 248, 207, 40, 112, 52, 81, 94, 227, 244, 95, 103, 194, 112, 52, 179, 110, 181, 195, 142, 142, 101, 15, 63, 0, 34, 113, 166, 161, 188, 152, 136, 118, 125, 59, 148, 103, 154, 233, 125, 19, 9, 212, 151, 173, 74, 191, 241, 18, 100, 154, 229, 44, 88, 187, 215, 170, 148, 146, 218, 206, 185, 162, 188, 2, 185, 207, 35, 81, 248, 175, 125, 181, 47, 140, 107, 93, 32, 63, 168, 86, 159, 150, 65, 75, 125, 192, 37, 62, 105, 88, 119, 150, 210, 16, 199, 57, 78, 203, 146, 213, 76, 17, 208, 105, 100, 108, 79, 232, 245, 73, 43, 138, 102, 38, 242, 168, 226, 9, 242, 35, 201, 213, 9, 119, 209, 9, 34, 189, 96, 102, 61, 160, 18, 59, 67, 240, 185, 141, 196, 51, 84, 161, 53, 156, 7, 74, 69, 87, 107, 31, 247, 122, 226, 54, 11, 13, 72, 91, 83, 56, 9, 128, 35, 251, 157, 233, 197, 133, 54, 39, 226, 140, 97, 109, 133, 172, 16, 213, 45, 241, 187, 33, 90, 239, 25, 119, 119, 210, 112, 120, 206, 82, 239, 2, 78, 30, 52, 96, 178, 35, 174, 163, 156, 139, 165, 149, 26, 202, 198, 75, 8, 8, 78, 1, 253, 213, 245, 241, 205, 17, 227, 253, 58, 194, 80, 18, 75, 16, 177, 85, 121, 253, 72, 89, 81, 211, 0, 53, 110, 85, 38, 15, 250, 25, 35, 55, 240, 165, 38, 22, 95, 9, 9, 215, 14, 10, 29, 138, 250, 158, 110, 45, 85, 13, 35, 57, 235, 131, 86, 55, 74, 33, 81, 67, 2, 237, 98, 0, 0, 0, 0, 0, 0, 0, 0]

_fsprg.get_key(state)
_fsprg.get_epoch(state)


print("--------------------------------------")
#rotation des clés
print(" ----- Evolve --------")

new = _fsprg.evolve(state)
_fsprg.get_key(new)
_fsprg.get_epoch(new)



# saut pour vérification
#state_900 = _fsprg.seek(state, 900, "a83e8b-5d792f-e8a118-6a47cb/1f0e7c2-2faf080")
#_fsprg.get_key(state_900)
#_fsprg.get_epoch(state_900)

print("---- Recover ------ ")
state_1 = _fsprg.seek(state, 1, "a83e8b-5d792f-e8a118-6a47cb/1f0e7c2-2faf080")

_fsprg.get_key(state_1)
_fsprg.get_epoch(state_1)

state_0 = _fsprg.seek(state_1, 0,  "a83e8b-5d792f-e8a118-6a47cb/1f0e7c2-2faf080")
_fsprg.get_key(state_0)
_fsprg.get_epoch(state_0)
