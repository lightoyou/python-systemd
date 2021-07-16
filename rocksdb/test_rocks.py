import rocksdb


db = rocksdb.DB("test.log")












db.put(b"DATA", b"this is data db")

print(db.get(b"DATA"))
