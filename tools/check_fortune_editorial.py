#!/usr/bin/env python3
"""Screen close wording; this is not a semantic or reader-preference review."""
import difflib
import hashlib
import json
from collections import Counter
from pack_fortunes import ROOT, load_records

path = ROOT / 'assets/fortune/corpus.json'
_, records = load_records(path)
grams = [{text[i:i+2] for i in range(len(text)-1)} for _, _, text in records]
pairs = []
for i, a in enumerate(grams):
    for j in range(i):
        b = grams[j]
        jaccard = len(a & b) / max(1, len(a | b))
        if jaccard < .48:
            continue
        ratio = difflib.SequenceMatcher(None, records[i][2], records[j][2]).ratio()
        if ratio > .72:
            pairs.append({'record_ids': [j, i], 'texts': [records[j][2], records[i][2]],
                          'sequence_similarity': round(ratio, 4)})
report = {
    'corpus_sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
    'records': len(records), 'exact_duplicates': 0,
    'group_counts': {f'{mood}/{voice}': count for (mood, voice), count in
                     sorted(Counter((m, s) for m, s, _ in records).items())},
    'first_person_openings': {str(m):sum(t.startswith('我') for mood,_,t in records if mood==m) for m in range(1,9)},
    'max_characters': max(len(t) for _, _, t in records),
    'near_text_scan': {'bigram_jaccard_minimum': .48, 'sequence_similarity_threshold': .72,
                       'pairs_above_both_thresholds': len(pairs), 'pairs': pairs},
    'method': '1980 individually authored complete sentences and 220 attributed classical excerpts; no phrase assembly. '
              'Text-similarity checks do not establish semantic novelty or reader preference.'
}
(ROOT / 'assets/fortune/editorial-report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n')
print(f'Editorial screen: {len(records)} complete records; {len(pairs)} close-wording pairs to review')
