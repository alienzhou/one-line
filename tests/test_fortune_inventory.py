import ctypes
import hashlib
import json
import re
from collections import Counter
import subprocess
import sys
import tempfile
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
                    'main/fortune_model.c','-o',str(lib)],cwd=ROOT,check=True)
    model = ctypes.CDLL(str(lib))
    model.fortune_decode.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_size_t]
    model.fortune_decode.restype = ctypes.c_bool
    _, legacy = load_records(ROOT / 'assets/fortune/legacy-corpus.json')
    for i, (_, _, text) in enumerate(records + legacy):
        out = ctypes.create_string_buffer(128)
        quote = i if i < len(records) else 0x80000000 | (i-len(records))
        assert model.fortune_decode(quote, out, len(out)) and out.value.decode() == text
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
