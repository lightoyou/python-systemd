state = "005fa133d23c2bd2e397067f83a7697175ecbdf997e0bdb357e437d7b5470e1597927ee19e0ee4250c7b2f9435026fb50d5a30a5f1a1947fd412422b8c61805d7156ad70b42fb9944ec394743f711dfaff9bbecd84334914045b2600a5ce4a643373a6773fe81b548e04ffa1288508447b609610649bbae6efa9328e5fe98e0367a7047a2f49d8d4959ad5c79f3fd5166e4623c56fe286ad6dab0511e3ae1aee315eff2fa7f8688cf91626c85d4093456e0e1ab76a049fb75347d54e360fcbddba891450795b874ebf55064a2af6e895269e16e8719bc7fe49735d61ea60ef78cc2235b320e6c981fb63702408a55be6f4d402ddb3e353eb28eeb1dc50f9c2f1b3e509e70bd8b46fb594b6bdb3754f862dd7c17206f5ba54db35be8e82b4a8b6f689b50734659c7ce47b5be8dd933e5bf29f3e6ab18a7e8cdf1eaa0c3f71200f075daad33001cef053a3dd6405cb333c247c7fd4e9a3ddbc062c35103f65c411cc9d488df5c8c81754f415aa22ef477b320219a24e01a2fa2280a85e80d161b95079000000000000017b"

n = 2
statehex = [state[i:i+n] for i in range(0, len(state), n)]
print(''.join(statehex))

for elem in statehex:
	print(int(elem, 16))


