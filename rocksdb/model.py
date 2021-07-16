from pydantic import BaseModel

class Header(BaseModel):
    file_id
    machine_id
    seqnum_id
    n_objects
    n_data
    n_tags

class Data(BaseModel):
    seqnum
    hash
    size
    realtime
    payload
    previous_hash

class Tag(BaseMode):
    seqnum
    epoch
    tag
