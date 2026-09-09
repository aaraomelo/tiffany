with open('j1939_signal_extract.erg') as f:
    lines = f.readlines()

pos = 0
labels = {}
for i, line in enumerate(lines):
    s = line.strip()
    if not s or s.startswith(';'):
        continue
    if s.startswith(':'):
        name = s[1:].split()[0].strip()
        labels[name] = pos
        print(f'Linha {i+1}: :{name} -> pos={pos}')
        continue
    parts = s.split()
    mnem = parts[0]
    if mnem in ('HALT', 'ADD', 'SUB', 'AND', 'OR', 'XOR', 'GOLD', 'CMP', 'NEGRO_OURO', 'ESQUILO', 'TROCA', 'VINCO'):
        pos += 1
    elif mnem in ('LOAD', 'STORE', 'LOADS', 'STORE_IND', 'INC'):
        pos += 3
    elif mnem in ('JMP', 'JZ', 'JNZ'):
        pos += 3
    else:
        print(f'Unknown: {mnem}')
print()
print('LE_len1 pos:', labels.get('LE_len1'))
print('LE_len2 pos:', labels.get('LE_len2'))
print('NA_ERR_CHECK pos:', labels.get('NA_ERR_CHECK'))