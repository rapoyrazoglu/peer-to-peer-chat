# PeerChat

Torrent-style, fully decentralized, serverless peer-to-peer chat application written in C++20.

No central server. No data collection. No censorship. Just peers talking to peers.

## Features

- Direct peer-to-peer messaging over TCP
- JSON wire protocol with length-prefixed framing
- Handshake, ACK, and heartbeat (ping/pong)
- Persistent peer identity (UUID v4)
- CLI with `/connect`, `/disconnect`, `/status`, `/quit`

### Planned

- End-to-end encryption (X25519 + XChaCha20-Poly1305)
- NAT traversal (STUN, UDP hole punching, UPnP)
- Distributed peer discovery (Kademlia DHT)
- Group chat support
- Offline message delivery (store-and-forward)
- Torrent-style file transfer (chunked, parallel)
- Terminal UI (ncurses)

## Installation

### Linux / macOS / Windows

```bash
git clone https://github.com/rapoyrazoglu/peer-to-peer-chat.git
cd peer-to-peer-chat
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
sudo cmake --install build
```

After installation, `peerchat` is available system-wide.

**Dependencies:** CMake 3.20+, C++20 compiler, libsodium (optional)

### Arch Linux (AUR)

```bash
yay -S peerchat
```

### Pre-built Binaries

Download from the [latest release](https://github.com/rapoyrazoglu/peer-to-peer-chat/releases/latest) page:

| Platform | File |
|----------|------|
| Linux x86_64 | `peerchat-linux-x86_64.tar.gz` |
| macOS Intel | `peerchat-macos-x86_64.tar.gz` |
| macOS ARM | `peerchat-macos-arm64.tar.gz` |
| Windows | `peerchat-windows-x86_64.zip` |

Extract and move to your PATH:

```bash
tar xzf peerchat-*.tar.gz
sudo mv peerchat /usr/local/bin/
```

## Usage

```bash
# Terminal 1
peerchat --port 9000 --nick alice

# Terminal 2
peerchat --port 9001 --nick bob
```

In bob's terminal:

```
/connect 127.0.0.1:9000
```

Then just type messages. Commands:

| Command | Description |
|---------|-------------|
| `/connect <host>:<port>` | Connect to a peer |
| `/disconnect` | Disconnect from peer |
| `/status` | Show connection info |
| `/quit` | Exit PeerChat |

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the full development plan.

## License

MIT
