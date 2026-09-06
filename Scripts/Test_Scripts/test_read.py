def readNextFrame(data):
    outLen = 0
    buf = bytearray(len(data))
    pos = 0
    
    # simulate file
    def read(n):
        nonlocal pos
        res = data[pos:pos+n]
        pos += len(res)
        return res
    def seek(p):
        nonlocal pos
        pos = p
        
    while pos < len(data):
        c = read(1)
        if not c: break
        if c[0] == 0xFF:
            c2 = read(1)
            if not c2: break
            if c2[0] == 0xD8:
                buf[0] = 0xFF
                buf[1] = 0xD8
                outLen = 2
                break
            else:
                seek(pos - 1)
                
    if outLen == 0: return False, 0, pos
    
    while pos < len(data) and outLen < len(buf):
        to_read = min(4096, len(buf) - outLen)
        chunk = read(to_read)
        if not chunk: break
        
        start_search = outLen - 1 if outLen > 2 else 2
        actually_read = len(chunk)
        buf[outLen:outLen+actually_read] = chunk
        outLen += actually_read
        
        for i in range(start_search, outLen - 1):
            if buf[i] == 0xFF and buf[i+1] == 0xD9:
                frameLen = i + 2
                over_read = outLen - frameLen
                if over_read > 0:
                    seek(pos - over_read)
                outLen = frameLen
                return True, outLen, pos

    return False, outLen, pos

data = b"junk\xff\xd8hello world\xff\xd9more junk\xff\xd8frame2\xff\xd9end"
ok, outLen, pos = readNextFrame(data)
print("1:", ok, data[pos:])
ok, outLen, pos = readNextFrame(data[pos:])
print("2:", ok)
