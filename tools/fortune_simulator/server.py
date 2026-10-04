#!/usr/bin/env python3
"""Run the real firmware's desktop adapter on loopback, without USB access."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import secrets
import shutil
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import webbrowser

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
BUILD = ROOT / 'build/fortune-ui-host'


def build_bridge():
    cache = BUILD / 'CMakeCache.txt'
    values = {}
    if cache.exists():
        for line in cache.read_text().splitlines():
            if '=' in line and ':' in line.split('=', 1)[0]:
                values[line.split(':', 1)[0]] = line.split('=', 1)[1]
    cmake = shutil.which('cmake') or values.get('CMAKE_COMMAND')
    ninja = shutil.which('ninja') or values.get('CMAKE_MAKE_PROGRAM')
    if not all(p and Path(p).is_file() for p in (cmake, ninja)):
        # Keep optional desktop build tools local and leave shell settings untouched.
        env = ROOT / 'build/fortune-simulator/tooling'
        if not (env / 'bin/python3').exists():
            subprocess.run([sys.executable, '-m', 'venv', str(env)], check=True)
        subprocess.run([str(env / 'bin/python3'), '-m', 'pip', 'install',
                        'cmake==3.31.6', 'ninja==1.13.0'], check=True)
        cmake, ninja = str(env / 'bin/cmake'), str(env / 'bin/ninja')
    lvgl = Path(os.environ.get('FORTUNE_LVGL_PATH', ROOT / 'managed_components/lvgl__lvgl'))
    if not (lvgl / 'lvgl.h').exists():
        raise RuntimeError('LVGL is missing. Build the firmware once with ESP-IDF 5.5.3, then retry.')
    subprocess.run([cmake, '-S', str(ROOT / 'tests/fortune_ui_host'), '-B', str(BUILD),
                    '-G', 'Ninja', '-D', 'CMAKE_BUILD_TYPE=Release',
                    '-D', f'LVGL_PATH={lvgl}', '-D', f'CMAKE_MAKE_PROGRAM={ninja}'], check=True)
    subprocess.run([cmake, '--build', str(BUILD), '--target', 'fortune_simulator', '-j', '4'], check=True)
    return BUILD / 'fortune_simulator'


def source_identity():
    paths = [ROOT / 'main' / n for n in (
        'main.c', 'fortune_model.c', 'fortune_model.h', 'fortune_data.c', 'fortune_ui.c',
        'fortune_text.h', 'fortune_pixels.c', 'fortune_sound.c')]
    paths += [ROOT / 'assets/fonts' / f'fortune_font_{size}.c' for size in (12, 20)]
    digest = hashlib.sha256()
    for path in paths:
        digest.update(path.name.encode()); digest.update(path.read_bytes())
    return digest.hexdigest()


class Engine:
    def __init__(self, binary, directory, seed=None):
        self.directory = Path(directory); self.directory.mkdir(parents=True, exist_ok=True)
        self.lock = threading.RLock()
        self.log_file = (self.directory / 'backend.log').open('a')
        self.seed = secrets.randbits(32) if seed is None else seed
        self.process = subprocess.Popen([str(binary), str(self.directory / 'state.bin'),
            str(self.directory / 'frame.bmp'), str(self.directory / 'sound.wav'), str(self.seed)],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self.log_file, text=True, bufsize=1)
        self.identity = source_identity()
        self.history_path = self.directory / 'history.json'
        try:
            self.history = json.loads(self.history_path.read_text())
        except (FileNotFoundError, ValueError):
            self.history = []
        self.state = self._read(); self.last_tick = time.monotonic()
        self.topics = json.loads((ROOT / 'assets/fortune/corpus.json').read_text())['moods']

    def _read(self):
        while True:
            line = self.process.stdout.readline()
            if not line:
                raise RuntimeError('Simulator stopped; inspect build/fortune-simulator/backend.log.')
            if line.startswith('{'):
                return json.loads(line)
            self.log_file.write(line); self.log_file.flush()

    def command(self, command):
        with self.lock:
            self.process.stdin.write(command + '\n'); self.process.stdin.flush()
            self.state = self._read()
            if self.state['drawn']:
                entry = dict(self.state['current'], scope=self.state['topic'],
                             cycle=self.state['cycle'], seed=self.state['seed'])
                self.history.append(entry)
                self.history_path.write_text(json.dumps(self.history, ensure_ascii=False))
            return self.state

    def tick(self):
        now = time.monotonic(); elapsed = min(500, int((now-self.last_tick)*1000))
        if elapsed >= 16:
            self.last_tick = now; self.command(f'tick {elapsed}')

    def response(self):
        with self.lock:
            self.tick()
            counts = [sum(h['theme'] == i for h in self.history) for i in range(9)]
            return dict(self.state, topics=self.topics, history=self.history[-100:],
                        history_count=len(self.history), history_counts=counts,
                        source_identity=self.identity)

    def action(self, body):
        with self.lock:
            self.tick(); action = body.get('action')
            if action == 'key':
                button, event = body.get('button'), body.get('event', 'click')
                if button not in ('up', 'down', 'ok') or event not in ('click', 'long'):
                    raise ValueError('Invalid button or event')
                self.command(f'key {button} {event}')
            elif action == 'topic':
                topic = body.get('topic')
                if type(topic) is not int or not 0 <= topic <= 8:
                    raise ValueError('Invalid topic')
                self.command(f'topic {topic}')
            elif action == 'batch':
                for _ in range(20):
                    self.command('draw')
            elif action == 'reboot':
                self.command('reboot')
            elif action == 'fresh':
                seed = body.get('seed')
                if seed is None: seed = secrets.randbits(32)
                if type(seed) is not int or not 0 <= seed <= 0xffffffff:
                    raise ValueError('Seed must be an unsigned 32-bit integer')
                self.history = []; self.history_path.write_text('[]')
                self.command(f'fresh {seed}')
            else:
                raise ValueError('Unknown action')
            return self.response()

    def close(self):
        with self.lock:
            try:
                if self.process.poll() is None:
                    try:
                        self.process.stdin.write('quit\n'); self.process.stdin.flush()
                    except OSError:
                        pass # Ctrl+C may already have reached the child process.
                    try:
                        self.process.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        self.process.terminate(); self.process.wait(timeout=5)
            finally:
                for stream in (self.process.stdin, self.process.stdout, self.log_file):
                    try: stream.close()
                    except OSError: pass


def make_handler(engine):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args): pass

        def send(self, data, content_type, status=200):
            self.send_response(status); self.send_header('Content-Type', content_type)
            self.send_header('Content-Length', str(len(data)))
            self.send_header('Cache-Control', 'no-store'); self.send_header('X-Content-Type-Options', 'nosniff')
            self.end_headers(); self.wfile.write(data)

        def send_json(self, data, status=200):
            self.send(json.dumps(data, ensure_ascii=False).encode(), 'application/json; charset=utf-8', status)

        def local_request(self):
            # Loopback binding plus Host/Origin checks: external sites cannot mutate the simulator.
            host = self.headers.get('Host', '')
            expected = f'127.0.0.1:{self.server.server_port}'
            origin = self.headers.get('Origin')
            return host == expected and (origin is None or origin == f'http://{expected}')

        def do_GET(self):
            if not self.local_request(): return self.send_json({'error': 'Local origin required'}, 403)
            path = self.path.split('?', 1)[0]
            try:
                if path == '/': self.send((HERE / 'index.html').read_bytes(), 'text/html; charset=utf-8')
                elif path == '/api/state': self.send_json(engine.response())
                elif path == '/api/export':
                    with engine.lock:
                        self.send_json(dict(engine.response(), history=engine.history,
                                            simulator_state_hex=(engine.directory / 'state.bin').read_bytes().hex()
                                            if (engine.directory / 'state.bin').exists() else ''))
                elif path in ('/frame.bmp', '/sound.wav'):
                    with engine.lock:
                        self.send((engine.directory / path[1:]).read_bytes(),
                                  'image/bmp' if path.endswith('.bmp') else 'audio/wav')
                elif path == '/favicon.ico': self.send(b'', 'image/x-icon', 204)
                else: self.send_json({'error': 'Not found'}, 404)
            except (OSError, RuntimeError) as error: self.send_json({'error': str(error)}, 500)

        def do_POST(self):
            if not self.local_request(): return self.send_json({'error': 'Local origin required'}, 403)
            if self.path != '/api/action': return self.send_json({'error': 'Not found'}, 404)
            try:
                length = int(self.headers.get('Content-Length', 0))
                if not 0 < length <= 1024: raise ValueError('Invalid body size')
                body = json.loads(self.rfile.read(length))
                if not isinstance(body, dict): raise ValueError('Expected an object')
                self.send_json(engine.action(body))
            except (ValueError, OSError, RuntimeError) as error: self.send_json({'error': str(error)}, 400)
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=8786)
    parser.add_argument('--seed', type=int)
    parser.add_argument('--no-open', action='store_true')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--state-dir', type=Path, default=ROOT / 'build/fortune-simulator')
    args = parser.parse_args()
    binary = build_bridge()
    if args.build_only: return
    engine = Engine(binary, args.state_dir, args.seed)
    try:
        server = ThreadingHTTPServer(('127.0.0.1', args.port), make_handler(engine))
        url = f'http://127.0.0.1:{server.server_port}/'
        print(f'One Fortune simulator: {url}\nStop with Ctrl+C. Local simulator data only; no USB access.', flush=True)
        if not args.no_open: webbrowser.open(url)
        try: server.serve_forever()
        except KeyboardInterrupt: pass
        finally: server.server_close()
    finally: engine.close()


if __name__ == '__main__':
    main()
