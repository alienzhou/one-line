import ctypes
import hashlib
import json
import re
from collections import Counter
import subprocess
import sys
import tempfile
import struct
import zlib
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from pack_fortunes import load_records
from generate_fortune_fonts import inventory

document, records = load_records(ROOT / 'assets/fortune/corpus.json')
editorial = json.loads((ROOT/'assets/fortune/editorial-report.json').read_text())
assert editorial['corpus_sha256'] == hashlib.sha256((ROOT/'assets/fortune/corpus.json').read_bytes()).hexdigest(), 'Rerun editorial screen after content changes'
assert editorial['near_text_scan']['pairs_above_both_thresholds'] == 0, 'Review close-wording records'
with tempfile.TemporaryDirectory() as tmp:
    lib = Path(tmp) / 'model.dylib'
    subprocess.run(['cc','-std=c11','-shared','-fPIC','-Imain','main/fortune_data.c','main/fortune_legacy_data.c',
                    'main/fortune_previous_data.c','main/fortune_model.c','-o',str(lib)],cwd=ROOT,check=True)
    model = ctypes.CDLL(str(lib))
    model.fortune_decode.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_size_t]
    model.fortune_decode.restype = ctypes.c_bool
    _, legacy = load_records(ROOT / 'assets/fortune/legacy-corpus.json')
    for i, (_, _, text) in enumerate(records + legacy):
        out = ctypes.create_string_buffer(128)
        quote = i if i < len(records) else 0x80000000 | (i-len(records))
        assert model.fortune_decode(quote, out, len(out)) and out.value.decode() == text
    _, previous = load_records(ROOT / 'assets/fortune/previous-corpus.json')
    current_ids = {t:i for i,(_,_,t) in enumerate(records)}
    mapping = (ctypes.c_uint32*len(previous)).in_dll(model,'FORTUNE_PREVIOUS_MAP')
    class Card(ctypes.Structure):
        _fields_=[('quote',ctypes.c_uint32),('art',ctypes.c_uint32)]
    class State(ctypes.Structure):
        _fields_=[('corpus_id',ctypes.c_uint32),('seed',ctypes.c_uint32),('cursor',ctypes.c_uint32),
                  ('art_cursor',ctypes.c_uint32),('cycle',ctypes.c_uint32),('current',Card),('pinned',Card),
                  ('mood',ctypes.c_uint8),('style',ctypes.c_uint8),('seen',ctypes.c_uint8*275),
                  ('favorite_count',ctypes.c_uint8),('favorites',Card*16),
                  ('rare_random',ctypes.c_uint32),('rare_unlocked',ctypes.c_uint32),
                  ('rare_misses',ctypes.c_uint8),('rare_last',ctypes.c_uint8)]
    model.fortune_decode_state.argtypes=[ctypes.POINTER(State),ctypes.c_char_p,ctypes.c_size_t]
    model.fortune_decode_state.restype=ctypes.c_bool
    model.fortune_encode_state.argtypes=[ctypes.POINTER(State),ctypes.c_char_p,ctypes.c_size_t]
    model.fortune_encode_state.restype=ctypes.c_size_t
    def old_save(quote):
        data=bytearray(323)
        struct.pack_into('<10I',data,0,0x46544331,3335999125,42,2199,24580,3,quote,1122,quote,1122)
        data[40:42]=bytes([0,0])
        data[44:319]=bytes([0xff])*275
        struct.pack_into('<I',data,319,zlib.crc32(data[:319]))
        return bytes(data)
    # Every current-device card migrates with its exact words/art and clears hidden voice.
    for old_id,(_,_,text) in enumerate(previous):
        state=State(); data=old_save(old_id)
        assert model.fortune_decode_state(ctypes.byref(state),data,len(data))
        assert state.style==3 and state.mood==0 and state.pinned.art==1122
        assert state.pinned.quote==mapping[old_id]
        assert state.favorite_count==1 and state.favorites[0].quote==state.pinned.quote
        assert state.favorites[0].art==1122
        out=ctypes.create_string_buffer(128)
        assert model.fortune_decode(state.pinned.quote,out,len(out)) and out.value.decode()==text
        if text in current_ids:
            assert mapping[old_id]==current_ids[text]
        else:
            assert (mapping[old_id]&0xc0000000)==0x40000000
        encoded=ctypes.create_string_buffer(467)
        assert model.fortune_encode_state(ctypes.byref(state),encoded,467)==467
        restored=State()
        assert model.fortune_decode_state(ctypes.byref(restored),encoded,467)
        assert restored.pinned.quote==state.pinned.quote
    previous_texts={r[2] for r in previous}
    expected_seen={i for t,i in current_ids.items() if t in previous_texts}
    assert {i for i in range(len(records)) if state.seen[i//8]&(1<<(i%8))}==expected_seen
    # A pin migrated from the earlier 2112-bank remains in that namespace.
    data=old_save(0x80000000|387); state=State()
    assert model.fortune_decode_state(ctypes.byref(state),data,len(data))
    assert state.pinned.quote==0x80000000|387
    for invalid in [2200,0x40000000,0xc0000001,0x80000000|2112]:
        data=old_save(invalid)
        assert not model.fortune_decode_state(ctypes.byref(state),data,len(data))
chars = (ROOT/'assets/fonts/fortune-characters.txt').read_text().rstrip('\n')
assert set(map(ord,chars)) == set(inventory()), 'Font inventory is stale'
assert len(records) >= 2001, 'The current milestone requires 2000+ independent texts'
assert len(records) >= document['production_minimum']
report = json.loads((ROOT/'assets/fortune/packing-report.json').read_text())
assert report['independent_records'] == len(records) and report['release_ready']
assert {s for _,s,_ in records} == {0,1,2}
assert {m for m,_,_ in records} == set(range(1,9))
assert [Counter(m for m,_,_ in records)[i] for i in range(1,9)] == [440,220,330,330,330,220,220,110]
assert document['theme_percentages'] == [20,10,15,15,15,10,10,5]
sources = json.loads((ROOT/'assets/fortune/poetry-sources.json').read_text())
assert len(sources) == 220 == len(document['citations'])
normalize = lambda s: re.sub(r'[^\w]','',s)
for source in sources:
    text = source['text']
    citation = document['citations'][text]
    assert normalize(text) in normalize(''.join(source['source_paragraphs']))
    assert all(citation[k] == source[k] for k in ['author','title','display','source_url'])
    assert source['source_revision'] in source['source_url'] and source['source_url'].startswith('https://github.com/chinese-poetry/chinese-poetry/blob/')
    assert (2,2,text) in records
for text, old_id in document['legacy_ids'].items():
    assert text == legacy[old_id][2] and (1,legacy[old_id][1],text) in records
assert len(document['legacy_ids']) == 440
assert document['original_records'] == 1980 and document['classical_excerpts'] == 220
print(f'Inventory: PASS ({len(records)} current / {len(legacy)} legacy exact roundtrips; eight theme proportions; 220 source-backed poetry excerpts; production gate enforced)')

# Guard the editorial direction: free themes must not revert to first-person declarations.
for theme in (4,6):
    assert not any(t.startswith('我') for m,_,t in records if m==theme)
assert sum(t.startswith('我') for _,_,t in records)<30
print('Balanced deck migration: PASS (all 2200 prior cards retained exactly, both older namespaces, changed content becomes unread; first-person openings below 30)')
