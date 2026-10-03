import ctypes
import hashlib
import json
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
    subprocess.run(['cc','-std=c11','-shared','-fPIC','-Imain','main/fortune_data.c',
                    'main/fortune_model.c','-o',str(lib)],cwd=ROOT,check=True)
    model = ctypes.CDLL(str(lib))
    model.fortune_decode.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_size_t]
    model.fortune_decode.restype = ctypes.c_bool
    for i, (_, _, text) in enumerate(records):
        out = ctypes.create_string_buffer(128)
        assert model.fortune_decode(i, out, len(out)) and out.value.decode() == text
chars = (ROOT/'assets/fonts/fortune-characters.txt').read_text().rstrip('\n')
assert set(map(ord,chars)) == set(inventory()), 'Font inventory is stale'
assert len(records) >= 2001, 'The current milestone requires 2000+ independent texts'
assert len(records) >= document['production_minimum']
report = json.loads((ROOT/'assets/fortune/packing-report.json').read_text())
assert report['independent_records'] == len(records) and report['release_ready']
assert {s for _,s,_ in records} == {0,1,2}
assert {m for m,_,_ in records} == set(range(1,9))
print(f'Inventory: PASS ({len(records)} exact complete-string roundtrips; all styles/moods; production gate enforced)')
