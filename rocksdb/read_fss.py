import struct
with open('/var/log/journal/927a64dd8de046a7ad5230a96a3c9439/fss', "rb") as f:
	data = f.read()
	signature = data[0:15]

	machine = data[16:32]
	print(hex(machine))
