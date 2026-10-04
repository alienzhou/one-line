#!/usr/bin/env python3
"""Integration checks against actual application input, LVGL and local storage."""
import json
import signal
from pathlib import Path
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/fortune_simulator'))
from server import BUILD, Engine, make_handler, ThreadingHTTPServer


class SimulatorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.engine = Engine(BUILD / 'fortune_simulator', self.temp.name, 42)

    def tearDown(self):
        self.engine.close(); self.temp.cleanup()

    def key(self, button, event='click'):
        return self.engine.action(dict(action='key', button=button, event=event))

    def test_batch_matches_real_button_sequence(self):
        self.engine.action(dict(action='batch'))
        batch = [h['id'] for h in self.engine.history]
        self.assertEqual(len(set(batch)), 20)
        self.assertEqual(self.engine.response()['history_counts'][2], 4)
        self.engine.action(dict(action='fresh', seed=42))
        for i in range(20):
            self.key('ok' if i == 0 else 'up'); self.key('ok') # Reveal skip.
        self.assertEqual(batch, [h['id'] for h in self.engine.history])
        self.assertEqual(self.engine.state['seen'], 20)
        self.assertTrue((Path(self.temp.name) / 'frame.bmp').read_bytes().startswith(b'BM'))

    def test_explicit_topic_confirm_cancel_and_restart(self):
        state = self.engine.action(dict(action='topic', topic=1))
        self.assertEqual((state['page'], state['topic'], state['topic_candidate']), (3, 0, 1))
        self.key('ok', 'long')
        self.assertEqual(self.engine.state['topic'], 0)
        self.engine.action(dict(action='topic', topic=1))
        self.engine.action(dict(action='batch'))
        self.assertTrue(all(h['theme'] == 1 for h in self.engine.history))
        self.key('ok'); pinned = self.engine.state['pinned']
        self.key('up', 'long'); self.key('down'); self.key('ok')
        self.assertEqual(self.engine.state['volume'], 70)
        self.engine.action(dict(action='reboot'))
        self.assertEqual(self.engine.state['topic'], 0)
        self.assertEqual(self.engine.state['pinned'], pinned)
        self.engine.close()
        self.engine = Engine(BUILD / 'fortune_simulator', self.temp.name, 999)
        self.assertEqual(self.engine.state['topic'], 0)
        self.assertEqual(self.engine.state['volume'], 70)
        self.assertEqual(self.engine.state['pinned'], pinned)
        self.assertEqual(self.engine.state['seen'], 20)
        self.key('up')
        self.assertEqual(self.engine.state['topic'], 0)
        self.assertNotIn(self.engine.state['current']['id'], [h['id'] for h in self.engine.history[:-1]])

    def test_http_export_validation_and_local_origin(self):
        server = ThreadingHTTPServer(('127.0.0.1', 0), make_handler(self.engine))
        thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
        base = f'http://127.0.0.1:{server.server_port}'
        try:
            req = urllib.request.Request(base + '/api/action',
                data=b'{"action":"batch"}', headers={'Content-Type': 'application/json'})
            with urllib.request.urlopen(req) as r: result = json.load(r)
            self.assertEqual(result['history_count'], 20)
            with urllib.request.urlopen(base + '/api/export') as r: export = json.load(r)
            self.assertEqual(export['history'], self.engine.history)
            self.assertTrue(export['simulator_state_hex'])
            req = urllib.request.Request(base + '/api/action', data=b'{"action":"fresh"}',
                                         headers={'Origin': 'https://example.com'})
            with self.assertRaises(urllib.error.HTTPError) as error: urllib.request.urlopen(req)
            self.assertEqual(error.exception.code, 403)
            self.assertEqual(len(self.engine.history), 20)
            req = urllib.request.Request(base + '/api/action', data=b'{"action":"topic","topic":9}')
            with self.assertRaises(urllib.error.HTTPError) as error: urllib.request.urlopen(req)
            self.assertEqual(error.exception.code, 400)
        finally:
            server.shutdown(); server.server_close(); thread.join()

    def test_close_when_interrupt_already_reached_child(self):
        self.engine.process.send_signal(signal.SIGINT)
        self.engine.close()
        self.assertIsNotNone(self.engine.process.poll())


if __name__ == '__main__':
    unittest.main()
