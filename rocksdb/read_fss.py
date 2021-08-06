import struct
import binascii
'''
struct FSSHeader {
        uint8_t signature[8]; /* "KSHHRHLP" */
        le32_t compatible_flags;
        le32_t incompatible_flags;
        sd_id128_t machine_id;
        sd_id128_t boot_id;    /* last writer */
        le64_t header_size;
        le64_t start_usec;
        le64_t interval_usec;
        le16_t fsprg_secpar;
        le16_t reserved[3];
        le64_t fsprg_state_size;
} _packed_;
'''
with open('/var/log/journal/927a64dd8de046a7ad5230a96a3c9439/fss', "rb") as f:
	f.seek(0, 0)
	#64 bits
	signature = f.read(8)
	print(signature)
	f.seek(8)
	
	compatible_flags = f.read(4)
	print(compatible_flags)
	f.seek(12)

	incompatible_flags = f.read(4)
	print(incompatible_flags)
	f.seek(16)


	machine_id = f.read(16)
	print(binascii.hexlify(machine_id))
	f.seek(32)


	boot_id = f.read(16)
	print(binascii.hexlify(boot_id))
	f.seek(48)
	

	header_size = f.read(8)
	print(int.from_bytes(header_size, byteorder='little'))
	f.seek(56)

	start_usec = f.read(8)
	print(int.from_bytes(start_usec, byteorder='little'))
	f.seek(64)
	
	interval_usec = f.read(8)
	print(int.from_bytes(interval_usec, byteorder='little'))
	f.seek(72)

	fsprg_secpar = f.read(2)
	print(int.from_bytes(fsprg_secpar, byteorder='little'))
	f.seek(74)

	reserved = f.read(2*3)
	f.seek(80)

	fsprg_state_size = f.read(8)
	fsprg_state_size = int.from_bytes(fsprg_state_size, byteorder='little')
	print(fsprg_state_size)
	f.seek(88)

	fsprg_state = f.read(fsprg_state_size)
	print(binascii.hexlify(fsprg_state))


	'''
	machine = data[16:32]
	print(hex(machine))


	boot_id = f.read(16)
	print(binascii.unhexlify(boot_id))
	f = f.seek(48)
	

	'''
